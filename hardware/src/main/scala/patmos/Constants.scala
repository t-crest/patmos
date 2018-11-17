/*
 * Constants for Patmos.
 *
 * Authors: Martin Schoeberl (martin@jopdesign.com)
 *          Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package patmos

import Chisel._

import util.log2Up

object Constants {

  val CLOCK_FREQ = util.Config.getConfig.frequency

  val PIPE_COUNT = util.Config.getConfig.pipeCount

  val ISPM_SIZE = util.Config.getConfig.ISPM.size
  val DSPM_SIZE = util.Config.getConfig.DSPM.size

  val ICACHE_TYPE = util.Config.getConfig.ICache.typ
  val ICACHE_SIZE = util.Config.getConfig.ICache.size
  val ICACHE_ASSOC = util.Config.getConfig.ICache.assoc
  val ICACHE_REPL = util.Config.getConfig.ICache.repl

  val DCACHE_SIZE = util.Config.getConfig.DCache.size
  val DCACHE_ASSOC = util.Config.getConfig.DCache.assoc
  val DCACHE_REPL = util.Config.getConfig.DCache.repl

  val CACHE_REPL_LRU  = "lru"
  val CACHE_REPL_FIFO = "fifo"
  def cacheRepl2Int(repl: String): Int = repl match {
    case CACHE_REPL_LRU  => 1
    case CACHE_REPL_FIFO => 2
    case _               => 0
  }

  val ICACHE_TYPE_METHOD = "method"
  val ICACHE_TYPE_LINE = "line"
  def iCacheType2Int(typ: String): Int = typ match {
    case ICACHE_TYPE_METHOD => 1
    case ICACHE_TYPE_LINE   => 2
    case _                  => 0
  }

  val DCACHE_WRITETHROUGH = util.Config.getConfig.DCache.writeThrough
  val SCACHE_SIZE = util.Config.getConfig.SCache.size

  // we use a very simple decoding of ISPM at address 0x00010000
  val ISPM_ONE_BIT = 16

  val EXTMEM_SIZE = util.Config.getConfig.ExtMem.size
  val EXTMEM_ADDR_WIDTH = log2Up(EXTMEM_SIZE)
  val BURST_LENGTH = util.Config.getConfig.burstLength // For SSRAM on DE2-70 board max. 4
  val WRITE_COMBINE = util.Config.getConfig.writeCombine

  // minimum size of internal program counter
  val MIN_OFF_WIDTH = if (ICACHE_TYPE == ICACHE_TYPE_METHOD) 0 else log2Up(EXTMEM_SIZE)

  // maximum width between ISPM size, ICache size and boot ROM size
  val MAX_OFF_WIDTH = List(log2Up(ICACHE_SIZE / 4), log2Up(ISPM_SIZE / 4),
    util.Config.minPcWidth, MIN_OFF_WIDTH).reduce(math.max)


  // Exceptions/interrupts
  val EXC_IO_OFFSET = 1
  val EXC_SRC_BITS = 5
  val EXC_COUNT  = 1 << EXC_SRC_BITS
  val INTR_COUNT = 16

  // Exceptions triggered by network interface
  val NI_MSG_INTR = 2
  val NI_EXT_INTR = 3

  // Memory management unit
  val HAS_MMU = util.Config.getConfig.mmu
  val MMU_IO_OFFSET = 7
  
  // CPU Info unit
  val CPUINFO_OFFSET = 0

  // The PC counts in words. 30 bits are enough for the 4 GB address space.
  // We might cut that down to what we actually really support (16 MB)
  val PC_SIZE = 30

  val REG_BITS = 5
  val REG_COUNT = 1 << REG_BITS

  val PRED_BITS = 3
  val PRED_COUNT = 1 << PRED_BITS
  val PRED_IFFALSE = Bits("b1") ## Bits(0, width = PRED_BITS)

  val INSTR_WIDTH = 32
  val DATA_WIDTH = 32
  val ADDR_WIDTH = 32

  val BYTE_WIDTH = 8
  val BYTES_PER_WORD = DATA_WIDTH / BYTE_WIDTH

  val OPCODE_ALUI = Bits("b00")
  val OPCODE_ALU = Bits("b01000")
  val OPCODE_SPC = Bits("b01001")
  val OPCODE_LDT = Bits("b01010")
  val OPCODE_STT = Bits("b01011")

  val OPCODE_STC = Bits("b01100")

  val OPCODE_CFL_CALLND = Bits("b10000")
  val OPCODE_CFL_BRND   = Bits("b10010")
  val OPCODE_CFL_BRCFND = Bits("b10100")
  val OPCODE_CFL_TRAP   = Bits("b10110")

  val OPCODE_CFL_CALL   = Bits("b10001")
  val OPCODE_CFL_BR     = Bits("b10011")
  val OPCODE_CFL_BRCF   = Bits("b10101")

  val OPCODE_CFL_CFLRND = Bits("b11000")
  val OPCODE_CFL_CFLR   = Bits("b11001")

  val OPCODE_ALUL = Bits("b11111")

  val OPC_ALUR  = Bits("b000")
  val OPC_ALUU  = Bits("b001")
  val OPC_ALUM  = Bits("b010")
  val OPC_ALUC  = Bits("b011")
  val OPC_ALUP  = Bits("b100")
  val OPC_ALUB  = Bits("b101")
  val OPC_ALUCI = Bits("b110")

  val OPC_MTS = Bits("b010")
  val OPC_MFS = Bits("b011")

  val MSIZE_W = Bits("b000")
  val MSIZE_H = Bits("b001")
  val MSIZE_B = Bits("b010")
  val MSIZE_HU = Bits("b011")
  val MSIZE_BU = Bits("b100")

  val MTYPE_S = Bits("b00")
  val MTYPE_L = Bits("b01")
  val MTYPE_C = Bits("b10")
  val MTYPE_M = Bits("b11")

  val FUNC_ADD = Bits("b0000")
  val FUNC_SUB = Bits("b0001")
  val FUNC_XOR = Bits("b0010")
  val FUNC_SL = Bits("b0011")
  val FUNC_SR = Bits("b0100")
  val FUNC_SRA = Bits("b0101")
  val FUNC_OR = Bits("b0110")
  val FUNC_AND = Bits("b0111")
  val FUNC_NOR = Bits("b1011")
  val FUNC_SHADD = Bits("b1100")
  val FUNC_SHADD2 = Bits("b1101")

  val MFUNC_MUL = Bits("b0000")
  val MFUNC_MULU = Bits("b0001")

  val CFUNC_EQ = Bits("b0000")
  val CFUNC_NEQ = Bits("b0001")
  val CFUNC_LT = Bits("b0010")
  val CFUNC_LE = Bits("b0011")
  val CFUNC_ULT = Bits("b0100")
  val CFUNC_ULE = Bits("b0101")
  val CFUNC_BTEST = Bits("b0110")

  val PFUNC_OR = Bits("b00")
  val PFUNC_AND = Bits("b01")
  val PFUNC_XOR = Bits("b10")
  val PFUNC_NOR = Bits("b11")

  val JFUNC_RET   = Bits("b0000")
  val JFUNC_XRET  = Bits("b0001")
  val JFUNC_CALL  = Bits("b0100")
  val JFUNC_BR    = Bits("b0101")
  val JFUNC_BRCF  = Bits("b1010")

  val SPEC_FL = Bits("b0000")
  val SPEC_SL = Bits("b0010")
  val SPEC_SH = Bits("b0011")
  val SPEC_SS = Bits("b0101")
  val SPEC_ST = Bits("b0110")

  val SPEC_SRB = Bits("b0111")
  val SPEC_SRO = Bits("b1000")
  val SPEC_SXB = Bits("b1001")
  val SPEC_SXO = Bits("b1010")

  val STC_SRES   = Bits("b0000")
  val STC_SENS   = Bits("b0100")
  val STC_SFREE  = Bits("b1000")
  val STC_SSPILL = Bits("b1100")

  val STC_SENSR   = Bits("b0101")
  val STC_SSPILLR = Bits("b1101")

  val SC_OP_BITS = 3
  val sc_OP_NONE :: sc_OP_SET_ST :: sc_OP_SET_MT :: sc_OP_RES :: sc_OP_ENS :: sc_OP_FREE :: sc_OP_SPILL :: Nil = Enum(UInt(), 7)
}
