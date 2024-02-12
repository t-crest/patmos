/*
 * Coprocessor featuring Saturated Addition and Vector-Addition
 *
 * Authors: Christoph Lehr (christoph.lehr@gmx.at)
 *          Alexander Baranyai (alexyai@gmx.at)
 *          Clemens Pircher (clemens.lukas@gmx.at)
 *
 */

package cop

import Chisel._

import patmos.Constants._
import util._
import ocp._

object Adder extends CoprocessorObject {

  def init(params: Map[String, String]) = {}

  def create(params: Map[String, String]): Adder = Module(new Adder())
}


class Adder() extends CoprocessorMemoryAccess() {
  //coprocessor definitions
  def FUNC_ADD = "b00000".U(5.W)
  def FUNC_ADD_STALL = "b00001".U(5.W)
  def FUNC_VECTOR_ADD = "b00010".U(5.W)

  // scalar state machine for ADD and ADD_STALL (saturating)
  val scalarIdle :: scalarAdd :: Nil = Enum(2)
  val scalarStateReg = Reg(init = scalarIdle)
  val scalarIntermediateResult = Wire(UInt(width = DATA_WIDTH + 1))
  
  // vector state machine for VECTOR_ADD
  val vectorIdle :: vectorRead1Req :: vectorRead1 :: vectorRead2Req :: vectorRead2 :: vectorWriteReq :: vectorWrite :: vectorDone :: Nil = Enum(8)
  val vectorStateReg = Reg(init = vectorIdle)
  val vectorSrcReg = Reg(UInt(width = DATA_WIDTH))
  val vectorDstReg = Reg(UInt(width = DATA_WIDTH))
  val vectorAccReg = Reg(Vec(BURST_LENGTH, UInt(width = DATA_WIDTH)))
  val vectorCntReg = Reg(UInt(width = log2Ceil(BURST_LENGTH)))

  // default values
  scalarIntermediateResult := io.copIn.opData(0) +& io.copIn.opData(1)
  io.copOut.result := UInt(0)
  io.copOut.ena_out := false.B

  // start operation
  when(io.copIn.trigger & io.copIn.ena_in) {
    // read or custom command
    when(io.copIn.read) {
      switch(io.copIn.funcId) {
        is(FUNC_ADD) {
          io.copOut.ena_out := true.B
          when(scalarIntermediateResult(DATA_WIDTH) === UInt(1, 1)) {
            io.copOut.result := Fill(DATA_WIDTH, 1.U)
          }.otherwise {
            io.copOut.result := scalarIntermediateResult(DATA_WIDTH - 1, 0)
          }
        }
        is(FUNC_ADD_STALL) {
          scalarStateReg := scalarAdd
        }
      }
    }.elsewhen(io.copIn.isCustom) {
      switch(io.copIn.funcId) {
        is(FUNC_VECTOR_ADD) {
          io.copOut.ena_out := true.B
          when(vectorStateReg === vectorDone) {
            io.copOut.result := UInt(1, DATA_WIDTH.W)
            vectorStateReg := vectorIdle
          }
        }
      }
    }.otherwise {
      switch(io.copIn.funcId) {
        is(FUNC_ADD) {
          io.copOut.ena_out := true.B
        }
        is(FUNC_ADD_STALL) {
          io.copOut.ena_out := true.B
        }
        is(FUNC_VECTOR_ADD) {
          io.copOut.ena_out := true.B
          when(vectorStateReg === vectorIdle) {
            vectorSrcReg := io.copIn.opData(0)
            vectorDstReg := io.copIn.opData(1)
            vectorStateReg := vectorRead1Req
            io.copOut.ena_out := true.B
          }
        }
      }
    }
  }
  
  // logic for ADD_STALL
  when(io.copIn.ena_in & scalarStateReg === scalarAdd) {
    io.copOut.ena_out := true.B
    when(scalarIntermediateResult(DATA_WIDTH) === UInt(1, 1)) {
      io.copOut.result := Fill(DATA_WIDTH, 1.U)
    }.otherwise {
      io.copOut.result := scalarIntermediateResult(DATA_WIDTH - 1, 0)
    }
    scalarStateReg := scalarIdle
  }
  
  // logic for VECTOR_ADD
  io.memPort.M.Cmd := OcpCmd.IDLE
  io.memPort.M.Addr := UInt(0)
  io.memPort.M.Data := UInt(0)
  io.memPort.M.DataValid := UInt(0)
  io.memPort.M.DataByteEn := "b1111".U
  switch(vectorStateReg) {
    is(vectorRead1Req) {
      io.memPort.M.Cmd := OcpCmd.RD
      io.memPort.M.Addr := vectorSrcReg
      when(io.memPort.S.CmdAccept === UInt(1)) {
        vectorCntReg := UInt(0)
        vectorStateReg := vectorRead1
      }
    }
    is(vectorRead1) {
      vectorAccReg(vectorCntReg) := io.memPort.S.Data
      when(io.memPort.S.Resp === OcpResp.DVA) {
        when(vectorCntReg < UInt(BURST_LENGTH - 1)) {
          vectorCntReg := vectorCntReg + UInt(1)
        }.otherwise {
          vectorStateReg := vectorRead2Req
        }
      }
    }
    is(vectorRead2Req) {
      io.memPort.M.Cmd := OcpCmd.RD
      io.memPort.M.Addr := vectorSrcReg + UInt(BURST_LENGTH * DATA_WIDTH / BYTE_WIDTH)
      when(io.memPort.S.CmdAccept === UInt(1)) {
        vectorCntReg := UInt(0)
        vectorStateReg := vectorRead2
      }
    }
    is(vectorRead2) {
      vectorAccReg(vectorCntReg) := vectorAccReg(vectorCntReg) + io.memPort.S.Data
      when(io.memPort.S.Resp === OcpResp.DVA) {
        when(vectorCntReg < UInt(BURST_LENGTH - 1)) {
          vectorCntReg := vectorCntReg + UInt(1)
        }.otherwise {
          vectorStateReg := vectorWriteReq
        }
      }
    }
    is(vectorWriteReq) {
      io.memPort.M.Cmd := OcpCmd.WR
      io.memPort.M.Addr := vectorDstReg
      io.memPort.M.Data := vectorAccReg(0)
      io.memPort.M.DataValid := UInt(1)
      when(io.memPort.S.CmdAccept === UInt(1) && io.memPort.S.DataAccept === UInt(1)) {
        vectorCntReg := UInt(1)
        vectorStateReg := vectorWrite
      }
    }
    is(vectorWrite) {
      io.memPort.M.Data := vectorAccReg(vectorCntReg)
      io.memPort.M.DataValid := UInt(1)
      when(io.memPort.S.DataAccept === UInt(1)) {
        when(vectorCntReg < UInt(BURST_LENGTH - 1)) {
          vectorCntReg := vectorCntReg + UInt(1)
        }.otherwise {
          vectorStateReg := vectorDone
        }
      }
    }
  }
  
}
