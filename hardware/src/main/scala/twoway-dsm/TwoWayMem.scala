/*
 * Copyright: 2017, Technical University of Denmark, DTU Compute
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * License: Simplified BSD License
 */

package twoway

import Chisel._
import s4noc_twoway._

/**
 * Create and connect a n x n NoC.
 */
class TwoWayMem(n: Int, memSize: Int) extends Module {
  val io = new Bundle {
    // val dout = UInt(width = 32).asOutput
    val memPorts = Vec(n * n, new DualPort(memSize))
  }

  // Dummy output keep hardware generated
  val dout = Reg(next = Vec(n * n, UInt(width = 32)))

  val tx_net = Module(new Network(n))
  val rx_net = Module(new Network(n))

  for (i <- 0 until n * n) {
    // val node = Module(new DummyNode(i))
    val ni = Module(new NI(n, memSize))
    // how do we avoid confusing in/out names?
    tx_net.io.local(i).in := ni.io.local.out
    ni.io.local.in := tx_net.io.local(i).out
    io.memPorts(i) <> ni.io.memPort
  }

  // TODO: Connect rx network to node

  // single output for the synthesizer results
  // val or = dout.fold(UInt(0))(_ | _)
  // io.dout := Reg(next = or)

}

object TwoWayMem extends App {

  chiselMain(Array("--backend", "v", "--targetDir", "generated"),
    () => Module(new OneWayMem(2, 1024)))
}
