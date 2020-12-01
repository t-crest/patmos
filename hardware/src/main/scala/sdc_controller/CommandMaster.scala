package sdc_controller

import chisel3._
import chisel3.util.Enum

class CommandMaster extends Module {
  val io = IO(new Bundle() {
    val sd_clk = Input(UInt(1.W))
    val start_i = Input(UInt(1.W))
  })
  val sIdle :: sExecute :: sBusyCheck :: Nil = Enum(3)

}


object CommandMaster extends App {
  println("Generating the ALU hardware")
  chisel3.Driver.execute(Array("--target-dir", "generated"), () => new CommandMaster())
}