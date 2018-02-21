/*
 * Copyright: 2017, Technical University of Denmark, DTU Compute
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * License: Simplified BSD License
 * 
 * A router for the S4NOC.
 * 
 */

package oneway

import Chisel._

/**
 * Channel directions
 */
object Const {
  val NORTH = 0
  val EAST = 1
  val SOUTH = 2
  val WEST = 3
  val LOCAL = 4
  val NR_OF_PORTS = 5
}
import Const._

class SingleChannel extends Bundle {
  val data = UInt(width = 32)
  val valid = Bool()
}

class Channel extends Bundle {
  val out = new SingleChannel().asOutput
  val in = new SingleChannel().asInput
}

class RouterPorts extends Bundle {
  val ports = Vec(new Channel(), Const.NR_OF_PORTS)
}

class Router(schedule: Array[Array[Int]]) extends Module {
  val io = new RouterPorts

  val regCounter = RegInit(init = UInt(0, 8))
  val end = regCounter === UInt(schedule.length - 1)

  regCounter := Mux(end, UInt(0), regCounter + UInt(1))

  // Convert schedule table to a Chisel types table
  val sched = Vec(schedule.length, Vec(Const.NR_OF_PORTS, UInt(width = 3)))
  for (i <- 0 until schedule.length) {
    for (j <- 0 until Const.NR_OF_PORTS) {
      sched(i)(j) := UInt(schedule(i)(j), 3)
    }
  }

  val currentSched = sched(regCounter)

  // This Mux is a little bit wasteful, as it allows from all
  // ports to all ports. A 4:1 instead of the 5:1 is way cheaper in an 4-bit LUT FPGA.
  // But synthesize removes the unused input.
  for (i <- 0 until Const.NR_OF_PORTS) {
    io.ports(i).out := Reg(next = io.ports(currentSched(i)).in)
  }
}

object Router extends App {

  chiselMain(Array("--backend", "v", "--targetDir", "generated"),
    () => Module(new Router(Schedule.genRandomSchedule(7))))
}
