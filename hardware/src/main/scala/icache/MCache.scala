
/*
 Method Cache for Patmos
 Author: Philipp Degasperi (philipp.degasperi@gmail.com)
 */

package patmos

import Chisel._
import chisel3.VecInit

import chisel3.dontTouch
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
  val ena_in = Input(Bool())
  val fetchEna = Output(Bool())
  val ctrlrepl = new MCacheCtrlRepl().asOutput
  val replctrl = new MCacheReplCtrl().asInput
  val femcache = new FeICache().asInput
  val exmcache = new ExICache().asInput
  val ocp_port = new OcpBurstMasterPort(ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH)
  val illMem = Output(Bool())
  val forceHit = Output(Bool())
}
class MCacheCtrlRepl extends Bundle() {
  val wEna = Bool()
  val wData = UInt(INSTR_WIDTH.W)
  val wAddr = UInt(ADDR_WIDTH.W)
  val wTag = Bool()
  val addrEven = UInt(MCACHE_SIZE_WIDTH.W)
  val addrOdd = UInt(MCACHE_SIZE_WIDTH.W)
  val instrStall = Bool()
}
class MCacheReplCtrl extends Bundle() {
  val hit = Bool()
}
class MCacheReplIO extends Bundle() {
  val ena_in = Input(Bool())
  val invalidate = Input(Bool())
  val hitEna = Output(Bool())
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
  val wData = UInt(DATA_WIDTH.W)
  val wAddr = UInt(log2Up(MCACHE_WORD_SIZE / 2).W)
  val addrEven = UInt(log2Up(MCACHE_WORD_SIZE / 2).W)
  val addrOdd = UInt(log2Up(MCACHE_WORD_SIZE / 2).W)
}
class MCacheMemOut extends Bundle() {
  val instrEven = UInt(INSTR_WIDTH.W)
  val instrOdd = UInt(INSTR_WIDTH.W)
}
class MCacheMemIO extends Bundle() {
  val memIn = new MCacheMemIn().asInput
  val memOut = new MCacheMemOut().asOutput
}

/*
 MCache: Top Level Class for the Method Cache
 */
class MCache() extends CacheType {
  val ctrl = Module(new MCacheCtrl())
  val repl = Module(new MCacheReplFifo())
  //Use MCacheReplFifo2 for replacement with fixed block size
  //val repl = Module(new MCacheReplFifo2())
  val mem = Module(new MCacheMem())
  //connect inputs to method cache ctrl unit
  repl.io.ctrlrepl <> ctrl.io.ctrlrepl
  ctrl.io.femcache <> io.feicache
  ctrl.io.exmcache <> io.exicache
  io.ocp_port <> ctrl.io.ocp_port
  //connect inputs to method cache repl unit
  repl.io.exmcache <> io.exicache
  io.icachefe <> repl.io.mcachefe
  ctrl.io.replctrl <> repl.io.replctrl
  io.perf <> repl.io.perf
  //connect repl to on chip memory
  mem.io.memIn <> repl.io.memIn
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
  val addrVec = RegInit(VecInit(Seq.fill(METHOD_COUNT)(0.U(ADDR_WIDTH.W))))
  val sizeVec = RegInit(VecInit(Seq.fill(METHOD_COUNT)(0.U((MCACHE_SIZE_WIDTH+1).W))))
  val validVec = RegInit(VecInit(Seq.fill(METHOD_COUNT)(false.B)))
  val posVec = RegInit(VecInit(Seq.fill(METHOD_COUNT)(0.U(MCACHE_SIZE_WIDTH.W))))
  //registers to save current replacement status
  val nextIndexReg = Reg(init = 0.U(log2Up(METHOD_COUNT).W))
  val nextTagReg = Reg(init = 0.U(log2Up(METHOD_COUNT).W))
  val nextPosReg = Reg(init = 0.U(MCACHE_SIZE_WIDTH.W))
  val freeSpaceReg = Reg(init = SInt(MCACHE_WORD_SIZE, width = MCACHE_SIZE_WIDTH+2))
  //variables when call/return occurs to check tag field
  val posReg = Reg(init = 0.U(MCACHE_SIZE_WIDTH.W))
  val hitReg = Reg(init = true.B)
  val hitNext = dontTouch(Wire(Bool())) //For emulator
  val wrPosReg = Reg(init = 0.U(MCACHE_SIZE_WIDTH.W))
  val callRetBaseReg = Reg(init = 1.U(DATA_WIDTH.W))
  val callRetBaseNext = dontTouch(Wire(UInt(DATA_WIDTH.W))) // emulator
  val callAddrReg = Reg(init = 1.U(DATA_WIDTH.W))
  val selSpmReg = Reg(init = false.B)
  val selSpmNext = dontTouch(Wire(Bool())) //for emulator
  val selCacheReg = Reg(init = false.B)
  val selCacheNext = dontTouch(Wire(Bool())) //for emulator

  io.perf.hit := false.B
  io.perf.miss := false.B
  hitNext := hitReg
  hitReg := hitNext
  callRetBaseNext := callRetBaseReg
  callRetBaseReg := callRetBaseNext
  selSpmNext := selSpmReg
  selSpmReg := selSpmNext
  selCacheNext := selCacheReg
  selCacheReg := selCacheNext
  // hit detection
  val hitVec =  Wire(Vec(METHOD_COUNT, Bool() ))
  val mergePosVec = Wire(Vec(METHOD_COUNT,  UInt(MCACHE_SIZE_WIDTH.W) ))
  for (i <- 0 until METHOD_COUNT) {
    hitVec(i) := false.B
    mergePosVec(i) := 0.U
    when (io.exmcache.callRetBase === addrVec(i) && validVec(i)) {
      hitVec(i) := true.B
      mergePosVec(i) := posVec(i)
    }
  }
  val hit = hitVec.fold(false.B)(_|_)
  val pos = Mux(hit, mergePosVec.fold(0.U)(_|_), nextPosReg)

  //read from tag memory on call/return to check if method is in the cache
  when (io.exmcache.doCallRet && io.ena_in) {

    callRetBaseNext := io.exmcache.callRetBase
    callAddrReg := io.exmcache.callRetAddr
    selSpmNext := io.exmcache.callRetBase(ADDR_WIDTH-1, ISPM_ONE_BIT-2) === 0x1.U
    val selCache = io.exmcache.callRetBase(ADDR_WIDTH-1, ISPM_ONE_BIT-1) >= 0x1.U
    selCacheNext := selCache
    when (selCache) {
      hitNext := hit
      posReg := pos

      when (hit) {
        io.perf.hit := true.B
      } .otherwise {
        io.perf.miss := true.B
      }
    }
  }

  val relBase = Mux(selCacheReg,
                    posReg.asUInt,
                    callRetBaseReg(ISPM_ONE_BIT-3, 0))
  val relPc = callAddrReg + relBase

  val reloc = Mux(selCacheReg,
                  callRetBaseReg - posReg.asUInt,
                  Mux(selSpmReg,
                      (1 << (ISPM_ONE_BIT - 2)).U,
                      0.U))

  //insert new tags when control unit requests
  when (io.ctrlrepl.wTag) {
    hitNext := true.B //start fetch, we have again a hit!
    wrPosReg := posReg
    //update free space
    freeSpaceReg := freeSpaceReg - io.ctrlrepl.wData(MCACHE_SIZE_WIDTH,0).asSInt + sizeVec(nextIndexReg).asSInt
    //update tag fields
    posVec(nextIndexReg) := nextPosReg
    sizeVec(nextIndexReg) := io.ctrlrepl.wData(MCACHE_SIZE_WIDTH, 0)
    addrVec(nextIndexReg) := io.ctrlrepl.wAddr
    validVec(nextIndexReg) := true.B
    //update pointers
    nextPosReg := nextPosReg + io.ctrlrepl.wData(MCACHE_SIZE_WIDTH-1,0)
    val nextTag = Mux(nextIndexReg === (METHOD_COUNT - 1).U, 0.U, nextIndexReg + 1.U)
    nextIndexReg := nextTag
    when (nextTagReg === nextIndexReg) {
      nextTagReg := nextTag
    }
  }
  //free new space if still needed -> invalidate next method
  when (freeSpaceReg < SInt(0)) {
    freeSpaceReg := freeSpaceReg + sizeVec(nextTagReg).asSInt
    sizeVec(nextTagReg) := 0.U
    validVec(nextTagReg) := false.B
    nextTagReg := Mux(nextTagReg === (METHOD_COUNT - 1).U, 0.U, nextTagReg + 1.U)
  }

  val wParity = io.ctrlrepl.wAddr(0)
  //adder could be moved to ctrl. unit to operate with rel. addresses here
  val wAddr = (wrPosReg + io.ctrlrepl.wAddr)(MCACHE_SIZE_WIDTH-1,1)
  val addrEven = (io.ctrlrepl.addrEven)(MCACHE_SIZE_WIDTH-1,1)
  val addrOdd = (io.ctrlrepl.addrOdd)(MCACHE_SIZE_WIDTH-1,1)

  io.memIn.wEven := Mux(wParity, false.B, io.ctrlrepl.wEna)
  io.memIn.wOdd := Mux(wParity, io.ctrlrepl.wEna, false.B)
  io.memIn.wData := io.ctrlrepl.wData
  io.memIn.wAddr := wAddr
  io.memIn.addrEven := addrEven
  io.memIn.addrOdd := addrOdd

  val instrEvenReg = Reg(UInt(INSTR_WIDTH.W))
  val instrOddReg = Reg(UInt(INSTR_WIDTH.W))
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
    validVec.map(_ := false.B)
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
  val addrEven = Wire(UInt(ADDR_WIDTH.W))
  val addrOdd = Wire(UInt(ADDR_WIDTH.W))
  val wData = Wire(UInt(DATA_WIDTH.W))
  val wTag = Wire(Bool()) //signalizes the transfer of begin of a write
  val wAddr = Wire(UInt(ADDR_WIDTH.W))
  val wEna = Wire(Bool())
  //signals for external memory
  val ocpCmdReg = Reg(init = OcpCmd.IDLE)
  val ocpAddrReg = Reg(UInt(ADDR_WIDTH.W))
  val fetchEna = Wire(Bool())
  val transferSizeReg = Reg(UInt(MCACHE_SIZE_WIDTH.W))
  val fetchCntReg = Reg(UInt(MCACHE_SIZE_WIDTH.W))
  val burstCntReg = Reg(UInt(log2Up(BURST_LENGTH).W))
  //input/output registers
  val callRetBaseReg = Reg(UInt(ADDR_WIDTH.W))
  val callRetBaseNext = dontTouch(Wire(UInt(ADDR_WIDTH.W))) //For emulator
  val msizeAddr = callRetBaseReg - 1.U
  val addrEvenReg = Reg(UInt())
  val addrOddReg = Reg(UInt())

  val ocpSlaveReg = Reg(next = io.ocp_port.S)

  //init signals
  addrEven := addrEvenReg
  addrOdd := addrOddReg
  wData := 0.U
  wTag := false.B
  wEna := false.B
  wAddr := 0.U
  fetchEna := true.B

  callRetBaseNext := callRetBaseReg
  callRetBaseReg := callRetBaseNext

  // reset command when accepted
  when (io.ocp_port.S.CmdAccept === 1.U) {
    ocpCmdReg := OcpCmd.IDLE
  }

  //output to external memory
  io.ocp_port.M.Addr := Cat(ocpAddrReg, 0.U(2.W))
  io.ocp_port.M.Cmd := ocpCmdReg
  io.ocp_port.M.Data := 0.U
  io.ocp_port.M.DataByteEn := "b1111".U(4.W)
  io.ocp_port.M.DataValid := 0.U

  when (io.exmcache.doCallRet) {
    callRetBaseNext := io.exmcache.callRetBase // use callret to save base address for next cycle
    addrEvenReg := io.femcache.addrEven
    addrOddReg := io.femcache.addrOdd
  }

  //check if instruction is available
  when (stateReg === idleState) {
    when(io.replctrl.hit === 1.U) {
      addrEven := io.femcache.addrEven
      addrOdd := io.femcache.addrOdd
    }
    //no hit... fetch from external memory
    .otherwise {
      burstCntReg := 0.U

      //aligned read from ssram
      io.ocp_port.M.Cmd := OcpCmd.RD
      when (io.ocp_port.S.CmdAccept === 0.U) {
        ocpCmdReg := OcpCmd.RD
      }
      io.ocp_port.M.Addr := Cat(msizeAddr(ADDR_WIDTH-1,log2Up(BURST_LENGTH)),
                                0.U((log2Up(BURST_LENGTH)+2).W))
      ocpAddrReg := Cat(msizeAddr(ADDR_WIDTH-1,log2Up(BURST_LENGTH)),
                        0.U(log2Up(BURST_LENGTH).W))

      stateReg := sizeState
    }
  }
  //fetch size of the required method from external memory address - 1
  when (stateReg === sizeState) {
    fetchEna := false.B
    when (ocpSlaveReg.Resp === OcpResp.DVA) {
      burstCntReg := burstCntReg + 1.U
      when (burstCntReg === msizeAddr(log2Up(BURST_LENGTH)-1,0)) {
        val size = ocpSlaveReg.Data(MCACHE_SIZE_WIDTH+2,2)
        //init transfer from external memory
        transferSizeReg := size - 1.U
        fetchCntReg := 0.U //start to write to cache with offset 0
        when (burstCntReg === (BURST_LENGTH - 1).U) {
          io.ocp_port.M.Cmd := OcpCmd.RD
          when (io.ocp_port.S.CmdAccept === 0.U) {
            ocpCmdReg := OcpCmd.RD
          }
          io.ocp_port.M.Addr := Cat(callRetBaseReg, 0.U(2.W))
          ocpAddrReg := callRetBaseReg
          burstCntReg := 0.U
        }
        //init transfer to on-chip method cache memory
        wTag := true.B
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
    fetchEna := false.B
    when (fetchCntReg <= transferSizeReg) {
      when (ocpSlaveReg.Resp === OcpResp.DVA) {
        fetchCntReg := fetchCntReg + 1.U
        burstCntReg := burstCntReg + 1.U
        when(fetchCntReg < transferSizeReg) {
          //fetch next address from external memory
          when (burstCntReg === (BURST_LENGTH - 1).U) {
            io.ocp_port.M.Cmd := OcpCmd.RD
            when (io.ocp_port.S.CmdAccept === 0.U) {
              ocpCmdReg := OcpCmd.RD
            }
            io.ocp_port.M.Addr := Cat(callRetBaseReg + fetchCntReg + 1.U, 0.U(2.W))
            ocpAddrReg := callRetBaseReg + fetchCntReg + 1.U //need +1 because start fetching with the size of method
            burstCntReg := 0.U
          }
        }
        .otherwise {
          //restart to idle state if burst is done now
          when (burstCntReg === (BURST_LENGTH - 1).U) {
            fetchEna := true.B
            addrEven := io.femcache.addrEven
            addrOdd := io.femcache.addrOdd
            stateReg := idleState
          }
        }
        //write current address to mcache memory
        wData := ocpSlaveReg.Data
        wEna := true.B
      }
      wAddr := fetchCntReg
    }
    //restart to idle state after burst is done
    .otherwise {
      when (ocpSlaveReg.Resp === OcpResp.DVA) {
        burstCntReg := burstCntReg + 1.U
      }
      when (burstCntReg === (BURST_LENGTH - 1).U) {
        fetchEna := true.B
        addrEven := io.femcache.addrEven
        addrOdd := io.femcache.addrOdd
        stateReg := idleState
      }
    }
  }

  // abort on error response
  io.illMem := false.B
  io.forceHit := false.B
  when (ocpSlaveReg.Resp === OcpResp.ERR) {
    io.ocp_port.M.Cmd := OcpCmd.IDLE
    ocpCmdReg := OcpCmd.IDLE
    burstCntReg := burstCntReg + 1.U
    stateReg := errorState
  }
  // wait for end of burst before signalling error
  when (stateReg === errorState) {
    when (ocpSlaveReg.Resp =/= OcpResp.NULL) {
      burstCntReg := burstCntReg + 1.U
    }
    when (burstCntReg === (BURST_LENGTH - 1).U) {
      io.illMem := true.B
      io.forceHit := true.B
      stateReg := errorDecState
    }
  }
  // force a fake hit while the exception is fed through the pipeline
  when (stateReg === errorDecState) {
      io.forceHit := true.B
      stateReg := errorExeState
  }
  when (stateReg === errorExeState) {
      io.forceHit := true.B
      stateReg := errorMemState
  }
  when (stateReg === errorMemState) {
      io.forceHit := true.B
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

