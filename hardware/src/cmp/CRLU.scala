/*
   Copyright (c) 2014 Technical University of Denmark, DTU Compute

   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted (subject to the limitations in the
   disclaimer below) provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.

	NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
	GRANTED BY THIS LICENSE.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
	HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
	WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
	MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
	BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
	WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
	OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
	IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	The views and conclusions contained in the software and documentation
	are those of the authors and should not be interpreted as representing
	official policies, either expressed or implied, of the copyright holder.

*/

/*
 * Speed Locking Unit
 * 
 * This component manages the synchronization locks for the processor cores. 
 * A core requests a lock and is halted until the lock becomes available.
 *
 * Author: Torur Biskopsto Strom (torur.strom@gmail.com)
 *
 */
package cmp
 
import Chisel._
import Node._

import patmos._
import patmos.Constants._
import ocp._
import io.CoreDeviceIO

class CRLU(coreCnt : Int,lckCnt : Int) extends Module {

  
  override val io = Vec.fill(coreCnt){new CRLUIO(lckCnt)}
  
  
  val queueReg = Vec.fill(lckCnt){Reg(init = Bits(0,coreCnt))}
  for (i <- 0 until lckCnt) {
    for (j <- 0 until coreCnt) {
      when(io(j).sel === UInt(i) && io(j).en === Bool(true)) {
        queueReg(i)(j) := io(j).op
      }
    }
  }

  
  
  
    val curReg = Vec.fill(lckCnt){Reg(init = UInt(0,log2Up(coreCnt)))}	
    
    // Circular priority encoder
    
  
  val hi = Vec.fill(lckCnt){Bits(width = coreCnt)}
  val lo = Vec.fill(lckCnt){Bits(width = coreCnt)}
  
  for (i <- 0 until lckCnt) {
    
    
    lo(i) := UInt(0)
    hi(i) := UInt(0)
    for (j <- 0 until coreCnt) {
      lo(i)(j) := queueReg(i)(j) && (curReg(i) < UInt(j))
      hi(i)(j) := queueReg(i)(j) && (curReg(i) >= UInt(j))
    }
    
    when(orR(hi(i))) {
      curReg(i) := PriorityEncoder(hi(i))
    }
    .otherwise {
      curReg(i) := PriorityEncoder(lo(i))
    }
  }
    
    // Counter
  
//  for (i <- 0 until lckCnt) {
//    
//    when(!queueReg(i)(curReg(i))) {
//      curReg(i) := curReg(i) + UInt(1)
//    }
//  }
  
  val blocks = Vec.fill(coreCnt)(Bits(width = lckCnt))
  
  for (i <- 0 until coreCnt) {
    blocks(i) := UInt(0)
    for (j <- 0 until lckCnt) {
      blocks(i)(j) := queueReg(j)(i) && (curReg(j) =/= UInt(i)) 
    }
    io(i).blck := orR(blocks(i))
  }
}

class CRLUOCPWrapper(coreCnt : Int,lckCnt : Int) extends Module {
  
  override val io = Vec.fill(coreCnt){new CoreDeviceIO}
  
  val crlu = Module(new CRLU(coreCnt,lckCnt))
  
  // Mapping between internal io and OCP here
  
  for (i <- 0 until coreCnt) {
    crlu.io(i).op := io(i).ocp.M.Data(0);
    crlu.io(i).sel := io(i).ocp.M.Data >> 1;
    crlu.io(i).en := Bool(false)
    when(io(i).ocp.M.Cmd === OcpCmd.WR) {
      crlu.io(i).en := Bool(true)
    }
    
    io(i).ocp.S.Resp := OcpResp.DVA
    when(crlu.io(i).blck === Bool(false)) {
        io(i).ocp.S.Resp := OcpResp.NULL
    }
    io(i).ocp.S.Data := UInt(0)
  }
}


class CRLUIO(lckCnt : Int) extends Bundle {
  val sel = UInt(INPUT,width = log2Up(lckCnt))
  val op = Bool(INPUT)
  val en = Bool(INPUT)
  val blck = Bool(OUTPUT)
}

class CRLUTest(c: CRLU) extends Tester(c) {
  poke(c.io(0).sel,0)
  poke(c.io(0).op,1)
  poke(c.io(0).en,1)
  poke(c.io(1).sel,0)
  poke(c.io(1).op,1)
  poke(c.io(1).en,1)
  peek(c.curReg)
  peek(c.queueReg)
  step(1)
  poke(c.io(0).op,0)
  poke(c.io(0).en,1)
  expect(c.io(0).blck, 0)
  expect(c.io(1).blck, 1)
  peek(c.curReg)
  peek(c.queueReg)
  step(1)
  poke(c.io(0).op,0)
  poke(c.io(0).en,1)
  expect(c.io(0).blck, 0)
  expect(c.io(1).blck, 1)
  peek(c.curReg)
  peek(c.queueReg)
  step(1)
  expect(c.io(0).blck, 0)
  expect(c.io(1).blck, 0)
  peek(c.curReg)
  peek(c.queueReg)
}

object CRLU {
  def main(args: Array[String]): Unit = {
    //chiselMainTest(Array[String]("--backend", "c", "--test", "--genHarness", "--compile"), () => Module(new SLU(8,8))){c => new SLUTests(c)}
    val crluargs = args.takeRight(2)
    val corecnt = crluargs.head.toInt;
    val lckcnt = crluargs.last.toInt;
    chiselMainTest(args.dropRight(2), () => Module(new CRLU(corecnt,lckcnt))) { f => new CRLUTest(f) }
  }
}
