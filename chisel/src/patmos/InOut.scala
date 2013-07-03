/*
   Copyright 2013 Technical University of Denmark, DTU Compute. 
   All rights reserved.
   
   This file is part of the time-predictable VLIW processor Patmos.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

      1. Redistributions of source code must retain the above copyright notice,
         this list of conditions and the following disclaimer.

      2. Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
   NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   The views and conclusions contained in the software and documentation are
   those of the authors and should not be interpreted as representing official
   policies, either expressed or implied, of the copyright holder.
 */

/*
 * IO component of Patmos.
 * 
 * Authors: Martin Schoeberl (martin@jopdesign.com)
 *          Wolfgang Puffitsch (wpuffitsch@gmail.com)
 * 
 */

package patmos

import Chisel._
import Node._

import Constants._

class InOut() extends Component {
  val io = new InOutIO()

  def splitData(word: Bits) = {
	val retval = Vec(BYTES_PER_WORD) { Bits(width = BYTE_WIDTH) }
	for (i <- 0 until BYTES_PER_WORD) {
	  retval(i) := word(DATA_WIDTH-i*BYTE_WIDTH-1,
						DATA_WIDTH-i*BYTE_WIDTH-BYTE_WIDTH)
	}
	retval
  }

  val memInOutReg = Reg(io.memInOut)

  // Compute selects
  val selIO = io.memInOut.address(DATA_WIDTH-1, DATA_WIDTH-4) === Bits("b1111")
  val selSpm = !selIO & io.memInOut.address(ISPM_ONE_BIT) === Bits(0x0)
  val selUart = selIO & io.memInOut.address(11, 8) === Bits(0x1)
  val selLed = selIO & io.memInOut.address(11, 8) === Bits(0x2)

  // Register selects
  val selSpmReg = Reg(resetVal = Bits("b0"))
  val selUartReg = Reg(resetVal = Bits("b0"))
  val selLedReg = Reg(resetVal = Bits("b0"))
  when(io.memInOut.rd | io.memInOut.wr) {
	selSpmReg := selSpm
	selUartReg := selUart
	selLedReg := selLed
  }

  // The SPM
  val spm = new Spm(1 << DSPM_BITS)
  spm.io.rd := io.memInOut.rd & selSpm
  spm.io.wr := io.memInOut.wr & selSpm
  spm.io.address := io.memInOut.address
  spm.io.wrData := io.memInOut.wrData
  spm.io.byteEna := io.memInOut.byteEna
  val spmData = spm.io.rdData;
  val spmRdyCnt = spm.io.rdyCnt;

  // The UART
  io.uart.rd := io.memInOut.rd & selUart
  io.uart.wr := io.memInOut.wr & selUart
  io.uart.address := io.memInOut.address(2)
  io.uart.wr_data := Cat(io.memInOut.wrData(0),
						 io.memInOut.wrData(1),
						 io.memInOut.wrData(2),
						 io.memInOut.wrData(3))
  val uartData = splitData(io.uart.rd_data)
  val uartRdyCnt = io.uart.rdy_cnt

  // The LED
  val ledReg = Reg(Bits(0, 8))
  when(io.memInOut.wr & selLed) {
    ledReg := Cat(io.memInOut.wrData(0),
				  io.memInOut.wrData(1),
				  io.memInOut.wrData(2),
				  io.memInOut.wrData(3))
  }
  io.led := ledReg

  // Return data to pipeline
  io.memInOut.rdData := Mux(selUartReg, uartData, spmData)
  io.memInOut.rdyCnt := spmRdyCnt | uartRdyCnt
}
