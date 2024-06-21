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

import chisel3._
import chisel3.util._
//import Node._ //cannot be imported with chisel3

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
      val pwmOut = Output(UInt(motorCount.W))
    }
  }
}

class BldcCtrl(clkFreq : Int, pwmFreq : Int, motorCount : Int) extends CoreDevice() {

  override val io = new CoreDeviceIO() with BldcCtrl.Pins

  val counterWidth = log2Up(clkFreq / pwmFreq)

  //val pwmOutReg = Reg(init = 0.U(motorCount.W))
  val counterReg = RegInit(init = 0.U(counterWidth.W))

  val pwmOut0Reg = RegInit(init = 0.U(1.W))
  val pwmOut1Reg = RegInit(init = 0.U(1.W))
  val pwmOut2Reg = RegInit(init = 0.U(1.W))
  val pwmOut3Reg = RegInit(init = 0.U(1.W))

  val motor0Reg = RegInit(init = 0.U(counterWidth.W))
  val motor1Reg = RegInit(init = 0.U(counterWidth.W))
  val motor2Reg = RegInit(init = 0.U(counterWidth.W))
  val motor3Reg = RegInit(init = 0.U(counterWidth.W))

  val motor0tmpReg = RegInit(init = 0.U(counterWidth.W))
  val motor1tmpReg = RegInit(init = 0.U(counterWidth.W))
  val motor2tmpReg = RegInit(init = 0.U(counterWidth.W))
  val motor3tmpReg = RegInit(init = 0.U(counterWidth.W))

  when (counterReg === (clkFreq / pwmFreq).U) {
    // update motor registers:
    motor0Reg := motor0tmpReg
    motor1Reg := motor1tmpReg
    motor2Reg := motor2tmpReg
    motor3Reg := motor3tmpReg
    counterReg := 0.U
  }
  .otherwise {
    counterReg := counterReg + 1.U
  }

  when (counterReg === 0.U) {
    // all outputs high:
    //pwmOutReg := ~0.U(motorCount.W)
    pwmOut0Reg := 1.U
    pwmOut1Reg := 1.U
    pwmOut2Reg := 1.U
    pwmOut3Reg := 1.U
  }
  when (counterReg === motor0Reg) {
    //pwmOutReg := pwmOutReg & (~1.U(motorCount.W))
    pwmOut0Reg := 0.U
  }
  when (counterReg === motor1Reg) {
    //pwmOutReg := pwmOutReg & (~2.U(motorCount.W))
    pwmOut1Reg := 0.U
  }
  when (counterReg === motor2Reg) {
    //pwmOutReg := pwmOutReg & (~4.U(motorCount.W))
    pwmOut2Reg := 0.U
  }
  when (counterReg === motor3Reg) {
    //pwmOutReg := pwmOutReg & (~8.U(motorCount.W))
    pwmOut3Reg := 0.U
  }

  // Default response
  val respReg = RegInit(init = OcpResp.NULL)
  val readReg = RegInit(init = 0.U(counterWidth.W))
  respReg := OcpResp.NULL
  readReg := 0.U

  // Read a register
  when(io.ocp.M.Cmd === OcpCmd.RD) {
    respReg := OcpResp.DVA
    switch(io.ocp.M.Addr(3,2)) {
      is("b00".U) {
        readReg := motor0tmpReg
      }
      is("b01".U) {
        readReg := motor1tmpReg
      }
      is("b10".U) {
        readReg := motor2tmpReg
      }
      is("b11".U) {
        readReg := motor3tmpReg
      }
    }
  }

  // Write a register
  when(io.ocp.M.Cmd === OcpCmd.WR) {
    respReg := OcpResp.DVA
    switch(io.ocp.M.Addr(3,2)){
      is("b00".U) {
        motor0tmpReg := io.ocp.M.Data(counterWidth - 1, 0)
      }
      is("b01".U) {
        motor1tmpReg := io.ocp.M.Data(counterWidth - 1, 0)
      }
      is("b10".U) {
        motor2tmpReg := io.ocp.M.Data(counterWidth - 1, 0)
      }
      is("b11".U) {
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
