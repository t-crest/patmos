package sdc_controller

import chisel3._
import chisel3.util._

class sd_data_master(RAM_MEM_WIDTH : Int, BD_WIDTH : Int, BD_SIZE : Int) extends Module {
  val io = IO(new Bundle {
    val dat_in_tx = Input(UInt(RAM_MEM_WIDTH.W))
    val free_tx_bd = Input(UInt(BD_WIDTH.W))
    val ack_i_s_tx = Input(UInt(1.W))
    val re_s_tx = Output(UInt(1.W)) // reg
    val a_cmp_tx = Output(UInt(1.W)) // reg
    val dat_in_rx = Input(UInt(RAM_MEM_WIDTH.W))
    val free_rx_bd = Input(UInt(BD_WIDTH.W))
    val ack_i_s_rx = Input(UInt(1.W))
    val re_s_rx = Output(UInt(1.W)) // reg
    val a_cmp_rx = Output(UInt(1.W)) // reg
    val cmd_busy = Input(UInt(1.W))
    val we_req = Output(UInt(1.W)) // reg
    val we_ack = Input(UInt(1.W))
    val d_write = Output(UInt(1.W)) // reg
    val d_read = Output(UInt(1.W)) // reg
    val cmd_arg = Output(UInt(32.W)) // reg
    val cmd_set = Output(UInt(16.W)) // reg
    val cmd_tsf_err = Input(UInt(1.W))
    val card_status = Input(UInt(5.W))
    val start_tx_fifo = Output(UInt(1.W)) // reg
    val start_rx_fifo = Output(UInt(1.W)) // reg
    val sys_adr = Output(UInt(32.W)) // reg
    val tx_empt = Input(UInt(1.W))
    val tx_full = Input(UInt(1.W))
    val rx_full = Input(UInt(1.W))
    val busy_n = Input(UInt(1.W))
    val transm_complete = Input(UInt(1.W))
    val crc_ok = Input(UInt(1.W))
    val ack_transfer = Output(UInt(1.W)) // reg
    val Dat_Int_Status = Output(UInt(8.W)) // reg
    val Dat_Int_Status_rst = Input(UInt(1.W))
    val CIDAT = Output(UInt(1.W)) // reg
    val transfer_type = Input(UInt(2.W))
})

  val RESEND_MAX_CNT = 3
  // if (RAM_MEM_WIDTH == 16)
  var READ_CYCLE = 4
  var BD_EMPTY = BD_SIZE/4
  var bd_cnt = RegInit(0.U(3.W))
  if (RAM_MEM_WIDTH == 32)
  {
    READ_CYCLE = 2
    BD_EMPTY = BD_SIZE/2
    bd_cnt = RegInit(0.U(2.W))
  }

  val send_done = RegInit(0.U(1.W))
  val rec_done = RegInit(0.U(1.W))
  val rec_failed = RegInit(0.U(1.W))
  val tx_cycle = RegInit(0.U(1.W))
  val rx_cycle = RegInit(0.U(1.W))
  val resend_try_cnt = RegInit(0.U(3.W))

  val CMD24 = Integer.parseInt("181A", 16).U
  val CMD17 = Integer.parseInt("111A", 16).U
  val CMD12 = Integer.parseInt("C1A", 16).U
  val ACMD13 = Integer.parseInt("D1A", 16).U
  val ACMD51 = Integer.parseInt("331A", 16).U

  val IDLE = 1.U(9.W)
  val GET_TX_BD = (1<<1).U(9.W)
  val GET_RX_BD = (1<<2).U(9.W)
  val SEND_CMD = (1<<3).U(9.W)
  val RECIVE_CMD = (1<<4).U(9.W)
  val DATA_TRANSFER = (1<<5).U(9.W)
  val STOP = (1<<6).U(9.W)
  val STOP_SEND = (1<<7).U(9.W)
  val STOP_RECIVE_CMD = (1<<8).U(9.W)

  val SIZE = 9
  val state = RegInit(IDLE)
  val next_state = Wire(UInt())

  val trans_done = RegInit(0.U(1.W))
  val trans_failed = RegInit(0.U(1.W))
  val internal_transm_complete = RegInit(0.U(1.W))
  val transm_complete_q = RegInit(0.U(1.W))

  transm_complete_q := io.transm_complete
  internal_transm_complete := transm_complete_q

  next_state := IDLE
  switch(state) {
    is(IDLE) {
      when(io.free_tx_bd =/= BD_EMPTY.U){
        next_state := GET_TX_BD
      }.elsewhen(io.free_rx_bd =/= BD_EMPTY.U) {
        next_state := GET_RX_BD
      }.otherwise{
        next_state := IDLE
      }
    }
    is(GET_TX_BD){
      when((bd_cnt > (READ_CYCLE-1).U) && (io.tx_full === 1.U)){
        next_state := SEND_CMD
      }.otherwise{
        next_state := GET_TX_BD
      }
    }
    is(GET_RX_BD){
      when(bd_cnt >= (READ_CYCLE-1).U){
        next_state := SEND_CMD
      }.otherwise{
        next_state := GET_RX_BD
      }
    }
    is(SEND_CMD){
      when(send_done.orR()){
        next_state := RECIVE_CMD
      }.otherwise{
        next_state := SEND_CMD
      }
    }
    is(RECIVE_CMD){
      when(rec_done.orR()){
        next_state := DATA_TRANSFER
      }.elsewhen(rec_failed.orR()){
        next_state := SEND_CMD
      }.otherwise{
        next_state := RECIVE_CMD
      }
    }
    is(DATA_TRANSFER){
      when(trans_done.orR()){
        next_state := IDLE
      }.elsewhen(trans_failed.orR()){
        next_state := STOP
      }.otherwise{
        next_state := DATA_TRANSFER
      }
    }
    is(STOP){
      next_state := STOP_SEND
    }
    is(STOP_SEND){
      when(send_done.orR()){
        next_state := IDLE
      }.otherwise{
        next_state := STOP_SEND
      }
    }
    is(STOP_RECIVE_CMD){
      when(rec_done.orR()){
        next_state := SEND_CMD
      }.elsewhen(rec_failed.orR()){
        next_state := STOP
      }.elsewhen(resend_try_cnt >= RESEND_MAX_CNT.U){
        next_state := IDLE
      }.otherwise{
        next_state := STOP_RECIVE_CMD
      }
    }
  }
  state := next_state

  val sys_adr = RegInit(0.U(32.W))
  io.sys_adr <>  sys_adr
  val cmd_arg = RegInit(0.U(32.W))
  io.cmd_arg <>  cmd_arg
  val start_tx_fifo = RegInit(0.U(1.W))
  io.start_tx_fifo <>  start_tx_fifo
  val start_rx_fifo = RegInit(0.U(1.W))
  io.start_rx_fifo <>  start_rx_fifo
  val d_write = RegInit(0.U(1.W))
  io.d_write <>  d_write
  val d_read = RegInit(0.U(1.W))
  io.d_read <>  d_read
  val ack_transfer = RegInit(0.U(1.W))
  io.ack_transfer <>  ack_transfer
  val a_cmp_tx = RegInit(0.U(1.W))
  io.a_cmp_tx <>  a_cmp_tx
  val a_cmp_rx = RegInit(0.U(1.W))
  io.a_cmp_rx <>  a_cmp_rx
  val CIDAT = RegInit(0.U(1.W))
  io.CIDAT <>  CIDAT
  val Dat_Int_Status = RegInit(0.U(8.W))
  io.Dat_Int_Status <>  Dat_Int_Status
  val we_req = RegInit(0.U(1.W))
  io.we_req <>  we_req
  val re_s_tx = RegInit(0.U(1.W))
  io.re_s_tx <>  re_s_tx
  val re_s_rx = RegInit(0.U(1.W))
  io.re_s_rx <>  re_s_rx
  val cmd_set = RegInit(0.U(16.W))
  io.cmd_set <>  cmd_set

  switch(state){
    is(IDLE){
      send_done := 0.U
      bd_cnt := 0.U
      sys_adr := 0.U
      cmd_arg := 0.U
      rec_done := 0.U
      rec_failed := 0.U
      start_tx_fifo := 0.U
      start_rx_fifo := 0.U
      send_done := 0.U
      d_write  := 0.U
      d_read  := 0.U
      trans_failed := 0.U
      trans_done := 0.U
      tx_cycle  := 0.U
      rx_cycle  := 0.U
      ack_transfer := 0.U
      a_cmp_tx := 0.U
      a_cmp_rx := 0.U
      resend_try_cnt := 0.U
    }
    is(GET_RX_BD){
      re_s_rx := 1.U
      if (RAM_MEM_WIDTH == 32) {
        when(io.ack_i_s_rx.orR()){
          when(bd_cnt === 0.U) {
            sys_adr := io.dat_in_rx
            bd_cnt := bd_cnt + 1.U
          }.elsewhen(bd_cnt === 1.U){
            cmd_arg := io.dat_in_rx
            re_s_rx := 0.U
          }
        }
      }
      else{
        when(io.ack_i_s_rx.orR()){
          when(bd_cnt === 0.U){
            sys_adr := Cat(sys_adr(31, 16), io.dat_in_rx)
          }.elsewhen(bd_cnt === 1.U){
            sys_adr := Cat(io.dat_in_rx, sys_adr(15,0))
          }.elsewhen(bd_cnt === 2.U){
            cmd_arg := Cat(cmd_arg(31, 16), io.dat_in_rx)
            re_s_rx := 0.U
          }.elsewhen(bd_cnt === 3.U){
            cmd_arg := Cat(io.dat_in_rx, cmd_arg(15, 0))
            re_s_rx := 0.U
          }
          bd_cnt := bd_cnt + 1.U
        }
      }
      when(io.transfer_type === 0.U){
        cmd_set := CMD17
      }.elsewhen(io.transfer_type === 1.U){
        cmd_set := ACMD13
      }.otherwise {
        cmd_set := ACMD51
      }
      rx_cycle := 1.U
    }
    is(GET_TX_BD){
      re_s_tx := 1.U
      when(bd_cnt === READ_CYCLE.U) {
        re_s_tx := 0.U
      }
      if(RAM_MEM_WIDTH == 32){
        when(io.ack_i_s_tx.orR()){
          when(bd_cnt === 0.U){
            sys_adr := io.dat_in_tx
            bd_cnt := bd_cnt + 1.U
          }.elsewhen(bd_cnt === 1.U){
            cmd_arg := io.dat_in_tx
            re_s_tx := 0.U
            start_tx_fifo := 1.U
          }
        }
      }else{
        when(io.ack_i_s_tx.orR()){
          when(bd_cnt === 0.U){
            sys_adr := Cat(sys_adr(31, 16), io.dat_in_tx)
            bd_cnt := bd_cnt + 1.U
          }.elsewhen(bd_cnt === 1.U){
            sys_adr := Cat(io.dat_in_tx, sys_adr(15, 0))
            bd_cnt := bd_cnt + 1.U
          }.elsewhen(bd_cnt === 2.U){
            cmd_arg := Cat(cmd_arg(31, 16), io.dat_in_tx)
            re_s_tx := 0.U
            bd_cnt := bd_cnt + 1.U
          }.elsewhen(bd_cnt === 3.U){
            cmd_arg := Cat(io.dat_in_tx, cmd_arg(15, 0))
            re_s_tx := 0.U
            bd_cnt := bd_cnt + 1.U
            start_tx_fifo := 1.U
          }
        }
      }
      cmd_set := CMD24
      tx_cycle := 1.U
    }
    is(SEND_CMD){
      rec_done := 0.U
      when(rx_cycle.orR()){
        re_s_rx := 0.U
        d_read := 1.U
      }.otherwise {
        re_s_tx := 0.U
        d_write := 1.U
      }
      start_rx_fifo := 0.U
      when(!io.cmd_busy.orR()){
        we_req := 1.U
      }
      when(io.we_ack.orR()){
        send_done := 1.U
        we_req := 1.U
      }
    }
    is(RECIVE_CMD){
      when(rx_cycle.orR()){
        start_rx_fifo := 1.U
      }
      we_req := 0.U
      send_done := 0.U
      when(!io.cmd_busy.orR()) {
        d_read := 0.U
        d_write := 0.U
        when(!io.cmd_tsf_err.orR()) {
          when(io.card_status(0)) {
            when((io.card_status(4, 1) === Integer.parseInt("0100", 2).U) ||
              (io.card_status(4, 1) === Integer.parseInt("0110", 2).U) ||
              (io.card_status(4, 1) === Integer.parseInt("0101", 2).U)) {
              rec_done := 1.U
            }.otherwise {
              rec_failed := 1.U
              Dat_Int_Status.bitSet(4.U, true.B)
              start_tx_fifo := 1.U
            }
          }
        }.otherwise {
          rec_failed := 1.U
          start_tx_fifo := 0.U
        }
      }
    }
    is(DATA_TRANSFER){
      CIDAT := 1.U
      when(tx_cycle.orR()){
        when(io.tx_empt.orR()){
          Dat_Int_Status.bitSet(2.U, true.B)
          trans_failed := 1.U
        }
      }.otherwise{
        when(io.rx_full.orR()){
          Dat_Int_Status.bitSet(2.U, true.B)
          trans_failed := 1.U
        }
      }
      when(internal_transm_complete.orR()){
        ack_transfer := 1.U
        when(!io.crc_ok.orR() && io.busy_n.orR()){
          Dat_Int_Status.bitSet(5.U, true.B)
          trans_failed := 1.U
        }.elsewhen(io.crc_ok.orR() && io.busy_n.orR()){
          trans_done := 1.U
          when(tx_cycle.orR()){
            a_cmp_tx := 1.U
            when(io.free_tx_bd === (BD_EMPTY-1).U){
              Dat_Int_Status.bitSet(0.U, true.B)
            }
          }.otherwise{
            a_cmp_rx := 1.U
            when(io.free_rx_bd === (BD_EMPTY-1).U){
              Dat_Int_Status.bitSet(0.U, true.B)
            }
          }
        }
      }
    }
    is(STOP){
      cmd_set := CMD12
      rec_done := 0.U
      rec_failed := 0.U
      send_done := 0.U
      trans_failed := 0.U
      trans_done := 0.U
      d_read := 1.U
      d_write := 1.U
      start_rx_fifo := 0.U
      start_tx_fifo := 0.U
    }
    is(STOP_SEND){
      resend_try_cnt := resend_try_cnt + 1.U
      when(resend_try_cnt === RESEND_MAX_CNT.U){
        Dat_Int_Status.bitSet(1.U, true.B)
      }
      when(!io.cmd_busy.orR()){
        we_req := 1.U
      }
      when(io.we_ack.orR()){
        send_done := 1.U
      }
    }
    is(STOP_RECIVE_CMD){
      we_req := 0.U
    }
  }
  when(io.Dat_Int_Status_rst.orR()){
    Dat_Int_Status := 0.U
  }
}

object sd_data_master extends App {
  chisel3.Driver.execute(Array("--target-dir", "generated"), () => new sd_data_master(
    RAM_MEM_WIDTH = 16,
    BD_WIDTH = 5,
    BD_SIZE = 32))
}