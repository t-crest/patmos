package ptp1588assist

import chisel3._
import ocp._
import patmos.Constants._
import _root_.util.BCDToSevenSegDecoder

class PTP1588Assist(addrWidth: Int = ADDR_WIDTH, dataWidth: Int = DATA_WIDTH, clockFreq: Int = CLOCK_FREQ, secondsWidth: Int = 32, nanoWidth: Int = 32, initialTime: BigInt = 0L, ppsDuration: Int = 1000) extends Module {
  val io = IO(new Bundle {
    val ocp = new OcpCoreSlavePort(addrWidth, dataWidth)
    val ethMacRX = Input(new MIIChannel())
    val ethMacTX = Input(new MIIChannel())
    val rtcPPS = Output(Bool())
    val rtcHexDisp = Vec(8, Output(UInt(7.W)))
    val ledTS = Output(Bool())
    val ledSOF = Output(Bool())
    val ledEOF = Output(Bool())
    val ledSFD = Output(UInt(8.W))
  })

  // Connections
  val rtc = Module(new RTC(clockFreq, secondsWidth, nanoWidth, initialTime, ppsDuration))

  val tsuRx = Module(new MIITimestampUnit(secondsWidth + nanoWidth))
  tsuRx.io.miiChannel <> io.ethMacRX
  tsuRx.io.rtcTimestamp := rtc.io.ptpTimestamp

  val tsuTx = Module(new MIITimestampUnit(secondsWidth + nanoWidth))
  tsuTx.io.miiChannel <> io.ethMacTX
  tsuTx.io.rtcTimestamp := rtc.io.ptpTimestamp

  // OCP
  val masterReg = RegNext(io.ocp.M)
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
  when(!masterReg.Addr(11)) {
    when(!masterReg.Addr(10)) {
      tsuRx.io.ocp.M.Cmd := masterReg.Cmd //TsuRx Cmd
      tsuTx.io.ocp.M.Cmd := OcpCmd.IDLE
      rtc.io.ocp.M.Cmd := OcpCmd.IDLE
    }.otherwise {
      tsuRx.io.ocp.M.Cmd := OcpCmd.IDLE
      tsuTx.io.ocp.M.Cmd := masterReg.Cmd //TsuTx Cmd
      rtc.io.ocp.M.Cmd := OcpCmd.IDLE
    }
  }.otherwise {
    tsuRx.io.ocp.M.Cmd := OcpCmd.IDLE
    tsuTx.io.ocp.M.Cmd := OcpCmd.IDLE
    rtc.io.ocp.M.Cmd := masterReg.Cmd //RTC   Cmd
  }
  // Arbitrate OCP slave based on response
  val replyTSURxReg = RegNext(tsuRx.io.ocp.S)
  val replyTSUTxReg = RegNext(tsuTx.io.ocp.S)
  val replyRTCReg = RegNext(rtc.io.ocp.S)
  when(OcpResp.NULL =/= replyTSURxReg.Resp) {
    io.ocp.S := replyTSURxReg //TsuRx Resp
  }.elsewhen(OcpResp.NULL =/= replyTSUTxReg.Resp) {
    io.ocp.S := replyTSUTxReg //TsuTx Resp
  }.elsewhen(OcpResp.NULL =/= replyRTCReg.Resp) {
    io.ocp.S := replyRTCReg //RTC   Resp
  }.otherwise {
    io.ocp.S.Resp := OcpResp.NULL
    io.ocp.S.Data := 0.U
  }

  io.rtcPPS := rtc.io.pps

  // [OPTIONAL] Hex & Led Connectivity
  // Led connections
  val dispRegVec = RegInit(VecInit(Seq.fill(8)(0.U(7.W))))
  for (i <- 0 until 7) {
    val decoder = Module(new BCDToSevenSegDecoder).io
    decoder.bcdData := rtc.io.ptpTimestamp(39 + i * 4, 32 + i * 4)
    decoder.segPolarity := false.B
    dispRegVec(i) := decoder.segData
  }
  io.rtcHexDisp := dispRegVec
  io.ledTS := tsuRx.io.timestampAvail | tsuTx.io.timestampAvail
  io.ledSOF := tsuRx.io.sofValid | tsuTx.io.sofValid
  io.ledEOF := tsuRx.io.eofValid | tsuTx.io.eofValid
  io.ledSFD := tsuRx.io.sfdValid | tsuTx.io.sfdValid
}

object PTP1588Assist extends App {
  chisel3.Driver.execute(Array("--target-dir", "generated/PTP1588Assist"), () => new PTP1588Assist(addrWidth = 16, dataWidth = 32, clockFreq = 80000000))
}
