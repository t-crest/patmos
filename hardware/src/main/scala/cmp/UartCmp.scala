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

object UartCmp {
  trait Pins {
    val uartCmpPins = new Bundle() {
      val tx = Bits(OUTPUT, 1)
      val rx = Bits(INPUT, 1)
    }
  }
}

class CmpIO(corecnt : Int) extends Bundle
{
  val cores = Vec(corecnt, new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH))

  override def clone = new CmpIO(corecnt).asInstanceOf[this.type]
}

class UartCmp(corecnt: Int, clk_freq: Int, baud_rate: Int, fifoDepth: Int) extends Module {

  val io = new CmpIO(corecnt) with UartCmp.Pins
  
  val uart = Module(new Uart(clk_freq,baud_rate,fifoDepth))
  
  io.uartCmpPins <> uart.io.uartPins

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
