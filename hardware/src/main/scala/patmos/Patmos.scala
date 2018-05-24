/*
 * Patmos top level component and test driver.
 *
 * Authors: Martin Schoeberl (martin@jopdesign.com)
 *          Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package patmos

import Chisel._
import Node._

import scala.collection.mutable.HashMap
import java.io.File

import Constants._

import util._
import io._
import datacache._
import ocp._

/**
 * Module for one Patmos core.
 */
class PatmosCore(binFile: String, nr: Int, cnt: Int, cmpdevs: List[(CoreDeviceIO, Int, String)] = List.empty) extends Module {

  val io = Config.getPatmosCoreIO()

  val icache =
    if (ICACHE_SIZE <= 0)
      Module(new NullICache())
    else if (ICACHE_TYPE == ICACHE_TYPE_METHOD && ICACHE_REPL == CACHE_REPL_FIFO)
      Module(new MCache())
    else if (ICACHE_TYPE == ICACHE_TYPE_LINE && ICACHE_ASSOC == 1)
      Module(new ICache())
    else {
      ChiselError.error("Unsupported instruction cache configuration:" +
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
  val iocomp = Module(new InOut(nr, cnt, cmpdevs))
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

  memory.io.localInOut <> iocomp.io.memInOut

  // Connect exception unit
  exc.io.ocp <> iocomp.io.excInOut
  exc.io.intrs <> iocomp.io.intrs
  exc.io.excdec <> decode.io.exc
  exc.io.memexc <> memory.io.exc

  // Connect data cache
  dcache.io.master <> memory.io.globalInOut

  // Merge OCP ports from data caches and method cache
  val burstBus = Module(new OcpBurstBus(ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH))
  val selICache = Bool()
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
  mmu.io.ctrl <> iocomp.io.mmuInOut
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
  iocomp.io.superMode := exc.io.superMode
  mmu.io.superMode := exc.io.superMode
  io.superMode := exc.io.superMode

  // Internal "I/O" data
  iocomp.io.internalIO.perf.ic := icache.io.perf
  iocomp.io.internalIO.perf.dc := dcache.io.dcPerf
  iocomp.io.internalIO.perf.sc := dcache.io.scPerf
  iocomp.io.internalIO.perf.wc := dcache.io.wcPerf
  iocomp.io.internalIO.perf.mem.read := (io.memPort.M.Cmd === OcpCmd.RD &&
    io.memPort.S.CmdAccept === Bits(1))
  iocomp.io.internalIO.perf.mem.write := (io.memPort.M.Cmd === OcpCmd.WR &&
    io.memPort.S.CmdAccept === Bits(1))

  // The inputs and outputs
  io.comConf <> iocomp.io.comConf
  io.comSpm <> iocomp.io.comSpm
  io.memPort <> mmu.io.phys
  Config.connectAllIOPins(io, iocomp.io)

  // Keep signal alive for debugging
  debug(enableReg)
}

/**
 * This is only used by aegean to strip off the memory
 * controller. Shall go with the new CMP configuration.
 */
object PatmosCoreMain {
  def main(args: Array[String]): Unit = {

    val chiselArgs = args.slice(3, args.length)
    val configFile = args(0)
    val binFile = args(1)
    val datFile = args(2)

    Config.loadConfig(configFile)
    Config.minPcWidth = util.log2Up((new File(binFile)).length.toInt / 4)
    Config.datFile = datFile
    chiselMain(chiselArgs, () => Module(new PatmosCore(binFile, 0, 0)))
    // Print out the configuration
    Utility.printConfig(configFile)
  }
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

  println("Config core count: " + nrCores)

  val memarbiter = Module(new ocp.TdmArbiterWrapper(nrCores, ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH))

  // Instantiate cores
  val cores = (0 until nrCores).map(i => Module(new PatmosCore(binFile, i, nrCores)))

  // Forward ports to/from core

  val cmpDevice = Config.getConfig.cmpDevice
  println("Config cmp: " + cmpDevice)
  // This is a hack and workaround for CMP experiments
  if (cmpDevice == 0) {
    val hardlock = Module(new cmp.HardlockOCPWrapper(() => new cmp.Hardlock(nrCores, nrCores)))
    for (i <- (0 until nrCores)) {
      hardlock.io(i) <> cores(i).io.comSpm
    }
  } else if (cmpDevice == 1) {
    val spm = Module(new cmp.SharedSPM(nrCores, 1024))
    for (i <- (0 until nrCores)) {
      spm.io(i) <> cores(i).io.comSpm
    }
  } else if (cmpDevice == 2) {
    val oneway = Module(new cmp.OneWayOCPWrapper(nrCores))
    for (i <- (0 until nrCores)) {
      oneway.io(i) <> cores(i).io.comSpm
    }
    // 3 and 4 are reserved for Oktay and Lefteris
  } else if (cmpDevice == 3) {
    val tdmArbiter = Module(new cmp.TdmArbiter(nrCores))
    for (i <- (0 until nrCores)) {
      tdmArbiter.io.slave(i) <> cores(i).io.comSpm
    }
  } else if (cmpDevice == 5) {
    val ownspm = Module(new cmp.OwnSPM(nrCores, nrCores, 1024))
    for (i <- (0 until nrCores)) {
      ownspm.io(i) <> cores(i).io.comSpm
    }
  } else if (cmpDevice == 6) {
    val spmpool = Module(new cmp.SPMPoolOCPWrapper(nrCores, nrCores, 256, 32))
    for (i <- (0 until nrCores)) {
      spmpool.io(i) <> cores(i).io.comSpm
    }
  } else if (cmpDevice == 7) {
    val s4noc = Module(new cmp.S4nocOCPWrapper(nrCores, 4, 4))
    for (i <- (0 until nrCores)) {
      s4noc.io(i) <> cores(i).io.comSpm
    }
  } else if (cmpDevice == 10) {
    val asynclock = Module(new cmp.AsyncLock(nrCores,nrCores))
    for (i <- (0 until nrCores)) {
      asynclock.io(i) <> cores(i).io.comSpm
    }
  } else if (cmpDevice == 11) {
    val caspm = Module(new cmp.CASPM(nrCores,nrCores*4))
    for (i <- (0 until nrCores)) {
      caspm.io(i) <> cores(i).io.comSpm
    }
  } else if (cmpDevice == 12) {
    val spm = conc.SharedLLSCSpm(64, nrCores, 1024)
    for (i <- (0 until cores.length)) {
      spm.io(i) <> cores(i).io.comSpm
    }
  }

  for (i <- (0 until cores.length)) {
    memarbiter.io.master(i) <> cores(i).io.memPort
  }

  // Only core 0 gets its devices connected to pins
  Config.connectAllIOPins(io, cores(0).io)

  // Connect memory controller
  val ramConf = Config.getConfig.ExtMem.ram
  val ramCtrl = Config.createDevice(ramConf).asInstanceOf[BurstDevice]
  ramCtrl.io.ocp <> memarbiter.io.slave
  Config.connectIOPins(ramConf.name, io, ramCtrl.io)

  // Print out the configuration
  Utility.printConfig(configFile)
}

// this testing and main file should go into it's own folder

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
