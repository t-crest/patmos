/*
 * The data cache unit for Patmos
 *
 * Author: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 */

package datacache

import Chisel._


import stackcache._
import patmos._
import patmos.Constants._

import ocp._

class DataCache extends Module {
  val io = IO(new Bundle {
    val master = new OcpCacheSlavePort(ADDR_WIDTH, DATA_WIDTH)
    val slave = new OcpBurstMasterPort(ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH)
    val scIO = new StackCacheIO()
    val invalDCache = Bool(INPUT)
    val dcPerf = new DataCachePerf()
    val scPerf = new StackCachePerf()
    val wcPerf = new WriteCombinePerf()
  })

  // Register selects
  val selDC = Bool(DCACHE_SIZE > 0) && io.master.M.AddrSpace === OcpCache.DATA_CACHE
  val selDCReg = Reg(Bool())
  val selSC = io.master.M.AddrSpace === OcpCache.STACK_CACHE
  val selSCReg = Reg(Bool())
  when(io.master.M.Cmd =/= OcpCmd.IDLE) {
    selDCReg := selDC
    selSCReg := selSC
  }

  // Instantiate direct-mapped cache for regular data cache
  val dm = 
    if (DCACHE_SIZE <= 0)
      Module(new NullCache())
    else if (DCACHE_ASSOC == 1 && DCACHE_WRITETHROUGH)
      Module(new DirectMappedCache(DCACHE_SIZE, BURST_LENGTH*BYTES_PER_WORD))
    else if (DCACHE_ASSOC == 1 && !DCACHE_WRITETHROUGH)
      Module(new DirectMappedCacheWriteBack(DCACHE_SIZE, BURST_LENGTH*BYTES_PER_WORD))
    else if (DCACHE_ASSOC == 2 && DCACHE_REPL == CACHE_REPL_LRU && DCACHE_WRITETHROUGH)
      Module(new TwoWaySetAssociativeCache(DCACHE_SIZE, BURST_LENGTH*BYTES_PER_WORD))
    else {
      throw new Error("Unsupported data cache configuration: "+
                        "associativity "+DCACHE_ASSOC+
                        " with replacement policy \""+DCACHE_REPL+"\""+
                        " and write "+(if (DCACHE_WRITETHROUGH) "through" else "back"))
      Module(new NullCache()) // return at least a dummy cache
    }

  dm.io.master.M := io.master.M
  dm.io.master.M.Cmd := Mux(selDC ||
                            (Bool(DCACHE_WRITETHROUGH) && Bool(DCACHE_SIZE > 0) &&
                             io.master.M.Cmd === OcpCmd.WR),
                            io.master.M.Cmd, OcpCmd.IDLE)
  dm.io.invalidate := io.invalDCache
  val dmS = dm.io.master.S

  // Instantiate stack cache
  val sc = Module(if (SCACHE_SIZE <= 0) new NullStackCache() else new StackCache())
  io.scIO <> sc.io

  // connect the stack cache to the Patmos core
  sc.io.fromCPU.M := io.master.M
  sc.io.fromCPU.M.Cmd := Mux(selSC, io.master.M.Cmd, OcpCmd.IDLE)
  val scS = sc.io.fromCPU.S

  // Instantiate bridge for bypasses and writes
  val bp = Module(new NullCache())
  bp.io.master.M := io.master.M
  bp.io.master.M.Cmd := Mux(!selDC && !selSC, io.master.M.Cmd, OcpCmd.IDLE)
  val bpS = bp.io.master.S

  // Join read requests
  val burstReadBus1 = Module(new OcpBurstBus(ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH))
  val burstReadJoin1 = new OcpBurstJoin(dm.io.slave, bp.io.slave, burstReadBus1.io.slave)

  val burstReadBus2 = Module(new OcpBurstBus(ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH))
  val burstReadJoin2 = new OcpBurstJoin(sc.io.toMemory, burstReadBus1.io.master, burstReadBus2.io.slave)

  // Combine writes
  val wc = Module(if (WRITE_COMBINE) new WriteCombineBuffer() else new WriteNoBuffer())
  wc.io.readMaster.M <> burstReadBus2.io.master.M
  burstReadBus2.io.master.S <> wc.io.readMaster.S
  wc.io.writeMaster.M := io.master.M
  wc.io.writeMaster.M.Cmd := Mux(!selSC && (Bool(DCACHE_WRITETHROUGH) || !selDC),
                                 io.master.M.Cmd, OcpCmd.IDLE)
  val wcWriteS = wc.io.writeMaster.S
  io.slave <> wc.io.slave

  // Pass data to pipeline
  io.master.S.Data := bpS.Data
  when(selDCReg) { io.master.S.Data := dmS.Data }
  when(selSCReg) { io.master.S.Data := scS.Data }

  // Merge responses
  io.master.S.Resp := dmS.Resp | scS.Resp | bpS.Resp | wcWriteS.Resp

  // Pass on performance counters
  io.dcPerf <> dm.io.perf
  io.scPerf <> sc.io.perf
  io.wcPerf <> wc.io.perf
}
