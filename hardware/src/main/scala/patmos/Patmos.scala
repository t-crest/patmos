/*
 * Patmos top level component and test driver.
 *
 * Authors: Martin Schoeberl (martin@jopdesign.com)
 *          Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package patmos

import Chisel._
import java.io.File

import Constants._
import util._
import io._
import datacache._
import ocp.{OcpCoreSlavePort, _}
import argo._

import scala.collection.immutable.Stream.Empty

/**
 * Module for one Patmos core.
 */
class PatmosCore(binFile: String, nr: Int, cnt: Int) extends Module {

  val io = IO(Config.getPatmosCoreIO(nr))

  val icache =
    if (ICACHE_SIZE <= 0)
      Module(new NullICache())
    else if (ICACHE_TYPE == ICACHE_TYPE_METHOD && ICACHE_REPL == CACHE_REPL_FIFO)
      Module(new MCache())
    else if (ICACHE_TYPE == ICACHE_TYPE_LINE && ICACHE_ASSOC == 1)
      Module(new ICache())
    else {
      throw new Error("Unsupported instruction cache configuration:" +
        " type \"" + ICACHE_TYPE + "\"" +
        " (must be \"" + ICACHE_TYPE_METHOD + "\" or \"" + ICACHE_TYPE_LINE + "\")" +
        " associativity " + ICACHE_ASSOC +
        " with replacement policy \"" + ICACHE_REPL + "\"")
      Module(new NullICache()) // return at least a dummy cache
    }

  val fetch = Module(new Fetch(binFile))
  val decode = Module(new Decode())
  val execute = Module(new Execute())
  val memory = Module(new Memory())
  val writeback = Module(new WriteBack())
  val exc = Module(new Exceptions())
  
  val dcache = Module(new DataCache())

  //connect icache
  icache.io.feicache <> fetch.io.feicache
  icache.io.icachefe <> fetch.io.icachefe
  icache.io.exicache <> execute.io.exicache
  icache.io.illMem <> memory.io.icacheIllMem

  decode.io.fedec <> fetch.io.fedec
  execute.io.decex <> decode.io.decex
  memory.io.exmem <> execute.io.exmem
  writeback.io.memwb <> memory.io.memwb
  // RF write connection
  decode.io.rfWrite <> writeback.io.rfWrite
  // This is forwarding of registered result
  // Take care that it is the plain register
  execute.io.exResult <> memory.io.exResult
  execute.io.memResult <> writeback.io.memResult

  // Connect stack cache
  execute.io.exsc <> dcache.io.scIO.exsc
  dcache.io.scIO.scex <> execute.io.scex
  dcache.io.scIO.illMem <> memory.io.scacheIllMem

  // We branch in EX
  fetch.io.exfe <> execute.io.exfe
  // We call in MEM
  fetch.io.memfe <> memory.io.memfe
  // We store the return base in EX (in cycle corresponding to MEM)
  fetch.io.feex <> execute.io.feex

  memory.io.localInOut <> io.inout.memInOut

  // Connect exception unit
  exc.io.ocp <> io.inout.excInOut
  exc.io.intrs <> io.inout.intrs
  exc.io.excdec <> decode.io.exc
  exc.io.memexc <> memory.io.exc

  // Connect data cache
  dcache.io.master <> memory.io.globalInOut

  // Merge OCP ports from data caches and method cache
  val burstBus = Module(new OcpBurstBus(ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH))
  val selICache = Wire(Bool())
  val burstJoin = if (ICACHE_TYPE == ICACHE_TYPE_METHOD) {
    // requests from D-cache and method cache never collide
    new OcpBurstJoin(icache.io.ocp_port, dcache.io.slave,
      burstBus.io.slave, selICache)
  } else {
    // join requests such that D-cache requests are buffered
    new OcpBurstPriorityJoin(icache.io.ocp_port, dcache.io.slave,
      burstBus.io.slave, selICache)
  }

  val mmu = Module(if (HAS_MMU) new MemoryManagement() else new NoMemoryManagement())
  mmu.io.exec <> selICache
  mmu.io.ctrl <> io.inout.mmuInOut
  mmu.io.virt <> burstBus.io.master

  // Enable signals for memory stage, method cache and stack cache
  memory.io.ena_in := icache.io.ena_out && !dcache.io.scIO.stall
  icache.io.ena_in := memory.io.ena_out && !dcache.io.scIO.stall
  dcache.io.scIO.ena_in := memory.io.ena_out && icache.io.ena_out

  // Enable signal
  val enable = memory.io.ena_out & icache.io.ena_out & !dcache.io.scIO.stall
  fetch.io.ena := enable
  decode.io.ena := enable
  execute.io.ena := enable
  writeback.io.ena := enable
  exc.io.ena := enable
  val enableReg = Reg(next = enable)

  // Flush signal
  val flush = memory.io.flush
  val brflush = execute.io.brflush
  decode.io.flush := flush || brflush
  execute.io.flush := flush

  // Software resets
  icache.io.invalidate := exc.io.invalICache
  dcache.io.invalDCache := exc.io.invalDCache

  // Make privileged mode visible internally and externally
  io.inout.superMode := exc.io.superMode
  mmu.io.superMode := exc.io.superMode
  io.superMode := exc.io.superMode

  // Internal "I/O" data
  io.inout.internalIO.perf.ic := icache.io.perf
  io.inout.internalIO.perf.dc := dcache.io.dcPerf
  io.inout.internalIO.perf.sc := dcache.io.scPerf
  io.inout.internalIO.perf.wc := dcache.io.wcPerf
  io.inout.internalIO.perf.mem.read := (io.memPort.M.Cmd === OcpCmd.RD &&
    io.memPort.S.CmdAccept === UInt(1))
    io.inout.internalIO.perf.mem.write := (io.memPort.M.Cmd === OcpCmd.WR &&
    io.memPort.S.CmdAccept === UInt(1))

  // The inputs and outputs
  io.comConf <> io.inout.comConf
  io.comSpm <> io.inout.comSpm
  io.memPort <> mmu.io.phys
  //Config.connectAllIOPins(io, iocomp.io)

  // Keep signal alive for debugging
  debug(enableReg)
}

/**
 * The main (top-level) component of Patmos.
 */
class Patmos(configFile: String, binFile: String, datFile: String) extends Module {
  Config.loadConfig(configFile)
  Config.minPcWidth = util.log2Up((new File(binFile)).length.toInt / 4)
  Config.datFile = datFile

  val io = Config.getPatmosIO()

  val nrCores = Config.getConfig.coreCount

  val aegeanMode = !Config.getConfig.cmpDevices.contains("Argo")

  println("Config core count: " + nrCores)

  // Instantiate cores
  val cores = (0 until nrCores).map(i => Module(new PatmosCore(binFile, i, nrCores)))

  // Forward ports to/from core
  val cmpDevices = Config.getConfig.cmpDevices
  println("Config cmp: ")
  val MAX_IO_DEVICES = 16
  val cmpdevs = new Array[Module](MAX_IO_DEVICES)
  
  for(dev <- cmpDevices) {
    println(dev)
    dev match {
      // Address 0 reserved for Argo
      case "Argo" =>  cmpdevs(0) = Module(new argo.Argo(nrCores, wrapped=false, emulateBB=false))
      case "Hardlock" => cmpdevs(1) = Module(new cmp.HardlockOCPWrapper(() => new cmp.Hardlock(nrCores, 1)))
      case "SharedSPM" => cmpdevs(2) = Module(new cmp.SharedSPM(nrCores, (nrCores-1)*2*1024))
      case "OneWay" => cmpdevs(3) = Module(new cmp.OneWayOCPWrapper(nrCores))
      case "TdmArbiter" => cmpdevs(4) = Module(new cmp.TdmArbiter(nrCores))
      case "OwnSPM" => cmpdevs(5) = Module(new cmp.OwnSPM(nrCores, (nrCores-1)*2, 1024))
      case "SPMPool" => cmpdevs(6) = Module(new cmp.SPMPool(nrCores, (nrCores-1)*2, 1024))
      case "S4noc" => cmpdevs(7) = Module(new cmp.S4nocOCPWrapper(nrCores, 4, 4))
      case "CASPM" => cmpdevs(8) = Module(new cmp.CASPM(nrCores, nrCores * 8))
      case "AsyncLock" => cmpdevs(9) = Module(new cmp.AsyncLock(nrCores, nrCores * 2))
      case "UartCmp" => cmpdevs(10) = Module(new cmp.UartCmp(nrCores,CLOCK_FREQ,115200,16))
      case "TwoWay" => cmpdevs(11) = Module(new cmp.TwoWayOCPWrapper(nrCores, 1024))
      case "TransactionalMemory" => cmpdevs(12) = Module(new cmp.TransactionalMemory(nrCores, 512))
      case "LedsCmp" => cmpdevs(13) = Module(new cmp.LedsCmp(nrCores, 1))
      case _ =>
    }
  }
  
  for(dev <- cmpdevs) {
    if(dev != null) {
      Config.connectIOPins(dev.getClass.getSimpleName, io, dev.io, "cmp.")
    }
  }

  for (i <- (0 until nrCores)) {

    // Compute selects
    val selIO = cores(i).io.inout.memInOut.M.Addr(ADDR_WIDTH-1, ADDR_WIDTH-4) === UInt(0xF)
    val selNI = cores(i).io.inout.memInOut.M.Addr(ADDR_WIDTH-1, ADDR_WIDTH-4) === UInt(0xE)

    val selISpm = !selIO & !selNI & cores(i).io.inout.memInOut.M.Addr(ISPM_ONE_BIT) === UInt(0x1)
    val selSpm = !selIO & !selNI & cores(i).io.inout.memInOut.M.Addr(ISPM_ONE_BIT) === UInt(0x0)

    val selComConf = if(aegeanMode) selNI & cores(i).io.inout.memInOut.M.Addr(ADDR_WIDTH-5) === UInt("b0") else false.B
    val selComSpm  = if(aegeanMode) selNI & cores(i).io.inout.memInOut.M.Addr(ADDR_WIDTH-5) === UInt("b1") else selNI

    val MAX_IO_DEVICES : Int = 0x10
    val IO_DEVICE_OFFSET = 16 // Number of address bits for each IO device
    val IO_DEVICE_ADDR_SIZE = 32 - Integer.numberOfLeadingZeros(MAX_IO_DEVICES-1)
    assert(Bool(IO_DEVICE_ADDR_SIZE + IO_DEVICE_OFFSET < ADDR_WIDTH-4),
                                      "Conflicting addressspaces of IO devices")

    val validDeviceVec = Vec.fill(MAX_IO_DEVICES) { Bool() }
    val validDevices = Array.fill(MAX_IO_DEVICES) { false }
    val selDeviceVec = Vec.fill(MAX_IO_DEVICES) { Bool() }
    val deviceSVec = Vec.fill(MAX_IO_DEVICES) { new OcpSlaveSignals(DATA_WIDTH) }
    println(IO_DEVICE_ADDR_SIZE)
    for (j <- 0 until MAX_IO_DEVICES) {
      validDeviceVec(j) := Bool(false)
      selDeviceVec(j) := selIO & cores(i).io.inout.memInOut.M.Addr(IO_DEVICE_ADDR_SIZE
                            + IO_DEVICE_OFFSET - 1, IO_DEVICE_OFFSET) === UInt(j)
      deviceSVec(j).Resp := OcpResp.NULL
      deviceSVec(j).Data := UInt(0)
    }
    validDeviceVec(EXC_IO_OFFSET) := Bool(true)
    validDevices(EXC_IO_OFFSET) = true
    validDeviceVec(MMU_IO_OFFSET) := Bool(HAS_MMU)
    validDevices(MMU_IO_OFFSET) = HAS_MMU

    // Register selects
    val selSpmReg = Reg(Bool())
    val selComConfReg = Reg(Bool())
    val selComSpmReg = Reg(Bool())

    val selDeviceReg = Vec.fill(MAX_IO_DEVICES) { Reg(Bool()) }

    when(cores(i).io.inout.memInOut.M.Cmd =/= OcpCmd.IDLE) {
      selSpmReg := selSpm
      selComConfReg := selComConf
      selComSpmReg := selComSpm

      selDeviceReg := selDeviceVec
    }

    // Default values for interrupt pins
    for (j <- 0 until INTR_COUNT) {
      cores(i).io.inout.intrs(j) := Bool(false)
    }

    // Register for error response
    val errResp = Reg(init = OcpResp.NULL)
    val validSelVec = selDeviceVec.zip(validDeviceVec).map{ case (x, y) => x && y }
    val validSel = validSelVec.fold(Bool(false))(_|_)
    errResp := Mux(cores(i).io.inout.memInOut.M.Cmd =/= OcpCmd.IDLE &&
                  selIO && !validSel,
                  OcpResp.ERR, OcpResp.NULL)

    // Dummy ISPM (create fake response)
    val ispmCmdReg = Reg(next = Mux(selISpm, cores(i).io.inout.memInOut.M.Cmd, OcpCmd.IDLE))
    val ispmResp = Mux(ispmCmdReg === OcpCmd.IDLE, OcpResp.NULL, OcpResp.DVA)

    // The SPM
    val spm = Module(new Spm(DSPM_SIZE))
    spm.io.M := cores(i).io.inout.memInOut.M
    spm.io.M.Cmd := Mux(selSpm, cores(i).io.inout.memInOut.M.Cmd, OcpCmd.IDLE)
    val spmS = spm.io.S

    // The communication configuration, including bridge to OcpIO interface
    val comConf = Module(new OcpCoreBus(ADDR_WIDTH, DATA_WIDTH))
    comConf.io.slave.M := cores(i).io.inout.memInOut.M
    comConf.io.slave.M.Cmd := Mux(selComConf, cores(i).io.inout.memInOut.M.Cmd, OcpCmd.IDLE)
    val comConfS = comConf.io.slave.S
    val comConfIO = Module(new OcpIOBus(ADDR_WIDTH, DATA_WIDTH))
    cores(i).io.inout.comConf.M := comConfIO.io.master.M
    comConfIO.io.master.S := cores(i).io.inout.comConf.S
    val comConfBridge = new OcpIOBridgeAlt(comConf.io.master, comConfIO.io.slave)

    // The communication scratchpad
    cores(i).io.inout.comSpm.M := cores(i).io.inout.memInOut.M
    cores(i).io.inout.comSpm.M.Cmd := Mux(selComSpm, cores(i).io.inout.memInOut.M.Cmd, OcpCmd.IDLE)
    val comSpmS = cores(i).io.inout.comSpm.S
    
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
        devio.ocp.M := cores(i).io.inout.memInOut.M
        devio.ocp.M.Cmd := Mux(selDeviceVec(off), cores(i).io.inout.memInOut.M.Cmd, OcpCmd.IDLE)
        devio.superMode <> cores(i).io.inout.superMode
        devio.internalPort <> cores(i).io.inout.internalIO
        deviceSVec(off) := devio.ocp.S
      }

    // Creation of IO devices
    val conf = Config.getConfig

    val cpuinfo = Module(new CpuInfoCmp(Config.datFile, i, nrCores))
    connectDevice(cpuinfo.io, CPUINFO_OFFSET, "CpuInfoCmp")

    for (devConf <- Config.getConfig.Devs) {
      val clazz = 
        try { Class.forName("io."+devConf.name+"$Pins") }
        catch { case e: Exception => null}

      if(i == 0 || clazz == null || !clazz.getMethods.nonEmpty) {
        val dev = Config.createDevice(devConf).asInstanceOf[CoreDevice]
        connectDevice(dev.io,devConf.offset,devConf.name)
        Config.connectIOPins(devConf.name, io, dev.io)
        Config.connectIntrPins(devConf, cores(i).io.inout, dev.io)
      }
    }

    // The exception and memory management units are special and outside this unit
    cores(i).io.inout.excInOut.M := cores(i).io.inout.memInOut.M
    cores(i).io.inout.excInOut.M.Cmd := Mux(selDeviceVec(EXC_IO_OFFSET), cores(i).io.inout.memInOut.M.Cmd, OcpCmd.IDLE)
    deviceSVec(EXC_IO_OFFSET) := cores(i).io.inout.excInOut.S

    if (HAS_MMU) {
      cores(i).io.inout.mmuInOut.M := cores(i).io.inout.memInOut.M
      cores(i).io.inout.mmuInOut.M.Cmd := Mux(selDeviceVec(MMU_IO_OFFSET), cores(i).io.inout.memInOut.M.Cmd, OcpCmd.IDLE)
      deviceSVec(MMU_IO_OFFSET) := cores(i).io.inout.mmuInOut.S
    }

    // Return data to pipeline
    cores(i).io.inout.memInOut.S.Data := spmS.Data
    when(selComConfReg) { cores(i).io.inout.memInOut.S.Data := comConfS.Data }
    when(selComSpmReg)  { cores(i).io.inout.memInOut.S.Data := comSpmS.Data }
    for (j <- 0 until MAX_IO_DEVICES) {
      when(selDeviceReg(j)) { cores(i).io.inout.memInOut.S.Data := deviceSVec(j).Data }
    }

    // Merge responses
    cores(i).io.inout.memInOut.S.Resp := errResp |
                          ispmResp | spmS.Resp |
                          comConfS.Resp | comSpmS.Resp |
                          deviceSVec.map(_.Resp).fold(OcpResp.NULL)(_|_)
    
    val cmpdevios = (0 until cmpdevs.length)
      .map(e => (e, cmpdevs(e)))
      .filter(e => e._2 != null)
      .map(e => new {
        val addr = (0xE8 << 8) + e._1;
        val addrwidth = 16;
        val io = e._2.io match {
          case cmpio: cmp.CmpIO => cmpio.cores(i)
          case _ => e._2.io.asInstanceOf[Vec[OcpCoreSlavePort]](i)
        }
        val name = e._2.moduleName
      })

    
    for(dupldev <- cmpdevios
                    .groupBy(e => e.addr)
                    .collect { case (addr,e) if e.lengthCompare(1) > 0 => e}
                    .flatten) {
      throw new Error("Can't assign multiple devices to the same address. " +
        "Device " + dupldev.name + " conflicting on address " +
        dupldev.addr + ". ")
    }

    cores(i).io.comSpm.S.Data := UInt(0)
    val validdev = Bool()
    validdev := false.B
    for(dev <- cmpdevios) {
      val addr = cores(i).io.comSpm.M.Addr(ADDR_WIDTH-1, ADDR_WIDTH-dev.addrwidth)
      println(addr.getWidth())
      println(dev.addr)
      println(dev.addrwidth)
      dev.io.M := cores(i).io.comSpm.M
      dev.io.M.Cmd := OcpCmd.IDLE
      when(addr === dev.addr.U) {
        dev.io.M.Cmd := cores(i).io.comSpm.M.Cmd
        validdev := true.B
      }
      val selReg = RegInit(false.B)
      when(cores(i).io.comSpm.M.Cmd =/= OcpCmd.IDLE) {
        selReg := addr === dev.addr.U
      }
      when(selReg) {
        cores(i).io.comSpm.S.Data := dev.io.S.Data
      }

      // TODO: maybe a better way is for all interfaces to have the bits 'superMode' and 'flags'
      // e.g., all IO devices should be possible to have interrupts
      if(dev.io.isInstanceOf[OcpArgoSlavePort]){
        val argoslaveport = dev.io.asInstanceOf[OcpArgoSlavePort]
        argoslaveport.superMode := UInt(0)
        argoslaveport.superMode(i) := cores(i).io.superMode

        // Hard-wire the sideband flags from the NI to interrupt pins
        cores(i).io.inout.intrs(NI_MSG_INTR) := argoslaveport.flags(2*i)
        cores(i).io.inout.intrs(NI_EXT_INTR) := argoslaveport.flags(2*i+1)
      }
    }

    val errRespReg = Reg(init = OcpResp.NULL)
    when(cores(i).io.comSpm.M.Cmd =/= OcpCmd.IDLE && !validdev) {
      errRespReg := OcpResp.ERR
    }

    cores(i).io.comSpm.S.Resp := errRespReg | cmpdevios.map(e => e.io.S.Resp).fold(OcpResp.NULL)(_|_)
  }

  // Connect memory controller
  val ramConf = Config.getConfig.ExtMem.ram
  val ramCtrl = Config.createDevice(ramConf).asInstanceOf[BurstDevice]

  Config.connectIOPins(ramConf.name, io, ramCtrl.io)

  // TODO: fix memory arbiter to have configurable memory timing.
  // E.g., it does not work with on-chip main memory.
  if (cores.length == 1) {
    ramCtrl.io.ocp <> cores(0).io.memPort
  } else {
    val memarbiter = Module(new ocp.TdmArbiterWrapper(nrCores, ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH))
    for (i <- (0 until cores.length)) {
      memarbiter.io.master(i) <> cores(i).io.memPort
    }
    ramCtrl.io.ocp <> memarbiter.io.slave
  }

  // Print out the configuration
  Utility.printConfig(configFile)
}

// this testing and main file should go into it's own folder
//commented out Chisel3 tester has changed see https://github.com/schoeberl/chisel-examples/blob/master/TowardsChisel3.md 
class PatmosTest(pat: Patmos) extends Tester(pat) {

  println("Patmos start")

  for (i <- 0 until 100) {
    step(1) // false as third argument disables printout
    // The PC printout is a little off on a branch
    val pc = peek(pat.cores(0).memory.io.memwb.pc) - 2
    print(pc + " - ")
    for (j <- 0 until 32)
      print(peek(pat.cores(0).decode.rf.rf(UInt(j))) + " ")
    println()
  }
}

object PatmosMain {
  def main(args: Array[String]): Unit = {

    val chiselArgs = args.slice(3, args.length)
    val configFile = args(0)
    val binFile = args(1)
    val datFile = args(2)

    chiselMainTest(chiselArgs, () => Module(new Patmos(configFile, binFile, datFile))) { f => new PatmosTest(f) }
  }
}
