/*
 * UART module in chisel.
 *
 * Author: Sahar Abbaspour (sabb@dtu.com)
 *    with quite some help from Wolfgang.
 *
 */


package io

import Chisel._

import patmos.Constants._

import chisel3.dontTouch

import ocp._

object Uart extends DeviceObject {
  var baudRate = -1
  var fifoDepth = -1

  def init(params: Map[String, String]) = {
    baudRate = getPosIntParam(params, "baudRate")
    fifoDepth = getPosIntParam(params, "fifoDepth")
  }

  def create(params: Map[String, String]) : Uart = {
    Module(new Uart(CLOCK_FREQ, getPosIntParam(params, "baudRate"), getPosIntParam(params, "fifoDepth")))
  }
}


class Uart(clk_freq: Int, baud_rate: Int, fifoDepth: Int) extends CoreDevice() {

    override val io = IO(new CoreDeviceIO() with patmos.HasPins {
      override val pins = new Bundle {
        val tx = Bits(OUTPUT, 1)
        val rx = Bits(INPUT, 1)
      }
    })
    //Forcing signals availability in emulator
    val uartOcpEmu = Wire(new OcpMasterSignals(ADDR_WIDTH, DATA_WIDTH))
    dontTouch(uartOcpEmu)
    uartOcpEmu <> io.ocp.M

    val c_tx_divider_val    = clk_freq/baud_rate
    val tx_baud_counter     = Reg(init = 0.U(log2Up(clk_freq/baud_rate).W))
    val tx_baud_tick        = Reg(init = 0.U(1.W))

    val tx_idle :: tx_send :: Nil  = Enum(UInt(), 2)
    val tx_state            = Reg(init = tx_idle)
    val tx_buff             = Reg(init = Bits(0, 10))
    val tx_reg              = Reg(init = Bits(1, 1))
    val tx_counter          = Reg(init = 0.U(4.W))

    val txQueue = Module(new Queue(Bits(width = 8), fifoDepth))
    txQueue.io.enq.bits     := io.ocp.M.Data(7, 0)
    txQueue.io.enq.valid    := false.B
    txQueue.io.deq.ready    := false.B

    val rxd_reg0            = Reg(init = Bits(1, 1))
    val rxd_reg1            = Reg(init = Bits(1, 1))
    val rxd_reg2            = Reg(init = Bits(1, 1))

    val rx_baud_counter     = Reg(init = 0.U(log2Up(clk_freq/baud_rate).W))
    val rx_baud_tick        = Reg(init = 0.U(1.W))
    val rx_enable           = Reg(init = false.B)

    val rx_buff             = Reg(init = Bits(0, 8))
    val rx_counter          = Reg(init = 0.U(3.W))
    val rx_idle  :: rx_start :: rx_receive_data :: rx_stop_bit :: Nil  = Enum(UInt(), 4)
    val rx_state            = Reg(init = rx_idle)

    val rxQueue = Module(new Queue(Bits(width = 8), fifoDepth))
    rxQueue.io.enq.bits     := rx_buff
    rxQueue.io.enq.valid    := false.B
    rxQueue.io.deq.ready    := false.B

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
        txQueue.io.enq.valid := true.B
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
    when (tx_baud_counter === (clk_freq/baud_rate).U){
        tx_baud_counter     := 0.U
        tx_baud_tick        := 1.U
    }
    .otherwise {
        tx_baud_counter     := tx_baud_counter + 1.U
        tx_baud_tick        := 0.U
    }

    // Send data

    when (tx_state === tx_idle) {
        when (txQueue.io.deq.valid) {
          txQueue.io.deq.ready := true.B
          tx_buff              := Cat(Bits(1), txQueue.io.deq.bits, Bits(0))
          tx_state             := tx_send
        }
    }

    when (tx_state === tx_send) {
        when (tx_baud_tick === 1.U){
            tx_buff         := Cat (0.U, tx_buff (9, 1))
            tx_reg          := tx_buff(0)
            tx_counter      := Mux(tx_counter === 10.U, 0.U, tx_counter + 1.U)

            when (tx_counter === 10.U) {
              when (txQueue.io.deq.valid) {
                txQueue.io.deq.ready := true.B
                tx_buff              := Cat(Bits(1), txQueue.io.deq.bits)
                tx_reg               := 0.U
                tx_counter           := 1.U
              }
              .otherwise {
                tx_reg          := 1.U
                tx_counter      := 0.U
                tx_state        := tx_idle
              }
            }
        }
    }

    // Connect TX pin
    io.pins.tx := tx_reg


    // UART RX clk
    when (rx_enable) {
        when (rx_baud_counter === (clk_freq/baud_rate).U){
            rx_baud_counter     := 0.U
            rx_baud_tick        := 1.U
        }
        .otherwise {
            rx_baud_counter     := rx_baud_counter + 1.U
            rx_baud_tick        := 0.U
        }
    }


    // Receive data

    rxd_reg0                := io.pins.rx;
    rxd_reg1                := rxd_reg0;
    rxd_reg2                := rxd_reg1


    // RX shift in
    when (rx_state === rx_idle) {
        when (rxd_reg2 === 0.U){
           rx_state         := rx_start
           rx_baud_counter  := (clk_freq/baud_rate).U / 2.U
           rx_enable        := true.B
        }
    }

    when (rx_state === rx_start){
        when (rx_baud_tick === 1.U) {
            when (rxd_reg2 === 0.U) {
                rx_state        := rx_receive_data
            }
            .otherwise{
                rx_state        := rx_idle
            }
        }
    }

    when (rx_state === rx_receive_data) {
        when (rx_baud_tick === 1.U){
            rx_state := Mux(rx_counter === 7.U, rx_stop_bit, rx_receive_data)
            rx_counter := Mux(rx_counter === 7.U, 0.U, rx_counter + 1.U)
            rx_buff :=  Cat(rxd_reg2, rx_buff(7, 1))
        }
    }

    when (rx_state === rx_stop_bit) {
        when (rx_baud_tick === 1.U){
            when (rxd_reg2 === 1.U) {
                rx_state        := rx_idle
                rx_enable       := false.B
                rxQueue.io.enq.bits  := rx_buff
                rxQueue.io.enq.valid := true.B
            }
            .otherwise{
                rx_state        := rx_idle
                rx_enable       := false.B
            }
        }
    }
}
