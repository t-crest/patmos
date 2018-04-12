/*
  Top level of the S4NOC.

  Author: Martin Schoeberl (martin@jopdesign.com)
  license see LICENSE
 */

package s4noc

import Chisel._

class S4noc(n: Int) extends Module  {
  val io = new Bundle {
    val cpuPorts = Vec(n, new CpuPort())
  }

  val dim = math.sqrt(n).toInt
  if (dim * dim != n) throw new Error("Number of cores must be quadratic")

  val net = Module(new Network(dim))

  for (i <- 0 until n) {
    val ni = Module(new NetworkInterface(dim))
    net.io.local(i).in := ni.io.local.out
    ni.io.local.in := net.io.local(i).out
    io.cpuPorts(i) <> ni.io.cpuPort
  }
}

