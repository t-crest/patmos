/*
 * Patmos top level component and test driver.
 *
 * Authors: Martin Schoeberl (martin@jopdesign.com)
 *          Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package patmos

// MS: if util is imported later, Config is confused as chisel3 has an util itself.
// MS: maybe avoid a utiel package with chisel3? A shame.
import util._
import chisel3._

import java.io.File
import chisel3.experimental._
import chisel3.dontTouch
import chisel3.util.experimental._
import chisel3.VecInit
import chisel3.WireDefault
import Constants._
import io._
import datacache._
import ocp.{OcpCoreSlavePort, _}
import argo._
import cop._

import scala.collection.immutable.Stream.Empty
import scala.collection.mutable

/**
 * Module for one Patmos core.
 */
class PatmosCore(binFile: String, nr: Int, cnt: Int, debug: Boolean = false) extends Module {

  val io = IO(new Bundle() with HasSuperMode with HasPerfCounter with HasInterrupts {
    override val superMode = Output(Bool())
    override val perf = Flipped(new PerfCounterIO())
    override val interrupts = Input(Vec(INTR_COUNT, Bool()))
    val memPort = new OcpBurstMasterPort(EXTMEM_ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH)
    val memInOut = new OcpCoreMasterPort(ADDR_WIDTH, DATA_WIDTH)
    val excInOut = new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH)
    val mmuInOut = new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH)
    val copInOut = Vec(COP_COUNT, new CoprocessorIO())
  })

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
  val decode = Module(new Decode(debug))
  val execute = Module(new Execute())
  val memory = Module(new Memory())
  val writeback = Module(new WriteBack())
  val exc = Module(new Exceptions())
  
  val dcache = Module(new DataCache())

  //connect icache
  icache.io.feicache <> fetch.io.feicache
  fetch.io.icachefe <> icache.io.icachefe
  icache.io.exicache <> execute.io.exicache
  memory.io.icacheIllMem <> icache.io.illMem

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
  dcache.io.scIO.exsc <> execute.io.exsc
  execute.io.scex <> dcache.io.scIO.scex
  memory.io.scacheIllMem <> dcache.io.scIO.illMem

  // We branch in EX
  fetch.io.exfe <> execute.io.exfe
  // We call in MEM
  fetch.io.memfe <> memory.io.memfe
  // We store the return base in EX (in cycle corresponding to MEM)
  execute.io.feex <> fetch.io.feex

  io.memInOut <> memory.io.localInOut

  // Connect exception unit
  io.excInOut <> exc.io.ocp
  exc.io.intrs <> io.interrupts
  decode.io.exc <> exc.io.excdec
  exc.io.memexc <> memory.io.exc

  // Connect data cache
  dcache.io.master.M <> memory.io.globalInOut.M
  memory.io.globalInOut.S <> dcache.io.master.S

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
  io.mmuInOut <> mmu.io.ctrl
  mmu.io.virt.M <> burstBus.io.master.M
  burstBus.io.master.S <> mmu.io.virt.S

  // Enable signals for memory stage, method cache and stack cache
  memory.io.ena_in := icache.io.ena_out && !dcache.io.scIO.stall && execute.io.ena_out
  icache.io.ena_in := memory.io.ena_out && !dcache.io.scIO.stall && execute.io.ena_out
  dcache.io.scIO.ena_in := memory.io.ena_out && icache.io.ena_out && execute.io.ena_out

  // Enable signals for execute stage
  execute.io.ena_in := memory.io.ena_out && icache.io.ena_out && !dcache.io.scIO.stall

  // Enable signal
  val enable = memory.io.ena_out & icache.io.ena_out & !dcache.io.scIO.stall & execute.io.ena_out
  fetch.io.ena := enable
  decode.io.ena := enable
  writeback.io.ena := enable
  exc.io.ena := enable

  // TODO: only used by the emulator, how usefull?
  val enableReg = RegNext(enable)
  dontTouch(enableReg)

  // Flush signal
  val flush = memory.io.flush
  val brflush = execute.io.brflush
  decode.io.flush := flush || brflush
  execute.io.flush := flush

  // Software resets
  icache.io.invalidate := exc.io.invalICache
  dcache.io.invalDCache := exc.io.invalDCache

  // Make privileged mode visible internally and externally
  mmu.io.superMode := exc.io.superMode
  io.superMode := exc.io.superMode

  // Internal "I/O" data
  io.perf.ic := icache.io.perf
  io.perf.dc := dcache.io.dcPerf
  io.perf.sc := dcache.io.scPerf
  io.perf.wc := dcache.io.wcPerf
  io.perf.mem.read := (io.memPort.M.Cmd === OcpCmd.RD &&
    io.memPort.S.CmdAccept === 1.U)
  io.perf.mem.write := (io.memPort.M.Cmd === OcpCmd.WR &&
    io.memPort.S.CmdAccept === 1.U)

  // The inputs and outputs
  io.memPort <> mmu.io.phys

  // Keep signal alive for debugging
  //debug(enableReg) does nothing in chisel3 (no proning in frontend of chisel3 anyway)

  // connect coprocessor to execute state 
  for (i <- (0 until COP_COUNT)) {
    io.copInOut(i).patmosCop <> execute.io.copOut(i)
    execute.io.copIn(i) <> io.copInOut(i).copPatmos
  }
}

trait HasPins {
  val pins : Bundle
}

trait HasInterrupts {
  val interrupts: Vec[Bool]
}

trait HasPerfCounter {
  val perf = new PerfCounterIO()
}

trait HasSuperMode {
  val superMode = Input(Bool());
}

final class PatmosBundle(elts: (String, Data)*) extends Record {
  override val elements = scala.collection.immutable.ListMap(elts map { case (field, elt) =>
    requireIsChiselType(elt)
    field -> elt
  }: _*)
  def apply(elt: String): Data = elements(elt)
  override def cloneType: this.type = {
    val cloned = elts map { case (n, d) => n -> DataMirror.internal.chiselTypeClone(d) }
    (new PatmosBundle(cloned: _*)).asInstanceOf[this.type]
  }
}

/**
 * The main (top-level) component of Patmos.
 */
class Patmos(configFile: String, binFile: String, datFile: String, genEmu: Boolean = false) extends Module {
  Config.loadConfig(configFile)
  Config.minPcWidth = util.log2Up((new File(binFile)).length.toInt / 4)
  Config.datFile = datFile
  val config = Config.getConfig
  val nrCores = config.coreCount

  println("Config core count: " + nrCores)

  // Instantiate cores
  val cores = (0 until nrCores).map(i => Module(new PatmosCore(binFile, i, nrCores, genEmu)))

  // Forward ports to/from core
  println("Config cmp: ")

  val pinids = scala.collection.mutable.Map[String, Int]()
  val pins = scala.collection.mutable.Map[String, Data]()
  def registerPins(name: String, _io: Data) = {
    _io match {
      case haspins: HasPins => {
        println(name + " has pins")
        var postfix = ""
        if(pinids.contains(name)) {
          var tmp = pinids(name)
          tmp += 1
          pinids(name) = tmp
          postfix = "_" + tmp
        } else {
          pinids(name) = 0
        }

        for((pinid, pin) <- haspins.pins.elements) {
          val _pinid = name + postfix + "_" + pinid
          pins(_pinid) = pin
        }
      }
      case _ =>
    }}

  var envinfoOpt = None: Option[cmp.EnvInfo]
  var uartcmpOpt = None: Option[cmp.UartCmp]
  
  val IO_DEVICE_ADDR_WIDTH = 16

  case class Device(name: String, io: Data, addr: Int, addrWidth: Int)

  val cmpdevios = config.cmpDevices.map(device => {
    println(s"CMP device: $device")
    val (off, width, dev) = device match {
      case "Argo" =>  (0x1C, 5, Module(new argo.Argo(nrCores, wrapped=false, emulateBB=false)))
      case "Hardlock" => (0xE801, IO_DEVICE_ADDR_WIDTH, Module(new cmp.HardlockOCPWrapper(nrCores, () => new cmp.Hardlock(nrCores, 1))))
      case "SharedSPM" => (0xE802, IO_DEVICE_ADDR_WIDTH, Module(new cmp.SharedSPM(nrCores, (nrCores-1)*2*1024)))
      case "EnvInfo" => {
        val envinfo = Module(new cmp.EnvInfo(nrCores))
        envinfoOpt = Some(envinfo)
        (0xE803, IO_DEVICE_ADDR_WIDTH, envinfo)
      }
      // case "OneWay" => (0xE803, IO_DEVICE_ADDR_WIDTH, Module(new cmp.OneWayOCPWrapper(nrCores)))
      // removed as it was never used, address is free
      // TODO: remove constants from patmos.h
      // case "TdmArbiter" => (0xE804, IO_DEVICE_ADDR_WIDTH, Module(new cmp.TdmArbiter(nrCores)))
      case "OwnSPM" => (0xE805, IO_DEVICE_ADDR_WIDTH, Module(new cmp.OwnSPM(nrCores, (nrCores-1)*2, 1024)))
      case "SPMPool" => (0xE806, IO_DEVICE_ADDR_WIDTH, Module(new cmp.SPMPool(nrCores, (nrCores-1)*2, 1024)))
      case "S4NoC" => (0xE807, IO_DEVICE_ADDR_WIDTH, Module(new cmp.PipeConWrapper(nrCores)))
      case "CASPM" => (0xE808, IO_DEVICE_ADDR_WIDTH, Module(new cmp.CASPM(nrCores, nrCores * 8)))
      case "AsyncLock" => (0xE809, IO_DEVICE_ADDR_WIDTH, Module(new cmp.AsyncLock(nrCores, nrCores * 2)))
      case "UartCmp" => {
        val uartcmp = Module(new cmp.UartCmp(nrCores,CLOCK_FREQ,UART_BAUD,16))
        uartcmpOpt = Some(uartcmp)
        (0xF008, IO_DEVICE_ADDR_WIDTH, uartcmp)
      }
      case "TwoWay" => (0xE80B, IO_DEVICE_ADDR_WIDTH, Module(new cmp.TwoWayOCPWrapper(nrCores, 1024)))
      case "TransactionalMemory" => (0xE80C, IO_DEVICE_ADDR_WIDTH, Module(new cmp.TransactionalMemory(nrCores, 512)))
      case "LedsCmp" => (0xE80D, IO_DEVICE_ADDR_WIDTH, Module(new cmp.LedsCmp(nrCores, 1)))
      case _ => throw new Error("Unknown device " + device)
    }

    registerPins(dev.getClass.getSimpleName, dev.io) // all devices have their pins registered in line 409

    Device(dev.getClass.getSimpleName, dev.io, off, width)
  })

  val cops = Array.ofDim[Coprocessor](nrCores,COP_COUNT)
  var memAccessCount = 0

  for (i <- 0 until nrCores) {

    println(s"Config core $i:")
    // Default values for interrupt pins
    cores(i).io.interrupts := VecInit(Seq.fill(INTR_COUNT)(false.B))
    cores(i).io.mmuInOut := DontCare

    // Creation of IO devices
    val cpuinfo = Module(new CpuInfo(Config.datFile, nrCores))
    cpuinfo.io.nr := i.U
    cpuinfo.io.cnt := nrCores.U
    cpuinfo.io.superMode := cores(i).io.superMode

    val configDevs = config.Devs
      .filter(e => e.allcores || e.core == i)
      .map(e => (e,Config.createDevice(e).asInstanceOf[CoreDevice]))
      .map { case (conf,dev) =>
          dev.io match {
            case io: HasSuperMode => io.superMode := cores(i).io.superMode
            case _ =>
          }
          dev.io match {
            case io: HasPerfCounter => io.perf := cores(i).io.perf
            case _ =>
          }
          dev.io match {
            case io: HasInterrupts => {
              if (io.interrupts.length != conf.intrs.length) {
                throw new Error("Inconsistent interrupt counts for IO device "+name)
              }
              for (j <- conf.intrs.indices) {
                cores(i).io.interrupts(conf.intrs(j)) := io.interrupts(j)
              }
            }
            case _ =>
          }
        (conf.offset, dev.io.asInstanceOf[Bundle], conf.ref) // (offset, io, name)
      }

    val cpuInfoDev = (CPUINFO_OFFSET, cpuinfo.io.asInstanceOf[Bundle], cpuinfo.getClass.getSimpleName)
    val exceptionUnitDev = (EXC_IO_OFFSET, cores(i).io.excInOut, "ExceptionUnit")
    val mmuDev = (MMU_IO_OFFSET, cores(i).io.mmuInOut, "MMU") // only included if HAS_MMU

    val allDevs = if (HAS_MMU) {
      cpuInfoDev :: exceptionUnitDev :: mmuDev :: configDevs
    } else {
      cpuInfoDev :: exceptionUnitDev :: configDevs
    }

    val singledevios = allDevs.map { case (offset, io, name) =>
      Device(name, io, (0xF0 << 8) + offset, IO_DEVICE_ADDR_WIDTH)
    }

    // The SPM
    val spm = Module(new Spm(DSPM_SIZE))

    // Dummy ISPM (create fake response)
    val ispmio = Wire(new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH))
    ispmio.S.Data := 0.U
    ispmio.S.Resp := RegNext(Mux(ispmio.M.Cmd === OcpCmd.IDLE, OcpResp.NULL, OcpResp.DVA))

    val devios = singledevios ++ cmpdevios ++ List(
      Device("spm", spm.io, 0x0, 4),
      Device("ispm", ispmio, 0x0001, 16)
    )

    devios.groupBy(_.addr).foreach { case (addr, devs) =>
      if (devs.length > 1) {
        throw new Error(s"Can't assign multiple devices to the same address. Devices: ${devs.map(_.name).mkString(", ")} conflicting on address $addr.")
      }
    }

    val getIO = (_io: Any, _i: Int) =>
      _io match {
        case cmpio: cmp.CmpIO => cmpio.cores(_i)
        case __io: Bundle => __io
        case vec => vec.asInstanceOf[Vec[OcpCoreSlavePort]](_i)
      }

    val getSlavePort = (_io: Bundle) =>
      _io match {
        case __io: OcpCoreSlavePort => __io
        case __io: CoreDeviceIO => __io.ocp
      }

    cores(i).io.memInOut.S.Data := 0.U
    val validdev = WireDefault(false.B)
    for(dev <- devios) {
      val _io = getIO(dev.io, i)
      val ocp = getSlavePort(_io)

      val sel = cores(i).io.memInOut.M.Addr(ADDR_WIDTH-1, ADDR_WIDTH-dev.addrWidth) === dev.addr.U &&
        (if(dev.name == "spm") !cores(i).io.memInOut.M.Addr(ISPM_ONE_BIT) 
         else true.B)
      ocp.M := cores(i).io.memInOut.M
      ocp.M.Cmd := OcpCmd.IDLE
      when(sel) {
        ocp.M.Cmd := cores(i).io.memInOut.M.Cmd
        validdev := true.B
      }
      val selReg = RegInit(false.B)
      when(cores(i).io.memInOut.M.Cmd =/= OcpCmd.IDLE) {
        selReg := sel
      }
      when(selReg) {
        cores(i).io.memInOut.S.Data := ocp.S.Data
      }

      // TODO: maybe a better way is for all interfaces to have the bits 'superMode' and 'flags'
      // e.g., all IO devices should be possible to have interrupts
      ocp match {
        case argoSlavePort: OcpArgoSlavePort =>
          argoSlavePort.superMode := 0.U
          when(cores(i).io.superMode === true.B) {
            argoSlavePort.superMode := (1.U(nrCores.W) << i)
          }

          // Hard-wire the sideband flags from the NI to interrupt pins
          cores(i).io.interrupts(NI_MSG_INTR) := argoSlavePort.flags(2*i)
          cores(i).io.interrupts(NI_EXT_INTR) := argoSlavePort.flags(2*i+1)
        case _ =>
      }

      registerPins(dev.name, _io)
    }

    // Register for error response
    val errRespReg = RegInit(OcpResp.NULL)
    when(cores(i).io.memInOut.M.Cmd =/= OcpCmd.IDLE && !validdev) {
      errRespReg := OcpResp.ERR
    }

    // Merge responses
    cores(i).io.memInOut.S.Resp := errRespReg | devios.map(e => getSlavePort(getIO(e.io, i)).S.Resp).fold(OcpResp.NULL)(_|_)


    // Instantiate coprocessors
    for (k <- (0 until COP_COUNT)) {
      val copConf = config.Coprocessors(k)
      val id = copConf.CoprocessorID;

      if(copConf.requiresMemoryAccess) {
        val copMem = Config.createCoprocessor(copConf).asInstanceOf[CoprocessorMemoryAccess]
        copMem.io.copIn <> cores(i).io.copInOut(k).patmosCop
        cores(i).io.copInOut(k).copPatmos <> copMem.io.copOut
        memAccessCount = memAccessCount + 1;
        cops(i)(id) = copMem
        
      } else {
        val copNoMem = Config.createCoprocessor(copConf).asInstanceOf[BaseCoprocessor]
        copNoMem.io.copIn <> cores(i).io.copInOut(k).patmosCop
        cores(i).io.copInOut(k).copPatmos <> copNoMem.io.copOut
        cops(i)(id) = copNoMem
      }
      
    }
  }

  // Connect memory controller
  val ramConf = config.ExtMem.ram
  val ramCtrl = Config.createDevice(ramConf).asInstanceOf[BurstDevice]
  

  registerPins(ramConf.name, ramCtrl.io)

  // TODO: fix memory arbiter to have configurable memory timing.
  // E.g., it does not work with on-chip main memory.
  if (cores.length + memAccessCount == 1) {
    ramCtrl.io.ocp.M <> cores(0).io.memPort.M
    cores(0).io.memPort.S <> ramCtrl.io.ocp.S
    ramCtrl.io.superMode <> cores(0).io.superMode
  } else {
    
    // memAccessCount stores the totale number of required memory access ports
    val memarbiterCount = if(memAccessCount > 0) memAccessCount + nrCores else nrCores

    val memarbiter =
      if(ramCtrl.isInstanceOf[DDR3Bridge] || ramCtrl.isInstanceOf[OCRamCtrl] || config.roundRobinArbiter) {
        Module(new ocp.Arbiter(memarbiterCount, ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH))
      } else {
        Module(new ocp.TdmArbiterWrapper(memarbiterCount, ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH))
      }
      
    var arbiterEntry = 0
    for (i <- cores.indices) {
      memarbiter.io.master(arbiterEntry).M <> cores(i).io.memPort.M
      cores(i).io.memPort.S <> memarbiter.io.master(arbiterEntry).S

      arbiterEntry = arbiterEntry +1
      
      for(j <- 0 until COP_COUNT) {
        val copConf = config.Coprocessors(j)
        val id = copConf.CoprocessorID;

        if(copConf.requiresMemoryAccess) {
          // as memory access is required object is actually of CoprocessorMemory
          val copMem = cops(i)(id).asInstanceOf[CoprocessorMemoryAccess]
          memarbiter.io.master(arbiterEntry).M <> copMem.io.memPort.M
          copMem.io.memPort.S <> memarbiter.io.master(arbiterEntry).S
          arbiterEntry = arbiterEntry +1
        }
    
      }
    }
    ramCtrl.io.ocp.M <> memarbiter.io.slave.M
    memarbiter.io.slave.S <> ramCtrl.io.ocp.S
    ramCtrl.io.superMode := false.B
  }

  val io = IO(new PatmosBundle(pins.map {
    case (pinid, devicepin) => pinid -> DataMirror.internal.chiselTypeClone(devicepin)
  }.toSeq: _*))

  for((pinid, devicepin) <- pins) {
    val patmospin = io.elements(pinid)
    DataMirror.specifiedDirectionOf(devicepin).toString match {
        case "Input" => devicepin := patmospin
        case "Output" => patmospin := devicepin
        case "Unspecified" => attach(devicepin.asInstanceOf[Analog], patmospin.asInstanceOf[Analog])
    }
  }
  
  // Print out the configuration
  Utility.printConfig(configFile)
  
  if (genEmu) {

    envinfoOpt match {
      case Some(envinfo) => 
      {
        val envinfoIO = IO(new Bundle
        {
          val platform = Input(envinfo.platform.cloneType)
          val entrypoint = Input(envinfo.entrypoint.cloneType)
          val exit = Output(envinfo.exitReg.cloneType)
          val exitcode = Output(envinfo.exitcodeReg.cloneType)
        })
        envinfoIO.suggestName("envinfo")
        envinfoIO.exit := 0.U
        envinfoIO.exitcode := 0.U
        BoringUtils.bore(envinfoIO.platform, Seq(envinfo.platform))
        BoringUtils.bore(envinfoIO.entrypoint, Seq(envinfo.entrypoint))
        BoringUtils.bore(envinfo.exitReg, Seq(envinfoIO.exit))
        BoringUtils.bore(envinfo.exitcodeReg, Seq(envinfoIO.exitcode))
      }
      case None =>
    }

    uartcmpOpt match {
      case Some(uartcmp) => 
      {
        val uartcmpIO = IO(new Bundle
        {
          val tx_baud_tick = Output(uartcmp.uart.tx_baud_tick.cloneType)
        })
        uartcmpIO.suggestName("uartcmp")
        uartcmpIO.tx_baud_tick := false.B
        BoringUtils.bore(uartcmp.uart.tx_baud_tick, Seq(uartcmpIO.tx_baud_tick))
      }
      case None =>
    }

    val core0IO = IO(new Bundle
    {
      val enable = Output(cores(0).enableReg.cloneType)
      val pcNext = Output(cores(0).fetch.pcNext.cloneType)
      val relBaseNext = Output(cores(0).fetch.relBaseNext.cloneType)
      val rf = Output(cores(0).decode.rf.rfDebug.get.cloneType)
    })
    core0IO.suggestName("core0")
    core0IO.enable := false.B
    BoringUtils.bore(cores(0).enableReg, Seq(core0IO.enable))
    core0IO.pcNext := 0.U
    BoringUtils.bore(cores(0).fetch.pcNext, Seq(core0IO.pcNext))
    core0IO.relBaseNext := 0.U
    BoringUtils.bore(cores(0).fetch.relBaseNext, Seq(core0IO.relBaseNext))

    for (i <- (0 until core0IO.rf.length)) {
      core0IO.rf(i) := 0.U
      BoringUtils.bore(cores(0).decode.rf.rfDebug.get(i), Seq(core0IO.rf(i)))
    }
  }
}

object PatmosMain extends App {

  val chiselArgs = args.slice(4, args.length)
  val configFile = args(0)
  val binFile = args(1)
  val datFile = args(2)
  val genEmu = args(3).toBoolean

  val buildDir = scala.util.Properties.envOrElse("HWBUILDDIR", "build" ) + "/"
  new java.io.File(buildDir).mkdirs // build dir is created
  Config.loadConfig(configFile)
  (new chisel3.stage.ChiselStage).emitVerilog(new Patmos(configFile, binFile, datFile, genEmu), chiselArgs)
}
