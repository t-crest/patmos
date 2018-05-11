package ptp1588assist

import Chisel._
import Node._
import io._
import ocp._
import patmos.Constants._

class PTP1588Assist(addrWidth: Int = ADDR_WIDTH, dataWidth: Int = DATA_WIDTH, clockFreq: Int = CLOCK_FREQ, secondsWidth: Int = 40, nanoWidth: Int = 24, initialTime: BigInt = 0) extends Module{
  val io = new Bundle{
    val ocp = new OcpCoreSlavePort(addrWidth, dataWidth)
    val ethMacRX = new MIIChannel().asInput()
    val ethMacTX = new MIIChannel().asInput()
    val intrs = Vec.fill(3){Bool(OUTPUT)}
    val rtcHexDisp = Vec.fill(8) {Bits(OUTPUT, 7)}
    val ledPHY = Bits(OUTPUT, width=1)
    val ledSOF = Bits(OUTPUT, width=1)
    val ledEOF = Bits(OUTPUT, width=1)
    val ledSFD = Bits(OUTPUT, width=8)
  }

  // Connections
  val rtc = Module(new RTC(clockFreq, secondsWidth, nanoWidth, initialTime))
//  rtc.io.ocp <> io.ocp

  val tsuRx = Module(new MIITimestampUnit(64))
  tsuRx.io.miiChannel <> io.ethMacRX
  tsuRx.io.rtcTimestamp := rtc.io.ptpTimestamp

  val tsuTx = Module(new MIITimestampUnit(64))
  tsuTx.io.miiChannel <> io.ethMacTX
  tsuTx.io.rtcTimestamp := rtc.io.ptpTimestamp

  // OCP
  val masterReg = Reg(next = io.ocp.M)
  rtc.io.ocp.M.Data := masterReg.Data
  rtc.io.ocp.M.ByteEn := masterReg.ByteEn
  rtc.io.ocp.M.Addr := masterReg.Addr
  tsuRx.io.ocp.M.Data := masterReg.Data
  tsuRx.io.ocp.M.ByteEn := masterReg.ByteEn
  tsuRx.io.ocp.M.Addr := masterReg.Addr
  tsuTx.io.ocp.M.Data := masterReg.Data
  tsuTx.io.ocp.M.ByteEn := masterReg.ByteEn
  tsuTx.io.ocp.M.Addr := masterReg.Addr
  // Arbitrate OCP master
  when(masterReg.Addr(11) === Bits("b0")){
    when(masterReg.Addr(10) === Bits("b0")) {
      tsuRx.io.ocp.M.Cmd := masterReg.Cmd //TsuRx Cmd
      tsuTx.io.ocp.M.Cmd := OcpCmd.IDLE
      rtc.io.ocp.M.Cmd := OcpCmd.IDLE
    }.otherwise{
      tsuRx.io.ocp.M.Cmd := OcpCmd.IDLE
      tsuTx.io.ocp.M.Cmd := masterReg.Cmd //TsuTx Cmd
      rtc.io.ocp.M.Cmd := OcpCmd.IDLE
    }
  }.otherwise{
    tsuRx.io.ocp.M.Cmd := OcpCmd.IDLE
    tsuTx.io.ocp.M.Cmd := OcpCmd.IDLE
    rtc.io.ocp.M.Cmd := masterReg.Cmd     //RTC   Cmd
  }
  // Arbitrate OCP slave based on response
  val replyTSURxReg = Reg(next = tsuRx.io.ocp.S)
  val replyTSUTxReg = Reg(next = tsuTx.io.ocp.S)
  val replyRTCReg = Reg(next = rtc.io.ocp.S)
  when(OcpResp.NULL =/= replyTSURxReg.Resp){
    io.ocp.S := replyTSURxReg            //TsuRx Resp
  }.elsewhen(OcpResp.NULL =/= replyTSUTxReg.Resp){
    io.ocp.S := replyTSUTxReg            //TsuTx Resp
  }.elsewhen(OcpResp.NULL =/= replyRTCReg.Resp){
    io.ocp.S := replyRTCReg              //RTC   Resp
  }.otherwise{
    io.ocp.S.Resp := OcpResp.NULL
    io.ocp.S.Data := 0.U
  }

  // IO
  io.ledPHY := tsuRx.io.listening | tsuTx.io.listening
  io.ledSOF := tsuRx.io.sofValid | tsuTx.io.sofValid
  io.ledEOF := tsuRx.io.eofValid | tsuTx.io.eofValid
  io.ledSFD := tsuRx.io.sfdValid | tsuTx.io.sfdValid

  // Interrupts
  io.intrs := false.B
  io.intrs(0) := rtc.io.periodIntr
  io.intrs(1) := tsuRx.io.ptpValid
  io.intrs(2) := tsuTx.io.ptpValid

  // [OPTIONAL] Hex Connectivity
  // Decode hardware
  def sevenSegBCDDecode(data : Bits, segmentPolarity: Int) : Bits = {
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

  val dispRegVec = RegInit(Vec.fill(8){Bits(0, width = 7)})

  dispRegVec(0) := sevenSegBCDDecode(rtc.io.ptpTimestamp(35, 32), segmentPolarity = 0)
  dispRegVec(1) := sevenSegBCDDecode(rtc.io.ptpTimestamp(39, 36), segmentPolarity = 0)
  dispRegVec(2) := sevenSegBCDDecode(rtc.io.ptpTimestamp(43, 40), segmentPolarity = 0)
  dispRegVec(3) := sevenSegBCDDecode(rtc.io.ptpTimestamp(47, 44), segmentPolarity = 0)
  dispRegVec(4) := sevenSegBCDDecode(rtc.io.ptpTimestamp(51, 48), segmentPolarity = 0)
  dispRegVec(5) := sevenSegBCDDecode(rtc.io.ptpTimestamp(55, 52), segmentPolarity = 0)
  dispRegVec(6) := sevenSegBCDDecode(rtc.io.ptpTimestamp(59, 56), segmentPolarity = 0)
  dispRegVec(7) := sevenSegBCDDecode(rtc.io.ptpTimestamp(63, 60), segmentPolarity = 0)

  io.rtcHexDisp := dispRegVec

}

object PTP1588Assist {
  def main(args: Array[String]): Unit = {
    chiselMain(Array[String]("--backend", "v", "--targetDir", "generated"),
      () => Module(new PTP1588Assist(addrWidth=16, dataWidth=32, clockFreq = 80000000)))
  }
}