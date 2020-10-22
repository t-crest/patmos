package ptp1588assist

import chisel3._
import chisel3.util._
import ocp._
import patmos.Constants.{ADDR_WIDTH, DATA_WIDTH}

class MIITimestampUnit(timestampWidth: Int) extends Module {
  val io = IO(new Bundle {
    val ocp = new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH)
    val miiChannel = Input(new MIIChannel())
    val rtcTimestamp = Input(Bits(timestampWidth.W))
    val timestampAvail = Output(Bool())
    val sfdValid = Output(Bits(8.W))
    val sofValid = Output(Bool())
    val eofValid = Output(Bool())
    val ptpTimestamp = Output(Bits(timestampWidth.W))
  })

  // Constants
  val constSFD = 0x55555555555555D5L.U(64.W)
  val constVLANt1 = 0x8100.U
  val constVLANt2 = 0x88A8.U
  val constVLANt3 = 0x9100.U
  val constMPLSt1 = 0x8847.U
  val constMPLSt2 = 0x8848.U
  val constIPv4t = 0x0800.U
  val constIPv6t = 0x86DD.U
  val constPCFt = 0x891D.U
  val constPTP2t = 0x88F7.U
  val constPTP4t1 = 0x013F.U
  val constPTP4t2 = 0x0140.U
  val constPTPGeneralPort = 319.U
  val constPTPEventPort = 320.U
  val constPTPSyncType = 0x00.U
  val constPTPFollowType = 0x08.U
  val constPTPDlyReqType = 0x01.U
  val constPTPDlyRplyType = 0x09.U

  // Buffers
  val sfdReg = RegInit(0.U(8.W))
  val srcMACReg = RegInit(0.U(48.W))
  val dstMACReg = RegInit(0.U(48.W))
  val ethTypeReg = RegInit(0.U(16.W))
  val udpDstPortReg = RegInit(0.U(16.W))
  val udpSrcPortReg = RegInit(0.U(16.W))
  val ptpMsgTypeReg = RegInit(0.U(8.W))
  val tempTimestampReg = RegInit(0.U(timestampWidth.W))
  val rtcTimestampReg = RegInit(0.U(timestampWidth.W))

  // Status registers
  val isVLANFrameReg = RegInit(false.B)
  val isIPFrameReg = RegInit(false.B)
  val isUDPFrameReg = RegInit(false.B)
  val isPTPFrameReg = RegInit(false.B)
  val isPCFFrameReg = RegInit(false.B)
  val sofReg = RegInit(false.B)
  val eofReg = RegInit(false.B)
  val bufClrReg = RegInit(false.B)
  val timestampAvailReg = RegInit(false.B)

  // Counters
  val byteCntReg = RegInit(0.U(11.W))

  // State machine
  val stCollectSFD :: stDstMAC :: stSrcMAC :: stTypeEth :: stIPhead :: stUDPhead :: stPCFHead :: stPTPhead :: stEOF :: Nil = Enum(9)
  val stateReg = RegInit(stCollectSFD)

  // Sampling clock and line of MII
  val miiDvReg = RegNext(io.miiChannel.dv)
  val miiDvReg2 = RegNext(miiDvReg)
  val miiErrReg = RegNext(io.miiChannel.err)
  val miiErrReg2 = RegNext(miiErrReg)
  val lastMIIClk = RegNext(io.miiChannel.clk)
  val miiClkReg = RegNext(io.miiChannel.clk)
  val miiClkReg2 = RegNext(miiClkReg)
  val miiDataReg = RegNext(io.miiChannel.data)
  val miiDataReg2 = RegNext(miiDataReg)
  // Flags
  val validPHYData = miiDvReg2 & !miiErrReg2
  val risingMIIEdge = miiClkReg & !miiClkReg2

  // Module
  val deserializePHYbyte = Module(new Deserializer(false, 4, 8))
  deserializePHYbyte.io.en := risingMIIEdge & validPHYData
  deserializePHYbyte.io.shiftIn := miiDataReg2
  deserializePHYbyte.io.clr := bufClrReg
  val byteReg = RegNext(next = deserializePHYbyte.io.shiftOut)
  val wrByteReg = RegNext(deserializePHYbyte.io.done)

  val deserializePHYBuffer = Module(new Deserializer(true, 8, 64))
  deserializePHYBuffer.io.en := deserializePHYbyte.io.done
  deserializePHYBuffer.io.shiftIn := deserializePHYbyte.io.shiftOut
  deserializePHYBuffer.io.clr := bufClrReg
  val regBuffer = RegNext(deserializePHYBuffer.io.shiftOut)

  when(bufClrReg) {
    bufClrReg := false.B
  }

  when(wrByteReg) {
    byteCntReg := byteCntReg + 1.U
  }

  val sofDetect = Mux(regBuffer === constSFD ||
                      regBuffer === (constSFD & 0x00FFFFFFFFFFFFFFL.U), true.B, false.B)

  switch(stateReg) {
    is(stCollectSFD) {
      when(sofDetect) { //Looking for SFD
        stateReg := stDstMAC
        sfdReg := regBuffer(7, 0)
        sofReg := true.B
        eofReg := false.B
        dstMACReg := 0.U
        srcMACReg := 0.U
        ethTypeReg := 0.U
        byteCntReg := 0.U
        isPTPFrameReg := false.B
        isVLANFrameReg := false.B
        isIPFrameReg := false.B
        isUDPFrameReg := false.B
        udpDstPortReg := 0.U
        udpSrcPortReg := 0.U
        bufClrReg := false.B
        tempTimestampReg := io.rtcTimestamp
        ptpMsgTypeReg := 0.U
        printf("[stCollectSFD]->[stDstMAC]\n")
      }
    }
    is(stDstMAC) {
      dstMACReg := regBuffer(47, 0)
      when(byteCntReg === 6.U) { //6 bytes
        stateReg := stSrcMAC
        byteCntReg := 0.U
        printf("[stDstMAC]->[stSrcMAC]\n")
      }
    }
    is(stSrcMAC) {
      srcMACReg := regBuffer(47, 0)
      when(byteCntReg === 6.U) { //6 bytes
        stateReg := stTypeEth
        byteCntReg := 0.U
        printf("[stSrcMAC]->[stTypeEth]\n")
      }
    }
    is(stTypeEth) {
      when(ethTypeReg =/= 0.U) {
        when(ethTypeReg === constVLANt1 || ethTypeReg === constVLANt2 || ethTypeReg === constVLANt3) {
          isVLANFrameReg := true.B
          ethTypeReg := 0.U
        }.elsewhen(ethTypeReg === constPTP2t || ethTypeReg === constPTP4t1 || ethTypeReg === constPTP4t2) {
          stateReg := stPTPhead
          isPTPFrameReg := true.B
          byteCntReg := 0.U
          printf("[stTypeEth]->[stPTPhead]\n")
        }.elsewhen(ethTypeReg === constIPv4t) {
          stateReg := stIPhead
          isIPFrameReg := true.B
          byteCntReg := 0.U
          printf("[stTypeEth]->[stIPhead]\n")
        }.elsewhen(ethTypeReg === constPCFt){
          stateReg := stPCFHead
          isPCFFrameReg := true.B
          byteCntReg := 0.U
          printf("[stTypeEth]->[stPCFHead]\n")
        }.otherwise {
          //Ignore rest of frame if not known, wait for new frame
          stateReg := stEOF
          byteCntReg := 0.U
          printf("[stTypeEth]->[stEOF]\n")
        }
      }.otherwise {
        when((byteCntReg === 2.U && !isVLANFrameReg) || (byteCntReg === 6.U && isVLANFrameReg)) {
          ethTypeReg := regBuffer(15, 0)
        }
      }
    }
    is(stIPhead) {
      when(byteCntReg === 20.U) { //20 byte IPv4 header
        when(isUDPFrameReg) {
          stateReg := stUDPhead
          byteCntReg := 0.U
          printf("[stIPhead]->[stUDPhead]\n")
        }.otherwise {
          //Ignore rest of frame if not PTP, wait for new frame
          stateReg := stEOF
          printf("[stIPhead]->[stEOF]\n")
        }
      }.elsewhen(byteCntReg === 10.U) {
        when(regBuffer(7, 0) === 17.U) {
          isUDPFrameReg := true.B
        }
      }
    }
    is(stUDPhead) {
      when(byteCntReg === 8.U) { //8 byte UDP header
        when(isPTPFrameReg) {
          stateReg := stPTPhead
          byteCntReg := 0.U
          printf("[stUDPhead]->[stPTPhead]\n")
        }.otherwise {
          //Ignore rest of frame if not PTP, wait for new frame
          stateReg := stEOF
          printf("[stUDPhead]->[stEOF]\n")
        }
      }.elsewhen(byteCntReg === 4.U) { //first 4 bytes are ports
        udpDstPortReg := regBuffer(31, 16)
        udpSrcPortReg := regBuffer(15, 0)
      }.elsewhen((udpDstPortReg === constPTPGeneralPort || udpDstPortReg === constPTPEventPort) &&
        (udpSrcPortReg === constPTPGeneralPort || udpSrcPortReg === constPTPEventPort)) {
        isPTPFrameReg := true.B
      }
    }
    is(stPCFHead) {
      //Timestamp the frame and ignore the rest
      rtcTimestampReg := tempTimestampReg
      timestampAvailReg := true.B
      stateReg := stEOF
      printf("[stPTPhead]->[stEOF]\n")
    }
    is(stPTPhead) {
      rtcTimestampReg := tempTimestampReg
      timestampAvailReg := true.B
      when(byteCntReg === 1.U) { //2 byte to get msgType
        ptpMsgTypeReg := regBuffer(7, 0)
      }.elsewhen(byteCntReg > 2.U) {
        when((ptpMsgTypeReg === constPTPDlyReqType && byteCntReg === 44.U) || (ptpMsgTypeReg =/= constPTPDlyReqType && byteCntReg === 34.U)) {
          //Timestamp the frame and ignore the rest
          stateReg := stEOF
          printf("[stPTPhead]->[stEOF]\n")
        }
      }
    }
    is(stEOF) {
      //Clear up everything before new frame
      sfdReg := 0.U
      sofReg := false.B
      byteCntReg := 0.U
      bufClrReg := true.B
      regBuffer := 0.U
      when(!validPHYData) {
        eofReg := true.B
        stateReg := stCollectSFD
        printf("[stEOF]->[stCollectSFD]\n")
      }
    }
  }

  // OCP Connectivity
  // Register command
  val masterReg = RegNext(io.ocp.M)

  // Default response
  val dataReg = RegInit(0.U(DATA_WIDTH.W))
  val respReg = RegInit(OcpResp.NULL)
  respReg := OcpResp.NULL

  // Read response
  when(masterReg.Cmd === OcpCmd.RD) {
    respReg := OcpResp.DVA
    switch(masterReg.Addr(4, 0)) {
      is(0x00.U) {
        dataReg := rtcTimestampReg(DATA_WIDTH - 1, 0)
      }
      is(0x04.U) {
        dataReg := rtcTimestampReg(timestampWidth - 1, DATA_WIDTH)
      }
      is(0x08.U) {
        dataReg := timestampAvailReg // Cat(validPTPReg, ptpMsgTypeReg)
      }
    }
  }

  // Write response
  when(masterReg.Cmd === OcpCmd.WR) {
    switch(masterReg.Addr(4, 0)){
      is(0x08.U){
        respReg := OcpResp.DVA
        timestampAvailReg := false.B
      }
    }
  }

  // OCP
  io.ocp.S.Data := dataReg
  io.ocp.S.Resp := respReg

  // IO Connectivity
  io.sfdValid := sfdReg
  io.sofValid := sofReg
  io.eofValid := eofReg
  io.timestampAvail := timestampAvailReg
  io.ptpTimestamp := rtcTimestampReg
}
