/*
 * Argo Instantiation and Interconnection
 *
 * Authors: Eleftherios Kyriakakis (elky@dtu.dk)
 *
 */
 
package argo

import chisel3._
import chisel3.util._
import ocp.OcpIOSlavePort
import patmos.Constants.{ADDR_WIDTH, DATA_WIDTH}

class ArgoNoC(val argoConf: ArgoConfig, wrapped: Boolean = false, emulateBB: Boolean = false) extends Module {
  val io = IO(new Bundle {
    val irq = Output(UInt((argoConf.CORES*2).W))
    val supervisor = Input(UInt(argoConf.CORES.W))
    val ocpPorts = Vec(argoConf.CORES, new OcpIOSlavePort(ADDR_WIDTH, DATA_WIDTH))
    val spmPorts = Vec(argoConf.CORES, new SPMMasterPort(argoConf.HEADER_FIELD_WIDTH, argoConf.HEADER_CTRL_WIDTH))
  })

  io.irq := 0.U
  val irqSets = Wire(Vec(argoConf.CORES, UInt(2.W)))
  //Interconnect
  if(!wrapped) {
    val masterRunWire = Wire(UInt(1.W))
    val argoNodes = (0 until argoConf.M).map(j =>
      (0 until argoConf.N).map(i =>
        if (emulateBB) Module(new NoCNodeDummy(argoConf, i == 0 && j == 0)) else Module(new NoCNodeWrapper(argoConf, i == 0 && j == 0))))

    if (!emulateBB) {
      argoNodes.flatten.foreach(n => {
        val node = n.asInstanceOf[NoCNodeWrapper]
        Seq(node.io.north_in, node.io.east_in, node.io.south_in, node.io.west_in).foreach(p => {
          p.f.req := false.B
        })
        Seq(node.io.north_out, node.io.east_out, node.io.south_out, node.io.west_out).foreach(p => {
          p.b.ack := false.B
        })
      })
    }

    val argoMesh = Wire(Vec(argoConf.M, Vec(argoConf.N, new NodeInterconnection(argoConf))))
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
    if (emulateBB){
      masterRunWire := argoNodes(0)(0).asInstanceOf[NoCNodeDummy].io.masterRun
    }else{
      masterRunWire := argoNodes(0)(0).asInstanceOf[NoCNodeWrapper].io.masterRun
    }
    for (i <- 0 until argoConf.M) {
      for (j <- 0 until argoConf.N) {
        //Linear index for mapping
        val index = (i * argoConf.N) + j
        println("|---Node #" + index + " @ (" + i + "," + j + ")")
        //Control Ports
        if (emulateBB) {
          argoNodes(i)(j).asInstanceOf[NoCNodeDummy].io.supervisor := io.supervisor(index)
          argoNodes(i)(j).asInstanceOf[NoCNodeDummy].io.proc.M := io.ocpPorts(index).M
          io.ocpPorts(index).S := argoNodes(i)(j).asInstanceOf[NoCNodeDummy].io.proc.S
          io.spmPorts(index).M := argoNodes(i)(j).asInstanceOf[NoCNodeDummy].io.spm.M
          argoNodes(i)(j).asInstanceOf[NoCNodeDummy].io.spm.S := io.spmPorts(index).S
          argoNodes(i)(j).asInstanceOf[NoCNodeDummy].io.run := masterRunWire
          //io.irq(2 + index * 2 - 1, index * 2) := argoNodes(i)(j).asInstanceOf[NoCNodeDummy].io.irq //No subfield assignment in Chisel3
          irqSets(index) := argoNodes(i)(j).asInstanceOf[NoCNodeDummy].io.irq
          argoNodes(i)(j).asInstanceOf[NoCNodeDummy].io.north_in.f.data := argoMesh(i)(j).north_wire_in
          argoNodes(i)(j).asInstanceOf[NoCNodeDummy].io.south_in.f.data := argoMesh(i)(j).south_wire_in
          argoNodes(i)(j).asInstanceOf[NoCNodeDummy].io.east_in.f.data := argoMesh(i)(j).east_wire_in
          argoNodes(i)(j).asInstanceOf[NoCNodeDummy].io.west_in.f.data := argoMesh(i)(j).west_wire_in
          argoMesh(i)(j).north_wire_out := argoNodes(i)(j).asInstanceOf[NoCNodeDummy].io.north_out.f.data
          argoMesh(i)(j).south_wire_out := argoNodes(i)(j).asInstanceOf[NoCNodeDummy].io.south_out.f.data
          argoMesh(i)(j).east_wire_out := argoNodes(i)(j).asInstanceOf[NoCNodeDummy].io.east_out.f.data
          argoMesh(i)(j).west_wire_out := argoNodes(i)(j).asInstanceOf[NoCNodeDummy].io.west_out.f.data
        } else {
          argoNodes(i)(j).asInstanceOf[NoCNodeWrapper].io.supervisor := io.supervisor(index)
          argoNodes(i)(j).asInstanceOf[NoCNodeWrapper].io.proc.M := io.ocpPorts(index).M
          io.ocpPorts(index).S := argoNodes(i)(j).asInstanceOf[NoCNodeWrapper].io.proc.S
          io.spmPorts(index).M := argoNodes(i)(j).asInstanceOf[NoCNodeWrapper].io.spm.M
          argoNodes(i)(j).asInstanceOf[NoCNodeWrapper].io.spm.S := io.spmPorts(index).S
          argoNodes(i)(j).asInstanceOf[NoCNodeWrapper].io.run := masterRunWire
          //io.irq(2 + index * 2 - 1, index * 2) := argoNodes(i)(j).asInstanceOf[NoCNodeWrapper].io.irq //No subfield assignment in Chisel3
          irqSets(index) := argoNodes(i)(j).asInstanceOf[NoCNodeWrapper].io.irq
          argoNodes(i)(j).asInstanceOf[NoCNodeWrapper].io.north_in.f.data := argoMesh(i)(j).north_wire_in
          argoNodes(i)(j).asInstanceOf[NoCNodeWrapper].io.south_in.f.data := argoMesh(i)(j).south_wire_in
          argoNodes(i)(j).asInstanceOf[NoCNodeWrapper].io.east_in.f.data := argoMesh(i)(j).east_wire_in
          argoNodes(i)(j).asInstanceOf[NoCNodeWrapper].io.west_in.f.data := argoMesh(i)(j).west_wire_in
          argoMesh(i)(j).north_wire_out := argoNodes(i)(j).asInstanceOf[NoCNodeWrapper].io.north_out.f.data
          argoMesh(i)(j).south_wire_out := argoNodes(i)(j).asInstanceOf[NoCNodeWrapper].io.south_out.f.data
          argoMesh(i)(j).east_wire_out := argoNodes(i)(j).asInstanceOf[NoCNodeWrapper].io.east_out.f.data
          argoMesh(i)(j).west_wire_out := argoNodes(i)(j).asInstanceOf[NoCNodeWrapper].io.west_out.f.data
          argoNodes(i)(j).asInstanceOf[NoCNodeWrapper].io.clk := clock
          argoNodes(i)(j).asInstanceOf[NoCNodeWrapper].io.reset := reset
        }
      }
    }
    io.irq := irqSets.asUInt
    
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
