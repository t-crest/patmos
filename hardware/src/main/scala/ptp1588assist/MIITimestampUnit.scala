package ptp1588assist

import Chisel._
import ocp._
import patmos.Constants.{ADDR_WIDTH, DATA_WIDTH}

class MIITimestampUnit(timestampWidth: Int) extends Module {
  val io = new Bundle {
    val ocp = new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH)
    val miiChannel = Input(new MIIChannel())
    val rtcTimestamp = Bits(INPUT, width = timestampWidth)
    val timestampAvail = Bool(OUTPUT)
    val sfdValid = Bits(OUTPUT, width = 8)
    val sofValid = Bool(OUTPUT)
    val eofValid = Bool(OUTPUT)
    val ptpValid = Bool(OUTPUT)
    val ptpTimestamp = Bits(OUTPUT, width = timestampWidth)
  }

  // Constants
  val constSFD = Bits("h55555555555555D5")
  val constVLANt1 = Bits("h8100")
  val constVLANt2 = Bits("h88A8")
  val constVLANt3 = Bits("h9100")
  val constMPLSt1 = Bits("h8847")
  val constMPLSt2 = Bits("h8848")
  val constIPv4t = Bits("h0800")
  val constIPv6t = Bits("h86DD")
  val constPCFt = Bits("h891D")
  val constPTP2t = Bits("h88F7")
  val constPTP4t1 = Bits("h013F")
  val constPTP4t2 = Bits("h0140")
  val constPTPGeneralPort = 319.U
  val constPTPEventPort = 320.U
  val constPTPSyncType = Bits("h00")
  val constPTPFollowType = Bits("h08")
  val constPTPDlyReqType = Bits("h01")
  val constPTPDlyRplyType = Bits("h09")

  // Buffers
  val sfdReg = Reg(init = UInt(0, width = 8))
  val srcMACReg = Reg(init = UInt(0, width = 48))
  val dstMACReg = Reg(init = UInt(0, width = 48))
  val ethTypeReg = Reg(init = UInt(0, width = 16))
  val udpDstPortReg = Reg(init = UInt(0, width = 16))
  val udpSrcPortReg = Reg(init = UInt(0, width = 16))
  val ptpMsgTypeReg = Reg(init = UInt(0, width = 8))
  val tempTimestampReg = Reg(init = UInt(0, width = timestampWidth))
  val rtcTimestampReg = Reg(init = UInt(0, width = timestampWidth))

  // Status registers
  val isVLANFrameReg = Reg(init = Bool(false))
  val isIPFrameReg = Reg(init = Bool(false))
  val isUDPFrameReg = Reg(init = Bool(false))
  val isPTPFrameReg = Reg(init = Bool(false))
  val isPCFFrameReg = Reg(init = Bool(false))
  val sofReg = Reg(init = Bool(false))
  val eofReg = Reg(init = Bool(false))
  val bufClrReg = Reg(init = Bool(false))
  val timestampAvailReg = Reg(init = Bool(false))

  // Counters
  val byteCntReg = Reg(init = UInt(0, width = 11))

  // State machine
  val stCollectSFD :: stDstMAC :: stSrcMAC :: stTypeEth :: stIPhead :: stUDPhead :: stPCFHead :: stPTPhead :: stEOF :: Nil = Enum(UInt(), 9)
  val stateReg = Reg(init = stCollectSFD)

  // Sampling clock and line of MII
  val miiDvReg = Reg(next = io.miiChannel.dv)
  val miiDvReg2 = Reg(next = miiDvReg)
  val miiErrReg = Reg(next = io.miiChannel.err)
  val miiErrReg2 = Reg(next = miiErrReg)
  val lastMIIClk = Reg(next = io.miiChannel.clk)
  val miiClkReg = Reg(next = io.miiChannel.clk)
  val miiClkReg2 = Reg(next = miiClkReg)
  val miiDataReg = Reg(next = io.miiChannel.data)
  val miiDataReg2 = Reg(next = miiDataReg)
  // Flags
  val validPHYData = miiDvReg2 & ~miiErrReg2
  val risingMIIEdge = miiClkReg & ~miiClkReg2

  // Module
  val deserializePHYbyte = Module(new Deserializer(false, 4, 8))
  deserializePHYbyte.io.en := risingMIIEdge & validPHYData
  deserializePHYbyte.io.shiftIn := miiDataReg2
  deserializePHYbyte.io.clr := bufClrReg
  val byteReg = Reg(init = Bits(0, width = 8), next = deserializePHYbyte.io.shiftOut)
  val wrByteReg = Reg(next = deserializePHYbyte.io.done)

  val deserializePHYBuffer = Module(new Deserializer(true, 8, 64))
  deserializePHYBuffer.io.en := deserializePHYbyte.io.done
  deserializePHYBuffer.io.shiftIn := deserializePHYbyte.io.shiftOut
  deserializePHYBuffer.io.clr := bufClrReg
  val regBuffer = Reg(init = Bits(0, width = 64), next = deserializePHYBuffer.io.shiftOut)

  when(bufClrReg) {
    bufClrReg := false.B
  }

  when(wrByteReg) {
    byteCntReg := byteCntReg + 1.U
  }

  val sofDetect = Mux(regBuffer === constSFD ||
                      regBuffer === (constSFD & "h00FFFFFFFFFFFFFF".U), true.B, false.B)

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
        when((byteCntReg === 2.U && ~isVLANFrameReg) || (byteCntReg === 6.U && isVLANFrameReg)) {
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
  val masterReg = Reg(next = io.ocp.M)

  // Default response
  val dataReg = Reg(init = Bits(0, width = DATA_WIDTH))
  val respReg = Reg(init = OcpResp.NULL)
  respReg := OcpResp.NULL

  // Read response
  when(masterReg.Cmd === OcpCmd.RD) {
    respReg := OcpResp.DVA
    switch(masterReg.Addr(4, 0)) {
      is(Bits("h00")) {
        dataReg := rtcTimestampReg(DATA_WIDTH - 1, 0)
      }
      is(Bits("h04")) {
        dataReg := rtcTimestampReg(timestampWidth - 1, DATA_WIDTH)
      }
      is(Bits("h08")) {
        dataReg := timestampAvailReg // Cat(validPTPReg, ptpMsgTypeReg)
      }
    }
  }

  // Write response
  when(masterReg.Cmd === OcpCmd.WR) {
    switch(masterReg.Addr(4, 0)){
      is(Bits("h08")){
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
