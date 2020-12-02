package sdc_controller

import chisel3._
import chisel3.util._

class sd_crc_16 extends Module {
  val io = IO(new Bundle {
    val bitval = Input(UInt(1.W))
    val enable = Input(UInt(1.W))
    val bitstrb = Input(UInt(1.W))
    val clear = Input(UInt(1.W))
    val crc = Output(UInt(16.W))
  })

  val crc_reg = Reg(Bits(16.W))
  val inv = io.bitval ^ crc_reg(15)

  when((RegNext(io.bitstrb) =/= io.bitstrb) || (RegNext(io.clear) =/= io.clear)) {
    when(io.clear === 1.U){
      crc_reg := 0.U
    }.elsewhen(io.enable === 1.U){
      val crc_reg_11 = crc_reg(11)
      val crc_reg_4 = crc_reg(4)
      crc_reg := crc_reg << 1
      crc_reg.bitSet(12.U, (crc_reg_11 ^ inv).asBool())
      crc_reg.bitSet(5.U, (crc_reg_4 ^ inv).asBool())
      crc_reg.bitSet(0.U, inv.asBool())
    }
  }
  io.crc <> crc_reg
}

object sd_crc_16 extends App {
  chisel3.Driver.execute(Array("--target-dir", "generated"), () => new sd_crc_16())
}