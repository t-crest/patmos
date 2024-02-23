/*
 * Copyright: 2017, Technical University of Denmark, DTU Compute
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * License: Simplified BSD License
 */

package oneway

import chisel3._
import s4noc._

/**
 * Create and connect a n x n NoC.
 */
class OneWayMem(n: Int, memSize: Int) extends Module {
  val io = IO(new Bundle {
    // val dout = UInt(32.W).asOutput
    val memPorts = Vec(n * n, new DualPort(memSize))
  })

  // Dummy output keep hardware generated
  val dout = Reg(Vec(n * n, UInt(32.W)))

  val net = Module(new Network(n, UInt(32.W)))

  for (i <- 0 until n * n) {
    // val node = Module(new DummyNode(i))
    val node = Module(new Node(n, memSize))
    // how do we avoid confusing in/out names?
    net.io.local(i).in := node.io.local.out
    node.io.local.in := net.io.local(i).out
    io.memPorts(i) <> node.io.memPort
  }

  // single output for the synthesizer results
  // val or = dout.fold(0.U)(_ | _)
  // io.dout := Reg(next = or)

}

object OneWayMem extends App {
  emitVerilog(new OneWayMem(2, 1024), Array("-td", "generated"))
}
