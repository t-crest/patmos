package argo

import Chisel._
import Node._
import patmos.Constants._
import util._
import ocp._
import patmos._
//
//class nocIO(N: Int, M: Int, HEADER_FIELD_WIDTH: Int, HEADER_CTRL_WIDTH: Int) extends Bundle() {
//  val comConf = Vec.fill(N*M){new OcpIOSlavePort(ADDR_WIDTH, DATA_WIDTH)}
//  val comSPM = Vec.fill(N*M){new ComSPMMasterPort(HEADER_FIELD_WIDTH, HEADER_CTRL_WIDTH)}
//  val supervisor = Bits(INPUT, N*M)
//  val irq = Bits(OUTPUT, 2*N*M)
//}
//
//// Wrapper for generated noc.vhd
//class NoCBB(argoConf: ArgoConfig) extends BlackBox() {
//  val io = new nocIO(argoConf.N, argoConf.M, argoConf.HEADER_FIELD_WIDTH, argoConf.HEADER_CTRL_WIDTH)
//  setModuleName("noc")
//  renameClock(clock, "clk")
//  reset.setName("reset")
//  io.supervisor.setName("supervisor")
//  io.irq.setName("irq")
////  for(i <- 0 to (argoConf.N*argoConf.M)-1){
////    io.comConf(i).Addr.setName("ocp_io_ms("+i+").Addr")
////    io.comConf(i).ByteEn.setName("ocp_io_ms("+i+").ByteEn")
////    io.comConf(i).Cmd.setName("ocp_io_ms("+i+").Cmd")
////    io.comConf(i).RespAccept.setName("ocp_io_ms("+i+").RespAccept")
////    io.comConf(i).Data.setName("ocp_io_ms("+i+").Data")
////    io.comConf(i).Resp.setName("ocp_io_ss("+i+").Resp")
////    io.comConf(i).Data.setName("ocp_io_ss("+i+").Data")
////    io.comConf(i).CmdAccept.setName("ocp_io_ss("+i+").CmdAccept")
////    io.spm_ports_m(i).en.setName("spm_ports_m("+i+").en")
////    io.spm_ports_m(i).addr.setName("spm_ports_m("+i+").addr")
////    io.spm_ports_m(i).wr.setName("spm_ports_m("+i+").wr")
////    io.spm_ports_m(i).wdata.setName("spm_ports_m("+i+").wdata")
////    io.spm_ports_s(i).rdata.setName("spm_ports_s("+i+").rdata")
////    io.spm_ports_s(i).error.setName("spm_ports_m("+i+").error")
////  }
//}
//
//class NoCWrapper(argoConf : ArgoConfig) extends Module {
//  val io = new Bundle(){
//    val comConf = Vec.fill(ArgoConfig.getSize){new OcpNISlavePort(ADDR_WIDTH, DATA_WIDTH)}
//    val spmIf = Vec.fill(ArgoConfig.getSize){new ComSPMMasterPort(argoConf.HEADER_FIELD_WIDTH, argoConf.HEADER_CTRL_WIDTH)}
//    val supervisor = Bits(INPUT, ArgoConfig.getSize)
//  }
//
//  val noCBB = Module(new NoCBB(argoConf))
//  noCBB.io.supervisor := io.supervisor
//  io.comConf <> noCBB.io.comConf
//  io.spmIf <> noCBB.io.comSPM
//  for(i <- 0 to ArgoConfig.getSize-1) {
//    io.comConf(i).S.Reset_n := 0.U
//    io.comConf(i).S.Flag := noCBB.io.irq(2*i+1, 2*i)
//  }
//}