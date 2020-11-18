package cop

import Chisel._

import patmos.Constants._
import util._
import ocp._

object Adder extends CoprocessorObject {

  def init(params: Map[String, String]) = {}

  def create(params: Map[String, String]): Adder = Module(new Adder())
}


class Adder() extends Coprocessor_MemoryAccess() {
  //coprocessor definitions
  def FUNC_ADD = "b00000".U(5.W)
  def FUNC_ADD_STALL = "b00001".U(5.W)
  def FUNC_VECTOR_ADD = "b00010".U(5.W)

  // scalar state machine for ADD and ADD_STALL (saturating)
  val scalar_idle :: scalar_add :: Nil = Enum(2)
  val scalar_state = Reg(init = scalar_idle)
  val scalar_intermediate_result = Wire(UInt(width = DATA_WIDTH + 1))
  
  // vector state machine for VECTOR_ADD
  val vector_idle :: vector_read1_req :: vector_read1 :: vector_read2_req :: vector_read2 :: vector_write_req :: vector_write :: vector_done :: Nil = Enum(8)
  val vector_state = Reg(init = vector_idle)
  val vector_src = Reg(UInt(width = DATA_WIDTH))
  val vector_dst = Reg(UInt(width = DATA_WIDTH))
  val vector_acc = Reg(Vec(BURST_LENGTH, UInt(width = DATA_WIDTH)))
  val vector_cnt = Reg(UInt(width = log2Ceil(BURST_LENGTH)))

  // default values
  scalar_intermediate_result := io.copIn.opData(0) +& io.copIn.opData(1)
  io.copOut.result := UInt(0)
  io.copOut.ena_out := Bool(false)

  // start operation
  when(io.copIn.trigger & io.copIn.ena_in) {
    // read or custom command
    when(io.copIn.read) {
      switch(io.copIn.funcId) {
        is(FUNC_ADD) {
          io.copOut.ena_out := Bool(true)
          when(scalar_intermediate_result(DATA_WIDTH) === UInt(1, 1)) {
            io.copOut.result := Fill(DATA_WIDTH, 1.U)
          }.otherwise {
            io.copOut.result := scalar_intermediate_result(DATA_WIDTH - 1, 0)
          }
        }
        is(FUNC_ADD_STALL) {
          scalar_state := scalar_add
        }
        is(FUNC_VECTOR_ADD) {
          io.copOut.ena_out := Bool(true)
          when(vector_state === vector_done) {
            io.copOut.result := UInt(1, DATA_WIDTH.W)
            vector_state := vector_idle
          }
        }
      }
    }.otherwise {
      switch(io.copIn.funcId) {
        is(FUNC_ADD) {
          io.copOut.ena_out := Bool(true)
        }
        is(FUNC_ADD_STALL) {
          io.copOut.ena_out := Bool(true)
        }
        is(FUNC_VECTOR_ADD) {
          io.copOut.ena_out := Bool(true)
          when(vector_state === vector_idle) {
            vector_src := io.copIn.opData(0)
            vector_dst := io.copIn.opData(1)
            vector_state := vector_read1_req
            io.copOut.ena_out := Bool(true)
          }
        }
      }
    }
  }
  
  // logic for ADD_STALL
  when(io.copIn.ena_in & scalar_state === scalar_add) {
    io.copOut.ena_out := Bool(true)
    when(scalar_intermediate_result(DATA_WIDTH) === UInt(1, 1)) {
      io.copOut.result := Fill(DATA_WIDTH, 1.U)
    }.otherwise {
      io.copOut.result := scalar_intermediate_result(DATA_WIDTH - 1, 0)
    }
    scalar_state := scalar_idle
  }
  
  // logic for VECTOR_ADD
  io.memPort.M.Cmd := OcpCmd.IDLE
  io.memPort.M.Addr := UInt(0)
  io.memPort.M.Data := UInt(0)
  io.memPort.M.DataValid := UInt(0)
  io.memPort.M.DataByteEn := "b1111".U
  switch(vector_state) {
    is(vector_read1_req) {
      io.memPort.M.Cmd := OcpCmd.RD
      io.memPort.M.Addr := vector_src
      when(io.memPort.S.CmdAccept === UInt(1)) {
        vector_cnt := UInt(0)
        vector_state := vector_read1
      }
    }
    is(vector_read1) {
      vector_acc(vector_cnt) := io.memPort.S.Data
      when(io.memPort.S.Resp === OcpResp.DVA) {
        when(vector_cnt < UInt(BURST_LENGTH - 1)) {
          vector_cnt := vector_cnt + UInt(1)
        }.otherwise {
          vector_state := vector_read2_req
        }
      }
    }
    is(vector_read2_req) {
      io.memPort.M.Cmd := OcpCmd.RD
      io.memPort.M.Addr := vector_src + UInt(BURST_LENGTH * DATA_WIDTH / BYTE_WIDTH)
      when(io.memPort.S.CmdAccept === UInt(1)) {
        vector_cnt := UInt(0)
        vector_state := vector_read2
      }
    }
    is(vector_read2) {
      vector_acc(vector_cnt) := vector_acc(vector_cnt) + io.memPort.S.Data
      when(io.memPort.S.Resp === OcpResp.DVA) {
        when(vector_cnt < UInt(BURST_LENGTH - 1)) {
          vector_cnt := vector_cnt + UInt(1)
        }.otherwise {
          vector_state := vector_write_req
        }
      }
    }
    is(vector_write_req) {
      io.memPort.M.Cmd := OcpCmd.WR
      io.memPort.M.Addr := vector_dst
      io.memPort.M.Data := vector_acc(0)
      io.memPort.M.DataValid := UInt(1)
      when(io.memPort.S.CmdAccept === UInt(1) && io.memPort.S.DataAccept === UInt(1)) {
        vector_cnt := UInt(1)
        vector_state := vector_write
      }
    }
    is(vector_write) {
      io.memPort.M.Data := vector_acc(vector_cnt)
      io.memPort.M.DataValid := UInt(1)
      when(io.memPort.S.DataAccept === UInt(1)) {
        when(vector_cnt < UInt(BURST_LENGTH - 1)) {
          vector_cnt := vector_cnt + UInt(1)
        }.otherwise {
          vector_state := vector_done
        }
      }
    }
  }
  
}
