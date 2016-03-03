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
 * UART module in chisel.
 *
 * Author: Sahar Abbaspour (sabb@dtu.com)
 *
 */


package io

import Chisel._
import Node._

import patmos.Constants._

import ocp._

object Uart extends DeviceObject {
  var baudRate = -1
  var fifoDepth = -1

  def init(params: Map[String, String]) = {
    baudRate = getPosIntParam(params, "baudRate")
    fifoDepth = getPosIntParam(params, "fifoDepth")
  }

  def create(params: Map[String, String]) : Uart = {
    Module(new Uart(CLOCK_FREQ, baudRate, fifoDepth))
  }

  trait Pins {
    val uartPins = new Bundle() {
      val tx = Bits(OUTPUT, 1)
      val rx = Bits(INPUT, 1)
    }
  }
}

class Uart(clk_freq: Int, baud_rate: Int, fifoDepth: Int) extends CoreDevice() {

  override val io = new CoreDeviceIO() with Uart.Pins

    val c_tx_divider_val    = clk_freq/baud_rate
    val tx_baud_counter     = Reg(init = UInt(0, log2Up(clk_freq/baud_rate)))
    val tx_baud_tick        = Reg(init = UInt(0, 1))

    val tx_idle :: tx_send :: Nil  = Enum(UInt(), 2)
    val tx_state            = Reg(init = tx_idle)
    val tx_buff             = Reg(init = Bits(0, 10))
    val tx_reg              = Reg(init = Bits(1, 1))
    val tx_counter          = Reg(init = UInt(0, 4))

    val txQueue = Module(new Queue(Bits(width = 8), fifoDepth))
    txQueue.io.enq.bits     := io.ocp.M.Data(7, 0)
    txQueue.io.enq.valid    := Bool(false)
    txQueue.io.deq.ready    := Bool(false)

    val rxd_reg0            = Reg(init = Bits(1, 1))
    val rxd_reg1            = Reg(init = Bits(1, 1))
    val rxd_reg2            = Reg(init = Bits(1, 1))

    val rx_baud_counter     = Reg(init = UInt(0, log2Up(clk_freq/baud_rate)))
    val rx_baud_tick        = Reg(init = UInt(0, 1))
    val rx_enable           = Reg(init = Bool(false))

    val rx_buff             = Reg(init = Bits(0, 8))
    val rx_counter          = Reg(init = UInt(0, 3))
    val rx_idle  :: rx_start :: rx_receive_data :: rx_stop_bit :: Nil  = Enum(UInt(), 4)
    val rx_state            = Reg(init = rx_idle)

    val rxQueue = Module(new Queue(Bits(width = 8), fifoDepth))
    rxQueue.io.enq.bits     := rx_buff
    rxQueue.io.enq.valid    := Bool(false)
    rxQueue.io.deq.ready    := Bool(false)

    // Default response and data
    val respReg = Reg(init = OcpResp.NULL)
    respReg := OcpResp.NULL

    val rdDataReg = Reg(init = Bits(0, width = 8))
    rdDataReg := Mux(io.ocp.M.Addr(2) === Bits(0),
                     Cat(Bits(0, width = 6), rxQueue.io.deq.valid, txQueue.io.enq.ready),
                     rxQueue.io.deq.bits)

    // Write to UART
    when (io.ocp.M.Cmd === OcpCmd.WR) {
        respReg := OcpResp.DVA
        txQueue.io.enq.bits := io.ocp.M.Data(7, 0)
        txQueue.io.enq.valid := Bool(true)
    }

    // Read data
    when(io.ocp.M.Cmd === OcpCmd.RD) {
        respReg := OcpResp.DVA
        rxQueue.io.deq.ready := io.ocp.M.Addr(2) =/= Bits(0)
    }

    // Connections to master
    io.ocp.S.Resp := respReg
    io.ocp.S.Data := rdDataReg

    // UART TX clk
    when (tx_baud_counter === UInt(clk_freq/baud_rate)){
        tx_baud_counter     := UInt(0)
        tx_baud_tick        := UInt(1)
    }
    .otherwise {
        tx_baud_counter     := tx_baud_counter + UInt(1)
        tx_baud_tick        := UInt(0)
    }

    // Send data

    when (tx_state === tx_idle) {
        when (txQueue.io.deq.valid) {
          txQueue.io.deq.ready := Bool(true)
          tx_buff              := Cat(Bits(1), txQueue.io.deq.bits, Bits(0))
          tx_state             := tx_send
        }
    }

    when (tx_state === tx_send) {
        when (tx_baud_tick === UInt(1)){
            tx_buff         := Cat (UInt(0), tx_buff (9, 1))
            tx_reg          := tx_buff(0)
            tx_counter      := Mux(tx_counter === UInt(10), UInt(0), tx_counter + UInt(1))

            when (tx_counter === UInt(10)) {
              when (txQueue.io.deq.valid) {
                txQueue.io.deq.ready := Bool(true)
                tx_buff              := Cat(Bits(1), txQueue.io.deq.bits)
                tx_reg               := UInt(0)
                tx_counter           := UInt(1)
              }
              .otherwise {
                tx_reg          := UInt(1)
                tx_counter      := UInt(0)
                tx_state        := tx_idle
              }
            }
        }
    }

    // Connect TX pin
    io.uartPins.tx := tx_reg


    // UART RX clk
    when (rx_enable) {
        when (rx_baud_counter === UInt(clk_freq/baud_rate)){
            rx_baud_counter     := UInt(0)
            rx_baud_tick        := UInt(1)
        }
        .otherwise {
            rx_baud_counter     := rx_baud_counter + UInt(1)
            rx_baud_tick        := UInt(0)
        }
    }


    // Receive data

    rxd_reg0                := io.uartPins.rx;
    rxd_reg1                := rxd_reg0;
    rxd_reg2                := rxd_reg1


    // RX shift in
    when (rx_state === rx_idle) {
        when (rxd_reg2 === UInt(0)){
           rx_state         := rx_start
           rx_baud_counter  := UInt(clk_freq/baud_rate) / UInt(2)
           rx_enable        := Bool(true)
        }
    }

    when (rx_state === rx_start){
        when (rx_baud_tick === UInt(1)) {
            when (rxd_reg2 === UInt(0)) {
                rx_state        := rx_receive_data
            }
            .otherwise{
                rx_state        := rx_idle
            }
        }
    }

    when (rx_state === rx_receive_data) {
        when (rx_baud_tick === UInt(1)){
            rx_state := Mux(rx_counter === UInt(7), rx_stop_bit, rx_receive_data)
            rx_counter := Mux(rx_counter === UInt(7), UInt(0), rx_counter + UInt(1))
            rx_buff :=  Cat(rxd_reg2, rx_buff(7, 1))
        }
    }

    when (rx_state === rx_stop_bit) {
        when (rx_baud_tick === UInt(1)){
            when (rxd_reg2 === UInt(1)) {
                rx_state        := rx_idle
                rx_enable       := Bool(false)
                rxQueue.io.enq.bits  := rx_buff
                rxQueue.io.enq.valid := Bool(true)
            }
            .otherwise{
                rx_state        := rx_idle
                rx_enable       := Bool(false)
            }
        }
    }
}
