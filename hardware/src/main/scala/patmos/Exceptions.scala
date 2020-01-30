/*
 * Exceptions for Patmos.
 *
 * Author: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package patmos

import Chisel._

import Constants._

import ocp._

class Exceptions extends Module {
  val io = IO(new ExcIO())

  val EXC_ADDR_WIDTH = 8

  val masterReg = RegNext(io.ocp.M)

  val statusReg = RegInit(UInt(2, width = DATA_WIDTH))
  val maskReg   = Reg(UInt(width = DATA_WIDTH))
  val sourceReg = Reg(UInt(width = DATA_WIDTH))

  val intrEna = statusReg(0) === UInt(1)
  val superMode = statusReg(1) === UInt(1)

  val localModeReg = RegInit(Bool(false))

  def checked(action: => Unit) {
    when (superMode) (action) .otherwise { io.ocp.S.Resp := OcpResp.ERR }
  }

  val vec    = Mem(UInt(width = DATA_WIDTH), EXC_COUNT)
  val vecDup = Mem(UInt(width = DATA_WIDTH), EXC_COUNT)

  val sleepReg = RegInit(Bool(false))

  // Latches for incoming exceptions and interrupts
  val excPend     = Wire(Vec(EXC_COUNT, Bool()))
  val excPendReg  = Wire(Vec(EXC_COUNT, RegInit(Bool(false))))
  val intrPend    = Wire(Vec(EXC_COUNT, Bool()))
  val intrPendReg = Wire(Vec(EXC_COUNT, RegInit(Bool(false))))
  excPend := excPendReg
  intrPend := intrPendReg

  // Default OCP response
  io.ocp.S.Resp := OcpResp.NULL
  io.ocp.S.Data := UInt(0, width = DATA_WIDTH)

  // Make privileged mode visible
  io.superMode := superMode

  // No resetting by default
  io.invalICache := Bool(false)
  io.invalDCache := Bool(false)

  // Handle OCP reads and writes
  when(masterReg.Cmd === OcpCmd.RD) {
    io.ocp.S.Resp := OcpResp.DVA

    switch(masterReg.Addr(EXC_ADDR_WIDTH-1, 2)) {
      is(UInt("b000000")) { io.ocp.S.Data := statusReg }
      is(UInt("b000001")) { io.ocp.S.Data := maskReg }
      is(UInt("b000011")) { io.ocp.S.Data := sourceReg }
      is(UInt("b000010")) { io.ocp.S.Data := intrPendReg.asUInt }
      is(UInt("b000101")) { io.ocp.S.Data := localModeReg ## UInt(0, DATA_WIDTH-1) }
    }
    when(masterReg.Addr(EXC_ADDR_WIDTH-1) === UInt("b1")) {
      io.ocp.S.Data := vec(masterReg.Addr(EXC_ADDR_WIDTH-2, 2))
    }
  }


  when(masterReg.Cmd === OcpCmd.WR) {
    io.ocp.S.Resp := OcpResp.DVA
    switch(masterReg.Addr(EXC_ADDR_WIDTH-1, 2)) {
      is(UInt("b000000")) { checked{ statusReg := masterReg.Data } }
      is(UInt("b000001")) { checked{ maskReg := masterReg.Data } }
      is(UInt("b000011")) { checked{ sourceReg := masterReg.Data } }
      is(UInt("b000010")) {
        checked {
          for(i <- 0 until EXC_COUNT) {
            intrPend(i) := intrPendReg(i) & masterReg.Data(i)
          }
        }
      }
      is(UInt("b000100")) {
        checked { // Go to sleep
          io.ocp.S.Resp := OcpResp.NULL
          sleepReg := Bool(true)
        }
      }
      is(UInt("b000101")) {
        checked {
          io.invalDCache := masterReg.Data(0)
          io.invalICache := masterReg.Data(1)
          localModeReg := localModeReg ^ masterReg.Data(DATA_WIDTH-1)
        }
      }
    }
    when(masterReg.Addr(EXC_ADDR_WIDTH-1) === UInt("b1")) {
      checked {
        vec(masterReg.Addr(EXC_ADDR_WIDTH-2, 2)) := masterReg.Data.asUInt
        vecDup(masterReg.Addr(EXC_ADDR_WIDTH-2, 2)) := masterReg.Data.asUInt
      }
    }
  }

  // Acknowledgement of exception
  when(io.memexc.call) {
    excPend(io.memexc.src) := Bool(false)
    intrPend(io.memexc.src) := Bool(false)
    when(io.ena) {
      sourceReg := io.memexc.src
      // Shift status, enable super mode, disable interrupts
      statusReg := (statusReg << UInt(2)) | UInt(2)
    }
  }
  // Return from exception
  when(io.memexc.ret) {
    when(io.ena) {
      // Shift back old status
      statusReg := statusReg >> UInt(2)
    }
  }

  // Latch interrupt pins
  for (i <- 0 until INTR_COUNT) {
    when(RegNext(io.intrs(i))) {
      intrPend(16+i) := Bool(true)
    }
  }

  // Trigger internal exceptions
  val excBaseReg = Reg(UInt(width = PC_SIZE))
  val excAddrReg = Reg(UInt(width = PC_SIZE))
  when(io.memexc.exc) {
    excPend(io.memexc.src) := Bool(true)
    excBaseReg := io.memexc.excBase
    excAddrReg := io.memexc.excAddr
  }

  // Latch new pending flags
  excPendReg := excPend
  intrPendReg := intrPend

  // Compute next exception source
  val src = Wire(UInt(width = EXC_SRC_BITS))
  val srcReg = RegNext(src)
  src := UInt(0)
  for (i <- (0 until EXC_COUNT).reverse) {
    when(intrPend(i) && (maskReg(i) === UInt(1))) { src := UInt(i) }
  }
  for (i <- (0 until EXC_COUNT).reverse) {
    when(excPend(i)) { src := UInt(i) }
  }

  // Create signals to decode stage
  val exc = RegNext(excPend.asUInt =/= UInt(0))
  val intr = RegNext((intrPend.asUInt & maskReg) =/= UInt(0))

  io.excdec.exc   := exc
  io.excdec.intr  := intr && intrEna
  io.excdec.addr  := vecDup(srcReg)
  io.excdec.src   := srcReg
  io.excdec.local := localModeReg

  io.excdec.excBase := excBaseReg
  io.excdec.excAddr := excAddrReg

  // Wake up
  when (sleepReg && (exc === UInt(1) || (intr && intrEna))) {
    io.ocp.S.Resp := OcpResp.DVA
    sleepReg := Bool(false)
  }
}
