/*
   Copyright 2016 Technical University of Denmark, DTU Compute.
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
 * Shared SPM. Later with ownership.
 *
 * Author: Martin Schoeberl (martin@jopdesign.com)
 *
 */

/*

Questions (to Wolfgang):
  How does OcpIOBridge 'emulate' not available CmdAccep? I think it should
  register unconditional when there is a command and set it back to IDLE
  when CmdAccept is '1'. Otherwise how would the master be forced to keep
  it's signal active?
  
  I don't think merging responses of OCP slaves is legal OCP.
  What are the rules for Resp? Can a slave unconditionally drive DVA?
  I assume yes.
  
 */
package cmp

import Chisel._
import Node._

import patmos._
import patmos.Constants._
import ocp._

class SharedSPM(cnt: Int) extends Module {
  // The names Conf and Spm have no real meaning here.
  // It is just two different types of OCP connections.
  // Legacy from the usage in Argo with two OCP ports.
  val io = new Bundle {
    val comConf = Vec.fill(cnt) { new OcpIOSlavePort(ADDR_WIDTH, DATA_WIDTH) }
    val comSpm = Vec.fill(cnt) { new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH) }
  }

  val spm = Module(new Spm(1024))

  io.comConf(0) <> spm.io
  io.comConf(0).S.CmdAccept := Bits(1)
  // What to do with the RespAccept?
  val resp = io.comConf(0).M.RespAccept

  //  val spm2 = Module(new Spm(1024))
  //  io.comSpm(0) <> spm2.io

  // TODO: a simple round robin arbiter - see class Arbiter

  //  spm.io.M.Addr := 
  //  spm.io.M.Cmd := Bits(1)
  //  val xxx = spm.io.S.Data
  //  val yyy = spm.io.S.Resp

  for (i <- 1 to cnt - 1) {
    io.comConf(i).S.Data := UInt(i + 'A')
    // Is it legal OCP to have the response flags hard wired?
    // Probably yes.
    io.comConf(i).S.CmdAccept := Bits(1)
    io.comConf(i).S.Resp := Mux(io.comConf(i).M.Cmd =/= OcpCmd.IDLE,
      OcpResp.DVA, OcpResp.NULL)
  }

  // Use the comSpm interface for the core id, as currently
  // the core id comes from an input port, set by the aegean
  // generated VHDL code
  
  

  for (i <- 0 to cnt - 1) {
    io.comSpm(i).S.Data := UInt(i + 3)
    // With this code here added the other SPM at comConf behaves strange!
    // But it should be legal OCP - issue with oring all responses in InOut
    // io.comSpm(i).S.Resp := OcpResp.DVA
    // And the following without a register with single cycle reply
    // leads to a combinational path
    io.comSpm(i).S.Resp := Mux(Reg(next = io.comSpm(i).M.Cmd) =/= OcpCmd.IDLE,
      OcpResp.DVA, OcpResp.NULL)
  }
}



