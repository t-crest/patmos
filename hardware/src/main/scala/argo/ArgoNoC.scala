/*
   Copyright 2013 Technical University of Denmark, DTU Compute.
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
 * Argo Instantiation and Interconnection
 *
 * Authors: Eleftherios Kyriakakis (elky@dtu.dk)
 *
 */
 
 package argo

import Chisel._
import ocp.OcpIOSlavePort
import patmos.Constants.{ADDR_WIDTH, DATA_WIDTH}

class ArgoNoC(argoConf: ArgoConfig, wrapped: Boolean = false, emulateBB: Boolean = false) extends Module {
  val io = new Bundle {
    val irq = Bits(OUTPUT, width = argoConf.CORES*2)
    val supervisor = Bits(INPUT, width = argoConf.CORES)
    val ocpPorts = Vec.fill(argoConf.CORES) {
      new OcpIOSlavePort(ADDR_WIDTH, DATA_WIDTH)
    }
    val spmPorts = Vec.fill(argoConf.CORES) {
      new SPMMasterPort(argoConf.HEADER_FIELD_WIDTH, argoConf.HEADER_CTRL_WIDTH)
    }
  }

  io.irq := 0.U
  //Interconnect
  if(!wrapped) {
    val masterRunWire = Bits(width=1)
    val argoMesh = (0 until argoConf.M).map(j => (0 until argoConf.N).map(i => if (emulateBB) Module(new NoCNodeDummy(argoConf, i == 0 && j == 0)).io else Module(new NoCNodeWrapper(argoConf, i == 0 && j == 0)).io))
    /*
    * Nodes Port Interconnect
    *
    *                     N
    *                     |
    *                     |
    *                     |
    *      WEST <---------|---------> EAST
    *                     |
    *                     |
    *                     |
    *                     S
    */
    println("o--Building Interconnect")
    for (i <- 0 until argoConf.M) {
      for (j <- 0 until argoConf.N) {
        //Linear index for mapping
        val index = (i * argoConf.N) + j
        println("|---Node #" + index + " @ (" + i + "," + j + ")")
        //Control Ports
        argoMesh(i)(j).supervisor := io.supervisor(index)
        argoMesh(i)(j).proc.M := io.ocpPorts(index).M
        io.ocpPorts(index).S := argoMesh(i)(j).proc.S
        io.spmPorts(index).M := argoMesh(i)(j).spm.M
        argoMesh(i)(j).spm.S := io.spmPorts(index).S
        masterRunWire := argoMesh(0)(0).masterRun
        argoMesh(i)(j).run := masterRunWire
        io.irq(2 + index * 2 - 1, index * 2) := argoMesh(i)(j).irq
        //Interconnect
        if (i == 0) {
          argoMesh(0)(j).south_in <> argoMesh(argoConf.M - 1)(j).north_out
          argoMesh(argoConf.M - 1)(j).north_in <> argoMesh(0)(j).south_out
        }
        if (j == 0) {
          argoMesh(i)(0).east_in <> argoMesh(i)(argoConf.N - 1).west_out
          argoMesh(i)(argoConf.N - 1).west_in <> argoMesh(i)(0).east_out
        }
        if (i > 0) {
          argoMesh(i)(j).south_in <> argoMesh(i - 1)(j).north_out
          argoMesh(i - 1)(j).north_in <> argoMesh(i)(j).south_out
        }
        if (j > 0) {
          argoMesh(i)(j).east_in <> argoMesh(i)(j - 1).west_out
          argoMesh(i)(j - 1).west_in <> argoMesh(i)(j).east_out
        }
      }
    }
  } else {
    println("o--Wrapping Interconnect")
    val nocBB = Module(new NoCWrapper(argoConf))
    io.irq <> nocBB.io.irq
    io.supervisor <> nocBB.io.supervisor
    io.ocpPorts <> nocBB.io.ocpPorts
    io.spmPorts <> nocBB.io.spmPorts
  }

}
