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


  //val outputVec = Vec((n*n -1): Bool())
  val numbArray = new Array[Int](n*n)

  for (i <- 0 until n*n){
    numbArray(i) = i
  }


  val nodesx = for (i <- 0 until n*n) yield
  {
   val exe_unit = Module(new Node(n,i, memSize))
   // any wiring or other logic can go here
   exe_unit
  }
  //val nodes = Vec(numbArray.map(Module(new Node(n,_, memSize))))


  val NIs = for (i <- 0 until n*n) yield
  {
   val exe_unit = Module(new NI(n,i, memSize))
   // any wiring or other logic can go here
   exe_unit
  }

  val exe_units_io = Vec(NIs.map(_.io))

  for (i <- 0 until n * n) {


    NIs(i).io.memReq <> nodesx(i).io.local
    //outputVec(i) <> node.output
    
    NIs(i).io.writeChannel.in := writeNetwork.io.local(i).out
    writeNetwork.io.local(i).in := NIs(i).io.writeChannel.out

    NIs(i).io.readBackChannel.in := readBackNetwork.io.local(i).out
    readBackNetwork.io.local(i).in := NIs(i).io.readBackChannel.out
  }
}

object TwoWayMem {
  def main(args: Array[String]): Unit = {
    chiselMain(Array("--backend", "v", "--targetDir", "generated"),
      () => Module(new TwoWayMem(2, 1024)))
  }
}
