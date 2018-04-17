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
    val nodearray = Vec{for (i <- 0 until n*n) yield
      {
        val node = Module(new Node(n,i, memSize))
        // any wiring or other logic can go here
        node.io
      }
    }
  val io = new Bundle {
    //val testSignal = new RwChannel(log2Down(memSize)).flip.asInput
    val nodearray = Vec(n*n, new Bundle{
        val local = new RwChannel(memoryWidth)
        val output = Bool().asOutput
        val test = new RwChannel(memoryWidth).flip
        }
    )

  }

  for(i <- 0 until n*n){
    io.nodearray(i).test <> nodearray(i).test
  }

  // Dummy output keep hardware generated
  val dout = Reg(next = Vec(n * n, UInt(width = 32)))

  val writeNetWidth = log2Down(memSize) + 1 + 32

  // Instantiate the two NoCs
  val readBackNetwork = Module(new Network(n, 32, true))
  val writeNetwork = Module(new Network(n, writeNetWidth, false))


  // Create (dummy) nodes


  // Create network interfaces (NI)
  val NIs = for (i <- 0 until n*n) yield
  {
   val singleNI = Module(new NI(n,i, memSize))
   // any wiring or other logic can go here
   singleNI
  }

  // Connect network interfaces with nodes & NoCs
  for (i <- 0 until n * n) {

    NIs(i).io.memReq.in <>  nodearray(i).local.out
    NIs(i).io.memReq.out <>  nodearray(i).local.in
    
    //NIs(i).io.memReq.out.rw := Bool(true)//nodearray(i).local.out.rw
    
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
      () => Module(new TwoWayMem(2, 1024)))
  }
}
