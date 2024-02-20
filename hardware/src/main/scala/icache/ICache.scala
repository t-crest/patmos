/*
  Instruction Cache for Patmos
  Authors: Philipp Degasperi (philipp.degasperi@gmail.com)
           Wolfgang Puffitsch (wpuffitsch@gmail.com)
 */

package patmos

import chisel3._
import chisel3.util._
import IConstants._
import Constants._
import ocp._

/*
  Instruction cache constants only used internally
 */
object IConstants {

  val ICACHE_WORD_SIZE = ICACHE_SIZE / 4
  val ICACHE_SIZE_WIDTH = log2Up(ICACHE_WORD_SIZE)

  val LINE_WORD_SIZE = BURST_LENGTH
  val LINE_WORD_SIZE_WIDTH = log2Up(LINE_WORD_SIZE)

  val LINE_COUNT = ICACHE_WORD_SIZE / LINE_WORD_SIZE
  val LINE_COUNT_WIDTH = log2Up(LINE_COUNT)

  val TAG_HIGH = ADDR_WIDTH - 1
  val TAG_LOW = LINE_COUNT_WIDTH + LINE_WORD_SIZE_WIDTH
  val TAG_SIZE = TAG_HIGH - TAG_LOW + 1

  val INDEX_HIGH = TAG_LOW - 1
  val INDEX_LOW = LINE_WORD_SIZE_WIDTH
  val INDEX_SIZE = INDEX_HIGH - INDEX_LOW + 1
}

/*
  Internal connections for the instruction cache
 */
class ICacheCtrlIO extends Bundle() {
  val ena_in = Input(Bool())
  val fetchEna = Output(Bool())
  val ctrlrepl = Output(new ICacheCtrlRepl())
  val replctrl = Input(new ICacheReplCtrl())
  val feicache = Input(new FeICache())
  val exicache = Input(new ExICache())
  val ocp_port = new OcpBurstMasterPort(ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH)
  val perf = new InstructionCachePerf()
  val illMem = Output(Bool())
}
class ICacheCtrlRepl extends Bundle() {
  val wEna = Bool()
  val wData = UInt(INSTR_WIDTH.W)
  val wAddr = UInt(ADDR_WIDTH.W)
  val wTag = Bool()
}
class ICacheReplCtrl extends Bundle() {
  val hit = Bool()
  val fetchAddr = UInt(ADDR_WIDTH.W)
  val selCache = Bool()
}
class ICacheReplIO extends Bundle() {
  val ena_in = Input(Bool())
  val invalidate = Input(Bool())
  val exicache = Input(new ExICache())
  val feicache = Input(new FeICache())
  val icachefe = Output(new ICacheFe())
  val ctrlrepl = Input(new ICacheCtrlRepl())
  val replctrl = Output(new ICacheReplCtrl())
  val memIn = Output(new ICacheMemIn())
  val memOut = Input(new ICacheMemOut())
}

class ICacheMemIn extends Bundle() {
  val wEven = Bool()
  val wOdd = Bool()
  val wData = UInt(DATA_WIDTH.W)
  val wAddr = UInt((INDEX_SIZE + LINE_WORD_SIZE_WIDTH).W)
  val addrOdd = UInt((INDEX_SIZE + LINE_WORD_SIZE_WIDTH).W)
  val addrEven = UInt((INDEX_SIZE + LINE_WORD_SIZE_WIDTH).W)
}
class ICacheMemOut extends Bundle() {
  val instrEven = UInt(INSTR_WIDTH.W)
  val instrOdd = UInt(INSTR_WIDTH.W)
}
class ICacheMemIO extends Bundle() {
  val memIn = Input(new ICacheMemIn())
  val memOut = Output(new ICacheMemOut())
}


/*
 ICache: Top Level Class for the Instruction Cache
 */
class ICache() extends CacheType {
  // Generate submodules of instruction cache
  val ctrl = Module(new ICacheCtrl())
  val repl = Module(new ICacheReplDm())
  val mem = Module(new ICacheMem())
  // Connect control unit
  ctrl.io.ctrlrepl <> repl.io.ctrlrepl
  ctrl.io.feicache <> io.feicache
  ctrl.io.exicache <> io.exicache
  ctrl.io.ocp_port <> io.ocp_port
  ctrl.io.perf <> io.perf
  // Connect replacement unit
  repl.io.exicache <> io.exicache
  repl.io.feicache <> io.feicache
  repl.io.icachefe <> io.icachefe
  repl.io.replctrl <> ctrl.io.replctrl
  // Connect replacement unit to on-chip memory
  repl.io.memIn <> mem.io.memIn
  repl.io.memOut <> mem.io.memOut
  // Connect enable signal
  ctrl.io.ena_in <> io.ena_in
  repl.io.ena_in := io.ena_in && ctrl.io.fetchEna
  // Output enable depending on hit/miss/fetch
  io.ena_out := ctrl.io.fetchEna
  // Connect illegal access signal
  io.illMem := ctrl.io.illMem
  // Connect invalidate signal
  repl.io.invalidate := io.invalidate
}

/*
 ICacheMem Class: On-Chip Instruction Cache Memory
 */
class ICacheMem extends Module {
  val io = IO(new ICacheMemIO())

  val icacheEven = MemBlock(ICACHE_WORD_SIZE / 2, INSTR_WIDTH)
  val icacheOdd = MemBlock(ICACHE_WORD_SIZE / 2, INSTR_WIDTH)

  icacheEven.io <= (io.memIn.wEven, io.memIn.wAddr, io.memIn.wData)
  icacheOdd.io <= (io.memIn.wOdd, io.memIn.wAddr, io.memIn.wData)

  io.memOut.instrEven := icacheEven.io(io.memIn.addrEven)
  io.memOut.instrOdd := icacheOdd.io(io.memIn.addrOdd)
}

/*
 Direct-Mapped Replacement Class
 */
class ICacheReplDm() extends Module {
  val io = IO(new ICacheReplIO())

  // Tag memory and vector for valid bits
  val tagMemEven = MemBlock(LINE_COUNT / 2, TAG_SIZE)
  val tagMemOdd = MemBlock(LINE_COUNT / 2, TAG_SIZE)
  val validVec = RegInit(VecInit(Seq.fill(LINE_COUNT)(false.B)))

  // Variables for call/return
  val callRetBaseReg = RegInit(1.U(DATA_WIDTH.W))
  val callRetBaseNext = dontTouch(Wire(UInt(DATA_WIDTH.W))) // for emulator
  val callAddrReg = RegInit(1.U(DATA_WIDTH.W))
  val selSpmReg = RegInit(false.B)
  val selSpmNext = dontTouch(Wire(Bool())) //for emulator
  val selCacheReg = RegInit(false.B)
  val selCacheNext = dontTouch(Wire(Bool())) //for emulator

  val fetchAddr = Wire(UInt(ADDR_WIDTH.W))
  val hitEven = Wire(Bool())
  val hitOdd = Wire(Bool())

  callRetBaseNext := callRetBaseReg
  callRetBaseReg := callRetBaseNext
  selSpmNext := selSpmReg
  selSpmReg := selSpmNext
  selCacheNext := selCacheReg
  selCacheReg := selCacheNext

  val relBase = Mux(selCacheReg,
                    callRetBaseReg(ADDR_WIDTH-1, 0),
                    callRetBaseReg(ISPM_ONE_BIT-3, 0))
  val relPc = callAddrReg + relBase

  val reloc = Mux(selCacheReg,
                  0.U,
                  Mux(selSpmReg,
                      (1 << (ISPM_ONE_BIT - 2)).U,
                      0.U))

  when (io.exicache.doCallRet && io.ena_in) {
    callRetBaseNext := io.exicache.callRetBase
    callAddrReg := io.exicache.callRetAddr
    selSpmNext := io.exicache.callRetBase(ADDR_WIDTH-1, ISPM_ONE_BIT-2) === 0x1.U
    selCacheNext := io.exicache.callRetBase(ADDR_WIDTH-1, ISPM_ONE_BIT-1) >= 0x1.U
  }

  // Register addresses
  val addrEvenReg = RegNext(io.feicache.addrEven)
  val addrOddReg = RegNext(io.feicache.addrOdd)

  // Addresses for tag memory
  val indexEven = io.feicache.addrEven(INDEX_HIGH, INDEX_LOW+1)
  val indexOdd = io.feicache.addrOdd(INDEX_HIGH, INDEX_LOW+1)
  val parityEven = io.feicache.addrEven(INDEX_LOW)
  val tagAddrEven = Mux(parityEven, indexOdd, indexEven)
  val tagAddrOdd = Mux(parityEven, indexEven, indexOdd)

  // Read from tag memory
  val toutEven = tagMemEven.io(tagAddrEven)
  val toutOdd = tagMemOdd.io(tagAddrOdd)
  // Multiplex tag memory output
  val tagEven = Mux(addrEvenReg(INDEX_LOW), toutOdd, toutEven)
  val tagOdd = Mux(addrOddReg(INDEX_LOW), toutOdd, toutEven)

  // Check if line is valid
  val validEven = validVec(addrEvenReg(INDEX_HIGH, INDEX_LOW))
  val validOdd = validVec(addrOddReg(INDEX_HIGH, INDEX_LOW))
  val valid = validEven && validOdd

  // Check for a hit of both instructions in the address bundle
  hitEven := true.B
  hitOdd := true.B
  when ((tagEven =/= addrEvenReg(TAG_HIGH, TAG_LOW)) || (!validEven)) {
    hitEven := false.B
  }
  fetchAddr := addrEvenReg
  when ((tagOdd =/= addrOddReg(TAG_HIGH, TAG_LOW)) || (!validOdd)) {
    hitOdd := false.B
    fetchAddr := addrOddReg
  }
  // Keep signals alive for emulator
  //debug(hitEven) does nothing in chisel3 (no proning in frontend of chisel3 anyway)
  //debug(hitOdd)

  val wrAddrTag = io.ctrlrepl.wAddr(TAG_HIGH,TAG_LOW)
  // Index for vector of valid bits
  val wrValidIndex = io.ctrlrepl.wAddr(INDEX_HIGH, INDEX_LOW)
  // Index for tag memory even/odd
  val wrAddrIndex = io.ctrlrepl.wAddr(INDEX_HIGH, INDEX_LOW+1)
  val wrAddrParity = io.ctrlrepl.wAddr(INDEX_LOW)
  // Update tag field when new write occurs
  tagMemEven.io <= (io.ctrlrepl.wTag && !wrAddrParity, wrAddrIndex, wrAddrTag)
  tagMemOdd.io <= (io.ctrlrepl.wTag && wrAddrParity, wrAddrIndex, wrAddrTag)
  when (io.ctrlrepl.wTag) {
    validVec(wrValidIndex) := true.B
  }

  val wrParity = io.ctrlrepl.wAddr(0)

  // Outputs to cache memory
  io.memIn.wEven := Mux(wrParity, false.B, io.ctrlrepl.wEna)
  io.memIn.wOdd := Mux(wrParity, io.ctrlrepl.wEna, false.B)
  io.memIn.wData := io.ctrlrepl.wData
  io.memIn.wAddr := io.ctrlrepl.wAddr(INDEX_HIGH,1)
  io.memIn.addrOdd := io.feicache.addrOdd(INDEX_HIGH,1)
  io.memIn.addrEven := io.feicache.addrEven(INDEX_HIGH,1)

  // Outputs to fetch stage
  io.icachefe.instrEven := io.memOut.instrEven
  io.icachefe.instrOdd := io.memOut.instrOdd

  io.icachefe.base := callRetBaseReg
  io.icachefe.relBase := relBase
  io.icachefe.relPc := relPc
  io.icachefe.reloc := reloc
  io.icachefe.memSel := Cat(selSpmReg, selCacheReg)

  // Hit/miss to control module
  io.replctrl.fetchAddr := fetchAddr
  io.replctrl.hit := hitEven && hitOdd
  io.replctrl.selCache := selCacheReg

  when (io.invalidate) {
    validVec.map(_ := false.B)
  }
}

/*
 Instruction Cache Control Class: handles block transfer from external Memory to the I-Cache
 */
class ICacheCtrl() extends Module {
  val io = IO(new ICacheCtrlIO())

  // States of the state machine
  val initState :: idleState :: transferState :: waitState :: errorState :: Nil = Enum(5)
  val stateReg = RegInit(initState)
  // Signal for replacement unit
  val wData = Wire(UInt(DATA_WIDTH.W))
  val wTag = Wire(Bool())
  val wAddr = Wire(UInt(ADDR_WIDTH.W))
  val wEna = Wire(Bool())
  // Signals for external memory
  val ocpCmd = Wire(UInt(3.W))
  val ocpAddr = Wire(UInt(ADDR_WIDTH.W))
  val fetchCntReg = RegInit(0.U(ICACHE_SIZE_WIDTH.W))
  val burstCntReg = RegInit(0.U(log2Up(BURST_LENGTH).W))
  val fetchEna = Wire(Bool())
  // Input/output registers
  val addrReg = RegInit(0.U((ADDR_WIDTH - LINE_WORD_SIZE_WIDTH).W))
  val ocpSlaveReg = RegNext(io.ocp_port.S)

  // Initialize signals
  wData := 0.U
  wTag := false.B
  wEna := false.B
  wAddr := 0.U
  ocpCmd := OcpCmd.IDLE
  ocpAddr := 0.U
  fetchEna := true.B

  // Wait till ICache is the selected source of instructions
  when (stateReg === initState) {
    when (io.replctrl.selCache) {
      stateReg := idleState
    }
  }
  when (stateReg === idleState) {
    when (!io.replctrl.selCache) {
      stateReg := initState
    } .otherwise {
      when (!io.replctrl.hit) {
        fetchEna := false.B
        val addr = io.replctrl.fetchAddr(ADDR_WIDTH-1, LINE_WORD_SIZE_WIDTH)
        addrReg := addr
        burstCntReg := 0.U
        fetchCntReg := 0.U
        // Write new tag field memory
        wTag := true.B
        wAddr := Cat(addr, 0.U(LINE_WORD_SIZE_WIDTH.W))
        // Check if command is accepted by the memory controller
        ocpAddr := Cat(addr, 0.U(LINE_WORD_SIZE_WIDTH.W))
        ocpCmd := OcpCmd.RD
        when (io.ocp_port.S.CmdAccept === 1.U) {
          stateReg := transferState
        } .otherwise {
          stateReg := waitState
        }
      }
    }
  }
  when (stateReg === waitState) {
    fetchEna := false.B
    ocpAddr := Cat(addrReg, 0.U(LINE_WORD_SIZE_WIDTH.W))
    ocpCmd := OcpCmd.RD
    when (io.ocp_port.S.CmdAccept === 1.U) {
      stateReg := transferState
    }
  }
  // Transfer/fetch cache block
  when (stateReg === transferState) {
    fetchEna := false.B
    when (fetchCntReg < LINE_WORD_SIZE.U) {
      when (ocpSlaveReg.Resp === OcpResp.DVA) {
        fetchCntReg := fetchCntReg + 1.U
        burstCntReg := burstCntReg + 1.U
        when(fetchCntReg < (LINE_WORD_SIZE-1).U) {
          // Fetch next address from external memory
          when (burstCntReg >= (BURST_LENGTH - 1).U) {
            ocpCmd := OcpCmd.RD
            ocpAddr := Cat(addrReg, 0.U(LINE_WORD_SIZE_WIDTH.W)) + fetchCntReg + 1.U
            burstCntReg := 0.U
          }
        }
        // Write current address to icache memory
        wData := ocpSlaveReg.Data
        wEna := true.B
      }
      wAddr := Cat(addrReg, 0.U(LINE_WORD_SIZE_WIDTH.W)) + fetchCntReg
    }
    // Restart to idle state
    .otherwise {
      stateReg := idleState
    }
  }

  // abort on error response
  io.illMem := false.B
  when (ocpSlaveReg.Resp === OcpResp.ERR) {
    burstCntReg := burstCntReg + 1.U
    stateReg := errorState
  }
  // wait for end of burst before signalling error
  when (stateReg === errorState) {
    fetchEna := false.B
    when (ocpSlaveReg.Resp =/= OcpResp.NULL) {
      burstCntReg := burstCntReg + 1.U
    }
    when (burstCntReg === (BURST_LENGTH - 1).U) {
      io.illMem := true.B
      stateReg := idleState
    }
  }

  // Outputs to cache memory
  io.ctrlrepl.wEna := wEna
  io.ctrlrepl.wData := wData
  io.ctrlrepl.wAddr := wAddr
  io.ctrlrepl.wTag := wTag

  io.fetchEna := fetchEna

  // Outputs to external memory
  io.ocp_port.M.Addr := Cat(ocpAddr, 0.U(2.W))
  io.ocp_port.M.Cmd := ocpCmd
  io.ocp_port.M.Data := 0.U //read-only
  io.ocp_port.M.DataByteEn := "b1111".U(4.W) //read-only
  io.ocp_port.M.DataValid := 0.U //read-only

  // Output to performance counters
  io.perf.hit := false.B
  io.perf.miss := false.B
  when (io.ena_in && io.replctrl.selCache && stateReg === idleState) {
    when (io.replctrl.hit) {
      io.perf.hit := true.B
    } .otherwise {
      io.perf.miss := true.B
    }
  }
}
