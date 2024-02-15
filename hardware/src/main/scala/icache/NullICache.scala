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

class NullICache() extends CacheType {

  val callRetBaseReg = RegInit(1.U(DATA_WIDTH.W))
  val callAddrReg = RegInit(1.U(DATA_WIDTH.W))
  val selIspmReg = RegInit(false.B)

  io.ena_out := true.B

  when (io.exicache.doCallRet && io.ena_in) {
    callRetBaseReg := io.exicache.callRetBase
    callAddrReg := io.exicache.callRetAddr
    selIspmReg := io.exicache.callRetBase(ADDR_WIDTH-1, ISPM_ONE_BIT-2) === 0x1.U
  }

  io.icachefe.instrEven := 0.U
  io.icachefe.instrOdd := 0.U
  io.icachefe.base := callRetBaseReg
  io.icachefe.relBase := callRetBaseReg(ISPM_ONE_BIT-3, 0)
  io.icachefe.relPc := callAddrReg + callRetBaseReg(ISPM_ONE_BIT-3, 0)
  io.icachefe.reloc := Mux(selIspmReg, (1 << (ISPM_ONE_BIT - 2)).U, 0.U)
  io.icachefe.memSel := Cat(selIspmReg, 0.U)

  io.ocp_port.M.Cmd := OcpCmd.IDLE
  io.ocp_port.M.Addr := 0.U
  io.ocp_port.M.Data := 0.U
  io.ocp_port.M.DataValid := 0.U
  io.ocp_port.M.DataByteEn := 0.U
}
