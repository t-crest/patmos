/*package ptp1588assist

import chisel3._
import ocp.{OcpCmd, OcpResp, OcpTestMain}

import sys.process._
import scala.language.postfixOps
/*commented out Chisel3 tester has changed see https://github.com/schoeberl/chisel-examples/blob/master/TowardsChisel3.md */
class MIITimestampUnitTester(dut: MIITimestampUnit, testStages: Int, iterations: Int) extends Tester(dut) {

  def testPTPFrame(ethernetFrame: EthernetFrame, initTime: Long): Long = {
    var time = initTime
    peek(dut.sofReg)
    peek(dut.eofReg)
    peek(dut.sfdReg)
    poke(dut.io.rtcTimestamp, time)
    if (testStages >= 1) {
      println("(Testing preamble)")
      expect(dut.stateReg, 0x0)
      expect(dut.byteCntReg, 0x0)
      expect(dut.regBuffer, 0x0)
      expect(dut.bufClrReg, false)
      poke(dut.io.miiChannel.dv, true)
      poke(dut.io.miiChannel.clk, 0)
      step(1)
      for (nibble <- ethernetFrame.preambleNibbles) {
        poke(dut.io.miiChannel.data, nibble)
        poke(dut.io.miiChannel.clk, 1)
        step(1)
        peek(dut.deserializePHYbyte.io.en)
        peek(dut.deserializePHYbyte.io.shiftIn)
        peek(dut.regBuffer)
        poke(dut.io.miiChannel.clk, 0)
        step(1)
        peek(dut.deserializePHYBuffer.io.en)
        peek(dut.deserializePHYBuffer.io.shiftIn)
        //Increase time
        time += 1
        poke(dut.io.rtcTimestamp, time)
      }
      step(1)
      step(1)
      step(1)
      println("----------------ethernetFrame.preamble")
      expect(dut.io.sofValid, true)
      expect(dut.io.eofValid, false)
      expect(dut.io.sfdValid, 0xd5)
      peek(dut.regBuffer)
      println("----------------ethernetFrame.preamble")
    }
    step(1)
    //Frame continues with DEST MAC - SRC MAC - TYPE - PAYLOAD - CRC - IGP
    if (testStages >= 2) {
      println("(Testing destination MAC)")
      expect(dut.stateReg, 0x1)
      expect(dut.byteCntReg, 0x0)
      for (nibble <- ethernetFrame.dstMacNibbles) {
        poke(dut.io.miiChannel.data, nibble)
        poke(dut.io.miiChannel.clk, 1)
        step(1)
        peek(dut.deserializePHYbyte.io.en)
        peek(dut.deserializePHYbyte.io.shiftIn)
        peek(dut.regBuffer)
        poke(dut.io.miiChannel.clk, 0)
        step(1)
        peek(dut.deserializePHYBuffer.io.en)
        peek(dut.deserializePHYBuffer.io.shiftIn)
        //Increase time
        time += 1
        poke(dut.io.rtcTimestamp, time)
      }
      step(1)
      expect(dut.byteCntReg, ethernetFrame.dstMacNibbles.length / 2)
      step(1)
      println("----------------ethernetFrame.dstMac")
      peek(dut.regBuffer)
      println("----------------ethernetFrame.dstMac")
    }
    step(1)
    if (testStages >= 3) {
      println("(Testing source MAC)")
      expect(dut.stateReg, 0x2)
      expect(dut.byteCntReg, 0x0)
      for (nibble <- ethernetFrame.srcMacNibbles) {
        poke(dut.io.miiChannel.data, nibble)
        poke(dut.io.miiChannel.clk, 1)
        step(1)
        peek(dut.deserializePHYbyte.io.en)
        peek(dut.deserializePHYbyte.io.shiftIn)
        peek(dut.regBuffer)
        poke(dut.io.miiChannel.clk, 0)
        step(1)
        peek(dut.deserializePHYBuffer.io.en)
        peek(dut.deserializePHYBuffer.io.shiftIn)
        //Increase time
        time += 1
        poke(dut.io.rtcTimestamp, time)
      }
      step(1)
      expect(dut.byteCntReg, ethernetFrame.srcMacNibbles.length / 2)
      step(1)
      println("----------------ethernetFrame.srcMac")
      peek(dut.regBuffer)
      println("----------------ethernetFrame.srcMac")
    }
    step(1)
    if (testStages >= 4) {
      println("(Testing Ethernet Type or Length)")
      expect(dut.stateReg, 0x3)
      expect(dut.byteCntReg, 0x0)
      for (nibble <- ethernetFrame.ethTypeNibbles) {
        poke(dut.io.miiChannel.data, nibble)
        poke(dut.io.miiChannel.clk, 1)
        step(1)
        peek(dut.deserializePHYbyte.io.en)
        peek(dut.deserializePHYbyte.io.shiftIn)
        peek(dut.regBuffer)
        poke(dut.io.miiChannel.clk, 0)
        step(1)
        peek(dut.deserializePHYBuffer.io.en)
        peek(dut.deserializePHYBuffer.io.shiftIn)
        //Increase time
        time += 1
        poke(dut.io.rtcTimestamp, time)
      }
      step(1)
      expect(dut.byteCntReg, ethernetFrame.ethTypeNibbles.length / 2)
      step(1)
      println("----------------ethernetFrame.ethType")
      peek(dut.regBuffer)
      if (ethernetFrame.ethType.sameElements(ethernetFrame.toBytes(0x80, 0x00))) {
        expect(dut.isIPFrameReg, true)
        expect(dut.isPTPFrameReg, false)
      }
      peek(dut.isVLANFrameReg)
      println("----------------ethernetFrame.ethType")

    }
    step(1)
    if (testStages >= 5) {
      println("(Testing IP Header)")
      expect(dut.stateReg, 0x4)
      expect(dut.byteCntReg, 0x0)
      for (nibble <- ethernetFrame.ipHeaderNibbles) {
        poke(dut.io.miiChannel.data, nibble)
        poke(dut.io.miiChannel.clk, 1)
        step(1)
        peek(dut.deserializePHYbyte.io.en)
        peek(dut.deserializePHYbyte.io.shiftIn)
        peek(dut.regBuffer)
        poke(dut.io.miiChannel.clk, 0)
        step(1)
        peek(dut.deserializePHYBuffer.io.en)
        peek(dut.deserializePHYBuffer.io.shiftIn)
        peek(dut.byteCntReg)
        //Increase time
        time += 1
        poke(dut.io.rtcTimestamp, time)
      }
      step(1)
      expect(dut.byteCntReg, ethernetFrame.ipHeaderNibbles.length / 2)
      step(1)
      println("----------------ethernetFrame.ipHeader")
      peek(dut.regBuffer)
      peek(dut.isUDPFrameReg)
      println("----------------ethernetFrame.ipHeader")
    }
    step(1)
    if (testStages >= 6) {
      println("(Testing UDP Header)")
      expect(dut.stateReg, 0x5)
      expect(dut.byteCntReg, 0x0)
      for (nibble <- ethernetFrame.udpHeaderNibbles) {
        poke(dut.io.miiChannel.data, nibble)
        poke(dut.io.miiChannel.clk, 1)
        step(1)
        peek(dut.deserializePHYbyte.io.en)
        peek(dut.deserializePHYbyte.io.shiftIn)
        peek(dut.regBuffer)
        poke(dut.io.miiChannel.clk, 0)
        step(1)
        peek(dut.deserializePHYBuffer.io.en)
        peek(dut.deserializePHYBuffer.io.shiftIn)
        peek(dut.byteCntReg)
        //Increase time
        time += 1
        poke(dut.io.rtcTimestamp, time)
      }
      step(1)
      expect(dut.byteCntReg, ethernetFrame.udpHeaderNibbles.length / 2)
      step(1)
      println("----------------ethernetFrame.udpHeader")
      peek(dut.regBuffer)
      peek(dut.udpDstPortReg)
      peek(dut.udpSrcPortReg)
      println("----------------ethernetFrame.udpHeader")
    }
    step(1)
    if (testStages >= 7) {
      println("(Testing PTP Header)")
      expect(dut.stateReg, 0x6)
      expect(dut.byteCntReg, 0x0)
      for (nibble <- ethernetFrame.ptpHeaderNibbles) {
        poke(dut.io.miiChannel.data, nibble)
        poke(dut.io.miiChannel.clk, 1)
        step(1)
        peek(dut.deserializePHYbyte.io.en)
        peek(dut.deserializePHYbyte.io.shiftIn)
        peek(dut.regBuffer)
        poke(dut.io.miiChannel.clk, 0)
        step(1)
        peek(dut.deserializePHYBuffer.io.en)
        peek(dut.deserializePHYBuffer.io.shiftIn)
        peek(dut.byteCntReg)
        //Increase time
        time += 1
        poke(dut.io.rtcTimestamp, time)
      }
      step(1)
      step(1)
      println("----------------ethernetFrame.ptpHeader")
      peek(dut.regBuffer)
      peek(dut.ptpMsgTypeReg)
      expect(dut.bufClrReg, true)
      println("----------------ethernetFrame.ptpHeader")
    }
    time
  }

  def testEthernetFrame(ethFrame: EthernetFrame, initTime: Long): Long = {
    var time = initTime
    step(2)
    peek(dut.sofReg)
    peek(dut.eofReg)
    peek(dut.sfdReg)
    poke(dut.io.rtcTimestamp, time)
    expect(dut.stateReg, 0x0)
    expect(dut.byteCntReg, 0x0)
    expect(dut.regBuffer, 0x0)
    expect(dut.bufClrReg, false)
    poke(dut.io.miiChannel.dv, true)
    poke(dut.io.miiChannel.clk, 0)
    for (nibble <- ethFrame.preambleNibbles ++ ethFrame.dstMacNibbles ++ ethFrame.srcMacNibbles ++ ethFrame.ethTypeNibbles ++ ethFrame.rawDataNibbles) {
      poke(dut.io.miiChannel.data, nibble)
      poke(dut.io.miiChannel.clk, 1)
      step(1)
      peek(dut.deserializePHYbyte.io.en)
      peek(dut.deserializePHYbyte.io.shiftIn)
      peek(dut.regBuffer)
      poke(dut.io.miiChannel.clk, 0)
      step(1)
      peek(dut.deserializePHYBuffer.io.en)
      peek(dut.deserializePHYBuffer.io.shiftIn)
      //Increase time
      time += 1
      poke(dut.io.rtcTimestamp, time)
    }
    step(2)
    poke(dut.io.miiChannel.dv, false)
    step(2)
    time
  }

  def simplePCFPokingTest(initTime: Long): Long = {
    var time = initTime

    val exampleCapturePCF = Array[Int](
      0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0xD,
      0xB, 0xA, 0xD, 0xA, 0xA, 0xB, 0xE, 0xB,
      0xF, 0x0, 0xE, 0xC,
      0x0, 0x0, 0x1, 0x1, 0x2, 0x2, 0x3, 0x3, 0x4, 0x4, 0x5, 0x5,
      0x9, 0x8, 0xD, 0x1,
      0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
      0x6, 0x5,
      0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
      0x3,
      0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
      0x1, 0x0, 0x1, 0x0,
      0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
      0xB, 0x1, 0x0, 0xB,
      0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
      0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
      0xB, 0xE, 0xF, 0x0, 0x2, 0xA, 0xC, 0x4
    )

    step(2)
    peek(dut.sofReg)
    peek(dut.eofReg)
    peek(dut.sfdReg)
    poke(dut.io.rtcTimestamp, time)
    expect(dut.stateReg, 0x0)
    expect(dut.byteCntReg, 0x0)
    expect(dut.regBuffer, 0x0)
    expect(dut.bufClrReg, false)
    poke(dut.io.miiChannel.dv, true)
    poke(dut.io.miiChannel.clk, 0)

    for (nibble <- exampleCapturePCF) {
      poke(dut.io.miiChannel.data, nibble)
      poke(dut.io.miiChannel.clk, 1)
      step(1)
      peek(dut.deserializePHYbyte.io.en)
      peek(dut.deserializePHYbyte.io.shiftIn)
      peek(dut.regBuffer)
      poke(dut.io.miiChannel.clk, 0)
      step(1)
      peek(dut.deserializePHYBuffer.io.en)
      peek(dut.deserializePHYBuffer.io.shiftIn)
      //Increase time
      time += 1
      poke(dut.io.rtcTimestamp, time)
    }

    step(2)
    poke(dut.io.miiChannel.dv, false)
    step(2)
    time
  }

  def testTimestampReadout(): Unit = {
    //Read timestamp
    poke(dut.io.ocp.M.Addr, 0xF00D0000+0xE000)
    poke(dut.io.ocp.M.Cmd, OcpCmd.RD.litValue())
    step(2)
    peek(dut.io.ocp.S.Data)
    expect(dut.io.ocp.S.Resp, OcpResp.DVA.litValue())
    //Reset ocp bus
    poke(dut.io.ocp.M.Addr, Int.MaxValue)
    poke(dut.io.ocp.M.Cmd, OcpCmd.IDLE.litValue())
    step(2)
    expect(dut.io.ocp.S.Resp, OcpResp.NULL.litValue())
    //Clear timestampAvail flag
    poke(dut.io.ocp.M.Addr, 0xF00D0000+0xE008)
    poke(dut.io.ocp.M.Cmd, OcpCmd.WR.litValue())
    poke(dut.io.ocp.M.Data, 0x1)
    step(2)
    peek(dut.io.ocp.S.Data)
    expect(dut.timestampAvailReg, false)
    //Reset ocp bus
    poke(dut.io.ocp.M.Addr, Int.MaxValue)
    poke(dut.io.ocp.M.Cmd, OcpCmd.IDLE.litValue())
    step(2)
    expect(dut.io.ocp.S.Resp, OcpResp.NULL.litValue())
  }

  println("Test Starting...")
  var time: Long = 0x53C38A1FFFFFFF00L
  println("...")
  println("TEST FRAME #0 at t = " + time.toHexString)
  time = testEthernetFrame(EthernetTesting.mockupPTPEthFrameOverIpUDP, time)
  time += 0x0000010000000001L
  testTimestampReadout()
//  time = testEthernetFrame(EthernetTesting.mockupTTEPCFFrame, time)
//  time += 0x0000010000000001L
//  testTimestampReadout()
//  time += 0x0000010000000001L
  time = simplePCFPokingTest(time)
  testTimestampReadout()
  time += 0x0000010000000001L
  step(10)
  println("...")
}

object MIITimestampUnitTester extends App {
  private val pathToVCD = "generated/" + this.getClass.getSimpleName.dropRight(1)
  private val nameOfVCD = this.getClass.getSimpleName.dropRight(7) + ".vcd"

  try {
    chiselMainTest(Array("--genHarness", "--test", "--backend", "c",
      "--compile", "--vcd", "--targetDir", "generated/" + this.getClass.getSimpleName.dropRight(1)),
      () => Module(new MIITimestampUnit(64))) {
      dut => new MIITimestampUnitTester(dut, testStages = 7, iterations = 1)
    }
  } finally {
    "gtkwave " + pathToVCD + "/" + nameOfVCD + " " + pathToVCD + "/" + "view.sav" !
  }
}*/