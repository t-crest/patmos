/*
 * Copyright: 2017, Technical University of Denmark, DTU Compute
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * License: Simplified BSD License
 */

package s4noc

import Chisel._
import Const._

/**
 * Create and connect a n x n NoC.
 */
class Network(n: Int) extends Module {
  val io = new Bundle {
    val local = Vec(n * n, new Channel())
  }

  val schedule = Schedule.getSchedule(n)._1

  val net = new Array[Router](n * n)
  for (i <- 0 until n * n) {
    net(i) = Module(new Router(schedule))
    io.local(i).out := net(i).io.ports(LOCAL).out
    net(i).io.ports(LOCAL).in := io.local(i).in
  }

  // Router indexes:
  // 0 - 1 - .. - n-1
  // |   |
  // n - n+1 .. - 2n-1
  // .
  // .
  // |
  // (n-1)*n ... - (n-1)*n+n-1

  def connect(r1: Int, p1: Int, r2: Int, p2: Int): Unit = {
    net(r1).io.ports(p1).in := net(r2).io.ports(p2).out
    net(r2).io.ports(p2).in := net(r1).io.ports(p1).out
  }

  for (i <- 0 until n) {
    for (j <- 0 until n) {
      val r = i * n + j
      connect(r, EAST, i * n + (j + 1) % n, WEST)
      connect(r, SOUTH, (i + 1) % n * n + j, NORTH)
    }
  }
}
