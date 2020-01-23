/*
 * Wrapper for Argo NxN bi-torus NoC
 *
 * Authors: Eleftherios Kyriakakis (elky@dtu.dk)
 *
 */

package argo

import Chisel._
import patmos.Constants._
import ocp._
import cmp._

class OcpArgoSlavePort(addrWidth : Int, dataWidth : Int, argoConf: ArgoConfig) 
  extends OcpCoreSlavePort(addrWidth, dataWidth) {
  val superMode = Bits(INPUT, argoConf.CORES)
  val flags = Bits(OUTPUT, 2*argoConf.CORES)
}

class CmpArgoIO(corecnt : Int, argoConf: ArgoConfig) extends CmpIO(corecnt : Int)
{
  override val cores = Vec(corecnt, new OcpArgoSlavePort(ADDR_WIDTH, DATA_WIDTH, argoConf)).asInstanceOf[Vec[OcpCoreSlavePort]]

  override def clone = new CmpArgoIO(corecnt, argoConf).asInstanceOf[this.type]
}

class Argo(nrCores: Int, wrapped: Boolean = false, emulateBB: Boolean = false) extends Module {
  ArgoConfig.setCores(nrCores)
  val argoConf = ArgoConfig.getConfig
  val io = IO(new CmpArgoIO(argoConf.CORES, argoConf))//Vec.fill(argoConf.CORES){new OcpArgoSlavePort(ADDR_WIDTH, DATA_WIDTH, argoConf)}

  val constSelSPMBitOffset = 26  

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
    argoNoc.io.supervisor(i) := io.cores(i).asInstanceOf[OcpArgoSlavePort].superMode(i)
  }
  
  val masterReg = Vec.fill(argoConf.CORES){Reg(new OcpArgoSlavePort(ADDR_WIDTH, DATA_WIDTH, argoConf)).M}
  val busyReg = Vec.fill(argoConf.CORES){Reg(init = Bool(false))}
  val selSpmRplyReg = Vec.fill(argoConf.CORES){Reg(init = Bool(false))}

	// Wire up Patmos - NoC + SPM
	for(i <- 0 until argoConf.CORES){

    //While not busy register a new master for NoC
    when(!busyReg(i)) {
      masterReg(i) := io.cores(i).M
    }
    //Is busy when command is RD/WR and address is for the NoC
    when((io.cores(i).M.Cmd === OcpCmd.RD || io.cores(i).M.Cmd === OcpCmd.WR) && io.cores(i).M.Addr(constSelSPMBitOffset) === Bits("b0")) {
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
    argoNoc.io.ocpPorts(i).M.RespAccept := (argoNoc.io.ocpPorts(i).S.Resp =/= OcpResp.NULL).asUInt //Accept all responses
    argoNoc.io.ocpPorts(i).M.Cmd := Mux(masterReg(i).Addr(constSelSPMBitOffset) === Bits("b0"), masterReg(i).Cmd, OcpCmd.IDLE) //0xE000_0000

    //Argo SPM gets immediate access to io
    comSPMWrapper(i).ocp.M.Data := io.cores(i).M.Data
    comSPMWrapper(i).ocp.M.ByteEn := io.cores(i).M.ByteEn
    comSPMWrapper(i).ocp.M.Addr := io.cores(i).M.Addr
    comSPMWrapper(i).ocp.M.Cmd := Mux(io.cores(i).M.Addr(constSelSPMBitOffset) === Bits("b1"), io.cores(i).M.Cmd, OcpCmd.IDLE) //0xE800_0000

    //Register slave resp/data
    val respSpmReg = Reg(next = comSPMWrapper(i).ocp.S.Resp)
    val respNoCReg = Reg(next = argoNoc.io.ocpPorts(i).S.Resp)
    val dataSpmReg = Reg(next = comSPMWrapper(i).ocp.S.Data)
    val dataNoCReg = Reg(next = argoNoc.io.ocpPorts(i).S.Data)

    //Mux spm/noc to master
    when(io.cores(i).M.Cmd =/= OcpCmd.IDLE && !busyReg(i)){
      selSpmRplyReg(i) := io.cores(i).M.Addr(constSelSPMBitOffset).toBool
    } .elsewhen(respSpmReg === OcpResp.DVA || respNoCReg ===OcpResp.DVA){
      selSpmRplyReg(i) := false.B
    }
    io.cores(i).S.Data := Mux(selSpmRplyReg(i), dataSpmReg, dataNoCReg)
    io.cores(i).S.Resp := Mux(selSpmRplyReg(i), respSpmReg, respNoCReg)

    // NoC - Patmos
    io.cores(i).asInstanceOf[OcpArgoSlavePort].flags := argoNoc.io.irq
	}

  // Generate config.vhd
  ArgoConfig.genConfigVHD("vhdl/argo/config.vhd")
  // Generate schedule
  ArgoConfig.genPoseidonSched("../../local/bin/", "../local/", "argo_platform.xml", "argo_communication.xml", "../local/argo_schedule.xml")
  ArgoConfig.genNoCInitFile("../../local/bin/", "../local/argo_schedule.xml", "../c/cmp/nocinit.c")
}

object Argo {
	def main(args: Array[String]): Unit = {
    chiselMain(Array[String]("--backend", "v", "--targetDir", "generated"),
      () => Module(new Argo(9, false, false)))
	}
}
