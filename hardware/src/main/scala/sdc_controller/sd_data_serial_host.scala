package sdc_controller

import chisel3._
import chisel3.util._

class sd_data_serial_host(BIT_BLOCK : Int, CRC_OFF : Int, SD_BUS_W : Int, BIT_BLOCK_REC : Int, BIT_CRC_CYCLE : Int) extends Module {
  val io = IO(new Bundle {
    val data_in = Input(UInt(32.W))
    val rd = Output(UInt(1.W)) // reg
    val data_out = Output(UInt(SD_BUS_W.W)) // reg
    val we = Output(UInt(1.W)) // reg
    val DAT_oe_o = Output(UInt(1.W)) // reg
    val DAT_dat_o = Output(UInt(SD_BUS_W.W)) // reg
    val DAT_dat_i = Input(UInt(SD_BUS_W.W))
    val start_dat = Input(UInt(2.W))
    val ack_transfer = Input(UInt(1.W))
    val busy_n = Output(UInt(1.W)) // reg
    val transm_complete = Output(UInt(1.W)) // reg
    val crc_ok = Output(UInt(1.W)) // reg
  })

  val crc_in = RegInit(0.U(SD_BUS_W.W))
  val crc_en = RegInit(0.U(1.W))
  val crc_rst = RegInit(1.U(1.W))
  val crc_out = Wire(Vec(SD_BUS_W, UInt(16.W)))
  //val temp_in = RegInit()
  var transf_cnt = RegInit(0.U(11.W))

  val IDLE        = 1.U(6.W)
  val WRITE_DAT   = (1<<1).U(6.W)
  val WRITE_CRC   = (1<<2).U(6.W)
  val WRITE_BUSY  = (1<<3).U(6.W)
  val READ_WAIT   = (1<<4).U(6.W)
  val READ_DAT    = (1<<5).U(6.W)

  val SIZE = 6.U
  val state = RegInit(IDLE)
  val next_state = WireDefault(IDLE)
  val crc_status = RegInit(7.U(3.W))
  val busy_int = RegInit(0.U(1.W))

  val sd_crc_16_arr = Array.fill(SD_BUS_W){Module(new sd_crc_16())}
  for(i <- 0 until SD_BUS_W-1){
    sd_crc_16_arr(i).io.bitval := crc_in(i)
    sd_crc_16_arr(i).io.enable := crc_en
    crc_out(i) := sd_crc_16_arr(i).io.crc
  }
  val ack_transfer_int = RegInit(0.U(1.W))
  val ack_q = RegInit(0.U(1.W))

  ack_q := io.ack_transfer
  ack_transfer_int := ack_q

  val q_start_bit = RegInit(1.U(1.W))
  //next_state := IDLE
  switch(state){
    is(IDLE){
      when(io.start_dat === 1.U){
        next_state := WRITE_DAT
      }.elsewhen(io.start_dat === 2.U){
        next_state := READ_WAIT
      }.otherwise{
        next_state := IDLE
      }
    }
    is(WRITE_DAT){
      when(transf_cnt >= BIT_BLOCK.U){
        next_state := WRITE_CRC
      }.elsewhen(io.start_dat === 3.U){
        next_state := IDLE
      }.otherwise{
        next_state := WRITE_DAT
      }
    }
    is(WRITE_CRC){
      when(crc_status === 0.U){
        next_state := WRITE_BUSY
      }.otherwise{
        next_state := WRITE_CRC
      }
    }
    is(WRITE_BUSY){
      when(busy_int === 1.U && ack_transfer_int.orR()){
        next_state := IDLE
      }.otherwise{
        next_state := WRITE_BUSY
      }
    }
    is(READ_WAIT){
      when(q_start_bit === 0.U){
        next_state := READ_DAT
      }.otherwise{
        next_state := READ_WAIT
      }
    }
    is(READ_DAT){
      when(ack_transfer_int.orR()){
        next_state := IDLE
      }.elsewhen(io.start_dat === 3.U){
        next_state := IDLE
      }.otherwise{
        next_state := READ_DAT
      }
    }
  }

  state := next_state

  when(!io.DAT_dat_i(0).orR() && state === READ_WAIT){
    q_start_bit := 0.U
  }.otherwise{
    q_start_bit := 1.U
  }

  val crc_c = RegInit(0.U(5.W))
  val last_din = RegInit(0.U(4.W))
  val crc_s = RegInit(0.U(3.W))
  val write_buf_0 = RegInit(0.U(32.W))
  val write_buf_1 = RegInit(0.U(32.W))
  val sd_data_out = Reg(UInt(32.W)) // Init?
  val out_buff_ptr = RegInit(0.U(1.W))
  val in_buff_ptr = RegInit(0.U(1.W))
  val data_send_index = RegInit(0.U(3.W))
  val DAT_oe_o = RegInit(0.U(1.W))
  io.DAT_oe_o <> DAT_oe_o
  val DAT_dat_o = RegInit(0.U(SD_BUS_W.W))
  io.DAT_dat_o <> DAT_dat_o
  val rd = RegInit(0.U(1.W))
  io.rd <> rd
  val we = RegInit(0.U(1.W))
  io.we <> we
  val data_out = RegInit(0.U(SD_BUS_W.W))
  io.data_out <> data_out
  val busy_n = RegInit(1.U(1.W))
  io.busy_n <> busy_n
  val transm_complete = RegInit(0.U(1.W))
  io.transm_complete <> transm_complete
  val crc_ok = RegInit(0.U(1.W))
  io.crc_ok <> crc_ok

  switch(state){
    is(IDLE){
      DAT_oe_o := 0.U
      DAT_dat_o := Integer.parseInt("1111",2).U
      crc_en := 0.U
      crc_rst := 1.U
      transf_cnt := 0.U
      crc_c := 16.U
      crc_status := 7.U
      crc_s := 0.U
      we := 0.U
      rd := 0.U
      busy_n := 1.U
      data_send_index := 0.U
      out_buff_ptr := 0.U
      in_buff_ptr := 0.U
    }
    is(WRITE_DAT){
      transm_complete := 0.U
      busy_n := 0.U
      crc_ok := 0.U
      transf_cnt := transf_cnt + 1.U
      rd := 0.U

      when(in_buff_ptr =/= out_buff_ptr || !transf_cnt.orR()){
        rd := 1.U
        when(!in_buff_ptr.orR()){
          write_buf_0 := io.data_in
        }.otherwise{
          write_buf_1 := io.data_in
        }
        in_buff_ptr := in_buff_ptr + 1.U
      }
      when(!out_buff_ptr){
        sd_data_out := write_buf_0
      }.otherwise{
        sd_data_out := write_buf_1
      }
      when(transf_cnt === 1.U){
        crc_rst := 0.U // reset path
        crc_en := 1.U
        // BIG ENDIAN
        last_din := write_buf_0(31,28)
        crc_in := write_buf_0(31,28)
        //
        DAT_oe_o := 1.U
        DAT_dat_o := 0.U
        data_send_index := 1.U
      }.elsewhen(transf_cnt >= 2.U && transf_cnt <= (BIT_BLOCK-CRC_OFF).U){
        DAT_oe_o := 1.U
        // BIG ENDIAN
        switch(data_send_index){
          is(0.U){
            last_din <=sd_data_out(31, 28)
            crc_in <=sd_data_out(31, 28)
          }
          is(1.U){
            last_din <=sd_data_out(27, 24)
            crc_in <=sd_data_out(27, 24)
          }
          is(2.U){
            last_din <=sd_data_out(23, 20)
            crc_in <=sd_data_out(23, 20)
          }
          is(3.U){
            last_din <=sd_data_out(19, 16)
            crc_in <=sd_data_out(19, 16)
          }
          is(4.U){
            last_din <=sd_data_out(15, 12)
            crc_in <=sd_data_out(15, 12)
          }
          is(5.U){
            last_din <=sd_data_out(11, 8)
            crc_in <=sd_data_out(11, 8)
          }
          is(6.U){
            last_din <=sd_data_out(7, 4)
            crc_in <=sd_data_out(7, 4)
          }
          is(7.U){
            last_din <=sd_data_out(3, 0)
            crc_in <=sd_data_out(3, 0)
          }
        }
        //
        data_send_index := data_send_index + 1.U
        DAT_dat_o := last_din
        when(transf_cnt >= (BIT_BLOCK-CRC_OFF).U){
          crc_en := 0.U
        }
      }.elsewhen(transf_cnt > (BIT_BLOCK-CRC_OFF).U && crc_c =/= 0.U){
        rd := 0.U
        crc_en := 0.U
        crc_c := crc_c-1.U
        DAT_oe_o := 1.U
        DAT_dat_o.bitSet(0.U, crc_out(0)(crc_c-1.U))
        DAT_dat_o.bitSet(1.U, crc_out(1)(crc_c-1.U))
        DAT_dat_o.bitSet(2.U, crc_out(2)(crc_c-1.U))
        DAT_dat_o.bitSet(3.U, crc_out(3)(crc_c-1.U))
      }.elsewhen(transf_cnt === (BIT_BLOCK-2).U){
        DAT_oe_o := 1.U
        DAT_dat_o := Integer.parseInt("1111",2).U
        rd := 0.U
      }.elsewhen(transf_cnt =/= 0.U){
        DAT_oe_o := 0.U
        rd := 0.U
      }
    }
    is(WRITE_CRC){
      rd := 0.U
      DAT_oe_o := 0.U
      crc_status := crc_status-1.U
      when(crc_status <= 4.U && crc_status >= 2.U){
        crc_s.bitSet(crc_status-2.U, io.DAT_dat_i.orR())
      }
    }
    is(WRITE_BUSY){
      transm_complete := 1.U
      when(crc_s === 2.U){
        crc_ok := 1.U
      }.otherwise{
        crc_ok := 0.U
      }
      busy_int := io.DAT_dat_i(0)
    }
    is(READ_WAIT){
      DAT_oe_o := 0.U
      crc_rst := 0.U
      crc_en := 1.U
      crc_in := 0.U
      crc_c := 15.U
      busy_n := 0.U
      transm_complete := 0.U
    }
    is(READ_DAT){
      when(transf_cnt < BIT_BLOCK_REC.U){
        we := 1.U
        data_out := io.DAT_dat_i
        crc_in := io.DAT_dat_i
        crc_ok := 1.U
        transf_cnt := transf_cnt+1.U
      }.elsewhen(transf_cnt <= (BIT_BLOCK_REC + BIT_CRC_CYCLE).U){
        transf_cnt := transf_cnt+1.U
        crc_en := 0.U
        last_din := io.DAT_dat_i
        when(transf_cnt > BIT_BLOCK_REC.U){
          crc_c := crc_c-1.U
          we := 0.U
          // SD BUS WIDTH 4
          when((crc_out(0)(crc_c) =/= last_din(0)) ||
               (crc_out(1)(crc_c) =/= last_din(1)) ||
               (crc_out(2)(crc_c) =/= last_din(2)) ||
               (crc_out(3)(crc_c) =/= last_din(3)))
          {
            crc_ok := 0.U
          }
          // CRC SIM:   crc_ok := 1.U
          when(crc_c === 0.U){
            transm_complete := 1.U
            busy_n := 0.U
            we := 0.U
          }
        }
      }
    }
  }
}
