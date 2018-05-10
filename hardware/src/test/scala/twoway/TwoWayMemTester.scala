/*
 * Copyright: 2017, Technical University of Denmark, DTU Compute
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * License: Simplified BSD License
 */

package twoway

import Chisel._

/**
 * Test a 2x2 Network.
 */
class TestLocalReadWrite(dut: TwoWayMem) extends Tester(dut) {  
  for (j <- 0 until 4) {
      //Write to all memReq channels, asking for memory in node 3
      poke(dut.io.nodearray(j).out.rw, 0)  
      poke(dut.io.nodearray(j).out.data, 0)
      poke(dut.io.nodearray(j).out.address, 0)
      poke(dut.io.nodearray(j).out.valid, false)
  }  

  step(1)

  //Write to all memReq channels, asking for memory in node 3
  poke(dut.io.nodearray(0).out.rw, 1)  
  poke(dut.io.nodearray(0).out.data, 0x42)
  poke(dut.io.nodearray(0).out.address, 0x42)
  poke(dut.io.nodearray(0).out.valid, true)

  step(1)


  //Wait for valid returns
  while(peek(dut.io.nodearray(0).in.valid) == 0){
    step(1)
  }

  poke(dut.io.nodearray(0).out.valid, false)
  step(1)


  //Read same value that we wrote, hopefully
  poke(dut.io.nodearray(0).out.rw, 0)  
  poke(dut.io.nodearray(0).out.data, 0x00)
  poke(dut.io.nodearray(0).out.address, 0x42)
  poke(dut.io.nodearray(0).out.valid, true)

  step(1)

  while(peek(dut.io.nodearray(0).in.valid) == 0){
    step(1)
  }
  poke(dut.io.nodearray(0).out.valid, false)

  step(3)
}

class TestExternalWrite(dut: TwoWayMem) extends Tester(dut) {

    //Set all inputs to 0
    for (j <- 0 until 4) {
      poke(dut.io.nodearray(j).out.rw, 0)  
      poke(dut.io.nodearray(j).out.data, 0)
      poke(dut.io.nodearray(j).out.address, 0)
      poke(dut.io.nodearray(j).out.valid, false)
  }  

  step(1)


  poke(dut.io.nodearray(0).out.rw, 1)  
  poke(dut.io.nodearray(0).out.data, 0x42)
  poke(dut.io.nodearray(0).out.address, 0x342) //Write to node 1
  poke(dut.io.nodearray(0).out.valid, true)

  step(1)

  peek(dut.NIs(0).io.memReq.in.data)

  //Wait for valid returns
  while(peek(dut.io.nodearray(0).in.valid) == 0){
    step(1)
  }

  poke(dut.io.nodearray(0).out.valid, false)
  step(1)


  var count = 0
  while(peek(dut.NIs(0).io.writeChannel.in.valid) == 0 && count <= 15){
  step(1)
  peek(dut.NIs(3).io.writeChannel.in.valid)
  peek(dut.NIs(3).io.writeChannel.in.data)
  peek(dut.NIs(3).io.writeChannel.in.address)
  peek(dut.NIs(3).io.memPort.io.portB.addr)
  peek(dut.NIs(3).io.memPort.io.portB.wrEna)
  


  count +=1
  }


  //Check new data:
  //Write to all memReq channels, asking for memory in node 3
  poke(dut.io.nodearray(3).out.rw, 0)  
  poke(dut.io.nodearray(3).out.address, 0x342)
  poke(dut.io.nodearray(3).out.valid, true)

  step(1)

  peek(dut.io.nodearray(3).in.data)

  step(1)
  peek(dut.io.nodearray(3).in.data)
}

class TestExternalReadback(dut: TwoWayMem) extends Tester(dut) {

  
  for (j <- 0 until 16) {
    //Write 0 to all nodes
    poke(dut.io.nodearray(j).out.rw, 0)  
    poke(dut.io.nodearray(j).out.data, 0)
    poke(dut.io.nodearray(j).out.address, 0)
    poke(dut.io.nodearray(j).out.valid, false)
  }  
  step(1)

  val transmittingNode = 0;
  val receivingNode = 3;

  //Write 0x42 to address 0x342, which is in node 3.
  poke(dut.io.nodearray(receivingNode).out.rw, 1)  
  poke(dut.io.nodearray(receivingNode).out.data, 0x2)
  poke(dut.io.nodearray(receivingNode).out.address, 0x2 + 0x40 * receivingNode)
  poke(dut.io.nodearray(receivingNode).out.valid, true)

  step(1)

  //Set node 3 to 0 again.
  poke(dut.io.nodearray(receivingNode).out.rw, 0)  
  poke(dut.io.nodearray(receivingNode).out.data, 0x00)
  poke(dut.io.nodearray(receivingNode).out.address, 0x00)
  poke(dut.io.nodearray(receivingNode).out.valid, false)

  step(1)


  //Ask for memory 0x342 from node 0.
  poke(dut.io.nodearray(transmittingNode).out.rw, 0)  
  poke(dut.io.nodearray(transmittingNode).out.address, 0x2 + 0x40 * receivingNode)
  poke(dut.io.nodearray(transmittingNode).out.valid, true)

  var counter = 0
  while(peek(dut.io.nodearray(transmittingNode).in.valid) == 0 && counter < 40){
    step(1)
    counter += 1


  }

  peek(dut.io.nodearray(transmittingNode).in.data)
  peek(dut.io.nodearray(transmittingNode).in.valid)

  step(3)
}

class TestLocalSequentialWriteRead(dut: TwoWayMem) extends Tester(dut) {
   //Write four different values in receiving node (address 42,43,44,45)
  val receivingNode = 0;
  for(i <- 0 until 4){
    poke(dut.io.nodearray(receivingNode).out.rw, 1)  
    poke(dut.io.nodearray(receivingNode).out.data, 0x42 + i)
    poke(dut.io.nodearray(receivingNode).out.address, 0x42 + i + 0x100 * receivingNode)
    poke(dut.io.nodearray(receivingNode).out.valid, true)
    step(1)
  }

  step(1)

  // Read the written adresses
  for(i <- 0 until 4){
    poke(dut.io.nodearray(receivingNode).out.rw, 0)  
    poke(dut.io.nodearray(receivingNode).out.address, 0x42 + i + 0x100 * receivingNode)
    poke(dut.io.nodearray(receivingNode).out.valid, true)
    step(1)
    peek(dut.io.nodearray(receivingNode).in.data)
  }
}

class TestSimultaniousReads(dut: TwoWayMem) extends Tester(dut) {
  for (j <- 0 until 4) {
    //Write 0 to all nodes
    poke(dut.io.nodearray(j).out.rw, 0)  
    poke(dut.io.nodearray(j).out.data, 0)
    poke(dut.io.nodearray(j).out.address, 0)
    poke(dut.io.nodearray(j).out.valid, false)
  }  
  step(1)

  val receivingNode = 0;

  //Write four different values in receiving node (address 42,43,44,45)
  for(i <- 0 until 4){
    poke(dut.io.nodearray(receivingNode).out.rw, 1)  
    poke(dut.io.nodearray(receivingNode).out.data, 0x42 + i)
    poke(dut.io.nodearray(receivingNode).out.address, 0x42 + i + 0x100 * receivingNode)
    poke(dut.io.nodearray(receivingNode).out.valid, true)
    step(1)
  }

  //Set receiving node to not write
  poke(dut.io.nodearray(receivingNode).out.rw, 0)  
  poke(dut.io.nodearray(receivingNode).out.data, 0x00)
  poke(dut.io.nodearray(receivingNode).out.address, 0x00)
  poke(dut.io.nodearray(receivingNode).out.valid, false)
  step(1)


  //Ask all nodes to receive memory from the receiving node
  for(i <- 0 until 4){
    poke(dut.io.nodearray(i).out.rw, 0)  
    poke(dut.io.nodearray(i).out.address, 0x42 + i + 0x100 * receivingNode)
    poke(dut.io.nodearray(i).out.valid, true)
  }
  step(1)

  var counter = 0
  while(counter < 20){
    step(1)
    counter += 1
    for(i <- 0 until 4){
      peek(dut.io.nodearray(i).in.valid)
      peek(dut.io.nodearray(i).in.data)
      if (peek(dut.io.nodearray(i).in.data) != 0){
        step(1)
        poke(dut.io.nodearray(i).out.valid, false)
      }
    }

  }


  step(3)
}

class TestExternalReadbackAll(dut: TwoWayMem) extends Tester(dut) {
  val n = 16

  for (j <- 0 until n) {
    //Write 0 to all nodes
    poke(dut.io.nodearray(j).out.rw, 0)  
    poke(dut.io.nodearray(j).out.data, 0)
    poke(dut.io.nodearray(j).out.address, 0)
    poke(dut.io.nodearray(j).out.valid, false)
  }  
  step(1)

  println(s"Starting")
  for(j <- 0 until n){
    //Write 0x42 to address 0x342, which is in node 3.
    poke(dut.io.nodearray(j).out.rw, 1)  
    poke(dut.io.nodearray(j).out.data, 0x2 + j)
    poke(dut.io.nodearray(j).out.address, 0x2 + 0x40 * j)
    poke(dut.io.nodearray(j).out.valid, true)
  }
  step(1)
  //Set nodes to 0 again.
  for(j <- 0 until n){
    poke(dut.io.nodearray(j).out.rw, 0)  
    poke(dut.io.nodearray(j).out.data, 0x00)
    poke(dut.io.nodearray(j).out.address, 0x00)
    poke(dut.io.nodearray(j).out.valid, false)
  }

  step(5)

  println(s"Requesting")

  //Have all nodes request a datapoint from all nodes, one at a time.
  //We only hold the request high for a single cycle, which is 
  //what the OCPCore dictates.
  for(j <- 0 until n){
    for(i <- 0 until n){
      //Ask for memory 0x342 from node 0.
      poke(dut.io.nodearray(j).out.rw, 0)  
      poke(dut.io.nodearray(j).out.address,  0x2 + 0x40 * i)
      poke(dut.io.nodearray(j).out.valid, true)

      step(1)

      poke(dut.io.nodearray(j).out.rw, 0)  
      poke(dut.io.nodearray(j).out.address, 0)
      poke(dut.io.nodearray(j).out.valid, false)


      var counter = 0
      while(peek(dut.io.nodearray(j).in.valid) == 0 && counter < 40){
        step(1)
        counter += 1
      }
      if(counter == 40){
        println(s"DID NOT RECEIVE")
      }
      expect(dut.io.nodearray(j).in.data, 0x2 + i)
      expect(dut.io.nodearray(j).in.valid, 1)


      poke(dut.io.nodearray(j).out.rw, 0)  
      poke(dut.io.nodearray(j).out.address, 0)
      poke(dut.io.nodearray(j).out.valid, false)
      step(20)
    }
  }
}

object TwoWayMemTester {
  def main(args: Array[String]): Unit = {
    chiselMainTest(Array("--genHarness", "--test", "--backend", "c",
      "--compile", "--vcd", "--targetDir", "generated","--debug"),
      () => Module(new TwoWayMem(4, 1024))) {
        c => new TestExternalReadback(c)

      }
  }
}