/*
 * A pool of shared scratchpad memories with owners.
 * 
 * Super simple solution without any ownership enforcement,
 * just merge all requests with an OR and have the response
 * routed to all (and gated with a cmd registered).
 * Ownership and transfer has to be organized in software
 * (e.g., use a shared variable in main memory).
 *
 * Author: Martin Schoeberl (martin@jopdesign.com)
 */

package cmp

import chisel3._
import chisel3.util._

import patmos._
import patmos.Constants._
import ocp._

class OwnSPM(nrCores: Int, nrSPMs: Int, size: Int) extends CmpDevice(nrCores) {

  val io = IO(new CmpIO(nrCores))

  val bits = log2Up(nrSPMs)
  println("OwnSPM: cnt = " + nrSPMs + " bits = " + bits)

  val masters = Wire(Vec(nrSPMs, Vec(nrCores, new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH))))
  masters := DontCare
  val spms = (0 until nrSPMs).map(i => Module(new Spm(size)))
  val cmdOutReg = RegInit(VecInit(Seq.fill(nrCores)(false.B)))

  for (s <- 0 until nrSPMs) {
    // And gate non-active masters.
    for (i <- 0 until nrCores) {
      masters(s)(i).M.Addr := 0.U
      masters(s)(i).M.Data := 0.U
      masters(s)(i).M.Cmd := 0.U
      masters(s)(i).M.ByteEn := 0.U
      when(io.cores(i).M.Cmd =/= OcpCmd.IDLE && io.cores(i).M.Addr(12 + bits - 1, 12) === s.U) {
        masters(s)(i).M := io.cores(i).M
      }
      masters(s)(i).S.Resp := DontCare
    }

    // Or the master signals
    spms(s).io.M.Addr := masters(s).map(_.M.Addr).reduce((x, y) => x | y)
    spms(s).io.M.Data := masters(s).map(_.M.Data).reduce((x, y) => x | y)
    spms(s).io.M.Cmd := masters(s).map(_.M.Cmd).reduce((x, y) => x | y)
    spms(s).io.M.ByteEn := masters(s).map(_.M.ByteEn).reduce((x, y) => x | y)
  }
  
  // Cmd out?
  for (i <- 0 until nrCores) {
    cmdOutReg(i) := false.B
    when(io.cores(i).M.Cmd =/= OcpCmd.IDLE) {
      cmdOutReg(i) := true.B
    }
  }
  
  // Connect SPM out to output muxes and muxes to slave responses
  val muxes = Wire(Vec(nrSPMs, Vec(nrSPMs, new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH))))
  muxes := DontCare
  for (i <- 0 until nrCores) {
    // val mux = Vec(nrSPMs, new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH))
    for (j <- 0 until nrSPMs) {
      muxes(i)(j).S := spms(j).io.S
    }
    // And gate S response as DVA from one core might cause an end of a main memory
    // transaction from a different core.
    // TODO: check if really needed for the data
    io.cores(i).S.Data := 0.U
    io.cores(i).S.Resp := OcpResp.NULL
    when(cmdOutReg(i)) {
      io.cores(i).S := muxes(i)(RegNext(io.cores(i).M.Addr(12 + bits - 1, 12))).S
    }
  }
}
