/*
 * SD host controller in SPI mode.
 *
 * Authors: Max Rishoej Pedersen (MaxRishoej@gmail.com)
 *
 */

package io

import Chisel._
import ocp._

object SDHostCtrl extends DeviceObject {
  def init(params: Map[String, String]) = {
  }

  def create(params: Map[String, String]) : SDHostCtrl = {
    Module(new SDHostCtrl())
  }
}

class SDHostCtrl() extends CoreDevice() {
  override val io = new CoreDeviceIO() with patmos.HasPins {
    override val pins = new Bundle() {
      val sdClk = Bits(OUTPUT, 1);
      val sdCs = Bits(OUTPUT, 1);
      val sdDatOut = Bits(INPUT, 1); // Data Out on the card
      val sdDatIn = Bits(OUTPUT, 1); // Data In on the card
      val sdWp = Bits(INPUT, 1);
    }
  }

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
        clkDivReg := io.ocp.M.Data
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
        bufInReg(bufIdx) := io.pins.sdDatOut

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

    io.pins.sdClk := clkReg
  }
  .otherwise { // Not enabled
    io.pins.sdClk := Bool(false)
    clkReg := Bool(false) // As to begin with a rising edge
  }

  io.pins.sdDatIn := bufOutReg(bufIdx)

  // Always
  io.pins.sdCs := csReg
} 
