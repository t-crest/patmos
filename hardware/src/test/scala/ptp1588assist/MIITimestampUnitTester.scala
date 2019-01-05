package ptp1588assist

import Chisel._
import sys.process._

class MIITimestampUnitTester(dut: MIITimestampUnit, ethernetFrame: EthernetFrame, testStages: Int, iterations: Int) extends Tester(dut){

  def testFrame(ethernetFrame: EthernetFrame, initTime : Long): Long ={
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
        time+=1
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
        time+=1
        poke(dut.io.rtcTimestamp, time)
      }
      step(1)
      expect(dut.byteCntReg, ethernetFrame.dstMacNibbles.length/2)
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
        time+=1
        poke(dut.io.rtcTimestamp, time)
      }
      step(1)
      expect(dut.byteCntReg, ethernetFrame.srcMacNibbles.length/2)
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
        time+=1
        poke(dut.io.rtcTimestamp, time)
      }
      step(1)
      expect(dut.byteCntReg, ethernetFrame.ethTypeNibbles.length/2)
      step(1)
      println("----------------ethernetFrame.ethType")
      peek(dut.regBuffer)
      if(ethernetFrame.ethType.sameElements(ethernetFrame.toBytes(0x80, 0x00))){
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
        time+=1
        poke(dut.io.rtcTimestamp, time)
      }
      step(1)
      expect(dut.byteCntReg, ethernetFrame.ipHeaderNibbles.length/2)
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
        time+=1
        poke(dut.io.rtcTimestamp, time)
      }
      step(1)
      expect(dut.byteCntReg, ethernetFrame.udpHeaderNibbles.length/2)
      step(1)
      println("----------------ethernetFrame.udpHeader")
      peek(dut.regBuffer)
      peek(dut.udpDstPortReg)
      peek(dut.udpSrcPortReg)
      println("----------------ethernetFrame.udpHeader")
    }
    step(1)
    if(testStages >= 7) {
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
        time+=1
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

  println("Test Starting...")
  var time : Long = 0x53C38A1000000000L
  for(i <- 0 until iterations) {
    println("...")
    println("TEST FRAME ITERATION #" + i + "at t = " + time.toHexString)
    if(i % 2 == 0) {
      time = testFrame(EthernetTesting.mockupPTPEthFrameOverIpUDP, time)
    } else if(i % 3 ==0) {
      time = testFrame(EthernetTesting.mockupPTPVLANFrameOverIpUDP, time)
    } else {
      time = testFrame(EthernetTesting.mockupPTPEthFrameDHCP, time)
    }
    println("END TEST FRAME ITERATION #"+ i + "at t = " + time.toHexString)
    time += 0x0000000100000000L
    step(1)
    step(1)
    println("...")
    println("...")
  }
}

object MIITimestampUnitTester {
  def main(args: Array[String]): Unit = {
    chiselMainTest(Array("--genHarness", "--test", "--backend", "c",
      "--compile", "--vcd", "--targetDir", "generated/"+this.getClass.getSimpleName.dropRight(1)),
      () => Module(new MIITimestampUnit(64))) {
      dut => new MIITimestampUnitTester(dut, ethernetFrame = EthernetTesting.mockupPTPEthFrameOverIpUDP, testStages = 7, iterations = 4)
    }
    "gtkwave generated/"++this.getClass.getSimpleName.dropRight(1)+"/"+"MIITimestampUnit.vcd" !
  }
}