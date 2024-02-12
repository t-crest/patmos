/*
 * Copyright: 2018, Technical University of Denmark, DTU Compute
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * License: Simplified BSD License
 * 
 * OCP wrapper
 */
package cmp

import Chisel._

import patmos._
import patmos.Constants._
import ocp._
import io.CoreDeviceIO
import twoway._

class XXXIO(lckCnt: Int) extends Bundle {
  val sel = UInt(INPUT, log2Up(lckCnt))
  val op = Input(Bool())
  val en = Input(Bool())
  val blck = Output(Bool())
}

// Maybe I should also use this functional approach, but not for a quick test now
// class TwoWayOCPWrapper(hardlockgen: () => AbstractHardlock) extends Module {

// TODO: is this dead code? If so, just delete it.
class TwoWayOCPWrapper(nrCores: Int, memSizePrNI : Int) extends CmpDevice(nrCores) {

  val dim = math.sqrt(nrCores).toInt
  if (dim * dim != nrCores) throw new Error("Number of cores must be quadratic")
  // just start with four words
  if (log2Down(memSizePrNI) != log2Up(memSizePrNI)) throw new Error("Memory per node must be an even power of 2")
  val size = nrCores * memSizePrNI
  val twowaymem = Module(new twoway.TwoWayMem(dim, size))

  println("TwoWayMem")

  // Mapping between TwoWay memories and OCP

  for (i <- 0 until nrCores) {


    // Should response be this or valid signal??
    //val resp = Mux(io(i).M.Cmd === OcpCmd.RD || io(i).M.Cmd === OcpCmd.WR,
    //OcpResp.DVA, OcpResp.NULL)
    val resp = twowaymem.io.nodearray(i).in.valid
    twowaymem.io.nodearray(i).out.address := io.cores(i).M.Addr >> 2
    twowaymem.io.nodearray(i).out.data := io.cores(i).M.Data
    twowaymem.io.nodearray(i).out.valid := io.cores(i).M.Cmd === OcpCmd.WR || io.cores(i).M.Cmd === OcpCmd.RD
    twowaymem.io.nodearray(i).out.rw := io.cores(i).M.Cmd === OcpCmd.WR
    io.cores(i).S.Data := twowaymem.io.nodearray(i).in.data
    
	//Our version already delays resposse??
    io.cores(i).S.Resp := resp//Reg(init = OcpResp.NULL, next = resp)




  }
}
