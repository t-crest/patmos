/*
  A simple bubbling FIFO.

  Author: Martin Schoeberl (martin@jopdesign.com)
  license see LICENSE
 */

// TODO: should extend an abstract FIFO class for different implementation.
// TODO: how shall we organize this kind of utility classes?

package s4noc

import Chisel._

/*
 * On signal naming:
 *
 * Alter's FIFO component:
 *
 * data - data in, q - data out, wrreq and rdreq
 * state: full and empty
 *
 * Xilinx's FIFO component:
 * din and dout, wr_en, rd_en
 * state: full and empty
 *
 */

class WriterIO() extends Bundle {
  val write = Input(Bool())
  val full = Output(Bool())
  val din = Input(new Entry())
}

class ReaderIO() extends Bundle {
  val read = Input(Bool())
  val empty = Output(Bool())
  val dout = Output(new Entry())
}

/**
  * A single register (=stage) to build the FIFO.
  */
class FifoRegister() extends Module {
  val io = new Bundle {
    val enq = new WriterIO()
    val deq = new ReaderIO()
  }

  val empty :: full :: Nil = Enum(UInt(), 2)
  val stateReg = Reg(init = empty)
  val dataReg = Reg(new Entry())

  when(stateReg === empty) {
    when(io.enq.write) {
      stateReg := full
      dataReg := io.enq.din
    }
  }.elsewhen(stateReg === full) {
    when(io.deq.read) {
      stateReg := empty
    }
  }.otherwise {
    // There should not be an otherwise state
  }

  io.enq.full := (stateReg === full)
  io.deq.empty := (stateReg === empty)
  io.deq.dout := dataReg
}
/**
  * This is a bubble FIFO.
  */
class BubbleFifo(depth: Int) extends Module {
  val io = new Bundle {
    val enq = new WriterIO()
    val deq = new ReaderIO()
  }

  val stage = Module(new FifoRegister())
  val buffers = Array.fill(depth) { Module(new FifoRegister()) }
  for (i <- 0 until depth - 1) {
    buffers(i + 1).io.enq.din := buffers(i).io.deq.dout
    buffers(i + 1).io.enq.write := ~buffers(i).io.deq.empty
    buffers(i).io.deq.read := ~buffers(i + 1).io.enq.full
  }
  io.enq <> buffers(0).io.enq
  io.deq <> buffers(depth - 1).io.deq
}

/**
  * Test the design.
  */
/* commented out Chisel3 tester has changed see https://github.com/schoeberl/chisel-examples/blob/master/TowardsChisel3.md 
class FifoTester(dut: BubbleFifo) extends Tester(dut) {

  // some defaults for all signals
  poke(dut.io.enq.din.data, 0xab)
  poke(dut.io.enq.write, 0)
  poke(dut.io.deq.read, 0)
  step(1)
  var full = peek(dut.io.enq.full)
  var empty = peek(dut.io.deq.empty)

  // write into the buffer
  poke(dut.io.enq.din.data, 0x12)
  poke(dut.io.enq.write, 1)
  step(1)
  full = peek(dut.io.enq.full)

  poke(dut.io.enq.din.data, 0xff)
  poke(dut.io.enq.write, 0)
  step(1)
  full = peek(dut.io.enq.full)

  step(3) // see the bubbling of the first element

  // Fill the whole buffer with a check for full condition
  // Only every second cycle a write can happen.
  for (i <- 0 until 7) {
    full = peek(dut.io.enq.full)
    poke(dut.io.enq.din.data, 0x80 + i)
    if (full == 0) {
      poke(dut.io.enq.write, 1)
    } else {
      poke(dut.io.enq.write, 0)
    }
    step(1)
  }

  // Now we know it is full, so do a single read and watch
  // how this empty slot bubble up to the FIFO input.
  poke(dut.io.deq.read, 1)
  step(1)
  poke(dut.io.deq.read, 0)
  step(6)

  // New read out the whole buffer.
  // Also watch that maximum read out is every second clock cycle
  for (i <- 0 until 7) {
    empty = peek(dut.io.deq.empty)
    if (empty == 0) {
      poke(dut.io.deq.read, 1)
    } else {
      poke(dut.io.deq.read, 0)
    }
    step(1)
  }

  // Now write and read at maximum speed for some time
  for (i <- 1 until 16) {
    full = peek(dut.io.enq.full)
    poke(dut.io.enq.din.data, i)
    if (full == 0) {
      poke(dut.io.enq.write, 1)
    } else {
      poke(dut.io.enq.write, 0)
    }
    empty = peek(dut.io.deq.empty)
    if (empty == 0) {
      poke(dut.io.deq.read, 1)
    } else {
      poke(dut.io.deq.read, 0)
    }
    step(1)
  }

}

object FifoTester {
  def main(args: Array[String]): Unit = {
    chiselMainTest(Array("--genHarness", "--test", "--backend", "c",
      "--compile", "--vcd", "--targetDir", "generated"),
      () => Module(new BubbleFifo(4))) {
      f => new FifoTester(f)
    }
  }
}*/
