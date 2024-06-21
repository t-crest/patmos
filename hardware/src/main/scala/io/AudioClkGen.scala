// BCLK & XCLK generator for WM8731 audio codec.
// BCLK & XCLK run at F = Patmos_freq/CLKDIV
//currently CLKDIV = 6,  F = 13.33 MHz (patmos 80 MHz)

package io

import chisel3._

class AudioClkGen(CLKDIV: Int) extends Module { //constants: from CONFIG parameters
  //val CLKDIV = 6;

  //IOs
  val io = new Bundle {
    //inputs from PATMOS
    val enAdcI = Input(UInt(1.W))
    val enDacI = Input(UInt(1.W))
    //outputs to WM8731
    val bclkO = Output(UInt(1.W))
    val xclkO = Output(UInt(1.W))
  }

  //register containing divider value
  val clkLimReg = RegInit(init = 6.U(5.W)) //5 bits, initialized to 6
  clkLimReg := (CLKDIV / 2).U //get from params!

  //Counter: clock divider for BCLK and XCLK
  val clkCntReg = RegInit(init = 0.U(5.W)) //counter reg for BCLK and XCLK

  //registers for XCLK and BCLK
  val clkReg = RegInit(init = 0.U(1.W))

  //connect outputs
  io.bclkO := clkReg
  io.xclkO := clkReg

  //count for CLK only when any enable signal is high
  when((io.enAdcI === 1.U) || (io.enDacI === 1.U)) {
    when(clkCntReg === clkLimReg - 1.U) {
      clkCntReg := 0.U
      clkReg := ~clkReg
    }.otherwise {
      clkCntReg := clkCntReg + 1.U
    }
  }.otherwise {
    clkCntReg := 0.U
    clkReg := 0.U
  }
}
