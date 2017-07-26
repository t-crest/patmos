/*
   Copyright 2013 Technical University of Denmark, DTU Compute.
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
 * SD host controller in SPI mode.
 *
 * Authors: Max Rishoej Pedersen (MaxRishoej@gmail.com)
 *
 */

package io

import Chisel._
import Node._
import ocp._

object SDHostCtrl extends DeviceObject {
  def init(params: Map[String, String]) = {
  }

  def create(params: Map[String, String]) : SDHostCtrl = {
    Module(new SDHostCtrl())
  }

  trait Pins {
    val sDHostCtrlPins = new Bundle() {
      val sdClk = Bits(OUTPUT, 1);
      val sdCs = Bits(OUTPUT, 1);
      val sdDatOut = Bits(INPUT, 1); // Data Out on the card
      val sdDatIn = Bits(OUTPUT, 1); // Data In on the card
      val sdWp = Bits(INPUT, 1);
    }
  }
}

class SDHostCtrl() extends CoreDevice() {
  override val io = new CoreDeviceIO() with SDHostCtrl.Pins

  // Internals
  val enReg = Reg(Bool(false)) // Is the controller enabled?
  val bufPntReg = Reg(UInt(0, 4)) // Counts the bit being transmitted.
                                  // Must be large enough to contain buffer size.

  // Clock
  val DEFAULTCLKDIV = 100 // 80MHz Patmos -> 400kHz SCLK
  val clkDivReg = Reg(UInt(DEFAULTCLKDIV, 16))
  val clkCntReg = Reg(UInt(0, 16)) // Could be smaller
  val clkReg = Reg(Bool(true))

  // Buffer
  val bufOutReg = Reg(Bits(width = 8))
  val bufInReg = Reg(Bits(width = 8))

  // Settings register
  val csReg = Reg(Bool(false)) // CS pin

  // OCP
  val ocpDataReg = Reg(Bits(width = 32))
  val ocpRespReg = Reg(Bits(width = 2))
  ocpRespReg := OcpResp.NULL

  // Write to registers
  when(io.ocp.M.Cmd === OcpCmd.WR) {
    ocpDataReg := io.ocp.M.Data
    ocpRespReg := OcpResp.DVA

    switch(io.ocp.M.Addr(5,2)) {
      // Data is written
      is(Bits("b0000")) {
        bufOutReg := io.ocp.M.Data
        bufInReg := UInt(0)

        // Trigger transaction
        enReg := Bool(true)
        bufPntReg := UInt(8)
        clkCntReg := clkDivReg
      }

      // Write to CS register
      is(Bits("b0001")) {
        csReg := io.ocp.M.Data =/= UInt(0)
      }

      // Write to CKLDIV register
      is(Bits("b0011")) {
        clkDivReg := UInt(io.ocp.M.Data)
      }
    }
  }

  // Read from registers
  when(io.ocp.M.Cmd === OcpCmd.RD) {
    ocpRespReg := OcpResp.DVA

    switch(io.ocp.M.Addr(5,2)) {
      // Reading data. Doesn't trigger enable.
      is(Bits("b0000")) {
        ocpDataReg := bufInReg
      }

      // Allow reading enReg
      is(Bits("b0010")) {
        ocpDataReg := enReg
      }
    }
  }

  // Connections to master
  io.ocp.S.Resp := ocpRespReg
  io.ocp.S.Data := ocpDataReg

  // Connections to pins
  val bufIdx = bufPntReg - UInt(1) // For convenience

  when(enReg === Bool(true)) {
    when (clkCntReg === UInt(1)) {
      clkCntReg := clkDivReg
      clkReg := ~clkReg

      when(clkReg === Bool(true)) { // Falling edge -> Sample
        bufInReg(bufIdx) := io.sDHostCtrlPins.sdDatOut

        // Count clock cycles
        when (bufPntReg === UInt(1)) {
          enReg := Bool(false) // Transaction done
        }
        .otherwise {
          bufPntReg := bufPntReg - UInt(1)
        }
      }
    }
    .otherwise {
      clkCntReg := clkCntReg - UInt(1)
    }

    io.sDHostCtrlPins.sdClk := clkReg
  }
  .otherwise { // Not enabled
    io.sDHostCtrlPins.sdClk := Bool(false) 
    clkReg := Bool(false) // As to begin with a rising edge
  }

  io.sDHostCtrlPins.sdDatIn := bufOutReg(bufIdx)

  // Always
  io.sDHostCtrlPins.sdCs := csReg
} 
