/*
 * Simple I/O module for Seven Segment Hex Display
 *
 * Authors: Eleftherios Kyriakakis (elky@dtu.dk)
 *
 */

package io

import util.BCDToSevenSegDecoder

import chisel3._
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
}

class SegmentDisplay(displayCount : Int, segmentPolarity: Int) extends CoreDevice() {

    // Override
    override val io = IO(new CoreDeviceIO() with patmos.HasPins {
      override val pins = new Bundle() {
        val hexDisp = Output(Vec(displayCount, UInt(7.W)))
      }
    })

    // Master register
    val masterReg = RegNext(next = io.ocp.M)
    
    // Default response
    val respReg = RegInit(init = OcpResp.NULL)
    respReg := OcpResp.NULL

    val dataReg = RegInit(init = 0.U(DATA_WIDTH.W))

    // Display register
    val dispRegVec = Reg(Vec(displayCount, segmentPolarity.U(7.W)))

    // Decoded master data
    val decodedMData = Wire(UInt(7.W))
    val decoder = Module(new BCDToSevenSegDecoder(BCDToSevenSegDecoder.ActiveLow)).io
    decoder.bcdData := masterReg.Data(3, 0)
    decodedMData := decoder.segData

    // Read/Write seven segment displays
    for(i <- 0 until displayCount by 1){
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
    io.pins.hexDisp := dispRegVec
    
}