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

  val memoryWidth = log2Down(memSize)
  val io = new Bundle {
    //val testSignal = new RwChannel(log2Down(memSize)).flip.asInput

    val nodearray = Vec(n*n, new RwChannel(memoryWidth)).flip

  }



  // Dummy output keep hardware generated
  val dout = Reg(next = Vec(n * n, UInt(32.W)))

  val writeNetWidth = log2Down(memSize) - log2Down(n*n)

  // Instantiate the two NoCs. Readback has no address so 1
  val readBackNetwork = Module(new Network(n, 1, true))
  val writeNetwork = Module(new Network(n, writeNetWidth, false))


  // Create network interfaces (NI)
  val NIs = for (i <- 0 until n*n) yield
  {
   val singleNI = Module(new NI(n,i, memSize))
   // any wiring or other logic can go here
   singleNI
  }

  // Connect network interfaces with nodes & NoCs
  for (i <- 0 until n * n) {

    NIs(i).io.memReq.in <>  io.nodearray(i).out
    NIs(i).io.memReq.out <>  io.nodearray(i).in
    
    //NIs(i).io.memReq.out.rw := true.B//nodearray(i).local.out.rw
    
    NIs(i).io.writeChannel.in := writeNetwork.io.local(i).out
    writeNetwork.io.local(i).in := NIs(i).io.writeChannel.out

    NIs(i).io.readBackChannel.in := readBackNetwork.io.local(i).out
    readBackNetwork.io.local(i).in := NIs(i).io.readBackChannel.out
  }

  //io.testSignal <> nodearray(0).io.test
  //io.testSignal.in.data := nodearray(0).io.local.out.data
}

object TwoWayMem {
  def main(args: Array[String]): Unit = {
    chiselMain(Array("--backend", "v", "--targetDir", "generated"),
      () => Module(new TwoWayMem(4, 1024)))
  }
}
