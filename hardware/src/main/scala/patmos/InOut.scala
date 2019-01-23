/*
 * IO component of Patmos.
 *
 * Authors: Martin Schoeberl (martin@jopdesign.com)
 *          Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package patmos

import Chisel._

import Constants._

import ocp._
import util._
import io.CoreDevice
import io.CoreDeviceIO
import io.CpuInfoCmp

class InOut(nr: Int, cnt: Int, withComConf: Boolean) extends Module {
  val io = IO(Config.getInOutIO(nr))

  // Compute selects
  val selIO = io.memInOut.M.Addr(ADDR_WIDTH-1, ADDR_WIDTH-4) === Bits("b1111")
  val selNI = io.memInOut.M.Addr(ADDR_WIDTH-1, ADDR_WIDTH-4) === Bits("b1110")

  val selISpm = !selIO & !selNI & io.memInOut.M.Addr(ISPM_ONE_BIT) === Bits(0x1)
  val selSpm = !selIO & !selNI & io.memInOut.M.Addr(ISPM_ONE_BIT) === Bits(0x0)

  val selComConf = if(withComConf) selNI & io.memInOut.M.Addr(ADDR_WIDTH-5) === Bits("b0") else false.B
  val selComSpm  = if(withComConf) selNI & io.memInOut.M.Addr(ADDR_WIDTH-5) === Bits("b1") else selNI

  val MAX_IO_DEVICES : Int = 0x10
  val IO_DEVICE_OFFSET = 16 // Number of address bits for each IO device
  val IO_DEVICE_ADDR_SIZE = 32 - Integer.numberOfLeadingZeros(MAX_IO_DEVICES-1)
  assert(Bool(IO_DEVICE_ADDR_SIZE + IO_DEVICE_OFFSET < ADDR_WIDTH-4),
                                    "Conflicting addressspaces of IO devices")

  val validDeviceVec = Vec.fill(MAX_IO_DEVICES) { Bool() }
  val validDevices = Array.fill(MAX_IO_DEVICES) { false }
  val selDeviceVec = Vec.fill(MAX_IO_DEVICES) { Bool() }
  val deviceSVec = Vec.fill(MAX_IO_DEVICES) { new OcpSlaveSignals(DATA_WIDTH) }
  for (i <- 0 until MAX_IO_DEVICES) {
    validDeviceVec(i) := Bool(false)
    selDeviceVec(i) := selIO & io.memInOut.M.Addr(IO_DEVICE_ADDR_SIZE
                          + IO_DEVICE_OFFSET - 1, IO_DEVICE_OFFSET) === Bits(i)
    deviceSVec(i).Resp := OcpResp.NULL
    deviceSVec(i).Data := Bits(0)
  }
  validDeviceVec(EXC_IO_OFFSET) := Bool(true)
  validDeviceVec(MMU_IO_OFFSET) := Bool(HAS_MMU)

  // Register selects
  val selSpmReg = Reg(Bool())
  val selComConfReg = Reg(Bool())
  val selComSpmReg = Reg(Bool())

  val selDeviceReg = Vec.fill(MAX_IO_DEVICES) { Reg(Bool()) }

  when(io.memInOut.M.Cmd =/= OcpCmd.IDLE) {
    selSpmReg := selSpm
    selComConfReg := selComConf
    selComSpmReg := selComSpm

    selDeviceReg := selDeviceVec
  }

  // Default values for interrupt pins
  for (i <- 0 until INTR_COUNT) {
    io.intrs(i) := Bool(false)
  }

  // Register for error response
  val errResp = Reg(init = OcpResp.NULL)
  val validSelVec = selDeviceVec.zip(validDeviceVec).map{ case (x, y) => x && y }
  val validSel = validSelVec.fold(Bool(false))(_|_)
  errResp := Mux(io.memInOut.M.Cmd =/= OcpCmd.IDLE &&
                 selIO && !validSel,
                 OcpResp.ERR, OcpResp.NULL)

  // Dummy ISPM (create fake response)
  val ispmCmdReg = Reg(next = Mux(selISpm, io.memInOut.M.Cmd, OcpCmd.IDLE))
  val ispmResp = Mux(ispmCmdReg === OcpCmd.IDLE, OcpResp.NULL, OcpResp.DVA)

  // The SPM
  val spm = Module(new Spm(DSPM_SIZE))
  spm.io.M := io.memInOut.M
  spm.io.M.Cmd := Mux(selSpm, io.memInOut.M.Cmd, OcpCmd.IDLE)
  val spmS = spm.io.S

  // The communication configuration, including bridge to OcpIO interface
  val comConf = Module(new OcpCoreBus(ADDR_WIDTH, DATA_WIDTH))
  comConf.io.slave.M := io.memInOut.M
  comConf.io.slave.M.Cmd := Mux(selComConf, io.memInOut.M.Cmd, OcpCmd.IDLE)
  val comConfS = comConf.io.slave.S
  val comConfIO = Module(new OcpIOBus(ADDR_WIDTH, DATA_WIDTH))
  io.comConf.M := comConfIO.io.master.M
  comConfIO.io.master.S := io.comConf.S
  val comConfBridge = new OcpIOBridgeAlt(comConf.io.master, comConfIO.io.slave)

  // The communication scratchpad
  io.comSpm.M := io.memInOut.M
  io.comSpm.M.Cmd := Mux(selComSpm, io.memInOut.M.Cmd, OcpCmd.IDLE)
  val comSpmS = io.comSpm.S
  
  val connectDevice = (devio: CoreDeviceIO, off: Int, name: String) => 
    {
      if(!validDevices(off)) {
        validDeviceVec(off) := Bool(true)
        validDevices(off) = true;
      } else {
        throw new Error("Can't assign multiple devices to the same offset. " +
                          "Device " + name + " conflicting on offset " +
                          off.toString + ". ")
      }
      // connect ports
      devio.ocp.M := io.memInOut.M
      devio.ocp.M.Cmd := Mux(selDeviceVec(off), io.memInOut.M.Cmd, OcpCmd.IDLE)
      devio.superMode <> io.superMode
      devio.internalPort <> io.internalIO
      deviceSVec(off) := devio.ocp.S
    }

  // Creation of IO devices
  val conf = Config.getConfig

  if(cnt != 0)
  {
    val cpuinfo = Module(new CpuInfoCmp(Config.datFile, nr, cnt))
    connectDevice(cpuinfo.io, CPUINFO_OFFSET, "CpuInfoCmp")
  }

  for (devConf <- Config.getConfig.Devs) {
    val clazz = 
      try { Class.forName("io."+devConf.name+"$Pins") }
      catch { case e: Exception => null}

    if(nr == 0 || clazz == null || !clazz.getMethods.nonEmpty) {
      val dev = Config.createDevice(devConf).asInstanceOf[CoreDevice]
      connectDevice(dev.io,devConf.offset,devConf.name)
      Config.connectIOPins(devConf.name, io, dev.io)
      Config.connectIntrPins(devConf, io, dev.io)
    }
  }

  // The exception and memory management units are special and outside this unit
  io.excInOut.M := io.memInOut.M
  io.excInOut.M.Cmd := Mux(selDeviceVec(EXC_IO_OFFSET), io.memInOut.M.Cmd, OcpCmd.IDLE)
  deviceSVec(EXC_IO_OFFSET) := io.excInOut.S

  // Hard-wire the sideband flags from the NI to interrupt pins
  io.intrs(NI_MSG_INTR) := io.comConf.S.Flag(0)
  io.intrs(NI_EXT_INTR) := io.comConf.S.Flag(1)

  if (HAS_MMU) {
    io.mmuInOut.M := io.memInOut.M
    io.mmuInOut.M.Cmd := Mux(selDeviceVec(MMU_IO_OFFSET), io.memInOut.M.Cmd, OcpCmd.IDLE)
    deviceSVec(MMU_IO_OFFSET) := io.mmuInOut.S
  }

  // Return data to pipeline
  io.memInOut.S.Data := spmS.Data

  when(selComConfReg) { io.memInOut.S.Data := comConfS.Data }
  when(selComSpmReg)  { io.memInOut.S.Data := comSpmS.Data }
  for (i <- 0 until MAX_IO_DEVICES) {
    when(selDeviceReg(i)) { io.memInOut.S.Data := deviceSVec(i).Data }
  }

  // Merge responses
  io.memInOut.S.Resp := errResp |
                        ispmResp | spmS.Resp |
                        comConfS.Resp | comSpmS.Resp |
                        deviceSVec.map(_.Resp).fold(OcpResp.NULL)(_|_)
}
