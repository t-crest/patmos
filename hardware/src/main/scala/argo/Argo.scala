/*
 * Wrapper for Argo NxN bi-torus NoC
 *
 * Authors: Eleftherios Kyriakakis (elky@dtu.dk)
 *
 */

package argo

import chisel3._
import chisel3.util._
import patmos.Constants._
import ocp._
import cmp._

class OcpArgoSlavePort(addrWidth : Int, dataWidth : Int, val argoConf: ArgoConfig)
  extends OcpCoreSlavePort(addrWidth, dataWidth) {
  val superMode = Input(UInt(argoConf.CORES.W))
  val flags = Output(UInt((2*argoConf.CORES).W))
}

class CmpArgoIO(corecnt : Int, val argoConf: ArgoConfig) extends CmpIO(corecnt : Int)
{
  override val cores = Vec(corecnt, new OcpArgoSlavePort(ADDR_WIDTH, DATA_WIDTH, argoConf)).asInstanceOf[Vec[OcpCoreSlavePort]]
}

class Argo(nrCores: Int, wrapped: Boolean = false, emulateBB: Boolean = false) extends CmpDevice(nrCores) {
  ArgoConfig.setCores(nrCores)
  val argoConf = ArgoConfig.getConfig
  val io = IO(new CmpArgoIO(argoConf.CORES, argoConf))

  val constSelSPMBitOffset = 26  

  println("Argo connecting "+ argoConf.CORES +" Patmos islands with configuration:")
  println("N=" + argoConf.N)
  println("M=" + argoConf.M)
  println("SPM_SIZE (Bytes)=" + argoConf.SPM_BYTES)
  println("Emulation is " + emulateBB)

  // Generate Argo and COM-SPMs
  val argoNoc = Module(new ArgoNoC(argoConf, wrapped, emulateBB))
  val comSPMWrapper = (0 until argoConf.CORES).map(i => if (emulateBB) Module(new ComSpmDummy(argoConf).asInstanceOf[ComSpmDummy]) else Module(new ComSpmWrapper(argoConf).asInstanceOf[ComSpmWrapper]))

  // Wire up SPM - NoC
  val supervisorSets = VecInit(Seq.fill(argoConf.CORES)(false.B))
  argoNoc.io.supervisor := Fill(argoConf.CORES, "b1".U)
  for(i <- 0 until argoConf.CORES){
    if(emulateBB){
      comSPMWrapper(i).asInstanceOf[ComSpmDummy].io.spm.M := argoNoc.io.spmPorts(i).M
      argoNoc.io.spmPorts(i).S := comSPMWrapper(i).asInstanceOf[ComSpmDummy].io.spm.S
    }else{
      comSPMWrapper(i).asInstanceOf[ComSpmWrapper].io.spm.M := argoNoc.io.spmPorts(i).M
      argoNoc.io.spmPorts(i).S := comSPMWrapper(i).asInstanceOf[ComSpmWrapper].io.spm.S
      comSPMWrapper(i).asInstanceOf[ComSpmWrapper].io.clk := clock
      comSPMWrapper(i).asInstanceOf[ComSpmWrapper].io.reset := reset
    }
    supervisorSets(i) := io.cores(i).asInstanceOf[OcpArgoSlavePort].superMode(i)
  }
  argoNoc.io.supervisor := supervisorSets.asUInt

  val masterReg = Reg(Vec(argoConf.CORES, new OcpArgoSlavePort(ADDR_WIDTH, DATA_WIDTH, argoConf).M))
  val busyReg = RegInit(VecInit(Seq.fill(argoConf.CORES)(false.B)))
  val selSpmRplyReg = RegInit(VecInit(Seq.fill(argoConf.CORES)(false.B)))
  val respSpmReg = RegInit(VecInit(Seq.fill(argoConf.CORES)(OcpResp.NULL)))
  val respNoCReg = RegInit(VecInit(Seq.fill(argoConf.CORES)(OcpResp.NULL)))
  val dataSpmReg = RegInit(VecInit(Seq.fill(argoConf.CORES)(0.U(DATA_WIDTH.W))))
  val dataNoCReg = RegInit(VecInit(Seq.fill(argoConf.CORES)(0.U(DATA_WIDTH.W))))

	// Wire up Patmos - NoC + SPM
	for(i <- 0 until argoConf.CORES){

    //While not busy register a new master for NoC
    when(!busyReg(i)) {
      masterReg(i) := io.cores(i).M
    }
    //Is busy when command is RD/WR and address is for the NoC
    when((io.cores(i).M.Cmd === OcpCmd.RD || io.cores(i).M.Cmd === OcpCmd.WR) && io.cores(i).M.Addr(constSelSPMBitOffset) === 0.U) {
      busyReg(i) := true.B
    }
    //Not busy when the command has been accepted
    when(busyReg(i) && argoNoc.io.ocpPorts(i).S.CmdAccept === 1.U) {
      busyReg(i) := false.B
    }

    //Argo NI communication
    argoNoc.io.ocpPorts(i).M.Data := masterReg(i).Data
    argoNoc.io.ocpPorts(i).M.ByteEn := masterReg(i).ByteEn
    argoNoc.io.ocpPorts(i).M.Addr := masterReg(i).Addr
    argoNoc.io.ocpPorts(i).M.RespAccept := (argoNoc.io.ocpPorts(i).S.Resp =/= OcpResp.NULL).asUInt //Accept all responses
    argoNoc.io.ocpPorts(i).M.Cmd := Mux(masterReg(i).Addr(constSelSPMBitOffset) === 0.U, masterReg(i).Cmd, OcpCmd.IDLE) //0xE000_0000

    //Argo SPM gets immediate access to io
    if (emulateBB){
      comSPMWrapper(i).asInstanceOf[ComSpmDummy].io.ocp.M.Data := io.cores(i).M.Data
      comSPMWrapper(i).asInstanceOf[ComSpmDummy].io.ocp.M.ByteEn := io.cores(i).M.ByteEn
      comSPMWrapper(i).asInstanceOf[ComSpmDummy].io.ocp.M.Addr := io.cores(i).M.Addr
      comSPMWrapper(i).asInstanceOf[ComSpmDummy].io.ocp.M.Cmd := Mux(io.cores(i).M.Addr(constSelSPMBitOffset) === 1.U, io.cores(i).M.Cmd, OcpCmd.IDLE) //0xE800_0000
    }else{
      comSPMWrapper(i).asInstanceOf[ComSpmWrapper].io.ocp.M.Data := io.cores(i).M.Data
      comSPMWrapper(i).asInstanceOf[ComSpmWrapper].io.ocp.M.ByteEn := io.cores(i).M.ByteEn
      comSPMWrapper(i).asInstanceOf[ComSpmWrapper].io.ocp.M.Addr := io.cores(i).M.Addr
      comSPMWrapper(i).asInstanceOf[ComSpmWrapper].io.ocp.M.Cmd := Mux(io.cores(i).M.Addr(constSelSPMBitOffset) === 1.U, io.cores(i).M.Cmd, OcpCmd.IDLE) //0xE800_0000
    }

    //Register slave resp/data
    if(emulateBB)
      respSpmReg(i) := comSPMWrapper(i).asInstanceOf[ComSpmDummy].io.ocp.S.Resp
    else 
      respSpmReg(i) := comSPMWrapper(i).asInstanceOf[ComSpmWrapper].io.ocp.S.Resp
    respNoCReg(i) := argoNoc.io.ocpPorts(i).S.Resp

    if(emulateBB)
      dataSpmReg(i) := comSPMWrapper(i).asInstanceOf[ComSpmDummy].io.ocp.S.Data
    else 
      dataSpmReg(i) := comSPMWrapper(i).asInstanceOf[ComSpmWrapper].io.ocp.S.Data
    dataNoCReg(i) := argoNoc.io.ocpPorts(i).S.Data

    //Mux spm/noc to master
    when(io.cores(i).M.Cmd =/= OcpCmd.IDLE && !busyReg(i)){
      selSpmRplyReg(i) := io.cores(i).M.Addr(constSelSPMBitOffset).asBool
    } .elsewhen(respSpmReg(i) === OcpResp.DVA || respNoCReg(i) === OcpResp.DVA){
      selSpmRplyReg(i) := false.B
    }
    io.cores(i).S.Data := Mux(selSpmRplyReg(i), dataSpmReg(i), dataNoCReg(i))
    io.cores(i).S.Resp := Mux(selSpmRplyReg(i), respSpmReg(i), respNoCReg(i))

    // NoC - Patmos
    io.cores(i).asInstanceOf[OcpArgoSlavePort].flags := argoNoc.io.irq
	}

  // Generate config.vhd
  ArgoConfig.genConfigVHD("vhdl/argo/config.vhd")
  // Generate schedule
  ArgoConfig.genPoseidonSched("../../local/bin/", "../local/", "argo_platform.xml", "argo_communication.xml", "../local/argo_schedule.xml")
  ArgoConfig.genNoCInitFile("../../local/bin/", "../local/argo_schedule.xml", "../c/cmp/nocinit.c")
}
