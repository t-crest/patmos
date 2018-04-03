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
  }
  // Dummy output keep hardware generated
  val dout = Reg(next = Vec(n * n, UInt(width = 32)))

  val writeNetWidth = log2Down(memSize) + 1 + 32

  val readBackNetwork = Module(new Network(n, 32, true))
  val writeNetwork = Module(new Network(n, writeNetWidth, false))

  for (i <- 0 until n * n) {
    val ni = Module(new NI(n,i, memSize))
    ni.io.writeChannel.in := writeNetwork.io.local(i).out
    writeNetwork.io.local(i).in := ni.io.writeChannel.out

    ni.io.readBackChannel.in := readBackNetwork.io.local(i).out
    readBackNetwork.io.local(i).in := ni.io.readBackChannel.out
  }
}

object TwoWayMem {
  def main(args: Array[String]): Unit = {
    chiselMain(Array("--backend", "v", "--targetDir", "generated"),
      () => Module(new TwoWayMem(2, 1024)))
  }
}
