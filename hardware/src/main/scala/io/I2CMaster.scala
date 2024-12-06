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

import chisel3._
import chisel3.util._

import patmos.Constants._

import ocp._

object I2CMaster extends DeviceObject {
  var bitRate = -1

  def init(params: Map[String, String]) = {
    bitRate = getPosIntParam(params, "i2cBitRate")
  }

  def create(params: Map[String, String]): I2CMaster = {
    Module(new I2CMaster(CLOCK_FREQ, bitRate))
  }
}

/** *****************************************************************************
 * I2C Master device registers: 4 registers, each 8 bit wide (32-bit aligned)
 *
 * NOTE: Writing any register while the busy flag in the status register is set
 * has no effect. Reading any register except the status register while
 * the busy flag is set returns 0.
 *
 * Control register (+ 0x00), read and write:
 *  - Bit 0: Specifies whether the bus responds with ACK (0) or NACK (1) upon
 *    reading. This bit must be set before initiating the read where it
 *    shall be effective, e.g. before addressing for the first read.
 *  - Bit 1: Writing 1 to this bit issues a stop condition if the bus is
 *    connected to a device in write mode; always reads as 0.
 *  - Bit 2: Writing 1 to this bit enables clock stretching, i.e. the master
 *    reads back the clock signal after releasing it and waits until the
 *    clock signal goes high; thus a slave can stretch the clock cycle.
 *  - Bit 3-7: Unused, always read as 0.
 *
 * Status register (+ 0x04), read-only:
 *  - Bit 0: Busy flag: if set the bus is busy processing a request.
 *  - Bit 1: Connected flag: if set the bus is connected to a device.
 *  - Bit 2: RW flag: indicates whether the bus is currently connected in read
 *    mode (1) or in write mode (0); reads as 0 if not connected.
 *  - Bit 3: Contains the last acknowledge transmitted on the bus (either sent
 *    or received); only valid if the busy flag is cleared.
 *  - Bit 4: Abort flag: if set the last connection was aborted because the bus
 *    got a NACK upon addressing or writing.
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
 * **************************************************************************** */
class I2CMaster(clkFreq: Int, bitRate: Int) extends CoreDevice() {

  override val io = IO(new CoreDeviceIO() with patmos.HasPins {
    val pins: Bundle {
      val sdaI: UInt
      val sdaO: UInt
      val sclI: UInt
      val sclO: UInt
    } = new Bundle {
      val sdaI = Input(UInt(1.W))
      val sdaO = Output(UInt(1.W)) // '0' when low, 'Z' when high
      val sclI = Input(UInt(1.W))
      val sclO = Output(UInt(1.W)) // '0' when low, 'Z' when high
    }
  })

  // Control: instruct the bus to ...
  val ackBehavReg = RegInit(init = 0.U(1.W)) // ... send ACK or NACK upon reading
  val stopReg = RegInit(init = 0.U(1.W)) // ... issue a stop condition
  val enClkStrReg = RegInit(init = 0.U(1.W)) // ... enable clock stretching

  val restartReg = RegInit(init = 0.U(1.W)) // issue a restart condition

  // Status: indicates whether the bus is ...
  val busyReg = RegInit(init = 0.U(1.W)) // ... busy reading or writing
  val connReg = RegInit(init = 0.U(1.W)) // ... connected to a device
  val rwReg = RegInit(init = 0.U(1.W)) // ... connected in read or write mode
  // ... and saves ...
  val ackReg = RegInit(init = 1.U(1.W)) // ... the last acknowledge (sent or received)
  val abrtReg = RegInit(init = 0.U(1.W)) // ... whether the last connection was aborted

  // Read and write buffers
  val writeBuf = RegInit(init = 0.U(8.W))
  val readBuf = RegInit(init = 0.U(8.W))

  // Default response
  val respReg = RegInit(init = OcpResp.NULL)
  val readReg = RegInit(init = 0.U(8.W))
  respReg := OcpResp.NULL
  readReg := 0.U

  // Read a register
  when(io.ocp.M.Cmd === OcpCmd.RD) {
    respReg := OcpResp.DVA
    switch(io.ocp.M.Addr(3, 2)) { // control register
      is("b00".U) {
        readReg := Cat(0.U(5.W), enClkStrReg, 0.U(2.W))
      } // status register
      is("b01".U) {
        readReg := Cat(0.U(3.W), abrtReg, ackReg, rwReg, connReg, busyReg)
      } // data register
      is("b11".U) {
        when(busyReg === 0.U && rwReg === 1.U) {
          readReg := readBuf
          busyReg := connReg // read the next byte if the bus is still connected
        }
      }
    }
  }

  // Write a register
  when(io.ocp.M.Cmd === OcpCmd.WR) {
    respReg := OcpResp.DVA
    switch(io.ocp.M.Addr(3, 2)) { // control register
      is("b00".U) {
        when(busyReg === 0.U) {
          ackBehavReg := io.ocp.M.Data(0)
          enClkStrReg := io.ocp.M.Data(2)
          when(io.ocp.M.Data(1) === 1.U && connReg === 1.U && rwReg === 0.U) {
            stopReg := 1.U // request to stop transmission
            busyReg := 1.U
          }
        }
      } // address register
      is("b10".U) {
        when(busyReg === 0.U && (connReg === 0.U || rwReg === 0.U)) {
          writeBuf := io.ocp.M.Data(7, 0)
          restartReg := connReg // restart if there is already a connection
          busyReg := 1.U
        }
      } // data register
      is("b11".U) {
        when(busyReg === 0.U && connReg === 1.U && rwReg === 0.U) {
          writeBuf := io.ocp.M.Data(7, 0)
          busyReg := 1.U
        }
      }
    }
  }

  // Connections to master
  io.ocp.S.Resp := respReg
  io.ocp.S.Data := readReg

  // SDA and SCL
  val sdaIReg = Reg(UInt(1.W))
  val sdaOReg = RegInit(init = 1.U(1.W))
  val sclIReg = Reg(UInt(1.W))
  val sclOReg = RegInit(init = 1.U(1.W))

  //val sclActive = RegInit(init = 0.U(1.W))
  val sclRateCounter = RegInit(init = (clkFreq / bitRate).U(log2Up(clkFreq / bitRate).W))

  val busIdle :: busReading :: busWriting :: busStopping :: Nil = Enum(4)
  val busState = RegInit(init = busIdle)
  val busRWCounter = RegInit(init = 0.U(4.W))

  //val sdaTick = RegInit(init = 0.U(1.W))
  //sdaTick := 0.U

  when(busState === busIdle && busyReg === 1.U) {
    when(connReg === 0.U) {
      busState := busWriting
      rwReg := writeBuf(0)
      sdaOReg := 0.U // issue start condition
      sclRateCounter := 0.U
      busRWCounter := 0.U
    }.elsewhen(restartReg === 1.U) {
      busState := busStopping
      sclRateCounter := 0.U
    }.elsewhen(stopReg === 1.U) {
      busState := busStopping
      sdaOReg := 0.U // pull SDA low to prepare for stop condition
      sclRateCounter := 0.U
    }.otherwise {
      busState := Mux(rwReg === 0.U, busWriting, busReading)
      busRWCounter := 0.U
    }
  }

  when(sclRateCounter === (clkFreq / bitRate).U) {
    when(busState === busWriting) { // shift bits and fill up with 1 (to leave SDA high after sending)
      writeBuf := Cat(writeBuf(6, 0), 1.U)
      sdaOReg := writeBuf(7)
      busRWCounter := busRWCounter + 1.U

      when(busRWCounter === 9.U) {
        when(ackReg === 0.U) {
          abrtReg := 0.U
          when(rwReg === 0.U) {
            busState := busIdle
            busyReg := 0.U
            connReg := 1.U
          }.otherwise {
            busState := busReading
            busyReg := 1.U
            connReg := 1.U
            busRWCounter := 0.U
          }
        }.otherwise { //busState := busIdle
          //busyReg := 0.U
          //connReg := 0.U
          // missing acknowledge, aborting by issuing stop condition
          abrtReg := 1.U
          stopReg := 1.U
          busState := busStopping
          sdaOReg := 0.U // pull SDA low to prepare for stop condition
          sclRateCounter := 0.U
        }
      }.otherwise {
        sclRateCounter := 0.U
      }
    }

    when(busState === busReading) {
      busRWCounter := busRWCounter + 1.U

      when(busRWCounter === 8.U) {
        sdaOReg := ackBehavReg
      }

      when(busRWCounter === 9.U) {
        when(ackBehavReg === 0.U) {
          sdaOReg := 1.U
          busState := busIdle
          busyReg := 0.U
          connReg := 1.U
        }.otherwise { // we just sent a NACK, issue stop condition
          stopReg := 1.U
          busState := busStopping
          sdaOReg := 0.U // pull SDA low to prepare for stop condition
          sclRateCounter := 0.U
        }
      }.otherwise {
        sclRateCounter := 0.U
      }
    }

    when(busState === busStopping) {
      busState := busIdle
      busyReg := 0.U
      connReg := 0.U
    }
  }.otherwise { // clock stretching (counter stops if sclOReg high but sclIReg still low)
    when(enClkStrReg === 0.U || sclOReg === 0.U || sclIReg === 1.U) {
      sclRateCounter := sclRateCounter + 1.U
    }

    when(sclRateCounter === (clkFreq / (bitRate * 4)).U) {
      sclOReg := 1.U
    }.elsewhen(sclRateCounter === ((clkFreq * 3) / (bitRate * 4)).U) {
      when(busState === busStopping) {
        when(restartReg === 1.U) {
          restartReg := 0.U
          connReg := 0.U
          busState := busWriting
          rwReg := writeBuf(0)
          sdaOReg := 0.U // issue start condition
          sclRateCounter := 0.U
          busRWCounter := 0.U
        }.elsewhen(stopReg === 1.U) {
          stopReg := 0.U
          sdaOReg := 1.U // issue stop condition
          sclRateCounter := 0.U // wait at least one cycle before next start
        }
      }.otherwise {
        sclOReg := 0.U
      }

      when(busState === busWriting) {
        ackReg := sdaIReg
      }

      when(busState === busReading) {
        ackReg := sdaIReg
        readBuf := Cat(readBuf(6, 0), ackReg)
      }
    }
  }

  // Connections to pins
  sdaIReg := io.pins.sdaI
  io.pins.sdaO := sdaOReg
  sclIReg := io.pins.sclI
  io.pins.sclO := sclOReg
}
