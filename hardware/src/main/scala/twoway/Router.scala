/*
 * Copyright: 2017, Technical University of Denmark, DTU Compute
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * License: Simplified BSD License
 * 
 * A router for the S4NOC.
 * 
 */

package s4noc_twoway

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

class SingleRwChannel(w: Int) extends Bundle {
  val rw = Bool() // 1: Write, 0 : read
  val address = UInt(width = w)
  val data = UInt(width = 32)
  val valid = Bool()
}

class SingleChannel extends Bundle {
  val data = UInt(width = 32)
  val valid = Bool()
}

class Channel extends Bundle {
  val out = new SingleChannel().asOutput
  val in = new SingleChannel().asInput
}

class RwChannel(w: Int) extends Bundle {
  // Channel with arbitrary address width, used in the two-way shared memory interface
  val out = new SingleRwChannel(w).asOutput
  val in = new SingleRwChannel(w).asInput
}

class RouterPorts extends Bundle {
  val ports = Vec(Const.NR_OF_PORTS, new Channel())
}

class Router(schedule: Array[Array[Int]], inverted : Boolean) extends Module {
  val io = new RouterPorts
  val timeshift = if(inverted){4}else{0} // Since as this version is works only 2x2 and 3x3. there are maximally 3 timeslots from the start of a route to local.

  val regCounter = RegInit(UInt(0+timeshift, log2Up(schedule.length)))
  val end = regCounter === UInt(schedule.length - 1)
  regCounter := Mux(end, UInt(0), regCounter + UInt(1))
  

  // Convert schedule table to a Chisel type table
  val sched = Vec(schedule.length, Vec(Const.NR_OF_PORTS, UInt(width = 3)))
  for (i <- 0 until schedule.length) {
    for (j <- 0 until Const.NR_OF_PORTS) {
      sched(i)(j) := UInt(schedule(i)(j), 3)
    }
  }

  // TDM schedule starts one cycles later for read data delay
  val regDelay = RegNext(regCounter, init=UInt(0))
  val currentSched = sched(regDelay)

  // We assume that on reset the valid signal is false.
  // Better have it reset. 
  for (j <- 0 until Const.NR_OF_PORTS) {
    io.ports(j).out := RegNext(io.ports(currentSched(j)).in)
  }
}

object Router extends App {

  chiselMain(Array("--backend", "v", "--targetDir", "generated"),
    () => Module(new Router(Schedule.genRandomSchedule(7), false)))
}
