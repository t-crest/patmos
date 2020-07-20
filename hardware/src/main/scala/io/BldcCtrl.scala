/*
   Copyright 2020 TU Wien, Austria.
   All rights reserved.

   This file is part of the time-predictable VLIW processor Patmos.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

      1. Redistributions of source code must retain the above copyright notice,
         this list of conditions and the following disclaimer.

      2. Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
   NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   The views and conclusions contained in the software and documentation are
   those of the authors and should not be interpreted as representing official
   policies, either expressed or implied, of the copyright holder.
 */

/*
 * Simple I/O module for steering of BLDC motor controllers (via PWM output).
 *
 * Authors: Michael Platzer (TU Wien)
 */

package io

import Chisel._
import Node._

import patmos.Constants._

import ocp._

object BldcCtrl extends DeviceObject {
  var pwmFreq = 50
  var motorCount = 4

  def init(params: Map[String, String]) = {
    pwmFreq = getPosIntParam(params, "pwmFreq")
    motorCount = getPosIntParam(params, "motorCount")
  }

  def create(params: Map[String, String]) : BldcCtrl = {
    Module(new BldcCtrl(CLOCK_FREQ, pwmFreq, motorCount))
  }

  trait Pins {
    val bldcCtrlPins = new Bundle() {
      val pwmOut = Bits(OUTPUT, motorCount)
    }
  }
}

class BldcCtrl(clkFreq : Int, pwmFreq : Int, motorCount : Int) extends CoreDevice() {

  override val io = new CoreDeviceIO() with BldcCtrl.Pins

  val counterWidth = log2Up(clkFreq / pwmFreq)

  //val pwmOutReg = Reg(init = Bits(0, motorCount))
  val counterReg = Reg(init = UInt(0, counterWidth))

  val pwmOut0Reg = Reg(init = Bits(0, 1))
  val pwmOut1Reg = Reg(init = Bits(0, 1))
  val pwmOut2Reg = Reg(init = Bits(0, 1))
  val pwmOut3Reg = Reg(init = Bits(0, 1))

  val motor0Reg = Reg(init = UInt(0, counterWidth))
  val motor1Reg = Reg(init = UInt(0, counterWidth))
  val motor2Reg = Reg(init = UInt(0, counterWidth))
  val motor3Reg = Reg(init = UInt(0, counterWidth))

  val motor0tmpReg = Reg(init = UInt(0, counterWidth))
  val motor1tmpReg = Reg(init = UInt(0, counterWidth))
  val motor2tmpReg = Reg(init = UInt(0, counterWidth))
  val motor3tmpReg = Reg(init = UInt(0, counterWidth))

  when (counterReg === UInt(clkFreq / pwmFreq)) {
    // update motor registers:
    motor0Reg := motor0tmpReg
    motor1Reg := motor1tmpReg
    motor2Reg := motor2tmpReg
    motor3Reg := motor3tmpReg
    counterReg := UInt(0)
  }
  .otherwise {
    counterReg := counterReg + UInt(1)
  }

  when (counterReg === UInt(0)) {
    // all outputs high:
    //pwmOutReg := ~Bits(0, motorCount)
    pwmOut0Reg := Bits(1)
    pwmOut1Reg := Bits(1)
    pwmOut2Reg := Bits(1)
    pwmOut3Reg := Bits(1)
  }
  when (counterReg === motor0Reg) {
    //pwmOutReg := pwmOutReg & (~Bits(1, motorCount))
    pwmOut0Reg := Bits(0)
  }
  when (counterReg === motor1Reg) {
    //pwmOutReg := pwmOutReg & (~Bits(2, motorCount))
    pwmOut1Reg := Bits(0)
  }
  when (counterReg === motor2Reg) {
    //pwmOutReg := pwmOutReg & (~Bits(4, motorCount))
    pwmOut2Reg := Bits(0)
  }
  when (counterReg === motor3Reg) {
    //pwmOutReg := pwmOutReg & (~Bits(8, motorCount))
    pwmOut3Reg := Bits(0)
  }

  // Default response
  val respReg = Reg(init = OcpResp.NULL)
  val readReg = Reg(init = Bits(0, counterWidth))
  respReg := OcpResp.NULL
  readReg := Bits(0)

  // Read a register
  when(io.ocp.M.Cmd === OcpCmd.RD) {
    respReg := OcpResp.DVA
    switch(io.ocp.M.Addr(3,2)) {
      is(Bits("b00")) {
        readReg := motor0tmpReg
      }
      is(Bits("b01")) {
        readReg := motor1tmpReg
      }
      is(Bits("b10")) {
        readReg := motor2tmpReg
      }
      is(Bits("b11")) {
        readReg := motor3tmpReg
      }
    }
  }

  // Write a register
  when(io.ocp.M.Cmd === OcpCmd.WR) {
    respReg := OcpResp.DVA
    switch(io.ocp.M.Addr(3,2)){
      is(Bits("b00")) {
        motor0tmpReg := io.ocp.M.Data(counterWidth - 1, 0)
      }
      is(Bits("b01")) {
        motor1tmpReg := io.ocp.M.Data(counterWidth - 1, 0)
      }
      is(Bits("b10")) {
        motor2tmpReg := io.ocp.M.Data(counterWidth - 1, 0)
      }
      is(Bits("b11")) {
        motor3tmpReg := io.ocp.M.Data(counterWidth - 1, 0)
      }
    }
  }

  // Connections to master
  io.ocp.S.Resp := respReg
  io.ocp.S.Data := readReg

  // Connection to pins
  //io.bldcCtrlPins.pwmOut := pwmOutReg
  io.bldcCtrlPins.pwmOut := Cat(pwmOut3Reg, pwmOut2Reg, pwmOut1Reg, pwmOut0Reg)
}
