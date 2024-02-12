/*
  Instruction Cache for Patmos
  Authors: Philipp Degasperi (philipp.degasperi@gmail.com)
           Wolfgang Puffitsch (wpuffitsch@gmail.com)
 */

package patmos

import Chisel._
import chisel3.VecInit
import chisel3.dontTouch
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
  val ctrlrepl = new ICacheCtrlRepl().asOutput
  val replctrl = new ICacheReplCtrl().asInput
  val feicache = new FeICache().asInput
  val exicache = new ExICache().asInput
  val ocp_port = new OcpBurstMasterPort(ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH)
  val perf = new InstructionCachePerf()
  val illMem = Output(Bool())
}
class ICacheCtrlRepl extends Bundle() {
  val wEna = Bool()
  val wData = UInt(width = INSTR_WIDTH)
  val wAddr = UInt(width = ADDR_WIDTH)
  val wTag = Bool()
}
class ICacheReplCtrl extends Bundle() {
  val hit = Bool()
  val fetchAddr = UInt(width = ADDR_WIDTH)
  val selCache = Bool()
}
class ICacheReplIO extends Bundle() {
  val ena_in = Input(Bool())
  val invalidate = Input(Bool())
  val exicache = new ExICache().asInput
  val feicache = new FeICache().asInput
  val icachefe = new ICacheFe().asOutput
  val ctrlrepl = new ICacheCtrlRepl().asInput
  val replctrl = new ICacheReplCtrl().asOutput
  val memIn = new ICacheMemIn().asOutput
  val memOut = new ICacheMemOut().asInput
}

class ICacheMemIn extends Bundle() {
  val wEven = Bool()
  val wOdd = Bool()
  val wData = UInt(width = DATA_WIDTH)
  val wAddr = UInt(width = INDEX_SIZE + LINE_WORD_SIZE_WIDTH)
  val addrOdd = UInt(width = INDEX_SIZE + LINE_WORD_SIZE_WIDTH)
  val addrEven = UInt(width = INDEX_SIZE + LINE_WORD_SIZE_WIDTH)
}
class ICacheMemOut extends Bundle() {
  val instrEven = UInt(width = INSTR_WIDTH)
  val instrOdd = UInt(width = INSTR_WIDTH)
}
class ICacheMemIO extends Bundle() {
  val memIn = new ICacheMemIn().asInput
  val memOut = new ICacheMemOut().asOutput
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
  val callRetBaseReg = RegInit(UInt(1, DATA_WIDTH))
  val callRetBaseNext = dontTouch(Wire(UInt(DATA_WIDTH.W))) // for emulator
  val callAddrReg = RegInit(UInt(1, DATA_WIDTH))
  val selSpmReg = RegInit(Bool(false))
  val selSpmNext = dontTouch(Wire(Bool())) //for emulator
  val selCacheReg = RegInit(Bool(false))
  val selCacheNext = dontTouch(Wire(Bool())) //for emulator

  val fetchAddr = Wire(UInt(width = ADDR_WIDTH))
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
                  UInt(0),
                  Mux(selSpmReg,
                      UInt(1 << (ISPM_ONE_BIT - 2)),
                      UInt(0)))

  when (io.exicache.doCallRet && io.ena_in) {
    callRetBaseNext := io.exicache.callRetBase
    callAddrReg := io.exicache.callRetAddr
    selSpmNext := io.exicache.callRetBase(ADDR_WIDTH-1, ISPM_ONE_BIT-2) === UInt(0x1)
    selCacheNext := io.exicache.callRetBase(ADDR_WIDTH-1, ISPM_ONE_BIT-1) >= UInt(0x1)
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
  hitEven := Bool(true)
  hitOdd := Bool(true)
  when ((tagEven =/= addrEvenReg(TAG_HIGH, TAG_LOW)) || (!validEven)) {
    hitEven := Bool(false)
  }
  fetchAddr := addrEvenReg
  when ((tagOdd =/= addrOddReg(TAG_HIGH, TAG_LOW)) || (!validOdd)) {
    hitOdd := Bool(false)
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
    validVec(wrValidIndex) := Bool(true)
  }

  val wrParity = io.ctrlrepl.wAddr(0)

  // Outputs to cache memory
  io.memIn.wEven := Mux(wrParity, Bool(false), io.ctrlrepl.wEna)
  io.memIn.wOdd := Mux(wrParity, io.ctrlrepl.wEna, Bool(false))
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
    validVec.map(_ := Bool(false))
  }
}

/*
 Instruction Cache Control Class: handles block transfer from external Memory to the I-Cache
 */
class ICacheCtrl() extends Module {
  val io = IO(new ICacheCtrlIO())

  // States of the state machine
  val initState :: idleState :: transferState :: waitState :: errorState :: Nil = Enum(UInt(), 5)
  val stateReg = RegInit(initState)
  // Signal for replacement unit
  val wData = Wire(UInt(width = DATA_WIDTH))
  val wTag = Wire(Bool())
  val wAddr = Wire(UInt(width = ADDR_WIDTH))
  val wEna = Wire(Bool())
  // Signals for external memory
  val ocpCmd = Wire(UInt(width = 3))
  val ocpAddr = Wire(UInt(width = ADDR_WIDTH))
  val fetchCntReg = RegInit(UInt(0, width = ICACHE_SIZE_WIDTH))
  val burstCntReg = RegInit(UInt(0, width = log2Up(BURST_LENGTH)))
  val fetchEna = Wire(Bool())
  // Input/output registers
  val addrReg = RegInit(UInt(0, width = ADDR_WIDTH - LINE_WORD_SIZE_WIDTH))
  val ocpSlaveReg = RegNext(io.ocp_port.S)

  // Initialize signals
  wData := UInt(0)
  wTag := Bool(false)
  wEna := Bool(false)
  wAddr := UInt(0)
  ocpCmd := OcpCmd.IDLE
  ocpAddr := UInt(0)
  fetchEna := Bool(true)

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
        fetchEna := Bool(false)
        val addr = io.replctrl.fetchAddr(ADDR_WIDTH-1, LINE_WORD_SIZE_WIDTH)
        addrReg := addr
        burstCntReg := UInt(0)
        fetchCntReg := UInt(0)
        // Write new tag field memory
        wTag := Bool(true)
        wAddr := Cat(addr, UInt(0, width = LINE_WORD_SIZE_WIDTH))
        // Check if command is accepted by the memory controller
        ocpAddr := Cat(addr, UInt(0, width =  LINE_WORD_SIZE_WIDTH))
        ocpCmd := OcpCmd.RD
        when (io.ocp_port.S.CmdAccept === UInt(1)) {
          stateReg := transferState
        } .otherwise {
          stateReg := waitState
        }
      }
    }
  }
  when (stateReg === waitState) {
    fetchEna := Bool(false)
    ocpAddr := Cat(addrReg, UInt(0, width = LINE_WORD_SIZE_WIDTH))
    ocpCmd := OcpCmd.RD
    when (io.ocp_port.S.CmdAccept === UInt(1)) {
      stateReg := transferState
    }
  }
  // Transfer/fetch cache block
  when (stateReg === transferState) {
    fetchEna := Bool(false)
    when (fetchCntReg < UInt(LINE_WORD_SIZE)) {
      when (ocpSlaveReg.Resp === OcpResp.DVA) {
        fetchCntReg := fetchCntReg + UInt(1)
        burstCntReg := burstCntReg + UInt(1)
        when(fetchCntReg < UInt(LINE_WORD_SIZE-1)) {
          // Fetch next address from external memory
          when (burstCntReg >= UInt(BURST_LENGTH - 1)) {
            ocpCmd := OcpCmd.RD
            ocpAddr := Cat(addrReg, UInt(0, width = LINE_WORD_SIZE_WIDTH)) + fetchCntReg + UInt(1)
            burstCntReg := UInt(0)
          }
        }
        // Write current address to icache memory
        wData := ocpSlaveReg.Data
        wEna := Bool(true)
      }
      wAddr := Cat(addrReg, UInt(0, width = LINE_WORD_SIZE_WIDTH)) + fetchCntReg
    }
    // Restart to idle state
    .otherwise {
      stateReg := idleState
    }
  }

  // abort on error response
  io.illMem := Bool(false)
  when (ocpSlaveReg.Resp === OcpResp.ERR) {
    burstCntReg := burstCntReg + UInt(1)
    stateReg := errorState
  }
  // wait for end of burst before signalling error
  when (stateReg === errorState) {
    fetchEna := Bool(false)
    when (ocpSlaveReg.Resp =/= OcpResp.NULL) {
      burstCntReg := burstCntReg + UInt(1)
    }
    when (burstCntReg === UInt(BURST_LENGTH - 1)) {
      io.illMem := Bool(true)
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
  io.ocp_port.M.Data := UInt(0) //read-only
  io.ocp_port.M.DataByteEn := "b1111".U(4.W) //read-only
  io.ocp_port.M.DataValid := UInt(0) //read-only

  // Output to performance counters
  io.perf.hit := Bool(false)
  io.perf.miss := Bool(false)
  when (io.ena_in && io.replctrl.selCache && stateReg === idleState) {
    when (io.replctrl.hit) {
      io.perf.hit := Bool(true)
    } .otherwise {
      io.perf.miss := Bool(true)
    }
  }
}
