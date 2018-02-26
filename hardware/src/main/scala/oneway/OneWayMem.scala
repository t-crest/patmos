/*
 * Copyright: 2017, Technical University of Denmark, DTU Compute
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * License: Simplified BSD License
 */

package oneway

import Chisel._
import Const._

/**
 * Create and connect a n x n NoC.
 */
class OneWayMem(n: Int, memSize: Int) extends Module {
  val io = new Bundle {
    // val dout = UInt(width = 32).asOutput
    val memPorts = Vec(n * n, new DualPort(memSize))
  }

  // Dummy output keep hardware generated
  val dout = Reg(next = Vec(n * n, UInt(width = 32)))

  val net = Module(new Network(n))

  for (i <- 0 until n * n) {
    // val node = Module(new DummyNode(i))
    val node = Module(new Node(n, memSize))
    // how do we avoid confusing in/out names?
    net.io.local(i).in := node.io.local.out
    node.io.local.in := net.io.local(i).out
    io.memPorts(i) <> node.io.memPort
  }

  // single output for the synthesizer results
  // val or = dout.fold(UInt(0))(_ | _)
  // io.dout := Reg(next = or)

}

object OneWayMem extends App {

  chiselMain(Array("--backend", "v", "--targetDir", "generated"),
    () => Module(new OneWayMem(2, 1024)))
}
