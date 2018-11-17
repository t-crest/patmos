
/*
 Method Cache for Patmos
 Author: Philipp Degasperi (philipp.degasperi@gmail.com)
 */

package patmos

import Chisel._
import MConstants._
import Constants._
import ocp._

/*
  Method cache constants only used internally
 */
object MConstants {
  val MCACHE_WORD_SIZE = ICACHE_SIZE / 4
  val METHOD_COUNT = ICACHE_ASSOC
  val METHOD_COUNT_WIDTH = log2Up(METHOD_COUNT)
  val MCACHE_SIZE_WIDTH = log2Up(MCACHE_WORD_SIZE)
}

/*
  Internal connections for the method cache
 */
class MCacheCtrlIO extends Bundle() {
  val ena_in = Bool(INPUT)
  val fetchEna = Bool(OUTPUT)
  val ctrlrepl = new MCacheCtrlRepl().asOutput
  val replctrl = new MCacheReplCtrl().asInput
  val femcache = new FeICache().asInput
  val exmcache = new ExICache().asInput
  val ocp_port = new OcpBurstMasterPort(ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH)
  val illMem = Bool(OUTPUT)
  val forceHit = Bool(OUTPUT)
}
class MCacheCtrlRepl extends Bundle() {
  val wEna = Bool()
  val wData = Bits(width = INSTR_WIDTH)
  val wAddr = Bits(width = ADDR_WIDTH)
  val wTag = Bool()
  val addrEven = Bits(width = MCACHE_SIZE_WIDTH)
  val addrOdd = Bits(width = MCACHE_SIZE_WIDTH)
  val instrStall = Bool()
}
class MCacheReplCtrl extends Bundle() {
  val hit = Bool()
}
class MCacheReplIO extends Bundle() {
  val ena_in = Bool(INPUT)
  val invalidate = Bool(INPUT)
  val hitEna = Bool(OUTPUT)
  val exmcache = new ExICache().asInput
  val mcachefe = new ICacheFe().asOutput
  val ctrlrepl = new MCacheCtrlRepl().asInput
  val replctrl = new MCacheReplCtrl().asOutput
  val memIn = new MCacheMemIn().asOutput
  val memOut = new MCacheMemOut().asInput
  val perf = new InstructionCachePerf()
}

class MCacheMemIn extends Bundle() {
  val wEven = Bool()
  val wOdd = Bool()
  val wData = Bits(width = DATA_WIDTH)
  val wAddr = Bits(width = (log2Up(MCACHE_WORD_SIZE / 2)))
  val addrEven = Bits(width = (log2Up(MCACHE_WORD_SIZE / 2)))
  val addrOdd = Bits(width = (log2Up(MCACHE_WORD_SIZE / 2)))
}
class MCacheMemOut extends Bundle() {
  val instrEven = Bits(width = INSTR_WIDTH)
  val instrOdd = Bits(width = INSTR_WIDTH)
}
class MCacheMemIO extends Bundle() {
  val memIn = new MCacheMemIn().asInput
  val memOut = new MCacheMemOut().asOutput
}

/*
 MCache: Top Level Class for the Method Cache
 */
class MCache() extends Module {
  val io = IO(new ICacheIO())
  val ctrl = Module(new MCacheCtrl())
  val repl = Module(new MCacheReplFifo())
  //Use MCacheReplFifo2 for replacement with fixed block size
  //val repl = Module(new MCacheReplFifo2())
  val mem = Module(new MCacheMem())
  //connect inputs to method cache ctrl unit
  ctrl.io.ctrlrepl <> repl.io.ctrlrepl
  ctrl.io.femcache <> io.feicache
  ctrl.io.exmcache <> io.exicache
  ctrl.io.ocp_port <> io.ocp_port
  //connect inputs to method cache repl unit
  repl.io.exmcache <> io.exicache
  repl.io.mcachefe <> io.icachefe
  repl.io.replctrl <> ctrl.io.replctrl
  repl.io.perf <> io.perf
  //connect repl to on chip memory
  repl.io.memIn <> mem.io.memIn
  repl.io.memOut <> mem.io.memOut
  //connect enables
  ctrl.io.ena_in <> io.ena_in
  repl.io.ena_in <> io.ena_in
  //output enable depending on hit/miss/fetch
  io.ena_out := ctrl.io.fetchEna & (repl.io.hitEna || ctrl.io.forceHit)
  //connect illegal access signal
  io.illMem := ctrl.io.illMem
  //connect invalidate signal
  repl.io.invalidate := io.invalidate || ctrl.io.illMem
}

/*
 MCacheMem: On-Chip Even/Odd Memory
 */
class MCacheMem() extends Module {
  val io = IO(new MCacheMemIO())

  val mcacheEven = MemBlock(MCACHE_WORD_SIZE / 2, INSTR_WIDTH)
  val mcacheOdd = MemBlock(MCACHE_WORD_SIZE / 2, INSTR_WIDTH)

  mcacheEven.io <= (io.memIn.wEven, io.memIn.wAddr, io.memIn.wData)
  mcacheOdd.io <= (io.memIn.wOdd, io.memIn.wAddr, io.memIn.wData)

  io.memOut.instrEven := mcacheEven.io(io.memIn.addrEven)
  io.memOut.instrOdd := mcacheOdd.io(io.memIn.addrOdd)
}


/*
 MCacheReplFifo: Class controlls a FIFO replacement strategy
 including tag-memory to keep history of methods in cache.
 A variable block size is used in this replacement.
 */
class MCacheReplFifo() extends Module {
  val io = IO(new MCacheReplIO())

  //tag field tables  for reading tag memory
  val addrVec = { Vec.fill(METHOD_COUNT) { Reg(Bits(width = ADDR_WIDTH)) }}
  val sizeVec = { Vec.fill(METHOD_COUNT) { Reg(init = Bits(0, width = MCACHE_SIZE_WIDTH+1)) }}
  val validVec = { Vec.fill(METHOD_COUNT) { Reg(init = Bool(false)) }}
  val posVec = { Vec.fill(METHOD_COUNT) { Reg(Bits(width = MCACHE_SIZE_WIDTH)) }}
  //registers to save current replacement status
  val nextIndexReg = Reg(init = Bits(0, width = log2Up(METHOD_COUNT)))
  val nextTagReg = Reg(init = Bits(0, width = log2Up(METHOD_COUNT)))
  val nextPosReg = Reg(init = Bits(0, width = MCACHE_SIZE_WIDTH))
  val freeSpaceReg = Reg(init = SInt(MCACHE_WORD_SIZE, width = MCACHE_SIZE_WIDTH+2))
  //variables when call/return occurs to check tag field
  val posReg = Reg(init = Bits(0, width = MCACHE_SIZE_WIDTH))
  val hitReg = Reg(init = Bool(true))
  val wrPosReg = Reg(init = Bits(0, width = MCACHE_SIZE_WIDTH))
  val callRetBaseReg = Reg(init = UInt(1, DATA_WIDTH))
  val callAddrReg = Reg(init = UInt(1, DATA_WIDTH))
  val selSpmReg = Reg(init = Bool(false))
  val selCacheReg = Reg(init = Bool(false))

  io.perf.hit := Bool(false)
  io.perf.miss := Bool(false)

  // hit detection
  val hitVec = { Vec.fill(METHOD_COUNT) { Bool() } }
  val mergePosVec = { Vec.fill(METHOD_COUNT) { Bits(width = MCACHE_SIZE_WIDTH) } }
  for (i <- 0 until METHOD_COUNT) {
    hitVec(i) := Bool(false)
    mergePosVec(i) := Bits(0)
    when (io.exmcache.callRetBase === addrVec(i) && validVec(i)) {
      hitVec(i) := Bool(true)
      mergePosVec(i) := posVec(i)
    }
  }
  val hit = hitVec.fold(Bool(false))(_|_)
  val pos = Mux(hit, mergePosVec.fold(Bits(0))(_|_), nextPosReg)

  //read from tag memory on call/return to check if method is in the cache
  when (io.exmcache.doCallRet && io.ena_in) {

    callRetBaseReg := io.exmcache.callRetBase
    callAddrReg := io.exmcache.callRetAddr
    selSpmReg := io.exmcache.callRetBase(ADDR_WIDTH-1, ISPM_ONE_BIT-2) === Bits(0x1)
    val selCache = io.exmcache.callRetBase(ADDR_WIDTH-1, ISPM_ONE_BIT-1) >= Bits(0x1)
    selCacheReg := selCache
    when (selCache) {
      hitReg := hit
      posReg := pos

      when (hit) {
        io.perf.hit := Bool(true)
      } .otherwise {
        io.perf.miss := Bool(true)
      }
    }
  }

  val relBase = Mux(selCacheReg,
                    posReg.toUInt,
                    callRetBaseReg(ISPM_ONE_BIT-3, 0))
  val relPc = callAddrReg + relBase

  val reloc = Mux(selCacheReg,
                  callRetBaseReg - posReg.toUInt,
                  Mux(selSpmReg,
                      UInt(1 << (ISPM_ONE_BIT - 2)),
                      UInt(0)))

  //insert new tags when control unit requests
  when (io.ctrlrepl.wTag) {
    hitReg := Bool(true) //start fetch, we have again a hit!
    wrPosReg := posReg
    //update free space
    freeSpaceReg := freeSpaceReg - io.ctrlrepl.wData(MCACHE_SIZE_WIDTH,0) + sizeVec(nextIndexReg)
    //update tag fields
    posVec(nextIndexReg) := nextPosReg
    sizeVec(nextIndexReg) := io.ctrlrepl.wData(MCACHE_SIZE_WIDTH, 0)
    addrVec(nextIndexReg) := io.ctrlrepl.wAddr
    validVec(nextIndexReg) := Bool(true)
    //update pointers
    nextPosReg := nextPosReg + io.ctrlrepl.wData(MCACHE_SIZE_WIDTH-1,0)
    val nextTag = Mux(nextIndexReg === Bits(METHOD_COUNT - 1), Bits(0), nextIndexReg + Bits(1))
    nextIndexReg := nextTag
    when (nextTagReg === nextIndexReg) {
      nextTagReg := nextTag
    }
  }
  //free new space if still needed -> invalidate next method
  when (freeSpaceReg < SInt(0)) {
    freeSpaceReg := freeSpaceReg + sizeVec(nextTagReg)
    sizeVec(nextTagReg) := Bits(0)
    validVec(nextTagReg) := Bool(false)
    nextTagReg := Mux(nextTagReg === Bits(METHOD_COUNT - 1), Bits(0), nextTagReg + Bits(1))
  }

  val wParity = io.ctrlrepl.wAddr(0)
  //adder could be moved to ctrl. unit to operate with rel. addresses here
  val wAddr = (wrPosReg + io.ctrlrepl.wAddr)(MCACHE_SIZE_WIDTH-1,1)
  val addrEven = (io.ctrlrepl.addrEven)(MCACHE_SIZE_WIDTH-1,1)
  val addrOdd = (io.ctrlrepl.addrOdd)(MCACHE_SIZE_WIDTH-1,1)

  io.memIn.wEven := Mux(wParity, Bool(false), io.ctrlrepl.wEna)
  io.memIn.wOdd := Mux(wParity, io.ctrlrepl.wEna, Bool(false))
  io.memIn.wData := io.ctrlrepl.wData
  io.memIn.wAddr := wAddr
  io.memIn.addrEven := addrEven
  io.memIn.addrOdd := addrOdd

  val instrEvenReg = Reg(Bits(width = INSTR_WIDTH))
  val instrOddReg = Reg(Bits(width = INSTR_WIDTH))
  val instrEven = io.memOut.instrEven
  val instrOdd = io.memOut.instrOdd
  when (!io.ctrlrepl.instrStall) {
    instrEvenReg := io.mcachefe.instrEven
    instrOddReg := io.mcachefe.instrOdd
  }
  io.mcachefe.instrEven := Mux(io.ctrlrepl.instrStall, instrEvenReg, instrEven)
  io.mcachefe.instrOdd := Mux(io.ctrlrepl.instrStall, instrOddReg, instrOdd)
  io.mcachefe.base := callRetBaseReg
  io.mcachefe.relBase := relBase
  io.mcachefe.relPc := relPc
  io.mcachefe.reloc := reloc
  io.mcachefe.memSel := Cat(selSpmReg, selCacheReg)

  io.replctrl.hit := hitReg

  io.hitEna := hitReg
  
  // reset valid bits
  when (io.invalidate) {
    validVec.map(_ := Bool(false))
  }
}


/*
 MCacheCtrl Class: Main Class of Method Cache, implements the State Machine and handles the R/W/Fetch of Cache and External Memory
 */
class MCacheCtrl() extends Module {
  val io = IO(new MCacheCtrlIO())

  //fsm state variables
  val idleState :: sizeState :: transferState :: errorState :: errorDecState :: errorExeState :: errorMemState :: Nil = Enum(UInt(), 7)
  val stateReg = Reg(init = idleState)
  //signals for method cache memory (repl)
  val addrEven = Wire(Bits(width = ADDR_WIDTH))
  val addrOdd = Wire(Bits(width = ADDR_WIDTH))
  val wData = Wire(Bits(width = DATA_WIDTH))
  val wTag = Wire(Bool()) //signalizes the transfer of begin of a write
  val wAddr = Wire(Bits(width = ADDR_WIDTH))
  val wEna = Wire(Bool())
  //signals for external memory
  val ocpCmdReg = Reg(init = OcpCmd.IDLE)
  val ocpAddrReg = Reg(Bits(width = ADDR_WIDTH))
  val fetchEna = Wire(Bool())
  val transferSizeReg = Reg(Bits(width = MCACHE_SIZE_WIDTH))
  val fetchCntReg = Reg(Bits(width = MCACHE_SIZE_WIDTH))
  val burstCntReg = Reg(UInt(width = log2Up(BURST_LENGTH)))
  //input/output registers
  val callRetBaseReg = Reg(Bits(width = ADDR_WIDTH))
  val msizeAddr = callRetBaseReg - Bits(1)
  val addrEvenReg = Reg(Bits())
  val addrOddReg = Reg(Bits())

  val ocpSlaveReg = Reg(next = io.ocp_port.S)

  //init signals
  addrEven := addrEvenReg
  addrOdd := addrOddReg
  wData := Bits(0)
  wTag := Bool(false)
  wEna := Bool(false)
  wAddr := Bits(0)
  fetchEna := Bool(true)

  // reset command when accepted
  when (io.ocp_port.S.CmdAccept === Bits(1)) {
    ocpCmdReg := OcpCmd.IDLE
  }

  //output to external memory
  io.ocp_port.M.Addr := Cat(ocpAddrReg, Bits("b00"))
  io.ocp_port.M.Cmd := ocpCmdReg
  io.ocp_port.M.Data := Bits(0)
  io.ocp_port.M.DataByteEn := Bits("b1111")
  io.ocp_port.M.DataValid := Bits(0)

  when (io.exmcache.doCallRet) {
    callRetBaseReg := io.exmcache.callRetBase // use callret to save base address for next cycle
    addrEvenReg := io.femcache.addrEven
    addrOddReg := io.femcache.addrOdd
  }

  //check if instruction is available
  when (stateReg === idleState) {
    when(io.replctrl.hit === Bits(1)) {
      addrEven := io.femcache.addrEven
      addrOdd := io.femcache.addrOdd
    }
    //no hit... fetch from external memory
    .otherwise {
      burstCntReg := UInt(0)

      //aligned read from ssram
      io.ocp_port.M.Cmd := OcpCmd.RD
      when (io.ocp_port.S.CmdAccept === Bits(0)) {
        ocpCmdReg := OcpCmd.RD
      }
      io.ocp_port.M.Addr := Cat(msizeAddr(ADDR_WIDTH-1,log2Up(BURST_LENGTH)),
                                Bits(0, width=log2Up(BURST_LENGTH)+2))
      ocpAddrReg := Cat(msizeAddr(ADDR_WIDTH-1,log2Up(BURST_LENGTH)),
                        Bits(0, width=log2Up(BURST_LENGTH)))

      stateReg := sizeState
    }
  }
  //fetch size of the required method from external memory address - 1
  when (stateReg === sizeState) {
    fetchEna := Bool(false)
    when (ocpSlaveReg.Resp === OcpResp.DVA) {
      burstCntReg := burstCntReg + Bits(1)
      when (burstCntReg === msizeAddr(log2Up(BURST_LENGTH)-1,0)) {
        val size = ocpSlaveReg.Data(MCACHE_SIZE_WIDTH+2,2)
        //init transfer from external memory
        transferSizeReg := size - Bits(1)
        fetchCntReg := Bits(0) //start to write to cache with offset 0
        when (burstCntReg === UInt(BURST_LENGTH - 1)) {
          io.ocp_port.M.Cmd := OcpCmd.RD
          when (io.ocp_port.S.CmdAccept === Bits(0)) {
            ocpCmdReg := OcpCmd.RD
          }
          io.ocp_port.M.Addr := Cat(callRetBaseReg, Bits("b00"))
          ocpAddrReg := callRetBaseReg
          burstCntReg := UInt(0)
        }
        //init transfer to on-chip method cache memory
        wTag := Bool(true)
        //size rounded to next double-word
        wData := size+size(0)
        //write base address to mem for tagfield
        wAddr := callRetBaseReg
        stateReg := transferState
      }
    }
  }

  //transfer/fetch method to the cache
  when (stateReg === transferState) {
    fetchEna := Bool(false)
    when (fetchCntReg <= transferSizeReg) {
      when (ocpSlaveReg.Resp === OcpResp.DVA) {
        fetchCntReg := fetchCntReg + Bits(1)
        burstCntReg := burstCntReg + Bits(1)
        when(fetchCntReg < transferSizeReg) {
          //fetch next address from external memory
          when (burstCntReg === UInt(BURST_LENGTH - 1)) {
            io.ocp_port.M.Cmd := OcpCmd.RD
            when (io.ocp_port.S.CmdAccept === Bits(0)) {
              ocpCmdReg := OcpCmd.RD
            }
            io.ocp_port.M.Addr := Cat(callRetBaseReg + fetchCntReg + Bits(1), Bits("b00"))
            ocpAddrReg := callRetBaseReg + fetchCntReg + Bits(1) //need +1 because start fetching with the size of method
            burstCntReg := UInt(0)
          }
        }
        .otherwise {
          //restart to idle state if burst is done now
          when (burstCntReg === UInt(BURST_LENGTH - 1)) {
            fetchEna := Bool(true)
            addrEven := io.femcache.addrEven
            addrOdd := io.femcache.addrOdd
            stateReg := idleState
          }
        }
        //write current address to mcache memory
        wData := ocpSlaveReg.Data
        wEna := Bool(true)
      }
      wAddr := fetchCntReg
    }
    //restart to idle state after burst is done
    .otherwise {
      when (ocpSlaveReg.Resp === OcpResp.DVA) {
        burstCntReg := burstCntReg + Bits(1)
      }
      when (burstCntReg === UInt(BURST_LENGTH - 1)) {
        fetchEna := Bool(true)
        addrEven := io.femcache.addrEven
        addrOdd := io.femcache.addrOdd
        stateReg := idleState
      }
    }
  }

  // abort on error response
  io.illMem := Bool(false)
  io.forceHit := Bool(false)
  when (ocpSlaveReg.Resp === OcpResp.ERR) {
    io.ocp_port.M.Cmd := OcpCmd.IDLE
    ocpCmdReg := OcpCmd.IDLE
    burstCntReg := burstCntReg + Bits(1)
    stateReg := errorState
  }
  // wait for end of burst before signalling error
  when (stateReg === errorState) {
    when (ocpSlaveReg.Resp =/= OcpResp.NULL) {
      burstCntReg := burstCntReg + Bits(1)
    }
    when (burstCntReg === UInt(BURST_LENGTH - 1)) {
      io.illMem := Bool(true)
      io.forceHit := Bool(true)
      stateReg := errorDecState
    }
  }
  // force a fake hit while the exception is fed through the pipeline
  when (stateReg === errorDecState) {
      io.forceHit := Bool(true)
      stateReg := errorExeState
  }
  when (stateReg === errorExeState) {
      io.forceHit := Bool(true)
      stateReg := errorMemState
  }
  when (stateReg === errorMemState) {
      io.forceHit := Bool(true)
      stateReg := idleState
  }

  //outputs to mcache memory
  io.ctrlrepl.addrEven := addrEven
  io.ctrlrepl.addrOdd := addrOdd
  io.ctrlrepl.wEna := wEna
  io.ctrlrepl.wData := wData
  io.ctrlrepl.wAddr := wAddr
  io.ctrlrepl.wTag := wTag
  io.ctrlrepl.instrStall := stateReg =/= idleState

  io.fetchEna := fetchEna
}

