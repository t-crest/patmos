// BCLK & XCLK generator for WM8731 audio codec.
// BCLK & XCLK run at F = Patmos_freq/CLKDIV
//currently CLKDIV = 6,  F = 13.33 MHz (patmos 80 MHz)

package io

import Chisel._

class AudioClkGen(CLKDIV: Int) extends Module
{
  //constants: from CONFIG parameters
  //val CLKDIV = 6;

  //IOs
  val io = new Bundle
  {
    //inputs from PATMOS
    val enAdcI = UInt(INPUT, 1)
    val enDacI = UInt(INPUT, 1)
    //outputs to WM8731
    val bclkO = UInt(OUTPUT, 1)
    val xclkO = UInt(OUTPUT, 1)
  }

  //register containing divider value
  val clkLimReg = Reg(init = UInt(6, 5)) //5 bits, initialized to 6
  clkLimReg := UInt(CLKDIV/2) //get from params!

  //Counter: clock divider for BCLK and XCLK
  val clkCntReg = Reg(init = UInt(0, 5)) //counter reg for BCLK and XCLK

  //registers for XCLK and BCLK
  val clkReg = Reg(init = UInt(0, 1))

  //connect outputs
  io.bclkO := clkReg
  io.xclkO := clkReg

  //count for CLK only when any enable signal is high
  when( (io.enAdcI === UInt(1)) || (io.enDacI === UInt(1)) ) {
    when( clkCntReg === clkLimReg - UInt(1)) {
      clkCntReg := UInt(0)
      clkReg := ~clkReg
    }
      .otherwise {
      clkCntReg := clkCntReg + UInt(1)
    }
  }
    .otherwise {
    clkCntReg := UInt(0)
    clkReg := UInt(0)
  }
}
