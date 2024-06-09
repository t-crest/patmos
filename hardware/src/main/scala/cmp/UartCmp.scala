/*
 * A multicore Uart wrapper
 *
 * Author: Torur Biskopsto Strom (torur.strom@gmail.com)
 */

package cmp

import chisel3._
import chisel3.util._
import ocp._
import patmos.Constants._
import patmos._
import io.Uart




class UartCmp(corecnt: Int, clk_freq: Int, baud_rate: Int, fifoDepth: Int) extends CmpDevice(corecnt) {


  val io = IO(new CmpIO(corecnt) with patmos.HasPins {
    override val pins = new Bundle {
      val tx = Output(UInt(1.W))
      val rx = Input(UInt(1.W))
    }
  })



/*
  val abc = IO(new Bundle {
    val tx = Output(UInt(1.W))
    val rx = Input(UInt(1.W))
  })

 */
  /*
  val pins = IO(new Bundle() with HasPins {
    override val pins: Bundle = new Bundle {
      val tx = Output(UInt(1.W))
      val rx = Input(UInt(1.W))
    }
  })

   */

  val uart = Module(new Uart(clk_freq,baud_rate,fifoDepth))

  uart.io.superMode := false.B

  io.pins <> uart.io.pins

  uart.io.ocp.M := PriorityMux(io.cores.map(e => (e.M.Cmd =/= OcpCmd.IDLE, e.M)))
  
  for (i <- 0 until corecnt) {
    val cmdReg = RegInit(init = false.B)
    when(io.cores(i).M.Cmd =/= OcpCmd.IDLE) {
      cmdReg := true.B
    }.elsewhen(uart.io.ocp.S.Resp === OcpResp.DVA) {
      cmdReg := false.B
    }
    
    io.cores(i).S.Data := uart.io.ocp.S.Data
    io.cores(i).S.Resp := uart.io.ocp.S.Resp
    when(cmdReg =/= true.B) {
      io.cores(i).S.Resp := OcpResp.NULL
    }
  }
}
