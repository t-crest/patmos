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
  val address = UInt(w.W)
  val data = UInt(32.W)
  val valid = Bool()
}

class SingleChannel extends Bundle {
  val data = UInt(32.W)
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

class RouterPorts(w : Int) extends Bundle {
  val ports = Vec(Const.NR_OF_PORTS, new RwChannel(w))
}

class Router(schedule: Array[Array[Int]], validTab: Array[Boolean], inverted : Boolean, w : Int, timeshift: Int) extends Module {
  val io = new RouterPorts(w)
  val shift = if (inverted) timeshift else 0
  val regCounter = RegInit(shift.U(log2Up(schedule.length).W))
  val end = regCounter === (schedule.length - 1).U
  regCounter := Mux(end, 0.U, regCounter + 1.U)
  

  // Convert schedule table to a Chisel type table
  val sched = Vec(schedule.length, Vec(Const.NR_OF_PORTS, UInt(3.W)))
  for (i <- 0 until schedule.length) {
    for (j <- 0 until Const.NR_OF_PORTS) {
      sched(i)(j) := schedule(i)(j).U(3.W)
    }
  }

  // TDM schedule starts one cycles later for read data delay
  val regDelay = RegNext(regCounter, init=0.U)
  val currentSched = sched(regDelay)


  val setToZero = new RwChannel(w)
  setToZero.in.valid := false.B
  setToZero.in.address := 0.U
  
  // We assume that on reset the valid signal is false.
  // Better have it reset. 
  for (j <- 0 until Const.NR_OF_PORTS) {
    io.ports(j).out := RegNext(io.ports(currentSched(j)).in, init = setToZero.in)
  }
}


object Router extends App {

  chiselMain(Array("--backend", "v", "--targetDir", "generated"),
    () => Module(new Router(Schedule.genRandomSchedule(7), null, false, 8, 0)))
}
