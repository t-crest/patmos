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
  override def cloneType: this.type = new SingleRwChannel(w).asInstanceOf[this.type]
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
  override def cloneType: this.type = new RwChannel(w).asInstanceOf[this.type]
  // Channel with arbitrary address width, used in the two-way shared memory interface
  val out = new SingleRwChannel(w).asOutput
  val in = new SingleRwChannel(w).asInput
}

class RouterPorts(w : Int) extends Bundle {
  val ports = Vec(Const.NR_OF_PORTS, new RwChannel(w))
}

class Router(schedule: Array[Array[Int]], validTab: Array[Boolean], inverted : Boolean, w : Int, timeshift: Int) extends Module {
  val io = new RouterPorts(w)
  //val timeshift = if(inverted){2}else{0} // Since as this version is works only 2x2 and 3x3. there are maximally 3 timeslots from the start of a route to local.

  val regCounter = RegInit(UInt( 0, log2Up(schedule.length)))
  val end = regCounter === UInt(schedule.length - 1)
  regCounter := Mux(end, UInt(0), regCounter + UInt(1))
  

  // Convert schedule table to a Chisel type table
  val sched = Vec(schedule.length, Vec(Const.NR_OF_PORTS, UInt(width = 3)))

  val valid = Vec(validTab.map(Bool(_)))

  for (i <- 0 until schedule.length) {
    for (j <- 0 until Const.NR_OF_PORTS) {
      sched(i)(j) := UInt(schedule(i)(j), 3)
    }
  }


  // TDM schedule starts one cycles later for read data delay
  val regDelay = RegNext(regCounter, init=UInt(0))
  val currentSched = sched(regDelay)





  //FIX DOWN HERE!
  
  //Delay the valid signal to avoid sending when router is not listening.
  val delayValid = Reg(Bool(true), next = RegNext(valid(regDelay)))
  val delayValid2 = RegNext(delayValid)

  debug(currentSched)
  debug(sched)
  debug(delayValid)
  debug(regDelay)

  // We assume that on reset the valid signal is false.
  // Better have it reset. 
  for (j <- 0 until Const.NR_OF_PORTS) {
    val del1 = RegNext(io.ports(currentSched(j)).in)
    val del2 = del1//RegNext(del1) //Is this wrong??
    io.ports(j).out := Mux(delayValid2, del1,del2)
  }

}

object Router extends App {

  chiselMain(Array("--backend", "v", "--targetDir", "generated"),
    () => Module(new Router(Schedule.genRandomSchedule(7), null, false, 8, 0)))
}
