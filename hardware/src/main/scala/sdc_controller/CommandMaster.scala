package sdc_controller

import chisel3._
import chisel3.experimental.ChiselEnum
import chisel3.util.Enum

class CommandMaster extends Module {
  val io = new Bundle() {
    val sd_clk = Input(Bits(1.W))
    val start_i = Input(Bits(1.W))
    val int_status_rst_i = Input(Bits(1.W))
    val setting_o = Output(Bits(2.W))
    val start_xfr_o = Output(Bits(1.W))
    val go_idle_o = Output(Bits(1.W))
    val cmd_o = Output(Bits(40.W))
    val response_i = Input(Bits(120.W))
    val crc_ok_i = Input(Bits(1.W))
    val index_ok_i = Input(Bits(1.W))
    val finish_i = Input(Bits(1.W))
    val busy_i = Input(Bits(1.W))
    //direct signal from data sd data input (data[0])
    //input card_detect,
    val argument_i = Input(Bits(32.W))
    val command_i = Input(Bits(14.W))
    val timeout_i = Input(Bits(24.W))
    val int_status_o = Output(Bits(5.W))
    val response_0_o = Output(Bits(32.W))
    val response_1_o = Output(Bits(32.W))
    val response_2_o = Output(Bits(32.W))
    val response_3_o = Output(Bits(32.W))
  }

  val sIdle :: sExecute :: sBusyCheck :: Nil = Enum(3)

}
