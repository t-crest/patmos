/*
   Copyright 2018 Technical University of Denmark, DTU Compute.
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
 * Wrapper for Argo NxN bi-torus NoC
 *
 * Authors: Eleftherios Kyriakakis (elky@dtu.dk)
 *
 */

package argo

import Chisel._
import Node._
import patmos.Constants._
import util._
import ocp._
import patmos._

class OcpArgoSlavePort(addrWidth : Int, dataWidth : Int, argoConf: ArgoConfig) 
  extends OcpCoreSlavePort(addrWidth, dataWidth) {
  val superMode = Bits(INPUT, argoConf.CORES)
  val flag = Bits(OUTPUT, 2)
}

class Argo(nrCores: Int, wrapped: Boolean = false, emulateBB: Boolean = false) extends Module {
  ArgoConfig.setCores(nrCores)
  val argoConf = ArgoConfig.getConfig
  val io = Vec.fill(argoConf.CORES){new OcpArgoSlavePort(ADDR_WIDTH, DATA_WIDTH, argoConf)}

  println("Connecting "+ argoConf.CORES +" Patmos islands with configuration:")
  println("N=" + argoConf.N)
  println("M=" + argoConf.M)
  println("SPM_SIZE (Bytes)=" + argoConf.SPM_BYTES)
  println("Emulation is " + emulateBB)

  // Generate Argo and COM-SPMs
  val argoNoc = Module(new ArgoNoC(argoConf, wrapped, emulateBB))
  val comSPMWrapper = Vec.fill(argoConf.CORES) {
    if(emulateBB){
      Module(new ComSpmDummy(argoConf)).io
    } else {
      Module(new ComSpmWrapper(argoConf)).io
    }
  }

  // Wire up SPM - NoC
  argoNoc.io.supervisor := Bits("hF")
  for(i <- 0 until argoConf.CORES){
		comSPMWrapper(i).spm.M := argoNoc.io.spmPorts(i).M
    argoNoc.io.spmPorts(i).S := comSPMWrapper(i).spm.S
    argoNoc.io.supervisor(i) := io(i).superMode(i)
  }

  val masterReg = Vec.fill(argoConf.CORES){Reg(new OcpArgoSlavePort(ADDR_WIDTH, DATA_WIDTH, argoConf)).M}
  val busyReg = Vec.fill(argoConf.CORES){Reg(init = Bool(false))}
  val selSpmRplyReg = Vec.fill(argoConf.CORES){Reg(init = Bool(false))}

//  val dataSpmReg = Vec.fill(argoConf.CORES){Reg(init = UInt(width = DATA_WIDTH))}
//  val respSpmReg = Vec.fill(argoConf.CORES){Reg(init = OcpResp.NULL)}
//  val dataNoCReg = Vec.fill(argoConf.CORES){Reg(init = UInt(width = DATA_WIDTH))}
//  val respNoCReg = Vec.fill(argoConf.CORES){Reg(init = OcpResp.NULL)}

	// Wire up Patmos - NoC + SPM
	for(i <- 0 until argoConf.CORES){

    //While not busy register a new master for NoC
    when(!busyReg(i)) {
      masterReg(i) := io(i).M
    }
    //Is busy when command is RD/WR and address is for the NoC
    when((io(i).M.Cmd === OcpCmd.RD || io(i).M.Cmd === OcpCmd.WR) && io(i).M.Addr(27) === Bits("b0")) {
      busyReg(i) := true.B
    }
    //Not busy when the command has been accepted
    when(busyReg(i) && argoNoc.io.ocpPorts(i).S.CmdAccept === Bits(1)) {
      busyReg(i) := false.B
    }

    //Argo NI communication
    argoNoc.io.ocpPorts(i).M.Data := masterReg(i).Data
    argoNoc.io.ocpPorts(i).M.ByteEn := masterReg(i).ByteEn
    argoNoc.io.ocpPorts(i).M.Addr := masterReg(i).Addr
    argoNoc.io.ocpPorts(i).M.RespAccept := (argoNoc.io.ocpPorts(i).S.Resp =/= OcpResp.NULL).toUInt //Accept all responses
    argoNoc.io.ocpPorts(i).M.Cmd := Mux(masterReg(i).Addr(27) === Bits("b0"), masterReg(i).Cmd, OcpCmd.IDLE) //0xE000_0000

    //Argo SPM gets immediate access to io
    comSPMWrapper(i).ocp.M.Data := io(i).M.Data
    comSPMWrapper(i).ocp.M.ByteEn := io(i).M.ByteEn
    comSPMWrapper(i).ocp.M.Addr := io(i).M.Addr
    comSPMWrapper(i).ocp.M.Cmd := Mux(io(i).M.Addr(27) === Bits("b1"), io(i).M.Cmd, OcpCmd.IDLE) //0xE800_0000

    //Register slave resp/data
    val respSpmReg = Reg(next = comSPMWrapper(i).ocp.S.Resp)
    val respNoCReg = Reg(next = argoNoc.io.ocpPorts(i).S.Resp)
    val dataSpmReg = RegEnable(updateData = comSPMWrapper(i).ocp.S.Data, enable = comSPMWrapper(i).ocp.S.Resp===OcpResp.DVA)
    val dataNoCReg = Reg(next = argoNoc.io.ocpPorts(i).S.Data)

    //Mux spm/noc to master
    when(io(i).M.Cmd =/= OcpCmd.IDLE && !busyReg(i)){
      selSpmRplyReg(i) := io(i).M.Addr(27).toBool
    } .elsewhen(respSpmReg === OcpResp.DVA || respNoCReg ===OcpResp.DVA){
      selSpmRplyReg(i) := false.B
    }

    io(i).S.Data := Mux(selSpmRplyReg(i), dataSpmReg, dataNoCReg)
    io(i).S.Resp := Mux(selSpmRplyReg(i), respSpmReg, respNoCReg)

    // NoC - Patmos
     io(i).flag := argoNoc.io.irq(2+i*2-1, i*2)
	}

  // Generate config.vhd
  ArgoConfig.genConfigVHD(argoConf, "vhdl/argo/config.vhd")
}

object Argo {
	def main(args: Array[String]): Unit = {
    chiselMain(Array[String]("--backend", "v", "--targetDir", "generated"),
      () => Module(new Argo(4, false, false)))
	}
}
