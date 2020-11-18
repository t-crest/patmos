/*
 * Connection definitions for the pipe stages.
 *
 * Authors: Martin Schoeberl (martin@jopdesign.com)
 *          Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package patmos

import Chisel._

import Constants._

import ocp._

class FeDec() extends Bundle() {
  val instr_a = UInt(width = INSTR_WIDTH)
  val instr_b = UInt(width = INSTR_WIDTH)
  val pc = UInt(width = PC_SIZE)
  val base = UInt(width = PC_SIZE)
  val reloc = UInt(width = ADDR_WIDTH)
  val relPc = UInt(width = PC_SIZE)

  def flush() = {
    // flush only necessary parts of instruction
    // instr_a(30, 27) := PRED_IFFALSE
    // instr_a(26, 25) := OPCODE_ALUI
    // instr_b(30, 27) := PRED_IFFALSE
    // instr_b(26, 25) := OPCODE_ALUI
    instr_a := UInt(0)
    instr_b := UInt(0)
  }
}

class AluOp() extends Bundle() {
  val func = UInt(width = 4)
  val isMul = Bool()
  val isCmp = Bool()
  val isPred = Bool()
  val isBCpy = Bool()
  val isMTS = Bool()
  val isMFS = Bool()

  def defaults() = {
    func := UInt(0)
    isMul := Bool(false)
    isCmp := Bool(false)
    isPred := Bool(false)
    isBCpy := Bool(false)
    isMTS := Bool(false)
    isMFS := Bool(false)
  }
}

class PredOp() extends Bundle() {
  val func = UInt(width = 2) // as they have a strange encoding
  val dest = UInt(width = PRED_BITS)
  val s1Addr = UInt(width = PRED_BITS+1)
  val s2Addr = UInt(width = PRED_BITS+1)

  def defaults() = {
    func := UInt(0)
    dest := UInt(0)
    s1Addr := UInt(0)
    s2Addr := UInt(0)
  }
}

class JmpOp() extends Bundle() {
  val branch = Bool()
  val target = UInt(width = PC_SIZE)
  val reloc = UInt(width = ADDR_WIDTH)

  def defaults() = {
    branch := Bool(false)
    target := UInt(0)
    reloc := UInt(0)
  }
}

class MemOp() extends Bundle() {
  val load = Bool()
  val store = Bool()
  val hword = Bool()
  val byte = Bool()
  val zext = Bool()
  val typ  = UInt(width = 2)

  def defaults() = {
    load := Bool(false)
    store := Bool(false)
    hword := Bool(false)
    byte := Bool(false)
    zext := Bool(false)
    typ := UInt(0)
  }
}

class CopOp() extends Bundle() {
  val isCop = Bool()
  val copId = UInt(width = COP_ID_WIDTH)
  val funcId = UInt(width = COP_FUNCID_WIDTH)

  def defaults() = {
    isCop := Bool(false)
    copId := UInt(0)
    funcId := UInt(0)
  }
}

class DecEx() extends Bundle() {
  val pc = UInt(width = PC_SIZE)
  val base = UInt(width = PC_SIZE)
  val relPc = UInt(width = PC_SIZE)
  val pred = Vec(PIPE_COUNT, UInt((PRED_BITS+1).W) )
  val aluOp = Vec(PIPE_COUNT, new AluOp() )
  val predOp = Vec(PIPE_COUNT, new PredOp() )
  val jmpOp = new JmpOp()
  val memOp = new MemOp()
  val stackOp = UInt(width = SC_OP_BITS)
  val copOp = new CopOp()

  // the register fields are very similar to RegFileRead
  // maybe join the structures
  val rsAddr = Vec(2*PIPE_COUNT, UInt(width = REG_BITS) )
  val rsData = Vec(2*PIPE_COUNT, UInt(width = DATA_WIDTH) )
  val rdAddr = Vec(PIPE_COUNT, UInt(width = REG_BITS) )
  val immVal = Vec(PIPE_COUNT, UInt(width = DATA_WIDTH) )
  val immOp  = Vec(PIPE_COUNT, Bool() )
  // maybe we should have similar structure as the Result one here
  val wrRd  = Vec(PIPE_COUNT, Bool() )

  val callAddr = UInt(width = DATA_WIDTH)
  val call = Bool()
  val ret = Bool()
  val brcf = Bool()
  val trap = Bool()
  val xcall = Bool()
  val xret = Bool()
  val xsrc = UInt(width = EXC_SRC_BITS)
  val nonDelayed = Bool()

  val illOp = Bool()

  def flush() = {
    pred := Vec.fill(PIPE_COUNT) { PRED_IFFALSE }
    illOp := Bool(false)
    copOp.isCop := Bool(false)
  }

  def defaults() = {
    pc := UInt(0)
    relPc := UInt(0)
    pred := Vec.fill(PIPE_COUNT) { PRED_IFFALSE }
    aluOp.map(_.defaults())
    predOp.map(_.defaults())
    jmpOp.defaults()
    memOp.defaults()
    stackOp := sc_OP_NONE
    copOp.defaults()
    rsAddr := Vec.fill(2*PIPE_COUNT) { UInt(0) }
    rsData := Vec.fill(2*PIPE_COUNT) { UInt(0) }
    rdAddr := Vec.fill(PIPE_COUNT) { UInt(0) }
    immVal := Vec.fill(PIPE_COUNT) { UInt(0) }
    immOp := Vec.fill(PIPE_COUNT) { Bool(false) }
    wrRd := Vec.fill(PIPE_COUNT) { Bool(false) }
    callAddr := UInt(0)
    call := Bool(false)
    ret := Bool(false)
    brcf := Bool(false)
    trap := Bool(false)
    xcall := Bool(false)
    xret := Bool(false)
    xsrc := UInt(0)
    nonDelayed := Bool(false)
    illOp := Bool(false)
  }
}

class Result() extends Bundle() {
  val addr = UInt(width = REG_BITS)
  val data = UInt(width = DATA_WIDTH)
  val valid = Bool()

  def flush() = {
    valid := Bool(false)
  }
}

class MemIn() extends Bundle() {
  val load = Bool()
  val store = Bool()
  val hword = Bool()
  val byte = Bool()
  val zext = Bool()
  val typ = UInt(width = 2)
  val addr = UInt(width = DATA_WIDTH)
  val data = UInt(width = DATA_WIDTH)
  val call = Bool()
  val ret = Bool()
  val brcf = Bool()
  val trap = Bool()
  val xcall = Bool()
  val xret = Bool()
  val xsrc = UInt(width = EXC_SRC_BITS)
  val illOp = Bool()
  val callRetAddr = UInt(width = DATA_WIDTH)
  val callRetBase = UInt(width = DATA_WIDTH)
  val nonDelayed = Bool()

  def flush() = {
    load := Bool(false)
    store := Bool(false)
    call := Bool(false)
    ret := Bool(false)
    brcf := Bool(false)
    trap := Bool(false)
    xcall := Bool(false)
    xret := Bool(false)
    illOp := Bool(false)
  }
}

// interface between the EX stage and the stack cache
class ExSc extends Bundle() {
  // indicate which stack-cache operation is performed
  val op = UInt(width = 3)

  // operands of the stack-cache operation
  //   - opSetStackTop, opSetMemTop: the new value of stackTop or memTop
  val opData = UInt(width = DATA_WIDTH)
  //   - opSRES, opSENS, opSFREE   : the operand of the instructions
  val opOff  = UInt(width = ADDR_WIDTH)
}

class ScEx extends Bundle() {
  // the current value of the stack top pointer
  val stackTop = UInt(width = ADDR_WIDTH)
  
  // the current value of the mem top pointer
  val memTop = UInt(width = ADDR_WIDTH)
}

class ExMem() extends Bundle() {
  val rd = Vec(PIPE_COUNT, new Result() )
  val mem = new MemIn()
  val pc = UInt(width = PC_SIZE)
  val base = UInt(width = PC_SIZE)
  val relPc = UInt(width = PC_SIZE)

  def flush() = {
    rd.map(_.flush())
    mem.flush()
  }
}

class ExFe() extends Bundle() {
  val doBranch = Bool()
  val branchPc = UInt(width = PC_SIZE)
}

class MemFe() extends Bundle() {
  val doCallRet = Bool()
  val callRetPc = UInt(width = PC_SIZE)
  val callRetBase = UInt(width = PC_SIZE)
  // for ISPM write
  val store = Bool()
  val addr = UInt(width = DATA_WIDTH)
  val data = UInt(width = DATA_WIDTH)
}

class FeEx() extends Bundle() {
  val pc = UInt(width = PC_SIZE)
}

class MemWb() extends Bundle() {
  val rd = Vec(PIPE_COUNT, new Result() )
  // PC value for debugging
  val pc = UInt(width = PC_SIZE)
}

class RegFileRead() extends Bundle() {
  // first two are for pipeline A, second two for pipeline B
  val rsAddr = Vec(2*PIPE_COUNT, UInt(INPUT, REG_BITS) )
  val rsData = Vec(2*PIPE_COUNT, UInt(OUTPUT, DATA_WIDTH) )
}

class RegFileIO() extends Bundle() {
  val ena = Bool(INPUT)
  val rfRead = new RegFileRead()
  val rfWrite = Vec(PIPE_COUNT, new Result().asInput )
}

class FetchIO extends Bundle() {
  val ena = Bool(INPUT)
  val fedec = new FeDec().asOutput
  // PC for returns
  val feex = new FeEx().asOutput
  // branch from EX
  val exfe = new ExFe().asInput
  // call from MEM
  val memfe = new MemFe().asInput
  // connections to instruction cache
  val feicache = new FeICache().asOutput
  val icachefe = new ICacheFe().asInput
}

class ExcDec() extends Bundle() {
  val exc = Bool()
  val excBase = UInt(width = PC_SIZE)
  val excAddr = UInt(width = PC_SIZE)
  val intr = Bool()
  val addr = UInt(width = ADDR_WIDTH)
  val src = UInt(width = EXC_SRC_BITS)
  val local = Bool()
}

class DecodeIO() extends Bundle() {
  val ena = Bool(INPUT)
  val flush = Bool(INPUT)
  val fedec = new FeDec().asInput
  val decex = new DecEx().asOutput
  val rfWrite =  Vec(PIPE_COUNT, new Result().asInput )
  val exc = new ExcDec().asInput
}

class ExecuteIO() extends Bundle() {
  val ena_in = Bool(INPUT)
  val ena_out = Bool(OUTPUT)
  val flush = Bool(INPUT)
  val brflush = Bool(OUTPUT)
  val decex = new DecEx().asInput
  val exmem = new ExMem().asOutput
  val exicache = new ExICache().asOutput
  val feex = new FeEx().asInput
  // forwarding inputs
  val exResult = Vec(PIPE_COUNT, new Result().asInput )
  val memResult = Vec(PIPE_COUNT, new Result().asInput )
  // branch for FE
  val exfe = new ExFe().asOutput
  //stack cache
  val exsc = new ExSc().asOutput
  val scex = new ScEx().asInput
  // coprocessor
  val cop_out = Vec(COP_COUNT, new PatmosToCoprocessor().asOutput)
  val cop_in = Vec(COP_COUNT, new CoprocessorToPatmos().asInput)
}

class BootMemIO() extends Bundle() {
  val memInOut = new OcpCacheSlavePort(ADDR_WIDTH, DATA_WIDTH)
  val extMem = new OcpCacheMasterPort(ADDR_WIDTH, DATA_WIDTH)
}

class MemExc() extends Bundle() {
  val call = Bool()
  val ret = Bool()
  val src = UInt(width = EXC_SRC_BITS)

  val exc = Bool()
  val excBase = UInt(width = PC_SIZE)
  val excAddr = UInt(width = PC_SIZE)
}

class MemoryIO() extends Bundle() {
  val ena_out = Bool(OUTPUT)
  val ena_in = Bool(INPUT)
  val flush = Bool(OUTPUT)
  val exmem = new ExMem().asInput
  val memwb = new MemWb().asOutput
  val memfe = new MemFe().asOutput
  // for result forwarding
  val exResult = Vec(PIPE_COUNT, new Result().asOutput )
  // local and global accesses
  val localInOut = new OcpCoreMasterPort(ADDR_WIDTH, DATA_WIDTH)
  val globalInOut = new OcpCacheMasterPort(ADDR_WIDTH, DATA_WIDTH)
  // exceptions
  val icacheIllMem = Bool(INPUT)
  val scacheIllMem = Bool(INPUT)
  val exc = new MemExc().asOutput
}

class PatmosToCoprocessor() extends Bundle()
{
  val ena_in  = Bool()
  val trigger = Bool()
  val read    = Bool()
  val funcId  = UInt(width = COP_FUNCID_WIDTH)
  val opData  = Vec(2, UInt(width = DATA_WIDTH) )
  
  def defaults() = {
    ena_in := Bool(false)
    trigger := Bool(false)
    read := Bool(false)
    funcId := UInt(0)
    opData := Vec.fill(2) { UInt(0) }
  }
}

class CoprocessorToPatmos() extends Bundle()
{
  val ena_out = Bool()
  val result  = UInt(width = DATA_WIDTH)
}

class CoprocessorIO() extends Bundle()
{
  val patmosCop = new PatmosToCoprocessor().asOutput
  val copPatmos = new CoprocessorToPatmos().asInput  
}

//stack cache
class StackCacheIO() extends Bundle() {
  // check if another transfer is active
  val ena_in = Bool(INPUT)
  // signals from EX stage to stack cache
  val exsc = new ExSc().asInput
  // signals from stack cache back to the EX stage
  val scex = new ScEx().asOutput
  // signal an illegal memory access
  val illMem = Bool(OUTPUT)
  // indicate a stall
  val stall = Bool(OUTPUT)
}

// method/instruction cache connections
class FeICache extends Bundle() {
  val addrEven = UInt(width = ADDR_WIDTH)
  val addrOdd = UInt(width = ADDR_WIDTH)
}
class ExICache() extends Bundle() {
  val doCallRet = Bool()
  val callRetBase = UInt(width = ADDR_WIDTH)
  val callRetAddr = UInt(width = ADDR_WIDTH)
}
class ICacheFe extends Bundle() {
  val instrEven = UInt(width = INSTR_WIDTH)
  val instrOdd = UInt(width = INSTR_WIDTH)
  // absolute basse address
  val base = UInt(width = ADDR_WIDTH)
  // relative base address
  val relBase = UInt(width = MAX_OFF_WIDTH)
  // relative program counter
  val relPc = UInt(width = MAX_OFF_WIDTH+1)
  // offset between relative and absolute program counter
  val reloc = UInt(width = DATA_WIDTH)
  val memSel = UInt(width = 2)
}
class ICacheIO extends Bundle() {
  val ena_out = Bool(OUTPUT)
  val ena_in = Bool(INPUT)
  val invalidate = Bool(INPUT)
  val feicache = new FeICache().asInput
  val exicache = new ExICache().asInput
  val icachefe = new ICacheFe().asOutput
  val ocp_port = new OcpBurstMasterPort(ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH)
  val illMem = Bool(OUTPUT)
  val perf = new InstructionCachePerf()
}

class WriteBackIO() extends Bundle() {
  val ena = Bool(INPUT)
  val memwb = new MemWb().asInput
  // wb result (unregistered)
  val rfWrite = Vec(PIPE_COUNT, new Result().asOutput )
  // for result forwarding (register)
  val memResult =  Vec(PIPE_COUNT, new Result().asOutput )
}

class ExcIO() extends Bundle() {
  val ena = Bool(INPUT)
  val ocp = new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH)
  val intrs = Vec(INTR_COUNT, Bool(INPUT) )
  val excdec = new ExcDec().asOutput
  val memexc = new MemExc().asInput
  val superMode = Bool(OUTPUT)
  val invalICache = Bool(OUTPUT)
  val invalDCache = Bool(OUTPUT)
}

class MMUIO() extends Bundle() {
  val ctrl = new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH)
  val superMode = Bool(INPUT)
  val exec = Bool(INPUT)
  val virt = new OcpBurstSlavePort(ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH)
  val phys = new OcpBurstMasterPort(EXTMEM_ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH)
}

// Performance counters
class InstructionCachePerf() extends Bundle() {
  val hit = Bool(OUTPUT)
  val miss = Bool(OUTPUT)
}
class DataCachePerf() extends Bundle() {
  val hit = Bool(OUTPUT)
  val miss = Bool(OUTPUT)
}
class StackCachePerf() extends Bundle() {
  val spill = Bool(OUTPUT)
  val fill = Bool(OUTPUT)
}
class WriteCombinePerf() extends Bundle() {
  val hit = Bool(OUTPUT)
  val miss = Bool(OUTPUT)
}
class MemPerf() extends Bundle() {
  val read = Bool(OUTPUT)
  val write = Bool(OUTPUT)
}
class PerfCounterIO() extends Bundle() {
  val ic = new InstructionCachePerf().asInput
  val dc = new DataCachePerf().asInput
  val sc = new StackCachePerf().asInput
  val wc = new WriteCombinePerf().asInput
  val mem = new MemPerf().asInput
}
