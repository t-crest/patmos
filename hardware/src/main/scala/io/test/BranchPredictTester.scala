/*
 * Copyright: 2017, Technical University of Denmark, DTU Compute
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * License: Simplified BSD License
 * 
 * Tester for branch prediction experiments.
 */

package io.test

import Chisel._

import io._

class BranchPredictTester(dut: BranchPredict) extends Tester(dut) {

  step(1)
  peek(dut.io.hit)
}

object BranchPredictTester {
  def main(args: Array[String]): Unit = {
    chiselMainTest(args, () => Module(new BranchPredict)) {
      f => new BranchPredictTester(f)
    }
  }
}
