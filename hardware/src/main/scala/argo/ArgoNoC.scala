package argo

import Chisel._
import ocp.OcpIOSlavePort
import patmos.Constants.{ADDR_WIDTH, DATA_WIDTH}

class ArgoNoC(argoConf: ArgoConfig) extends Module {
  val io = new Bundle {
    val irq = Bits(OUTPUT, width = argoConf.N*argoConf.M*2)
    val supervisor = Bits(INPUT, width = argoConf.N*argoConf.M)
    val ocpPorts = Vec.fill(argoConf.N * argoConf.M) {
      new OcpIOSlavePort(ADDR_WIDTH, DATA_WIDTH)
    }
    val spmPorts = Vec.fill(argoConf.N * argoConf.M) {
      new SPMMasterPort(argoConf.HEADER_FIELD_WIDTH, argoConf.HEADER_CTRL_WIDTH)
    }
  }

  val argoMesh = (0 until argoConf.M).map(j=>(0 until argoConf.M).map(i=>Module(new NoCNodeWrapper(argoConf, i==0 && j==0)).io))

  //Interconnect
  io.irq := 0.U
  println("\to--Building Interconnect with topology = " + argoConf.topoType)
  for(i <- 0 until argoConf.M){
    for(j <- 0 until argoConf.N){
      val index = (i * argoConf.N) + j
      println("\t|---Node #" + index +" @ ("+i+","+j+")")
      //Control Ports
      io.irq(2+index*2-1, index*2) := argoMesh(i)(j).irq
      argoMesh(i)(j).supervisor := io.supervisor(index)
      argoMesh(i)(j).proc.M := io.ocpPorts(index).M
      io.ocpPorts(index).S := argoMesh(i)(j).proc.S
      io.spmPorts(index).M := argoMesh(i)(j).spm.M
      argoMesh(i)(j).spm.S := io.spmPorts(index).S
      argoMesh(i)(j).run := argoMesh(0)(0).masterRun

      //Nodes
      if(i==0){
        println("\t -Top")
        if("bitorus" == argoConf.topoType) {
          argoMesh(i)(j).north_in.f := argoMesh(argoConf.M - 1)(j).south_out.f
          argoMesh(argoConf.M - 1)(j).south_out.b := argoMesh(i)(j).north_in.b
        }
        argoMesh(i)(j).south_in.f := argoMesh(i+1)(j).north_out.f
        argoMesh(i+1)(j).north_out.b := argoMesh(i)(j).south_in.b
      }else if (i==(argoConf.M-1)){
        println("\t -Bottom")
        argoMesh(i)(j).north_in.f := argoMesh(i-1)(j).south_out.f
        argoMesh(i-1)(j).south_out.b := argoMesh(i)(j).north_in.b
        if("bitorus" == argoConf.topoType) {
          argoMesh(i)(j).south_in.f := argoMesh(0)(j).north_out.f
          argoMesh(0)(j).north_out.b := argoMesh(i)(j).south_in.b
        }
      }
      if(j==0){
        println("\t -Left")
        argoMesh(i)(j).east_in.f := argoMesh(i)(j+1).west_out.f
        argoMesh(i)(j+1).west_out.b := argoMesh(i)(j).east_in.b
        if("bitorus" == argoConf.topoType) {
          argoMesh(i)(j).west_in.f := argoMesh(i)(argoConf.N - 1).east_out.f
          argoMesh(i)(argoConf.N - 1).east_out.b := argoMesh(i)(j).west_in.b
        }
      } else if (j==(argoConf.N-1)){
        println("\t -Right")
        if("bitorus" == argoConf.topoType) {
          argoMesh(i)(j).east_in.f := argoMesh(i)(0).west_out.f
          argoMesh(i)(0).west_out.b := argoMesh(i)(j).east_in.b
        }
        argoMesh(i)(j).west_in.f := argoMesh(i)(j-1).east_out.f
        argoMesh(i)(j-1).east_out.b := argoMesh(i)(j).west_in.b
      }
      if (i>0 && i<(argoConf.M-1) && j>0 && j<(argoConf.N-1)){
        println("\t -Center")
        argoMesh(i)(j).north_in.f := argoMesh(i-1)(j).south_out.f
        argoMesh(i-1)(j).south_out.b := argoMesh(i)(j).north_in.b
        argoMesh(i)(j).south_in.f := argoMesh(i+1)(j).north_out.f
        argoMesh(i+1)(j).north_out.b := argoMesh(i)(j).south_in.b
        argoMesh(i)(j).east_in.f := argoMesh(i)(j+1).west_out.f
        argoMesh(i)(j+1).west_out.b := argoMesh(i)(j).east_in.b
        argoMesh(i)(j).west_in.f := argoMesh(i)(j-1).east_out.f
        argoMesh(i)(j-1).east_out.b := argoMesh(i)(j).west_in.b
      }
    }
  }

}
