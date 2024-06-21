/*
   Copyright 2017 Technical University of Denmark, DTU Compute.
   All rights reserved.

   This file is part of the time-predictable VLIW processor Patmos.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

      1. Redistributions of source code must retain the above copyright notice,
         this list of conditions and the following disclaimer.

      2. Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
   NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   The views and conclusions contained in the software and documentation are
   those of the authors and should not be interpreted as representing official
   policies, either expressed or implied, of the copyright holder.
 */

/*
 * IO components for experiments with a branch predictor.
 *
 * Authors: Martin Schoeberl (martin@jopdesign.com)
 *
 */

package io

import chisel3._

import patmos.Constants._

import ocp._

object BranchPredictIO extends DeviceObject {

  def init(params: Map[String, String]) = {}

  def create(params: Map[String, String]): BranchPredictIO = Module(new BranchPredictIO())
}

/*
 * Now just a deadline copy.
 */

class BranchPredictIO() extends CoreDevice() {

  val io = IO(new CoreDeviceIO())

  val freeRunningReg = RegInit(init = 0.U(32.W))
  val downCountReg = RegInit(init = 0.U(32.W))
  
  freeRunningReg := freeRunningReg + 1.U
  val downDone = downCountReg === 0.U
  
  when (!downDone) {
    downCountReg := downCountReg - 1.U
  }
  when (io.ocp.M.Cmd === OcpCmd.WR) {
    downCountReg := io.ocp.M.Data 
  }
  
  val timeOver = downDone
  val stallReg = RegInit(init = false.B)
  
  // read data shall be in a register as used in the
  // following clock cycle
  
  val respReg = RegInit(init = OcpResp.NULL)
  respReg := OcpResp.NULL
  
  when((io.ocp.M.Cmd === OcpCmd.RD && timeOver) || io.ocp.M.Cmd === OcpCmd.WR) {
    respReg := OcpResp.DVA
  }
  // Remember that we are stalling now
  when ((io.ocp.M.Cmd === OcpCmd.RD && !timeOver)) {
    stallReg := true.B
  }
  
  // release stall when timeout is done
  when (stallReg && timeOver) {
    respReg := OcpResp.DVA
    stallReg := false.B
  }
  
  io.ocp.S.Data := downCountReg  
  io.ocp.S.Resp := respReg
}
