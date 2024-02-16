/*
 * Exceptions for Patmos.
 *
 * Author: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package patmos

import chisel3._
import chisel3.util._

import Constants._

import ocp._

class Exceptions extends Module {
  val io = IO(new ExcIO())

  val EXC_ADDR_WIDTH = 8

  val masterReg = RegNext(io.ocp.M)

  val statusReg = RegInit(2.U(DATA_WIDTH.W))
  val maskReg   = Reg(UInt(DATA_WIDTH.W))
  val sourceReg = Reg(UInt(DATA_WIDTH.W))

  val intrEna = statusReg(0) === 1.U
  val superMode = statusReg(1) === 1.U

  val localModeReg = RegInit(false.B)

  def checked(action: => Unit) {
    when (superMode) (action) .otherwise { io.ocp.S.Resp := OcpResp.ERR }
  }

  val vec    = Mem(EXC_COUNT, UInt(DATA_WIDTH.W))
  val vecDup = Mem(EXC_COUNT, UInt(DATA_WIDTH.W))

  val sleepReg = RegInit(false.B)

  // Latches for incoming exceptions and interrupts
  val excPend     = Wire(Vec(EXC_COUNT, Bool()))
  val excPendReg  = RegInit(VecInit(Seq.fill(EXC_COUNT)(false.B)))
  val intrPend    = Wire(Vec(EXC_COUNT, Bool()))
  val intrPendReg = RegInit(VecInit(Seq.fill(EXC_COUNT)(false.B)))
  excPend := excPendReg
  intrPend := intrPendReg

  // Default OCP response
  io.ocp.S.Resp := OcpResp.NULL
  io.ocp.S.Data := 0.U(DATA_WIDTH.W)

  // Make privileged mode visible
  io.superMode := superMode

  // No resetting by default
  io.invalICache := false.B
  io.invalDCache := false.B

  // Handle OCP reads and writes
  when(masterReg.Cmd === OcpCmd.RD) {
    io.ocp.S.Resp := OcpResp.DVA

    switch(masterReg.Addr(EXC_ADDR_WIDTH-1, 2)) {
      is("b000000".U) { io.ocp.S.Data := statusReg }
      is("b000001".U) { io.ocp.S.Data := maskReg }
      is("b000011".U) { io.ocp.S.Data := sourceReg }
      is("b000010".U) { io.ocp.S.Data := intrPendReg.asUInt }
      is("b000101".U) { io.ocp.S.Data := localModeReg ## 0.U((DATA_WIDTH-1).W) }
    }
    when(masterReg.Addr(EXC_ADDR_WIDTH-1) === "b1".U) {
      io.ocp.S.Data := vec(masterReg.Addr(EXC_ADDR_WIDTH-2, 2))
    }
  }


  when(masterReg.Cmd === OcpCmd.WR) {
    io.ocp.S.Resp := OcpResp.DVA
    switch(masterReg.Addr(EXC_ADDR_WIDTH-1, 2)) {
      is("b000000".U) { checked{ statusReg := masterReg.Data } }
      is("b000001".U) { checked{ maskReg := masterReg.Data } }
      is("b000011".U) { checked{ sourceReg := masterReg.Data } }
      is("b000010".U) {
        checked {
          for(i <- 0 until EXC_COUNT) {
            intrPend(i) := intrPendReg(i) & masterReg.Data(i)
          }
        }
      }
      is("b000100".U) {
        checked { // Go to sleep
          io.ocp.S.Resp := OcpResp.NULL
          sleepReg := true.B
        }
      }
      is("b000101".U) {
        checked {
          io.invalDCache := masterReg.Data(0)
          io.invalICache := masterReg.Data(1)
          localModeReg := localModeReg ^ masterReg.Data(DATA_WIDTH-1)
        }
      }
    }
    when(masterReg.Addr(EXC_ADDR_WIDTH-1) === "b1".U) {
      checked {
        vec(masterReg.Addr(EXC_ADDR_WIDTH-2, 2)) := masterReg.Data.asUInt
        vecDup(masterReg.Addr(EXC_ADDR_WIDTH-2, 2)) := masterReg.Data.asUInt
      }
    }
  }

  // Acknowledgement of exception
  when(io.memexc.call) {
    excPend(io.memexc.src) := false.B
    intrPend(io.memexc.src) := false.B
    when(io.ena) {
      sourceReg := io.memexc.src
      // Shift status, enable super mode, disable interrupts
      statusReg := (statusReg << 2.U).asUInt | 2.U
    }
  }
  // Return from exception
  when(io.memexc.ret) {
    when(io.ena) {
      // Shift back old status
      statusReg := statusReg >> 2.U
    }
  }

  // Latch interrupt pins
  for (i <- 0 until INTR_COUNT) {
    when(RegNext(io.intrs(i))) {
      intrPend(16+i) := true.B
    }
  }

  // Trigger internal exceptions
  val excBaseReg = Reg(UInt(PC_SIZE.W))
  val excAddrReg = Reg(UInt(PC_SIZE.W))
  when(io.memexc.exc) {
    excPend(io.memexc.src) := true.B
    excBaseReg := io.memexc.excBase
    excAddrReg := io.memexc.excAddr
  }

  // Latch new pending flags
  excPendReg := excPend
  intrPendReg := intrPend

  // Compute next exception source
  val src = Wire(UInt(EXC_SRC_BITS.W))
  val srcReg = RegNext(src)
  src := 0.U
  for (i <- (0 until EXC_COUNT).reverse) {
    when(intrPend(i) && (maskReg(i) === 1.U)) { src := i.U }
  }
  for (i <- (0 until EXC_COUNT).reverse) {
    when(excPend(i)) { src := i.U }
  }

  // Create signals to decode stage
  val exc = RegNext(excPend.asUInt =/= 0.U)
  val intr = RegNext((intrPend.asUInt & maskReg) =/= 0.U)

  io.excdec.exc   := exc
  io.excdec.intr  := intr && intrEna
  io.excdec.addr  := vecDup(srcReg)
  io.excdec.src   := srcReg
  io.excdec.local := localModeReg

  io.excdec.excBase := excBaseReg
  io.excdec.excAddr := excAddrReg

  // Wake up
  when (sleepReg && (exc === 1.U || (intr && intrEna))) {
    io.ocp.S.Resp := OcpResp.DVA
    sleepReg := false.B
  }
}
