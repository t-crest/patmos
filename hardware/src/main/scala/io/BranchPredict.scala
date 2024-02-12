/*
 * Copyright: 2017, Technical University of Denmark, DTU Compute
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * License: Simplified BSD License
 * 
 * Branch predictor experiments
 */

package io

import Chisel._


class BranchPredict() extends Module {

  val io = new Bundle() {
    val hit = Output(Bool())
  }
  
  io.hit := Bool(true)
}

