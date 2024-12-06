/*
 * A connection to the DDR3 Avalon-MM device
 *
 * Authors: Torur Biskopsto Strom (torur.strom@gmail.com)
 *
 */

package io

import chisel3._
import chisel3.util._
import chisel3.experimental._
import ocp._
import patmos.Constants._

object DDR3Bridge extends DeviceObject {
  def init(params: Map[String, String]) = {
  }
  def create(params: Map[String, String]) : DDR3Bridge = {
    Module(new DDR3Bridge())
  }
}

// class DDR3Bridge() extends CoreDevice() {
//   override val io = IO(new CoreDeviceIO() with patmos.HasPins
class DDR3Bridge() extends BurstDevice(30) {
  override val io = IO(new BurstDeviceIO(30) with patmos.HasPins
  {
    val pins: Bundle {
      val mem_a: UInt
      val mem_ba: UInt
      val mem_ck: Bool
      val mem_ck_n: Bool
      val mem_cke: Bool
      val mem_cs_n: Bool
      val mem_ras_n: Bool
      val mem_cas_n: Bool
      val mem_we_n: Bool
      val mem_reset_n: Bool
      val mem_dq: Analog
      val mem_dqs: Analog
      val mem_dqs_n: Analog
      val mem_odt: Bool
      val mem_dm: UInt
      val oct_rzqin: Bool
    } = new Bundle
    {
      val mem_a = Output(UInt(15.W))
      val mem_ba = Output(UInt(3.W))
      val mem_ck = Output(Bool())
      val mem_ck_n = Output(Bool())
      val mem_cke = Output(Bool())
      val mem_cs_n = Output(Bool())
      val mem_ras_n = Output(Bool())
      val mem_cas_n = Output(Bool())
      val mem_we_n = Output(Bool())
      val mem_reset_n = Output(Bool())
      val mem_dq = Analog(32.W)
      val mem_dqs = Analog(4.W)
      val mem_dqs_n = Analog(4.W)
      val mem_odt = Output(Bool())
      val mem_dm = Output(UInt(4.W))
      val oct_rzqin = Input(Bool())
    }
  })

  val bb = Module(new soc_system_ddr3())
  io.pins.mem_a := bb.io.memory_mem_a
  io.pins.mem_ba := bb.io.memory_mem_ba
  io.pins.mem_ck := bb.io.memory_mem_ck
  io.pins.mem_ck_n := bb.io.memory_mem_ck_n
  io.pins.mem_cke := bb.io.memory_mem_cke
  io.pins.mem_cs_n := bb.io.memory_mem_cs_n
  io.pins.mem_ras_n := bb.io.memory_mem_ras_n
  io.pins.mem_cas_n := bb.io.memory_mem_cas_n
  io.pins.mem_we_n := bb.io.memory_mem_we_n
  io.pins.mem_reset_n := bb.io.memory_mem_reset_n
  attach(io.pins.mem_dq, bb.io.memory_mem_dq)
  attach(io.pins.mem_dqs, bb.io.memory_mem_dqs)
  attach(io.pins.mem_dqs_n, bb.io.memory_mem_dqs_n)
  io.pins.mem_odt := bb.io.memory_mem_odt
  io.pins.mem_dm := bb.io.memory_mem_dm
  bb.io.clk_clk := clock
  bb.io.hps_f2h_sdram0_clock_clk := clock
  bb.io.memory_oct_rzqin := io.pins.oct_rzqin
  






  val stateIdle::stateRead::stateWrite::Nil = Enum(3)
  val stateReg = RegInit(stateIdle)

  val addrReg = Reg(UInt(32.W))
  when(io.ocp.M.Cmd =/= OcpCmd.IDLE) {
    addrReg := io.ocp.M.Addr
  }
  var dataRegs = Reg(Vec(4, UInt(32.W)))
  var byteEnableRegs = Reg(Vec(4, UInt(4.W)))
  val burstCountReg = RegInit(0.U(2.W))

  val writeReg = RegInit(false.B)
  val readReg = RegInit(false.B)
  val readingReg = RegInit(false.B)

  io.ocp.S.Resp := OcpResp.NULL
  bb.io.hps_f2h_sdram0_data_read := false.B
  switch(stateReg) {
    is(stateIdle) {
      when(io.ocp.M.Cmd === OcpCmd.WR) {
        dataRegs(0) := io.ocp.M.Data
        byteEnableRegs(0) := io.ocp.M.DataByteEn
        burstCountReg := 1.U
        stateReg := stateWrite
      }.elsewhen(io.ocp.M.Cmd === OcpCmd.RD) {
        readReg := true.B
        stateReg := stateRead
      }

      when(readingReg) {
        io.ocp.S.Resp := OcpResp.DVA
        when(burstCountReg === 3.U) {
          readingReg := false.B
        }.otherwise {
          burstCountReg := burstCountReg + 1.U
        }
      }.elsewhen(writeReg && !bb.io.hps_f2h_sdram0_data_waitrequest) {
        io.ocp.S.Resp := OcpResp.DVA
        writeReg := false.B
      }
    }
    is(stateRead) {
      when(!bb.io.hps_f2h_sdram0_data_waitrequest) {
        readReg := false.B
      }
      when(bb.io.hps_f2h_sdram0_data_readdatavalid) {
        dataRegs(0) := bb.io.hps_f2h_sdram0_data_readdata(31, 0)
        dataRegs(1) := bb.io.hps_f2h_sdram0_data_readdata(63, 32)
        dataRegs(2) := bb.io.hps_f2h_sdram0_data_readdata(95, 64)
        dataRegs(3) := bb.io.hps_f2h_sdram0_data_readdata(127, 96)
        burstCountReg := 0.U
        readingReg := true.B
        stateReg := stateIdle
      }
    }
    is(stateWrite) {
      dataRegs(burstCountReg) := io.ocp.M.Data
      byteEnableRegs(burstCountReg) := io.ocp.M.DataByteEn
      burstCountReg := burstCountReg + 1.U
      when(burstCountReg === 3.U) {
        writeReg := true.B
        stateReg := stateIdle
      }
    }
  }

  bb.io.hps_f2h_sdram0_data_burstcount := 1.U
  bb.io.hps_f2h_sdram0_data_address := addrReg(29, 4)
  bb.io.hps_f2h_sdram0_data_write := writeReg
  bb.io.hps_f2h_sdram0_data_writedata := dataRegs.asUInt
  bb.io.hps_f2h_sdram0_data_read := readReg
  bb.io.hps_f2h_sdram0_data_byteenable := byteEnableRegs.asUInt

  io.ocp.S.Data := dataRegs(burstCountReg)

  // always accept
  io.ocp.S.CmdAccept := 1.U
  io.ocp.S.DataAccept := 1.U





  // val isReadReg = RegInit(false.B)
  // val MReg = Reg(new CoreDeviceIO().ocp.M)

  // when(io.ocp.M.Cmd =/= OcpCmd.IDLE) {
  //   MReg := io.ocp.M
  // }.elsewhen(!bb.io.hps_f2h_sdram0_data_waitrequest) {
  //   MReg.Cmd := OcpCmd.IDLE
  // }

  // io.ocp.S.Resp := OcpResp.NULL
  // when(!bb.io.hps_f2h_sdram0_data_waitrequest) {
  //   when(MReg.Cmd === OcpCmd.WR){
  //     io.ocp.S.Resp := OcpResp.DVA
  //   }.elsewhen(MReg.Cmd === OcpCmd.RD && !bb.io.hps_f2h_sdram0_data_readdatavalid) {
  //     isReadReg := true.B
  //   }.elsewhen((MReg.Cmd === OcpCmd.RD || isReadReg) && bb.io.hps_f2h_sdram0_data_readdatavalid) {
  //     isReadReg := false.B
  //     io.ocp.S.Resp := OcpResp.DVA
  //   }
  // }

  // bb.io.hps_f2h_sdram0_data_burstcount := 1.U
  // bb.io.hps_f2h_sdram0_data_write := MReg.Cmd === OcpCmd.WR
  // bb.io.hps_f2h_sdram0_data_read := MReg.Cmd === OcpCmd.RD
  // bb.io.hps_f2h_sdram0_data_address := MReg.Addr(29, 4)
  
  // io.ocp.S.Data := 0.U
  // bb.io.hps_f2h_sdram0_data_byteenable := 0.U
  // switch(MReg.Addr(3, 2)) {
  //   is(0.U) {
  //     io.ocp.S.Data := bb.io.hps_f2h_sdram0_data_readdata(31, 0)
  //     bb.io.hps_f2h_sdram0_data_writedata := MReg.Data
  //     bb.io.hps_f2h_sdram0_data_byteenable := MReg.ByteEn
  //   }
  //   is(1.U) {
  //     io.ocp.S.Data := bb.io.hps_f2h_sdram0_data_readdata(63, 32)
  //     bb.io.hps_f2h_sdram0_data_writedata := MReg.Data << 32
  //     bb.io.hps_f2h_sdram0_data_byteenable := MReg.ByteEn << 4
  //   }
  //   is(2.U) {
  //     io.ocp.S.Data := bb.io.hps_f2h_sdram0_data_readdata(95, 64)
  //     bb.io.hps_f2h_sdram0_data_writedata := MReg.Data << 64
  //     bb.io.hps_f2h_sdram0_data_byteenable := MReg.ByteEn << 8
  //   }
  //   is(3.U) {
  //     io.ocp.S.Data := bb.io.hps_f2h_sdram0_data_readdata(127, 96)
  //     bb.io.hps_f2h_sdram0_data_writedata := MReg.Data << 96
  //     bb.io.hps_f2h_sdram0_data_byteenable := MReg.ByteEn << 12
  //   }
  // }






}


class soc_system_ddr3 extends BlackBox() {

  val io = IO(new Bundle() {
    val clk_clk = Input(Clock())
    val h2f_reset_reset_n = Output(Bool())
    val hps_f2h_sdram0_clock_clk = Input(Clock())
    val hps_f2h_sdram0_data_address = Input(UInt(26.W))
    val hps_f2h_sdram0_data_read = Input(Bool())
    val hps_f2h_sdram0_data_readdata = Output(UInt(128.W))
    val hps_f2h_sdram0_data_write = Input(Bool())
    val hps_f2h_sdram0_data_writedata = Input(UInt(128.W))
    val hps_f2h_sdram0_data_readdatavalid = Output(Bool())
    val hps_f2h_sdram0_data_waitrequest = Output(Bool())
    val hps_f2h_sdram0_data_byteenable = Input(UInt(16.W))
    val hps_f2h_sdram0_data_burstcount = Input(UInt(9.W))
    val memory_mem_a = Output(UInt(15.W))
    val memory_mem_ba = Output(UInt(3.W))
    val memory_mem_ck = Output(Bool())
    val memory_mem_ck_n = Output(Bool())
    val memory_mem_cke = Output(Bool())
    val memory_mem_cs_n = Output(Bool())
    val memory_mem_ras_n = Output(Bool())
    val memory_mem_cas_n = Output(Bool())
    val memory_mem_we_n = Output(Bool())
    val memory_mem_reset_n = Output(Bool())
    val memory_mem_dq = Analog(32.W)
    val memory_mem_dqs = Analog(4.W)
    val memory_mem_dqs_n = Analog(4.W)
    val memory_mem_odt = Output(Bool())
    val memory_mem_dm = Output(UInt(4.W))
    val memory_oct_rzqin = Input(Bool())
  })
}
