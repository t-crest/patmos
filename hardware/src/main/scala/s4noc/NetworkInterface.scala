/*
  Network interface for the S4NOC.

  Author: Martin Schoeberl (martin@jopdesign.com)
  license see LICENSE
 */
package s4noc

import chisel3._
import chisel3.util._

class CpuPort() extends Bundle {
  val addr = Input(UInt(8.W))
  val rdData = Output(UInt(32.W))
  val wrData = Input(UInt(32.W))
  val rd = Input(Bool())
  val wr = Input(Bool())
}

// This should be a generic for the FIFO
class Entry extends Bundle {
  val data = Output(UInt(32.W))
  val time = Input(UInt(8.W))
}

class NetworkInterface[T <: Data](dim: Int, txFifo: Int, rxFifo: Int, dt: T) extends Module {
  val io = new Bundle {
    val cpuPort = new CpuPort()
    val local = new Channel(dt)
  }

  // TODO: too much repetition
  // Either provide the schedule as parameter
  // or simply read out the TDM counter from the router.
  // Why duplicating it? Does it matter?
  val len = Schedule.getSchedule(dim)._1.length

  val regCnt = RegInit(init = 0.U(log2Up(len).W))
  regCnt := Mux(regCnt === (len - 1).U, 0.U, regCnt + 1.U)
  // TDM schedule starts one cycles later for read data delay of OneWayMemory
  // Maybe we can use that delay here as well for something good
  val regDelay = RegNext(regCnt, init = 0.U)


  val entryReg = Reg(new Entry())
  when(io.cpuPort.wr) {
    entryReg.data := io.cpuPort.wrData
    entryReg.time := io.cpuPort.addr
  }

  val inFifo = Module(new BubbleFifo(rxFifo))
  inFifo.io.enq.write := false.B
  inFifo.io.enq.din.data := io.cpuPort.wrData
  inFifo.io.enq.din.time := io.cpuPort.addr
  when (io.cpuPort.wr && !inFifo.io.enq.full) {
    inFifo.io.enq.write := true.B
  }

  io.local.out.data := inFifo.io.deq.dout.data
  val doDeq = !inFifo.io.deq.empty && regDelay === inFifo.io.deq.dout.time
  io.local.out.valid := doDeq
  inFifo.io.deq.read := doDeq

  // TODO: what is the rd timing? Same clock cycle or next clock cycle?
  // Maybe we should do the AXI interface for all future designs
  // In our OCP interface we have defined it...

  // for now same clock cycle

  val outFifo = Module(new BubbleFifo(txFifo))
  outFifo.io.enq.write := false.B
  outFifo.io.enq.din.data := io.local.in.data
  outFifo.io.enq.din.time := regDelay
  when (io.local.in.valid && !outFifo.io.enq.full) {
    outFifo.io.enq.write := true.B
  }

  io.cpuPort.rdData := outFifo.io.deq.dout.data
  outFifo.io.deq.read := false.B
  val regTime = RegInit(0.U(6.W))
  when (io.cpuPort.rd) {
    val addr = io.cpuPort.addr
    when (addr === 0.U)  {
      outFifo.io.deq.read := true.B
    } .elsewhen(addr === 1.U) {
      io.cpuPort.rdData := regTime
    } .elsewhen(addr === 2.U) {
      io.cpuPort.rdData := Cat(0.U(31.W), !inFifo.io.enq.full)
    } .elsewhen(addr === 3.U) {
      io.cpuPort.rdData := Cat(0.U(31.W), !outFifo.io.deq.empty)
    }
  }
}
