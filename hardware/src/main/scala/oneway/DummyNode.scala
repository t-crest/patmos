/*
 * Copyright: 2017, Technical University of Denmark, DTU Compute
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * License: Simplified BSD License
 */

package oneway

import chisel3._
import s4noc._

/**
 * One NoC node, just containing some dummy computation to keep
 * synthesizer from optimzing everything away.
 * Provide some data in dout to get synthesize results.
 */
class DummyNode(n: Int) extends Module {
  val io = IO(new Bundle {
    val local = new Channel(UInt(32.W))
    val dout = Output(UInt(32.W))
  })
  
  val regAccu = RegInit(init=n.U(32.W))
  regAccu := regAccu + io.local.in.data
  io.local.out.data := regAccu
  io.dout := regAccu
}
