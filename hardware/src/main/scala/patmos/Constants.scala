/*
 * Constants for Patmos.
 *
 * Authors: Martin Schoeberl (martin@jopdesign.com)
 *          Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package patmos

import util.Config

import chisel3._
import chisel3.util._

object ConstantsForConf { // TODO Remove when baud is obtained from configuration .xml
  val UART_BAUD = 115200
}

object Constants {

  val CLOCK_FREQ = Config.getConfig.frequency

  val PIPE_COUNT = Config.getConfig.pipeCount

  val ISPM_SIZE = Config.getConfig.ISPM.size
  val DSPM_SIZE = Config.getConfig.DSPM.size

  val ICACHE_TYPE = Config.getConfig.ICache.typ
  val ICACHE_SIZE = Config.getConfig.ICache.size
  val ICACHE_ASSOC = Config.getConfig.ICache.assoc
  val ICACHE_REPL = Config.getConfig.ICache.repl

  val DCACHE_SIZE = Config.getConfig.DCache.size
  val DCACHE_ASSOC = Config.getConfig.DCache.assoc
  val DCACHE_REPL = Config.getConfig.DCache.repl

  val UART_BAUD = ConstantsForConf.UART_BAUD // TODO obtain from configuration .xml through Config.scala

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

  val DCACHE_WRITETHROUGH = Config.getConfig.DCache.writeThrough
  val SCACHE_SIZE = Config.getConfig.SCache.size

  // we use a very simple decoding of ISPM at address 0x00010000
  val ISPM_ONE_BIT = 16

  val EXTMEM_SIZE = Config.getConfig.ExtMem.size
  val EXTMEM_ADDR_WIDTH = log2Up(EXTMEM_SIZE)
  val BURST_LENGTH = Config.getConfig.burstLength // For SSRAM on DE2-70 board max. 4
  val WRITE_COMBINE = Config.getConfig.writeCombine

  // minimum size of internal program counter
  val MIN_OFF_WIDTH = if (ICACHE_TYPE == ICACHE_TYPE_METHOD) 0 else log2Up(EXTMEM_SIZE)

  // maximum width between ISPM size, ICache size and boot ROM size
  val MAX_OFF_WIDTH = List(log2Up(ICACHE_SIZE / 4), log2Up(ISPM_SIZE / 4),
    Config.minPcWidth, MIN_OFF_WIDTH).reduce(math.max)


  // Exceptions/interrupts
  val EXC_IO_OFFSET = 1
  val EXC_SRC_BITS = 5
  val EXC_COUNT  = 1 << EXC_SRC_BITS
  val INTR_COUNT = 16

  // Exceptions triggered by network interface
  val NI_MSG_INTR = 2
  val NI_EXT_INTR = 3

  // Memory management unit
  val HAS_MMU = Config.getConfig.mmu
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
  def PRED_IFFALSE = "b1".U ## 0.U(PRED_BITS.W)

  val INSTR_WIDTH = 32
  val DATA_WIDTH = 32
  val ADDR_WIDTH = 32

  val BYTE_WIDTH = 8
  val BYTES_PER_WORD = DATA_WIDTH / BYTE_WIDTH

  def OPCODE_ALUI = "b00".U(2.W)
  def OPCODE_ALU  = "b01000".U(5.W)
  def OPCODE_SPC  = "b01001".U(5.W)
  def OPCODE_LDT  = "b01010".U(5.W)
  def OPCODE_STT  = "b01011".U(5.W)

  def OPCODE_STC  = "b01100".U(5.W)

  def OPCODE_CFL_CALLND = "b10000".U(5.W)
  def OPCODE_CFL_BRND   = "b10010".U(5.W)
  def OPCODE_CFL_BRCFND = "b10100".U(5.W)
  def OPCODE_CFL_TRAP   = "b10110".U(5.W)

  def OPCODE_CFL_CALL   = "b10001".U(5.W)
  def OPCODE_CFL_BR     = "b10011".U(5.W)
  def OPCODE_CFL_BRCF   = "b10101".U(5.W)

  def OPCODE_CFL_CFLRND = "b11000".U(5.W)
  def OPCODE_CFL_CFLR   = "b11001".U(5.W)

  def OPCODE_ALUL = "b11111".U(5.W)

  def OPC_ALUR    = "b000".U(3.W)
  def OPC_ALUU    = "b001".U(3.W)
  def OPC_ALUM    = "b010".U(3.W)
  def OPC_ALUC    = "b011".U(3.W)
  def OPC_ALUP    = "b100".U(3.W)
  def OPC_ALUB    = "b101".U(3.W)
  def OPC_ALUCI   = "b110".U(3.W)

  def OPC_MTS     = "b010".U(3.W)
  def OPC_MFS     = "b011".U(3.W)

  def MSIZE_W     = "b000".U(3.W)
  def MSIZE_H     = "b001".U(3.W)
  def MSIZE_B     = "b010".U(3.W)
  def MSIZE_HU    = "b011".U(3.W)
  def MSIZE_BU    = "b100".U(3.W)

  def MTYPE_S     = "b00".U(2.W)
  def MTYPE_L     = "b01".U(2.W)
  def MTYPE_C     = "b10".U(2.W)
  def MTYPE_M     = "b11".U(2.W)

  def FUNC_ADD    = "b0000".U(4.W)
  def FUNC_SUB    = "b0001".U(4.W)
  def FUNC_XOR    = "b0010".U(4.W)
  def FUNC_SL     = "b0011".U(4.W)
  def FUNC_SR     = "b0100".U(4.W)
  def FUNC_SRA    = "b0101".U(4.W)
  def FUNC_OR     = "b0110".U(4.W)
  def FUNC_AND    = "b0111".U(4.W)
  def FUNC_NOR    = "b1011".U(4.W)
  def FUNC_SHADD  = "b1100".U(4.W)
  def FUNC_SHADD2 = "b1101".U(4.W)

  def MFUNC_MUL   = "b0000".U(4.W)
  def MFUNC_MULU  = "b0001".U(4.W)

  def CFUNC_EQ    = "b0000".U(4.W)
  def CFUNC_NEQ   = "b0001".U(4.W)
  def CFUNC_LT    = "b0010".U(4.W)
  def CFUNC_LE    = "b0011".U(4.W)
  def CFUNC_ULT   = "b0100".U(4.W)
  def CFUNC_ULE   = "b0101".U(4.W)
  def CFUNC_BTEST = "b0110".U(4.W)

  def PFUNC_OR    = "b00".U(2.W)
  def PFUNC_AND   = "b01".U(2.W)
  def PFUNC_XOR   = "b10".U(2.W)
  def PFUNC_NOR   = "b11".U(2.W)

  def JFUNC_RET   = "b0000".U(4.W)
  def JFUNC_XRET  = "b0001".U(4.W)
  def JFUNC_CALL  = "b0100".U(4.W)
  def JFUNC_BR    = "b0101".U(4.W)
  def JFUNC_BRCF  = "b1010".U(4.W)

  def SPEC_FL     = "b0000".U(4.W)
  def SPEC_SL     = "b0010".U(4.W)
  def SPEC_SH     = "b0011".U(4.W)
  def SPEC_SS     = "b0101".U(4.W)
  def SPEC_ST     = "b0110".U(4.W)
 
  def SPEC_SRB    = "b0111".U(4.W)
  def SPEC_SRO    = "b1000".U(4.W)
  def SPEC_SXB    = "b1001".U(4.W)
  def SPEC_SXO    = "b1010".U(4.W)
 
  def STC_SRES    = "b0000".U(4.W)
  def STC_SENS    = "b0100".U(4.W)
  def STC_SFREE   = "b1000".U(4.W)
  def STC_SSPILL  = "b1100".U(4.W)

  def STC_SENSR   = "b0101".U(4.W)
  def STC_SSPILLR = "b1101".U(4.W)

  def SC_OP_BITS = 3
  val sc_OP_NONE :: sc_OP_SET_ST :: sc_OP_SET_MT :: sc_OP_RES :: sc_OP_ENS :: sc_OP_FREE :: sc_OP_SPILL :: Nil = Enum(7)

  def OPCODE_COP = "b01101".U(5.W)
  def COP_CUSTOM_BIT = "b0".U(1.W)
  def COP_READ_BIT = "b1".U(1.W)
  val COP_ID_WIDTH      = 3
  val COP_COUNT         = Config.getConfig.coprocessorCount
  val COP_FUNCID_WIDTH  = 5

}
