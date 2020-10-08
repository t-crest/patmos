package util

import Chisel._

class BCDToSevenSegDecoder() extends Module{
  val io = IO(new Bundle{
    val bcdData = Input(UInt(width = 4))
    val segPolarity = Input(Bool())
    val segData = Output(UInt(width = 7))
  })

  val result = Wire(UInt(width = 7))
  result := "b1000001".U

  switch(io.bcdData){
    is("b0000".U){
      result := "b1000000".U    //0
    }
    is("b0001".U){
      result := "b1111001".U    //1
    }
    is("b0010".U){
      result := "b0100100".U    //2
    }
    is("b0011".U){
      result := "b0110000".U    //3
    }
    is("b0100".U){
      result := "b0011001".U    //4
    }
    is("b0101".U){
      result := "b0010010".U    //5
    }
    is("b0110".U){
      result := "b0000010".U    //6
    }
    is("b0111".U){
      result := "b1111000".U    //7
    }
    is("b1000".U){
      result := "b0000000".U    //8
    }
    is("b1001".U){
      result := "b0011000".U    //9
    }
    is("b1010".U){
      result := "b0001000".U    //A
    }
    is("b1011".U){
      result := "b0000011".U    //B
    }
    is("b1100".U){
      result := "b1000110".U    //C
    }
    is("b1101".U){
      result := "b0100001".U    //D
    }
    is("b1110".U){
      result := "b0000110".U    //E
    }
    is("b1111".U){
      result := "b0001110".U    //F
    }
  }

  when (~io.segPolarity) {
    io.segData := result
  }.otherwise {
    io.segData := ~result
  }

}
