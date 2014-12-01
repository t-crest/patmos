/*
   Copyright 2014 Technical University of Denmark, DTU Compute.
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
 * Stack cache without actual functionality
 *
 * Author: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *         Sahar Abbaspour (sabb@dtu.dk)
 *         Florian Brandner (florian.brandner@ensta-paristech.fr)
 */

package stackcache

import Chisel._
import Node._

import patmos._
import patmos.Constants._
import datacache.NullCache
import datacache.WriteNoBuffer

import ocp._

class NullStackCache() extends Module {
  val io = new StackCacheIO() {
    // slave to cpu
    val fromCPU = new OcpCoreSlavePort(EXTMEM_ADDR_WIDTH, DATA_WIDTH)
    // master to memory
    val toMemory = new OcpBurstMasterPort(EXTMEM_ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH)
  }

  // stack top pointer
  val stackTopReg = Reg(UInt(width = DATA_WIDTH))
  // memory top pointer
  val memTopReg = Reg(UInt(width = DATA_WIDTH))

  // never stall
  io.stall := Bool(false)

  // signals to execute stage
  io.scex.stackTop := stackTopReg
  io.scex.memTop := memTopReg
  
  // translate read requests
  val nc = Module(new NullCache())
  nc.io.master.M := io.fromCPU.M
  nc.io.master.M.Addr := io.fromCPU.M.Addr + stackTopReg

  // translate write requests
  val wc = Module(new WriteNoBuffer())
  wc.io.readMaster <> nc.io.slave
  wc.io.writeMaster.M := io.fromCPU.M
  wc.io.writeMaster.M.Addr := io.fromCPU.M.Addr + stackTopReg
  wc.io.slave <> io.toMemory

  // construct response
  io.fromCPU.S.Data := nc.io.master.S.Data
  io.fromCPU.S.Resp := nc.io.master.S.Resp | wc.io.writeMaster.S.Resp

  /*
   * Stack Control Interface (mfs, sres, sens, sfree)
   */
  when (io.ena_in) {
    switch(io.exsc.op) {
      is(sc_OP_NONE) {
        // don't do anything
      }
      is(sc_OP_SET_ST) {
        // assign the operation's operand to the stack top pointer
        stackTopReg := io.exsc.opData
      }
      is(sc_OP_SET_MT) {
        // assign the operation's operand to the mem top pointer
        memTopReg := io.exsc.opData
      }
      is(sc_OP_ENS) {
        // ignore
      }
      is(sc_OP_FREE) {
        // move stack top pointer upwards
        stackTopReg := stackTopReg + io.exsc.opOff
      }
      is(sc_OP_RES) {
        // decrement the stack top pointer
        stackTopReg := stackTopReg - io.exsc.opOff
      }
      is (sc_OP_SPILL) {
        // ignore
      }
    }
  }
}
