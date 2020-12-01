package sdc_controller

import chisel3._
import chisel3.util._

class sd_cmd_master extends Module {
  val io = IO(new Bundle {
    val new_cmd = Input(UInt(1.W))
    val data_write = Input(UInt(1.W))
    val data_read = Input(UInt(1.W))

    val arg_reg = Input(UInt(32.W))
    val cmd_set_reg = Input(UInt(14.W))
    val timeout_reg = Input(UInt(16.W))
    val status_reg = Output(UInt(16.W)) // reg
    val resp_1_reg = Output(UInt(32.W)) // reg

    val err_int_reg = Output(UInt(5.W)) // reg
    val normal_int_reg = Output(UInt(16.W))
    val err_int_rst = Input(UInt(1.W))
    val normal_int_rst = Input(UInt(1.W))

    val settings = Output(UInt(16.W)) // reg
    val go_idle_o = Output(UInt(1.W)) // reg
    val cmd_out = Output(UInt(40.W)) // reg
    val req_out = Output(UInt(1.W)) // reg
    val ack_out = Output(UInt(1.W)) // reg
    val req_in = Input(UInt(1.W))
    val ack_in = Input(UInt(1.W))
    val cmd_in = Input(UInt(40.W)) // reg
    val serial_status = Input(UInt(8.W))
    val card_detect = Input(UInt(1.W))
  })
  val crc_check_enable = RegInit(0.U(1.W))
  val index_check_enable = RegInit(0.U(1.W))
  val response_size = RegInit(0.U(7.W))
  val card_present = RegInit(0.U(1.W))
  val status = RegInit(0.U(16.W))
  val debounce = RegInit(0.U(4.W))
  val watchdog_cnt = RegInit(0.U(16.W))
  val complete = RegInit(0.U(1.W))
  val state = RegInit(0.U(3.W))
  //val next_state = RegInit(0.U(3.W)) no reg

  val IDLE = (1<<0).U(3.W)
  val SETUP = (1<<1).U(3.W)
  val EXECUTE = (1<<2).U(3.W)

  val ack_in_int = RegInit(0.U(1.W))
  val ack_q = RegInit(0.U(1.W))
  val req_q = RegInit(0.U(1.W))
  val req_in_int = RegInit(0.U(1.W))

  req_q := io.req_in
  req_in_int := req_q

  when(!io.card_detect.asBool()){
    when(debounce =/= 0xf.U(4.W)){
      debounce := debounce + 1.U
    }.otherwise{
      debounce := 0.U //?
      when(debounce === 0xf.U(4.W)){
        card_present := 1.U
      }.otherwise{
        card_present := 0.U
      }
    }
  }

  ack_q := io.ack_in
  ack_in_int := ack_q

  val en_next_state = state.asBool() || io.new_cmd.asBool() || complete.asBool() ||  ack_in_int.asBool()
  val next_state = IDLE //?

  switch(state){
    is(IDLE){
      when(io.new_cmd.asBool()){
        next_state := SETUP
      }.otherwise{
        next_state := IDLE
      }
    }
    is(SETUP){
      when(ack_in_int.asBool()){
        next_state := EXECUTE
      }.otherwise{
        next_state := SETUP
      }
    }
    is(EXECUTE){
      when(complete.asBool()){
        next_state := IDLE
      }.otherwise{
        next_state := EXECUTE
      }
    }
  }

  state := next_state //?

  val normal_int_reg = RegInit(0.U(16.W))
  val go_idle_o = RegInit(0.U(1.W))
  val req_out = RegInit(0.U(1.W))
  val ack_out = RegInit(0.U(1.W))
  val status_reg = RegInit(0.U(16.W))
  val err_int_reg = RegInit(0.U(5.W))
  normal_int_reg(1) := card_present
  normal_int_reg(2) := ~card_present
  complete := 0.U
  switch(state) {
    is(IDLE) {
      go_idle_o := 0.U
      req_out := 0.U
      ack_out := 0.U
      status_reg(0) := 0.U
      when(req_in_int.asBool()) {
        status := io.serial_status
        ack_out := 1.U
      }
    }
    is(SETUP) {
      normal_int_reg := 0.U
      err_int_reg := 0.U
      index_check_enable := io.cmd_set_reg(4)
      crc_check_enable := io.cmd_set_reg(3)

      when(((io.cmd_set_reg & 0x3.U) === (1 << 1).U) || ((io.cmd_set_reg & 0x3.U) === 3.U)) {
        response_size := 7.U
      } //.elsewhen(io.cmd_set_reg & 0x3 ===)
    }
  }

  //   if ( (`RTS  == 2'b10 ) || ( `RTS == 2'b11)) begin
  //   response_size =  7'b0101000;
  //   end
  //   else if (`RTS == 2'b01) begin
  //   response_size = 7'b1111111;
  //   end
  //   else begin
  //   response_size=0;
  //   end

  //   cmd_out[39:38]=2'b01;
  //   cmd_out[37:32]=`CMDI;  //CMD_INDEX
  //   cmd_out[31:0]= ARG_REG;           //CMD_Argument
  //   settings[14:13]=`WORD_SELECT;             //Reserved
  //   settings[12] = data_read; //Type of command
  //   settings[11] = data_write;
  //   settings[10:8]=3'b111;            //Delay
  //   settings[7]=`CRCE;         //CRC-check
  //   settings[6:0]=response_size;   //response size
  //   Watchdog_Cnt = 0;

  //   `CICMD =1;
  // }
  //

}
