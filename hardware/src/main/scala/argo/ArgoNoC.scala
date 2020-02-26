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
  val io = IO(new Bundle {
    val irq = Bits(OUTPUT, width = argoConf.CORES*2)
    val supervisor = Bits(INPUT, width = argoConf.CORES)
    val ocpPorts = Vec(argoConf.CORES, new OcpIOSlavePort(ADDR_WIDTH, DATA_WIDTH))
    val spmPorts = Vec(argoConf.CORES, new SPMMasterPort(argoConf.HEADER_FIELD_WIDTH, argoConf.HEADER_CTRL_WIDTH))
  })

  io.irq := 0.U
  //Interconnect
  if(!wrapped) {
    val masterRunWire = Wire(Bits(width=1))
    val argoNodes = (0 until argoConf.M).map(j =>
      (0 until argoConf.N).map(i =>
        if (emulateBB) Module(new NoCNodeDummy(argoConf, i == 0 && j == 0)).io else Module(new NoCNodeWrapper(argoConf, i == 0 && j == 0)).io))
    val argoMesh = Vec(argoConf.M, Vec(argoConf.N, new NodeInterconnection(argoConf)))
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
    println("o--Instantiating Nodes")
    masterRunWire := argoNodes(0)(0).masterRun
    for (i <- 0 until argoConf.M) {
      for (j <- 0 until argoConf.N) {
        //Linear index for mapping
        val index = (i * argoConf.N) + j
        println("|---Node #" + index + " @ (" + i + "," + j + ")")
        //Control Ports
        argoNodes(i)(j).supervisor := io.supervisor(index)
        argoNodes(i)(j).proc.M := io.ocpPorts(index).M
        io.ocpPorts(index).S := argoNodes(i)(j).proc.S
        io.spmPorts(index).M := argoNodes(i)(j).spm.M
        argoNodes(i)(j).spm.S := io.spmPorts(index).S
        argoNodes(i)(j).run := masterRunWire
        io.irq(2 + index * 2 - 1, index * 2) := argoNodes(i)(j).irq
        argoNodes(i)(j).north_in.f.data := argoMesh(i)(j).north_wire_in
        argoNodes(i)(j).south_in.f.data := argoMesh(i)(j).south_wire_in
        argoNodes(i)(j).east_in.f.data := argoMesh(i)(j).east_wire_in
        argoNodes(i)(j).west_in.f.data := argoMesh(i)(j).west_wire_in
        argoMesh(i)(j).north_wire_out := argoNodes(i)(j).north_out.f.data
        argoMesh(i)(j).south_wire_out := argoNodes(i)(j).south_out.f.data
        argoMesh(i)(j).east_wire_out := argoNodes(i)(j).east_out.f.data
        argoMesh(i)(j).west_wire_out := argoNodes(i)(j).west_out.f.data

      }
    }
    println("o--Building Interconnect")
    for (i <- 0 until argoConf.M) {
      for (j <- 0 until argoConf.N) {
        if (i == 0) {
          //wrap ns
          argoMesh(0)(j).south_wire_in := argoMesh(argoConf.M - 1)(j).north_wire_out
          argoMesh(argoConf.M - 1)(j).north_wire_in := argoMesh(0)(j).south_wire_out
        }
        if (j == 0) {
          //wrap ew
          argoMesh(i)(0).east_wire_in := argoMesh(i)(argoConf.N - 1).west_wire_out
          argoMesh(i)(argoConf.N - 1).west_wire_in := argoMesh(i)(0).east_wire_out
        }
        if (i > 0) {
          //ns
          argoMesh(i)(j).south_wire_in := argoMesh(i - 1)(j).north_wire_out
          argoMesh(i - 1)(j).north_wire_in := argoMesh(i)(j).south_wire_out
        }
        if (j > 0) {
          //ew
          argoMesh(i)(j).east_wire_in := argoMesh(i)(j - 1).west_wire_out
          argoMesh(i)(j - 1).west_wire_in := argoMesh(i)(j).east_wire_out
        }
      }
    }
  } else {
    println("o--Wrapping Nodes and Interconnect")
    val nocBB = Module(new NoCWrapper(argoConf))
    io.irq <> nocBB.io.irq
    io.supervisor <> nocBB.io.supervisor
    io.ocpPorts <> nocBB.io.ocpPorts
    io.spmPorts <> nocBB.io.spmPorts
  }

}
