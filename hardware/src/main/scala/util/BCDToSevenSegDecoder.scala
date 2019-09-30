package util

import Chisel._

class BCDToSevenSegDecoder() extends Module{
  val io = new Bundle{
    val bcdData = Input(UInt(width = 4))
    val segPolarity = Input(Bool())
    val segData = Output(UInt(width = 7))
  }

  val result = Bits(width = 7)
  result := Bits("b1000001")

  switch(io.bcdData){
    is(Bits("b0000")){
      result := Bits("b1000000")    //0
    }
    is(Bits("b0001")){
      result := Bits("b1111001")    //1
    }
    is(Bits("b0010")){
      result := Bits("b0100100")    //2
    }
    is(Bits("b0011")){
      result := Bits("b0110000")    //3
    }
    is(Bits("b0100")){
      result := Bits("b0011001")    //4
    }
    is(Bits("b0101")){
      result := Bits("b0010010")    //5
    }
    is(Bits("b0110")){
      result := Bits("b0000010")    //6
    }
    is(Bits("b0111")){
      result := Bits("b1111000")    //7
    }
    is(Bits("b1000")){
      result := Bits("b0000000")    //8
    }
    is(Bits("b1001")){
      result := Bits("b0011000")    //9
    }
    is(Bits("b1010")){
      result := Bits("b0001000")    //A
    }
    is(Bits("b1011")){
      result := Bits("b0000011")    //B
    }
    is(Bits("b1100")){
      result := Bits("b1000110")    //C
    }
    is(Bits("b1101")){
      result := Bits("b0100001")    //D
    }
    is(Bits("b1110")){
      result := Bits("b0000110")    //E
    }
    is(Bits("b1111")){
      result := Bits("b0001110")    //F
    }
  }

  when (~io.segPolarity) {
    io.segData := result
  }.otherwise {
    io.segData := ~result
  }

}
