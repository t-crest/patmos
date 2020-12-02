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

  // output regs
  val normal_int_reg = RegInit(0.U(16.W))
  val go_idle_o = RegInit(0.U(1.W))
  val req_out = RegInit(0.U(1.W))
  val ack_out = RegInit(0.U(1.W))
  val status_reg = RegInit(0.U(16.W))
  val err_int_reg = RegInit(0.U(5.W))
  val cmd_out = RegInit(0.U(40.W))
  val settings = RegInit(0.U(16.W))
  val resp_1_reg = RegInit(0.U(32.W))

  //defines
  val dat_ava = status(6)
  val crc_valid = status(5)
  val CMDI = io.cmd_set_reg(13, 8)
  val WORD_SELECT = io.cmd_set_reg(7, 6)
  val CICE = io.cmd_set_reg(4)
  val CRCE = io.cmd_set_reg(3)
  val RTS = io.cmd_set_reg(1, 0)
  val CTE = err_int_reg(0)
  def CTE(b : Bool){err_int_reg.bitSet(0.U, b)}
  val CCRCE = err_int_reg(1)
  def CCRCE(b : Bool){err_int_reg.bitSet(1.U, b)}
  val CIE = err_int_reg(3)
  def CIE(b :Bool){err_int_reg.bitSet(3.U,b)}
  val EI = normal_int_reg(15)
  def EI(b : Bool){normal_int_reg.bitSet(15.U, b)}
  val CC = normal_int_reg(0)
  def CC(b : Bool){normal_int_reg.bitSet(0.U, b)}
  val CICMD = status_reg(0)
  def CICMD(b : Bool){status_reg.bitSet(0.U, b)}

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

  val en_next_state = state.andR() || io.new_cmd.andR() || complete.andR() ||  ack_in_int.andR()
  val next_state = WireDefault(IDLE) //?

  switch(state){
    is(IDLE){
      when(io.new_cmd.andR()){
        next_state := SETUP
      }.otherwise{
        next_state := IDLE
      }
    }
    is(SETUP){
      when(ack_in_int.andR()){
        next_state := EXECUTE
      }.otherwise{
        next_state := SETUP
      }
    }
    is(EXECUTE){
      when(complete.andR()){
        next_state := IDLE
      }.otherwise{
        next_state := EXECUTE
      }
    }
  }

  state := next_state //?

  normal_int_reg.bitSet(1.U, card_present.andR())
  normal_int_reg.bitSet(2.U, !card_present.andR())
  complete := 0.U
  switch(state) {
    is(IDLE) {
      go_idle_o := 0.U
      req_out := 0.U
      ack_out := 0.U
      CICMD(false.B)
      when(req_in_int.andR()) {
        status := io.serial_status
        ack_out := 1.U
      }
    }
    is(SETUP) {
      normal_int_reg := 0.U
      err_int_reg := 0.U
      index_check_enable := CICE
      crc_check_enable := CRCE

      when((RTS === (1 << 1).U) || (RTS === 3.U)) {
        response_size := ((1<<5)|(1<<3)).U
      }.elsewhen(RTS === 1.U){
        response_size := ~0.U(7.W)
      }.otherwise{
        response_size := 0.U
      }

      class Slicer extends Module {
        implicit class SeqHelper(val seq: Seq[Bits]) {
          /**
            * Promotes a Seq of Bits to a class that supports the connect operator
            */
          def := (other: Seq[Bits]): Unit = {
            seq.zip(other).foreach { case (a, b) => a := b}
          }
        }

        val io = IO(new Bundle {
          val in1  = Input(Vec(4, Bool()))
          val out1 = Output(Vec(4, Bool()))
        })

        io.out1.slice(0, 2) := io.in1.slice(0, 2)
      }
      cmd_out := Cat(1.U(2.W), Cat(CMDI, io.arg_reg))
      settings := Cat(WORD_SELECT, Cat(io.data_read, Cat(io.data_write, Cat(~0.U(3.W), Cat(CRCE, response_size)))))
      //cmd_out(39, 38) := 1.U
      //cmd_out(37, 32) := CMDI
      //cmd_out(31, 0) := io.arg_reg
      //settings(14, 13) := WORD_SELECT
      //settings(12) := io.data_read
      //settings(11) := io.data_write
      //settings(10, 8) := ~0.U(3.W)
      //settings(7) := CRCE
      //settings(6,0) := response_size
      watchdog_cnt := 0.U
      CICMD(1.U)
    }
    is(EXECUTE){
      watchdog_cnt := watchdog_cnt + 1.U
      when(watchdog_cnt > io.timeout_reg){
        CTE(1.U)
        EI(1.U) // EI
        when(io.ack_in === 1.U){
          complete := 1.U
        }
        go_idle_o := 1.U
      }
      req_out := 0.U
      ack_out := 0.U
      when(ack_in_int === 1.U){
        req_out := 1.U
      }.elsewhen(req_in_int === 1.U){
        status := io.serial_status
        ack_out := 1.U
        when(dat_ava.asBool()){
          complete := 1.U
          EI(0.U)
          when((crc_check_enable & !crc_valid).andR()){
            CCRCE(1.U)
            EI(1.U)
          }
          when((index_check_enable & (cmd_out(37, 32) =/= io.cmd_in(37, 32))).andR()){
            CIE(1.U)
            EI(1.U)
          }
          CC(1.U)
          when(response_size =/= 0.U){
            resp_1_reg := io.cmd_in(31, 0)
          }
        }
      }
    }
  }
  when(io.err_int_rst.andR()){
    err_int_reg := 0.U
  }
  when(io.normal_int_rst.andR()){
    normal_int_reg := 0.U
  }

  io.status_reg <> status_reg
  io.resp_1_reg <> resp_1_reg
  io.err_int_reg <> err_int_reg
  io.normal_int_reg <> normal_int_reg
  io.settings <> settings
  io.go_idle_o <> go_idle_o
  io.cmd_out <> cmd_out
  io.req_out <> req_out
  io.ack_out <> ack_out
}

object sd_cmd_master extends App {
  chisel3.Driver.execute(Array("--target-dir", "generated"), () => new sd_cmd_master())
}
