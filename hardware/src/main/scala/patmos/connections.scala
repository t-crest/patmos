/*
 * Connection definitions for the pipe stages.
 *
 * Authors: Martin Schoeberl (martin@jopdesign.com)
 *          Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package patmos

import Chisel._
import chisel3.VecInit

import Constants._

import ocp._

class FeDec() extends Bundle() {
  val instr_a = UInt(INSTR_WIDTH.W)
  val instr_b = UInt(INSTR_WIDTH.W)
  val pc = UInt(PC_SIZE.W)
  val base = UInt(PC_SIZE.W)
  val reloc = UInt(ADDR_WIDTH.W)
  val relPc = UInt(PC_SIZE.W)

  def flush() = {
    // flush only necessary parts of instruction
    // instr_a(30, 27) := PRED_IFFALSE
    // instr_a(26, 25) := OPCODE_ALUI
    // instr_b(30, 27) := PRED_IFFALSE
    // instr_b(26, 25) := OPCODE_ALUI
    instr_a := 0.U
    instr_b := 0.U
  }
}

class AluOp() extends Bundle() {
  val func = UInt(4.W)
  val isMul = Bool()
  val isCmp = Bool()
  val isPred = Bool()
  val isBCpy = Bool()
  val isMTS = Bool()
  val isMFS = Bool()

  def defaults() = {
    func := 0.U
    isMul := false.B
    isCmp := false.B
    isPred := false.B
    isBCpy := false.B
    isMTS := false.B
    isMFS := false.B
  }
}

class PredOp() extends Bundle() {
  val func = UInt(2.W) // as they have a strange encoding
  val dest = UInt(PRED_BITS.W)
  val s1Addr = UInt((PRED_BITS+1).W)
  val s2Addr = UInt((PRED_BITS+1).W)

  def defaults() = {
    func := 0.U
    dest := 0.U
    s1Addr := 0.U
    s2Addr := 0.U
  }
}

class JmpOp() extends Bundle() {
  val branch = Bool()
  val target = UInt(PC_SIZE.W)
  val reloc = UInt(ADDR_WIDTH.W)

  def defaults() = {
    branch := false.B
    target := 0.U
    reloc := 0.U
  }
}

class MemOp() extends Bundle() {
  val load = Bool()
  val store = Bool()
  val hword = Bool()
  val byte = Bool()
  val zext = Bool()
  val typ  = UInt(2.W)

  def defaults() = {
    load := false.B
    store := false.B
    hword := false.B
    byte := false.B
    zext := false.B
    typ := 0.U
  }
}

class CopOp() extends Bundle() {
  val isCop = Bool()
  val isCustom = Bool()
  val rsAddrCop = Vec(2, Bool())
  val copId = UInt(COP_ID_WIDTH.W)
  val funcId = UInt(COP_FUNCID_WIDTH.W)

  def defaults() = {
    isCop := false.B
    isCustom := false.B
    rsAddrCop := Vec.fill(2) { false.B }
    copId := 0.U
    funcId := 0.U
  }
}

class DecEx() extends Bundle() {
  val pc = UInt(PC_SIZE.W)
  val base = UInt(PC_SIZE.W)
  val relPc = UInt(PC_SIZE.W)
  val pred = Vec(PIPE_COUNT, UInt((PRED_BITS+1).W) )
  val aluOp = Vec(PIPE_COUNT, new AluOp() )
  val predOp = Vec(PIPE_COUNT, new PredOp() )
  val jmpOp = new JmpOp()
  val memOp = new MemOp()
  val stackOp = UInt(SC_OP_BITS.W)
  val copOp = new CopOp()

  // the register fields are very similar to RegFileRead
  // maybe join the structures
  val rsAddr = Vec(2*PIPE_COUNT, UInt(REG_BITS.W) )
  val rsData = Vec(2*PIPE_COUNT, UInt(DATA_WIDTH.W) )
  val rdAddr = Vec(PIPE_COUNT, UInt(REG_BITS.W) )
  val immVal = Vec(PIPE_COUNT, UInt(DATA_WIDTH.W) )
  val immOp  = Vec(PIPE_COUNT, Bool() )
  // maybe we should have similar structure as the Result one here
  val wrRd  = Vec(PIPE_COUNT, Bool() )

  val callAddr = UInt(DATA_WIDTH.W)
  val call = Bool()
  val ret = Bool()
  val brcf = Bool()
  val trap = Bool()
  val xcall = Bool()
  val xret = Bool()
  val xsrc = UInt(EXC_SRC_BITS.W)
  val nonDelayed = Bool()

  val illOp = Bool()

  def flush() = {
    pred := Vec.fill(PIPE_COUNT) { PRED_IFFALSE }
    illOp := false.B
    copOp.isCop := false.B
  }

  def defaults() = {
    pc := 0.U
    relPc := 0.U
    pred := Vec.fill(PIPE_COUNT) { PRED_IFFALSE }
    aluOp.map(_.defaults())
    predOp.map(_.defaults())
    jmpOp.defaults()
    memOp.defaults()
    stackOp := sc_OP_NONE
    copOp.defaults()
    // rsAddr := Vec.fill(2*PIPE_COUNT) { 0.U }
    rsAddr := VecInit(Seq.fill(2*PIPE_COUNT)(0.U))
    rsData := Vec.fill(2*PIPE_COUNT) { 0.U }
    rdAddr := Vec.fill(PIPE_COUNT) { 0.U }
    immVal := Vec.fill(PIPE_COUNT) { 0.U }
    immOp := Vec.fill(PIPE_COUNT) { false.B }
    wrRd := Vec.fill(PIPE_COUNT) { false.B }
    callAddr := 0.U
    call := false.B
    ret := false.B
    brcf := false.B
    trap := false.B
    xcall := false.B
    xret := false.B
    xsrc := 0.U
    nonDelayed := false.B
    illOp := false.B
  }
}

class Result() extends Bundle() {
  val addr = UInt(REG_BITS.W)
  val data = UInt(DATA_WIDTH.W)
  val valid = Bool()

  def flush() = {
    valid := false.B
  }
}

class MemIn() extends Bundle() {
  val load = Bool()
  val store = Bool()
  val hword = Bool()
  val byte = Bool()
  val zext = Bool()
  val typ = UInt(2.W)
  val addr = UInt(DATA_WIDTH.W)
  val data = UInt(DATA_WIDTH.W)
  val call = Bool()
  val ret = Bool()
  val brcf = Bool()
  val trap = Bool()
  val xcall = Bool()
  val xret = Bool()
  val xsrc = UInt(EXC_SRC_BITS.W)
  val illOp = Bool()
  val callRetAddr = UInt(DATA_WIDTH.W)
  val callRetBase = UInt(DATA_WIDTH.W)
  val nonDelayed = Bool()

  def flush() = {
    load := false.B
    store := false.B
    call := false.B
    ret := false.B
    brcf := false.B
    trap := false.B
    xcall := false.B
    xret := false.B
    illOp := false.B
  }
}

// interface between the EX stage and the stack cache
class ExSc extends Bundle() {
  // indicate which stack-cache operation is performed
  val op = UInt(3.W)

  // operands of the stack-cache operation
  //   - opSetStackTop, opSetMemTop: the new value of stackTop or memTop
  val opData = UInt(DATA_WIDTH.W)
  //   - opSRES, opSENS, opSFREE   : the operand of the instructions
  val opOff  = UInt(ADDR_WIDTH.W)
}

class ScEx extends Bundle() {
  // the current value of the stack top pointer
  val stackTop = UInt(ADDR_WIDTH.W)
  
  // the current value of the mem top pointer
  val memTop = UInt(ADDR_WIDTH.W)
}

class ExMem() extends Bundle() {
  val rd = Vec(PIPE_COUNT, new Result() )
  val mem = new MemIn()
  val pc = UInt(PC_SIZE.W)
  val base = UInt(PC_SIZE.W)
  val relPc = UInt(PC_SIZE.W)

  def flush() = {
    rd.map(_.flush())
    mem.flush()
  }
}

class ExFe() extends Bundle() {
  val doBranch = Bool()
  val branchPc = UInt(PC_SIZE.W)
}

class MemFe() extends Bundle() {
  val doCallRet = Bool()
  val callRetPc = UInt(PC_SIZE.W)
  val callRetBase = UInt(PC_SIZE.W)
  // for ISPM write
  val store = Bool()
  val addr = UInt(DATA_WIDTH.W)
  val data = UInt(DATA_WIDTH.W)
}

class FeEx() extends Bundle() {
  val pc = UInt(PC_SIZE.W)
}

class MemWb() extends Bundle() {
  val rd = Vec(PIPE_COUNT, new Result() )
  // PC value for debugging
  val pc = UInt(PC_SIZE.W)
}

class RegFileRead() extends Bundle() {
  // first two are for pipeline A, second two for pipeline B
  val rsAddr = Vec(2*PIPE_COUNT, UInt(INPUT, REG_BITS) )
  val rsData = Vec(2*PIPE_COUNT, UInt(OUTPUT, DATA_WIDTH) )
}

class RegFileIO() extends Bundle() {
  val ena = Input(Bool())
  val rfRead = new RegFileRead()
  val rfWrite = Vec(PIPE_COUNT, Input(new Result()) )
}

class FetchIO extends Bundle() {
  val ena = Input(Bool())
  val fedec = Output(new FeDec())
  // PC for returns
  val feex = Output(new FeEx())
  // branch from EX
  val exfe = Input(new ExFe())
  // call from MEM
  val memfe = Input(new MemFe())
  // connections to instruction cache
  val feicache = Output(new FeICache())
  val icachefe = Input(new ICacheFe())
}

class ExcDec() extends Bundle() {
  val exc = Bool()
  val excBase = UInt(PC_SIZE.W)
  val excAddr = UInt(PC_SIZE.W)
  val intr = Bool()
  val addr = UInt(ADDR_WIDTH.W)
  val src = UInt(EXC_SRC_BITS.W)
  val local = Bool()
}

class DecodeIO() extends Bundle() {
  val ena = Input(Bool())
  val flush = Input(Bool())
  val fedec = Input(new FeDec())
  val decex = Output(new DecEx())
  val rfWrite =  Vec(PIPE_COUNT, Input(new Result()))
  val exc = new ExcDec().asInput
}

class ExecuteIO() extends Bundle() {
  val ena_in = Input(Bool())
  val ena_out = Output(Bool())
  val flush = Input(Bool())
  val brflush = Output(Bool())
  val decex = Input(new DecEx())
  val exmem = Output(new ExMem())
  val exicache = Output(new ExICache())
  val feex = Input(new FeEx())
  // forwarding inputs
  val exResult = Vec(PIPE_COUNT, Input(new Result()))
  val memResult = Vec(PIPE_COUNT, Input(new Result()))
  // branch for FE
  val exfe = Output(new ExFe())
  //stack cache
  val exsc = Output(new ExSc())
  val scex = Input(new ScEx())
  // coprocessor
  val copOut = Vec(COP_COUNT, Output(new PatmosToCoprocessor()))
  val copIn = Vec(COP_COUNT, Input(new CoprocessorToPatmos()))
}

class BootMemIO() extends Bundle() {
  val memInOut = new OcpCacheSlavePort(ADDR_WIDTH, DATA_WIDTH)
  val extMem = new OcpCacheMasterPort(ADDR_WIDTH, DATA_WIDTH)
}

class MemExc() extends Bundle() {
  val call = Bool()
  val ret = Bool()
  val src = UInt(EXC_SRC_BITS.W)

  val exc = Bool()
  val excBase = UInt(PC_SIZE.W)
  val excAddr = UInt(PC_SIZE.W)
}

class MemoryIO() extends Bundle() {
  val ena_out = Output(Bool())
  val ena_in = Input(Bool())
  val flush = Output(Bool())
  val exmem = Input(new ExMem())
  val memwb = Output(new MemWb())
  val memfe = Output(new MemFe())
  // for result forwarding
  val exResult = Vec(PIPE_COUNT, Output(new Result()))
  // local and global accesses
  val localInOut = new OcpCoreMasterPort(ADDR_WIDTH, DATA_WIDTH)
  val globalInOut = new OcpCacheMasterPort(ADDR_WIDTH, DATA_WIDTH)
  // exceptions
  val icacheIllMem = Input(Bool())
  val scacheIllMem = Input(Bool())
  val exc = Output(new MemExc())
}

class PatmosToCoprocessor() extends Bundle()
{
  val ena_in  = Bool()
  val trigger = Bool()
  val isCustom = Bool() // custom-instruction
  val read = Bool()     // read
                        // write if neither custom-instruction nor read
  val funcId = UInt(COP_FUNCID_WIDTH.W)
  val opAddr = Vec(2, UInt(REG_BITS.W))
  val opData = Vec(2, UInt(DATA_WIDTH.W))
  val opAddrCop = Vec(2, Bool())
  
  def defaults() = {
    ena_in := false.B
    trigger := false.B
    isCustom := false.B
    read := false.B
    funcId := 0.U
    opAddr := Vec.fill(2) { 0.U }
    opData := Vec.fill(2) { 0.U }
    opAddrCop := Vec.fill(2) { false.B }
  }
}

class CoprocessorToPatmos() extends Bundle()
{
  val ena_out = Bool()
  val result  = UInt(DATA_WIDTH.W)
}

class CoprocessorIO() extends Bundle()
{
  val patmosCop = Output(new PatmosToCoprocessor())
  val copPatmos = Input(new CoprocessorToPatmos())
}

//stack cache
class StackCacheIO() extends Bundle() {
  // check if another transfer is active
  val ena_in = Input(Bool())
  // signals from EX stage to stack cache
  val exsc = Input(new ExSc())
  // signals from stack cache back to the EX stage
  val scex = Output(new ScEx())
  // signal an illegal memory access
  val illMem = Output(Bool())
  // indicate a stall
  val stall = Output(Bool())
}

// method/instruction cache connections
class FeICache extends Bundle() {
  val addrEven = UInt(ADDR_WIDTH.W)
  val addrOdd = UInt(ADDR_WIDTH.W)
}
class ExICache() extends Bundle() {
  val doCallRet = Bool()
  val callRetBase = UInt(ADDR_WIDTH.W)
  val callRetAddr = UInt(ADDR_WIDTH.W)
}
class ICacheFe extends Bundle() {
  val instrEven = UInt(INSTR_WIDTH.W)
  val instrOdd = UInt(INSTR_WIDTH.W)
  // absolute basse address
  val base = UInt(ADDR_WIDTH.W)
  // relative base address
  val relBase = UInt(MAX_OFF_WIDTH.W)
  // relative program counter
  val relPc = UInt((MAX_OFF_WIDTH+1).W)
  // offset between relative and absolute program counter
  val reloc = UInt(DATA_WIDTH.W)
  val memSel = UInt(2.W)
}
class ICacheIO extends Bundle() {
  val ena_out = Output(Bool())
  val ena_in = Input(Bool())
  val invalidate = Input(Bool())
  val feicache = Input(new FeICache())
  val exicache = Input(new ExICache())
  val icachefe = Output(new ICacheFe())
  val ocp_port = new OcpBurstMasterPort(ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH)
  val illMem = Output(Bool())
  val perf = new InstructionCachePerf()
}

class WriteBackIO() extends Bundle() {
  val ena = Input(Bool())
  val memwb = Input(new MemWb())
  // wb result (unregistered)
  val rfWrite = Vec(PIPE_COUNT, Output(new Result()))
  // for result forwarding (register)
  val memResult =  Vec(PIPE_COUNT, Output(new Result()))
}

class ExcIO() extends Bundle() {
  val ena = Input(Bool())
  val ocp = new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH)
  val intrs = Vec(INTR_COUNT, Input(Bool()))
  val excdec = Output(new ExcDec())
  val memexc = Input(new MemExc())
  val superMode = Output(Bool())
  val invalICache = Output(Bool())
  val invalDCache = Output(Bool())
}

class MMUIO() extends Bundle() {
  val ctrl = new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH)
  val superMode = Input(Bool())
  val exec = Input(Bool())
  val virt = new OcpBurstSlavePort(ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH)
  val phys = new OcpBurstMasterPort(EXTMEM_ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH)
}

// Performance counters
class InstructionCachePerf() extends Bundle() {
  val hit = Output(Bool())
  val miss = Output(Bool())
}
class DataCachePerf() extends Bundle() {
  val hit = Output(Bool())
  val miss = Output(Bool())
}
class StackCachePerf() extends Bundle() {
  val spill = Output(Bool())
  val fill = Output(Bool())
}
class WriteCombinePerf() extends Bundle() {
  val hit = Output(Bool())
  val miss = Output(Bool())
}
class MemPerf() extends Bundle() {
  val read = Output(Bool())
  val write = Output(Bool())
}
class PerfCounterIO() extends Bundle() {
  val ic = Input(new InstructionCachePerf())
  val dc = Input(new DataCachePerf())
  val sc = Input(new StackCachePerf())
  val wc = Input(new WriteCombinePerf())
  val mem = Input(new MemPerf())
}
