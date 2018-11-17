/*
 * Method cache without actual functionality
 * 
 * Authors: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *        Philipp Degasperi (philipp.degasperi@gmail.com)
 */

package patmos

import Chisel._
import Constants._
import ocp._

class NullICache() extends Module {
  val io = IO(new ICacheIO())

  val callRetBaseReg = RegInit(UInt(1, DATA_WIDTH))
  val callAddrReg = RegInit(UInt(1, DATA_WIDTH))
  val selIspmReg = RegInit(Bool(false))

  io.ena_out := Bool(true)

  when (io.exicache.doCallRet && io.ena_in) {
    callRetBaseReg := io.exicache.callRetBase
    callAddrReg := io.exicache.callRetAddr
    selIspmReg := io.exicache.callRetBase(ADDR_WIDTH-1, ISPM_ONE_BIT-2) === Bits(0x1)
  }

  io.icachefe.instrEven := Bits(0)
  io.icachefe.instrOdd := Bits(0)
  io.icachefe.base := callRetBaseReg
  io.icachefe.relBase := callRetBaseReg(ISPM_ONE_BIT-3, 0)
  io.icachefe.relPc := callAddrReg + callRetBaseReg(ISPM_ONE_BIT-3, 0)
  io.icachefe.reloc := Mux(selIspmReg, UInt(1 << (ISPM_ONE_BIT - 2)), UInt(0))
  io.icachefe.memSel := Cat(selIspmReg, Bits(0))

  io.ocp_port.M.Cmd := OcpCmd.IDLE
  io.ocp_port.M.Addr := Bits(0)
  io.ocp_port.M.Data := Bits(0)
  io.ocp_port.M.DataValid := Bits(0)
  io.ocp_port.M.DataByteEn := Bits(0)
}
