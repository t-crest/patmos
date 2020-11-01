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
 * I/O module for a I2C master with optional clock stretching.
 *
 * Authors: Michael Platzer (TU Wien)
 */

package io

import Chisel._
//import Node._ // cannot be imported with chisel3

import patmos.Constants._

import ocp._

object I2CMaster extends DeviceObject {
  var bitRate = -1

  def init(params: Map[String, String]) = {
    bitRate = getPosIntParam(params, "i2cBitRate")
  }

  def create(params: Map[String, String]) : I2CMaster = {
    Module(new I2CMaster(CLOCK_FREQ, bitRate))
  }

  //trait Intrs {
  //  val i2cMasterIntrs = Vec.fill(1) { Bool(OUTPUT) }
  //}
}

/*******************************************************************************
 * I2C Master device registers: 4 registers, each 8 bit wide (32-bit aligned)
 *
 * NOTE: Writing any register while the busy flag in the status register is set
 *       has no effect. Reading any register except the status register while
 *       the busy flag is set returns 0.
 *
 * Control register (+ 0x00), read and write:
 *  - Bit 0: Specifies whether the bus responds with ACK (0) or NACK (1) upon
 *           reading. This bit must be set before initiating the read where it
 *           shall be effective, e.g. before addressing for the first read.
 *  - Bit 1: Writing 1 to this bit issues a stop condition if the bus is
 *           connected to a device in write mode; always reads as 0.
 *  - Bit 2: Writing 1 to this bit enables clock stretching, i.e. the master
 *           reads back the clock signal after releasing it and waits until the
 *           clock signal goes high; thus a slave can stretch the clock cycle.
 *  - Bit 3-7: Unused, always read as 0.
 *
 * Status register (+ 0x04), read-only:
 *  - Bit 0: Busy flag: if set the bus is busy processing a request.
 *  - Bit 1: Connected flag: if set the bus is connected to a device.
 *  - Bit 2: RW flag: indicates whether the bus is currently connected in read
 *           mode (1) or in write mode (0); reads as 0 if not connected.
 *  - Bit 3: Contains the last acknowledge transmitted on the bus (either sent
 *           or received); only valid if the busy flag is cleared.
 *  - Bit 4: Abort flag: if set the last connection was aborted because the bus
 *           got a NACK upon addressing or writing.
 *  - Bit 5-7: Unused, always read as 0.
 *
 * Address register (+ 0x08), write-only (always reads as 0):
 * Writing this register attempts to connect the bus to a device. If the bus is
 * not connected a start condition is issued. If the bus is already connected
 * in write mode a repeated start is issued, canceling the current connection.
 * Writing this register has no effect if the bus is already connected to a
 * device in read mode.
 * If the bus successfully connects to a device in read mode, it immediately
 * starts reading the first byte.
 *  - Bit 0: Select read mode (1) or write mode(0).
 *  - Bit 1-7: Address of the device to connect to.
 *
 * Data register (+ 0x0C), read and write:
 * In read mode this register holds the data received on the bus. The first
 * byte is read immediately after a connection has been established. Reading
 * this register triggers reading of the next byte. Issue a stop condition
 * before reading this register if you do not want further bytes to be read.
 * Writing this register in read mode has no effect.
 * In write mode the data to be transmitted on the bus is written to this
 * register. If the bus is connected in write mode, writing this register
 * triggers the transmission of the data. Reading this register in write mode
 * returns 0.
 ******************************************************************************/
class I2CMaster(clkFreq : Int, bitRate : Int) extends CoreDevice() {

  override val io = new CoreDeviceIO() with patmos.HasPins { //with I2CMaster.Intrs
    override val pins = new Bundle() {
      val sdaI = Bits(INPUT, 1)
      val sdaO = Bits(OUTPUT, 1) // '0' when low, 'Z' when high
      val sclI = Bits(INPUT, 1)
      val sclO = Bits(OUTPUT, 1) // '0' when low, 'Z' when high
    }
  }

  // Control: instruct the bus to ...
  val ackBehavReg = Reg(init = Bits(0, 1))  // ... send ACK or NACK upon reading
  val stopReg = Reg(init = Bits(0, 1))      // ... issue a stop condition
  val enClkStrReg = Reg(init = Bits(0, 1))  // ... enable clock stretching

  val restartReg = Reg(init = Bits(0, 1)) // issue a restart condition

  // Status: indicates whether the bus is ...
  val busyReg = Reg(init = Bits(0, 1)) // ... busy reading or writing
  val connReg = Reg(init = Bits(0, 1)) // ... connected to a device
  val rwReg = Reg(init = Bits(0, 1))   // ... connected in read or write mode
  // ... and saves ...
  val ackReg = Reg(init = Bits(1, 1))  // ... the last acknowledge (sent or received)
  val abrtReg = Reg(init = Bits(0, 1)) // ... whether the last connection was aborted

  // Read and write buffers
  val writeBuf = Reg(init = Bits(0, 8))
  val readBuf = Reg(init = Bits(0, 8))

  // Default response
  val respReg = Reg(init = OcpResp.NULL)
  val readReg = Reg(init = Bits(0, width = 8))
  respReg := OcpResp.NULL
  readReg := Bits(0)

  // Read a register
  when(io.ocp.M.Cmd === OcpCmd.RD) {
    respReg := OcpResp.DVA
    switch(io.ocp.M.Addr(3,2)) {
      // control register
      is(Bits("b00")) {
        readReg := Cat(Bits(0, width = 5), enClkStrReg, Bits(0, width = 2))
      }
      // status register
      is(Bits("b01")) {
        readReg := Cat(Bits(0, width = 3), abrtReg, ackReg, rwReg, connReg, busyReg)
      }
      // data register
      is(Bits("b11")) {
        when(busyReg === Bits(0) && rwReg === Bits(1)) {
          readReg := readBuf
          busyReg := connReg // read the next byte if the bus is still connected
        }
      }
    }
  }

  // Write a register
  when(io.ocp.M.Cmd === OcpCmd.WR) {
    respReg := OcpResp.DVA
    switch(io.ocp.M.Addr(3,2)){
      // control register
      is(Bits("b00")) {
        when (busyReg === Bits(0)) {
          ackBehavReg := io.ocp.M.Data(0)
          enClkStrReg := io.ocp.M.Data(2)
          when (io.ocp.M.Data(1) === Bits(1) && connReg === Bits(1) && rwReg === Bits(0)) {
            stopReg := Bits(1) // request to stop transmission
            busyReg := Bits(1)
          }
        }
      }
      // address register
      is(Bits("b10")) {
        when (busyReg === Bits(0) && (connReg === Bits(0) || rwReg === Bits(0))) {
          writeBuf := io.ocp.M.Data(7,0)
          restartReg := connReg // restart if there is already a connection
          busyReg := Bits(1)
        }
      }
      // data register
      is(Bits("b11")) {
        when (busyReg === Bits(0) && connReg === Bits(1) && rwReg === Bits(0)) {
          writeBuf := io.ocp.M.Data(7,0)
          busyReg := Bits(1)
        }
      }
    }
  }

  // Connections to master
  io.ocp.S.Resp := respReg
  io.ocp.S.Data := readReg

  // SDA and SCL
  val sdaIReg = Reg(Bits(width = 1))
  val sdaOReg = Reg(init = Bits(1, 1))
  val sclIReg = Reg(Bits(width = 1))
  val sclOReg = Reg(init = Bits(1, 1))

  //val sclActive = Reg(init = Bits(0, 1))
  val sclRateCounter = Reg(init = UInt(clkFreq / bitRate, log2Up(clkFreq / bitRate)))

  val busIdle :: busReading :: busWriting :: busStopping :: Nil = Enum(UInt(), 4)
  val busState = Reg(init = busIdle)
  val busRWCounter = Reg(init = UInt(0, 4))

  //val sdaTick = Reg(init = Bits(0, 1))
  //sdaTick := Bits(0)

  when (busState === busIdle && busyReg === Bits(1)) {
    when (connReg === Bits(0)) {
      busState := busWriting
      rwReg := writeBuf(0)
      sdaOReg := Bits(0) // issue start condition
      sclRateCounter := UInt(0)
      busRWCounter := UInt(0)
    }
    .elsewhen (restartReg === Bits(1)) {
      busState := busStopping
      sclRateCounter := UInt(0)
    }
    .elsewhen (stopReg === Bits(1)) {
      busState := busStopping
      sdaOReg := Bits(0) // pull SDA low to prepare for stop condition
      sclRateCounter := UInt(0)
    }
    .otherwise {
      busState := Mux(rwReg === Bits(0), busWriting, busReading)
      busRWCounter := UInt(0)
    }
  }

  when (sclRateCounter === UInt(clkFreq / bitRate)) {
    when (busState === busWriting) {
      // shift bits and fill up with 1 (to leave SDA high after sending)
      writeBuf := Cat(writeBuf(6,0), Bits(1))
      sdaOReg := writeBuf(7)
      busRWCounter := busRWCounter + UInt(1)

      when (busRWCounter === UInt(9)) {
        when (ackReg === Bits(0)) {
          abrtReg := Bits(0)
          when (rwReg === Bits(0)) {
            busState := busIdle
            busyReg := Bits(0)
            connReg := Bits(1)
          } .otherwise {
            busState := busReading
            busyReg := Bits(1)
            connReg := Bits(1)
            busRWCounter := UInt(0)
          }
        } .otherwise {
          //busState := busIdle
          //busyReg := Bits(0)
          //connReg := Bits(0)
          // missing acknowledge, aborting by issuing stop condition
          abrtReg := Bits(1)
          stopReg := Bits(1)
          busState := busStopping
          sdaOReg := Bits(0) // pull SDA low to prepare for stop condition
          sclRateCounter := UInt(0)
        }
      } .otherwise {
        sclRateCounter := UInt(0)
      }
    }

    when (busState === busReading) {
      busRWCounter := busRWCounter + UInt(1)

      when (busRWCounter === UInt(8)) {
        sdaOReg := ackBehavReg
      }

      when (busRWCounter === UInt(9)) {
        when (ackBehavReg === Bits(0)) {
          sdaOReg := Bits(1)
          busState := busIdle
          busyReg := Bits(0)
          connReg := Bits(1)
        } .otherwise {
          // we just sent a NACK, issue stop condition
          stopReg := Bits(1)
          busState := busStopping
          sdaOReg := Bits(0) // pull SDA low to prepare for stop condition
          sclRateCounter := UInt(0)
        }
      }
      .otherwise {
        sclRateCounter := UInt(0)
      }
    }

    when (busState === busStopping) {
      busState := busIdle
      busyReg := Bits(0)
      connReg := Bits(0)
    }
  }
  .otherwise {
    // clock stretching (counter stops if sclOReg high but sclIReg still low)
    when (enClkStrReg === Bits(0) || sclOReg === Bits(0) || sclIReg === Bits(1)) {
      sclRateCounter := sclRateCounter + UInt(1)
    }

    when (sclRateCounter === UInt(clkFreq / (bitRate * 4))) {
      sclOReg := Bits(1)
    }
    .elsewhen (sclRateCounter === UInt((clkFreq * 3) / (bitRate * 4))) {
      when (busState === busStopping) {
        when (restartReg === Bits(1)) {
          restartReg := Bits(0)
          connReg := Bits(0)
          busState := busWriting
          rwReg := writeBuf(0)
          sdaOReg := Bits(0) // issue start condition
          sclRateCounter := UInt(0)
          busRWCounter := UInt(0)
        }
        .elsewhen (stopReg === Bits(1)) {
          stopReg := Bits(0)
          sdaOReg := Bits(1) // issue stop condition
          sclRateCounter := UInt(0) // wait at least one cycle before next start
        }
      } .otherwise {
        sclOReg := Bits(0)
      }

      when (busState === busWriting) {
        ackReg := sdaIReg
      }

      when (busState === busReading) {
        ackReg := sdaIReg
        readBuf := Cat(readBuf(6,0), ackReg)
      }
    }
  }

  // Connections to pins
  sdaIReg := io.pins.sdaI
  io.pins.sdaO := sdaOReg
  sclIReg := io.pins.sclI
  io.pins.sclO := sclOReg
}
