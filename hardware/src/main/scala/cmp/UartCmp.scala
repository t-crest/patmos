/*
 * A multicore Uart wrapper
 *
 * Author: Torur Biskopsto Strom (torur.strom@gmail.com)
 */

package cmp

import Chisel._
import ocp._
import patmos.Constants._
import patmos._
import io.Uart



class UartCmp(corecnt: Int, clk_freq: Int, baud_rate: Int, fifoDepth: Int) extends Module {

  override val io = IO(new CmpIO(corecnt) with patmos.HasPins {
    override val pins = new Bundle {
      val tx = Bits(OUTPUT, 1)
      val rx = Bits(INPUT, 1)
    }
  })

  val uart = Module(new Uart(clk_freq,baud_rate,fifoDepth))

  io.pins <> uart.io.pins

  uart.io.ocp.M := PriorityMux(io.cores.map(e => (e.M.Cmd =/= OcpCmd.IDLE, e.M)))
  
  for (i <- 0 until corecnt) {
    val cmdReg = Reg(init = Bool(false))
    when(io.cores(i).M.Cmd =/= OcpCmd.IDLE) {
      cmdReg := Bool(true)
    }.elsewhen(uart.io.ocp.S.Resp === OcpResp.DVA) {
      cmdReg := Bool(false)
    }
    
    io.cores(i).S.Data := uart.io.ocp.S.Data
    io.cores(i).S.Resp := uart.io.ocp.S.Resp
    when(cmdReg =/= Bool(true)) {
      io.cores(i).S.Resp := OcpResp.NULL
    }
  }
}
