/*
 * Floating-Point Unit Interface
 *
 * Author: Eleftherios Kyriakakis (elky@dtu.dk)
 *
 */

package io

import chisel3._
import chisel3.util._
import chisel3.internal.HasId
import ocp._
import patmos.Constants._

class FPUInterface() extends Bundle() {
  val clk = Input(Clock())
  val rst = Input(Bool())
  // inputs
  val enable = Input(Bool())
  val rmode = Input(UInt(2.W))
  val fpu_op = Input(UInt(3.W))
  val opa = Input(UInt(64.W))
  val opb = Input(UInt(64.W))
  //outputs
  val out_fp = Output(UInt(64.W))
  val ready = Output(Bool())
  val underflow = Output(Bool())
  val overflow = Output(Bool())
  val inexact = Output(Bool())
  val exception = Output(Bool())
  val invalid = Output(Bool())
}


object FPUDouble extends DeviceObject {

  def init(params: Map[String, String]) = {
  }

  def create(params: Map[String, String]): FPUDouble = {
    Module(new FPUDouble())
  }
}

/**
  * Wrapper for ~/t-crest/patmos/hardware/vhdl/fpu_double.vhd
  */
class FPUDoubleBB() extends BlackBox() {
  val io = IO(new FPUInterface)

  io.clk.suggestName("clk")
  io.rst.suggestName("rst")
  io.enable.suggestName("enable")
  io.rmode.suggestName("rmode")
  io.fpu_op.suggestName("fpu_op")
  io.opa.suggestName("opa")
  io.opb.suggestName("opb")
  io.out_fp.suggestName("out_fp")
  io.ready.suggestName("ready")
  io.underflow.suggestName("underflow")
  io.overflow.suggestName("overflow")
  io.inexact.suggestName("inexact")
  io.exception.suggestName("exception")
  io.invalid.suggestName("invalid")

  override def desiredName: String = "fpu_double"
}

class FPUDouble() extends CoreDevice() {
  override val io = IO(new CoreDeviceIO())

  // The BB
  val fpuBB = Module(new FPUDoubleBB())

  // Register command
  val masterReg = RegNext(io.ocp.M)

  // Default response
  val respReg = RegInit(OcpResp.NULL)
  respReg := OcpResp.NULL
  val dataReg = RegInit(0.U(DATA_WIDTH.W))

  // Somewhere to store the interface state
  val enableReg = RegInit(false.B)
  val roundingModeReg = RegInit(0.U(2.W))
  val selOperationReg = RegInit(0.U(3.W))
  val operandAReg = RegInit(0.U(64.W))
  val operandBReg = RegInit(0.U(64.W))
  val resultsReg = RegInit(0.U(64.W))
  val statusReg = RegInit(0.U(6.W))

  // Write response
  enableReg := false.B
  when(masterReg.Cmd === OcpCmd.WR) {
    respReg := OcpResp.DVA
    switch(masterReg.Addr(5, 0)) {
      is(0x00.U) {
        operandAReg := operandAReg(2*DATA_WIDTH-1, 0) ## masterReg.Data
      }
      is(0x04.U) {
        operandAReg := masterReg.Data ## operandAReg(DATA_WIDTH-1, 0)
      }
      is(0x08.U) {
        operandBReg := operandBReg(2*DATA_WIDTH-1, 0) ## masterReg.Data
      }
      is(0x0C.U) {
        operandBReg := masterReg.Data ## operandBReg(DATA_WIDTH-1, 0)
      }
      is(0x10.U) {
        enableReg := true.B
      }
      is(0x14.U) {
        selOperationReg := masterReg.Data(2, 0)
      }
      is(0x18.U) {
        roundingModeReg := masterReg.Data(1, 0)
      }
    }
  }

  when(masterReg.Cmd === OcpCmd.RD) {
    respReg := OcpResp.DVA
    switch(masterReg.Addr(5, 0)) {
      is(0x00.U) {
        dataReg := operandAReg(DATA_WIDTH-1, 0)
      }
      is(0x04.U) {
        dataReg := operandAReg(2*DATA_WIDTH-1, 0)
      }
      is(0x08.U) {
        dataReg := operandBReg(DATA_WIDTH-1, 0)
      }
      is(0x0C.U) {
        dataReg := operandBReg(2*DATA_WIDTH-1, 0)
      }
      is(0x14.U) {
        dataReg := 0.U(DATA_WIDTH-1, 3) ## selOperationReg
      }
      is(0x18.U) {
        dataReg := 0.U(DATA_WIDTH-1, 2) ## roundingModeReg
      }
      is(0x1C.U) {
        dataReg := 0.U(DATA_WIDTH-1, 2) ## statusReg
      }
      is(0x20.U) {
        dataReg := resultsReg(DATA_WIDTH-1, 0)
      }
      is(0x24.U) {
        dataReg := resultsReg(2*DATA_WIDTH-1, DATA_WIDTH)
      }
    }
  }

  // Connections to FPUBB
  fpuBB.io.clk := clock
  fpuBB.io.rst := reset
  fpuBB.io.enable := enableReg
  fpuBB.io.rmode := roundingModeReg
  fpuBB.io.fpu_op := selOperationReg
  fpuBB.io.opa := operandAReg
  fpuBB.io.opb := operandBReg
  resultsReg := Mux(fpuBB.io.ready, fpuBB.io.out_fp, resultsReg)
  statusReg := Mux(fpuBB.io.ready, fpuBB.io.underflow ## fpuBB.io.overflow ## fpuBB.io.inexact ## fpuBB.io.exception ## fpuBB.io.invalid ## fpuBB.io.ready, statusReg)

  // Connections to master
  io.ocp.S.Resp := respReg
  io.ocp.S.Data := dataReg
}