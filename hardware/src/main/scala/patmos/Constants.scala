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
  val PRED_IFFALSE = UInt("b1") ## UInt(0, width = PRED_BITS)

  val INSTR_WIDTH = 32
  val DATA_WIDTH = 32
  val ADDR_WIDTH = 32

  val BYTE_WIDTH = 8
  val BYTES_PER_WORD = DATA_WIDTH / BYTE_WIDTH

  val OPCODE_ALUI = UInt("b00")
  val OPCODE_ALU = UInt("b01000")
  val OPCODE_SPC = UInt("b01001")
  val OPCODE_LDT = UInt("b01010")
  val OPCODE_STT = UInt("b01011")

  val OPCODE_STC = UInt("b01100")

  val OPCODE_CFL_CALLND = UInt("b10000")
  val OPCODE_CFL_BRND   = UInt("b10010")
  val OPCODE_CFL_BRCFND = UInt("b10100")
  val OPCODE_CFL_TRAP   = UInt("b10110")

  val OPCODE_CFL_CALL   = UInt("b10001")
  val OPCODE_CFL_BR     = UInt("b10011")
  val OPCODE_CFL_BRCF   = UInt("b10101")

  val OPCODE_CFL_CFLRND = UInt("b11000")
  val OPCODE_CFL_CFLR   = UInt("b11001")

  val OPCODE_ALUL = UInt("b11111")

  val OPC_ALUR  = UInt("b000")
  val OPC_ALUU  = UInt("b001")
  val OPC_ALUM  = UInt("b010")
  val OPC_ALUC  = UInt("b011")
  val OPC_ALUP  = UInt("b100")
  val OPC_ALUB  = UInt("b101")
  val OPC_ALUCI = UInt("b110")

  val OPC_MTS = UInt("b010")
  val OPC_MFS = UInt("b011")

  val MSIZE_W = UInt("b000")
  val MSIZE_H = UInt("b001")
  val MSIZE_B = UInt("b010")
  val MSIZE_HU = UInt("b011")
  val MSIZE_BU = UInt("b100")

  val MTYPE_S = UInt("b00")
  val MTYPE_L = UInt("b01")
  val MTYPE_C = UInt("b10")
  val MTYPE_M = UInt("b11")

  val FUNC_ADD = UInt("b0000")
  val FUNC_SUB = UInt("b0001")
  val FUNC_XOR = UInt("b0010")
  val FUNC_SL = UInt("b0011")
  val FUNC_SR = UInt("b0100")
  val FUNC_SRA = UInt("b0101")
  val FUNC_OR = UInt("b0110")
  val FUNC_AND = UInt("b0111")
  val FUNC_NOR = UInt("b1011")
  val FUNC_SHADD = UInt("b1100")
  val FUNC_SHADD2 = UInt("b1101")

  val MFUNC_MUL = UInt("b0000")
  val MFUNC_MULU = UInt("b0001")

  val CFUNC_EQ = UInt("b0000")
  val CFUNC_NEQ = UInt("b0001")
  val CFUNC_LT = UInt("b0010")
  val CFUNC_LE = UInt("b0011")
  val CFUNC_ULT = UInt("b0100")
  val CFUNC_ULE = UInt("b0101")
  val CFUNC_BTEST = UInt("b0110")

  val PFUNC_OR = UInt("b00")
  val PFUNC_AND = UInt("b01")
  val PFUNC_XOR = UInt("b10")
  val PFUNC_NOR = UInt("b11")

  val JFUNC_RET   = UInt("b0000")
  val JFUNC_XRET  = UInt("b0001")
  val JFUNC_CALL  = UInt("b0100")
  val JFUNC_BR    = UInt("b0101")
  val JFUNC_BRCF  = UInt("b1010")

  val SPEC_FL = UInt("b0000")
  val SPEC_SL = UInt("b0010")
  val SPEC_SH = UInt("b0011")
  val SPEC_SS = UInt("b0101")
  val SPEC_ST = UInt("b0110")

  val SPEC_SRB = UInt("b0111")
  val SPEC_SRO = UInt("b1000")
  val SPEC_SXB = UInt("b1001")
  val SPEC_SXO = UInt("b1010")

  val STC_SRES   = UInt("b0000")
  val STC_SENS   = UInt("b0100")
  val STC_SFREE  = UInt("b1000")
  val STC_SSPILL = UInt("b1100")

  val STC_SENSR   = UInt("b0101")
  val STC_SSPILLR = UInt("b1101")

  val SC_OP_BITS = 3
  val sc_OP_NONE :: sc_OP_SET_ST :: sc_OP_SET_MT :: sc_OP_RES :: sc_OP_ENS :: sc_OP_FREE :: sc_OP_SPILL :: Nil = Enum(UInt(), 7)
}
