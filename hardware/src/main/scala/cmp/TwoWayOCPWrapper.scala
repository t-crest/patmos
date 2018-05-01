/*
 * Copyright: 2018, Technical University of Denmark, DTU Compute
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * License: Simplified BSD License
 * 
 * OCP wrapper
 */
package cmp

import Chisel._
import Node._

import patmos._
import patmos.Constants._
import ocp._
import io.CoreDeviceIO
import twoway._

class XXXIO(lckCnt: Int) extends Bundle {
  val sel = UInt(INPUT, log2Up(lckCnt))
  val op = Bool(INPUT)
  val en = Bool(INPUT)
  val blck = Bool(OUTPUT)
}

// Maybe I should also use this functional approach, but not for a quick test now
// class TwoWayOCPWrapper(hardlockgen: () => AbstractHardlock) extends Module {

class TwoWayOCPWrapper(nrCores: Int) extends Module {

  val dim = math.sqrt(nrCores).toInt
  if (dim * dim != nrCores) throw new Error("Number of cores must be quadratic")
  // just start with four words
  val size = 128 * nrCores
  val twowaymem = Module(new twoway.TwoWayMem(dim, size))

  println("TwoWayMem")

  val io = Vec.fill(nrCores) { new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH) }

  // Mapping between TwoWay memories and OCP

  for (i <- 0 until nrCores) {


    // Should response be this or valid signal??
    //val resp = Mux(io(i).M.Cmd === OcpCmd.RD || io(i).M.Cmd === OcpCmd.WR,
    //OcpResp.DVA, OcpResp.NULL)
    val resp = twowaymem.io.nodearray(i).in.valid
    twowaymem.io.nodearray(i).out.address := io(i).M.Addr >> 2
    twowaymem.io.nodearray(i).out.data := io(i).M.Data
    twowaymem.io.nodearray(i).out.valid := io(i).M.Cmd === OcpCmd.WR || io(i).M.Cmd === OcpCmd.RD
    twowaymem.io.nodearray(i).out.rw := io(i).M.Cmd === OcpCmd.WR
    io(i).S.Data := twowaymem.io.nodearray(i).in.data
    
	//Our version already delays resposse??
    io(i).S.Resp := resp//Reg(init = OcpResp.NULL, next = resp)




  }
}
