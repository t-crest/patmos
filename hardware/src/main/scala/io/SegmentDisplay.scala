/*
 * Simple I/O module for Seven Segment Hex Display
 *
 * Authors: Eleftherios Kyriakakis (elky@dtu.dk)
 *
 */

package io

import Chisel._
import patmos.Constants._
import ocp._

object SegmentDisplay extends DeviceObject {
  var displayCount = -1
  var segmentPolarity = -1

  def init(params: Map[String, String]) = {
      displayCount = getPosIntParam(params, "displayCount")
      segmentPolarity = getIntParam(params, "segmentPolarity")
  }

  def create(params: Map[String, String]) : SegmentDisplay = {
    Module(new SegmentDisplay(displayCount, segmentPolarity))
  }

  trait Pins {
    val segmentDisplayPins = new Bundle() {
      val hexDisp = Vec.fill(displayCount) {Bits(OUTPUT, 7)}
    }
  }
}

class SegmentDisplay(displayCount : Int, segmentPolarity: Int) extends CoreDevice() {

    // Override
    override val io = new CoreDeviceIO() with SegmentDisplay.Pins

    // Decode hardware
    def sevenSegBCDDecode(data : Bits) : Bits = {
        val result = Bits(width = 7)
        result := Bits("b1000001")
        switch(data(3,0)){
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
        if (segmentPolarity==0) {
            result
        } else {
            ~result
        }
    } 

    // Master register
    val masterReg = Reg(next = io.ocp.M)
    
    // Default response
    val respReg = Reg(init = OcpResp.NULL)
    respReg := OcpResp.NULL

    val dataReg = Reg(init = Bits(0, width = DATA_WIDTH))

    // Display register
    val dispRegVec = RegInit(Vec.fill(displayCount){Bits(segmentPolarity, width = 7)})

    // Decoded master data
    val decodedMData = Bits(width = 7)

    decodedMData := sevenSegBCDDecode(masterReg.Data)

    // Read/Write seven segment displays
    for(i <- 0 to displayCount-1 by 1){
        when(masterReg.Addr(4,2) === i.U){
            when(masterReg.Cmd === OcpCmd.WR){
                when(masterReg.Data(7)){
                    dispRegVec(i) := masterReg.Data(6, 0) //Drive display seven segments directly from OCP
                } .otherwise {
                    dispRegVec(i) := decodedMData //Drive display through decoded hex number from OCP
                }
            }
            dataReg := dispRegVec(i)
        }
    }

    when(masterReg.Cmd === OcpCmd.WR || masterReg.Cmd === OcpCmd.RD){
        respReg := OcpResp.DVA
    }

    // Connections to master
    io.ocp.S.Resp := respReg
    io.ocp.S.Data := dataReg

    // Connections to IO
    io.segmentDisplayPins.hexDisp := dispRegVec
    
}