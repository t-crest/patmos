/*
 * A shared scratchpad memory supporting CAS.
 *
 * Author: Torur Biskopsto Strom (torur.strom@gmail.com)
 */

package cmp

import Chisel._
import ocp._
import patmos.Constants._
import patmos._

class CASPM(corecnt: Int, size: Int) extends Module {

  val io = IO(new CmpIO(corecnt)) //Vec(corecnt, new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH))

  val spm = Module(new Spm(size))

  val cntmax = 2.U
  val precnt = Reg(init = UInt(0, cntmax.getWidth))
  precnt := Mux(precnt === cntmax, 0.U, precnt + 1.U)
  val cnt = Reg(init = UInt(0, log2Up(corecnt)))
  cnt := Mux(precnt =/= cntmax, cnt, Mux(cnt === (corecnt-1).U, 0.U, cnt + 1.U))

  val cmdRegs = RegInit(Vec.fill(corecnt) {OcpCmd.RD})
  val addrRegs = Reg(Vec(corecnt, spm.io.M.Addr))
  val newvalRegs = Reg(Vec(corecnt, spm.io.M.Data))
  val bytenRegs = Reg(Vec(corecnt, spm.io.M.ByteEn))

  val expvalRegs = Reg(Vec(corecnt, spm.io.S.Data))

  val sIdle :: sRead :: sWrite :: Nil = Enum(UInt(),3)
  val states = RegInit(Vec.fill(corecnt) {sIdle})

  spm.io.M.Cmd := cmdRegs(cnt)
  spm.io.M.Addr := addrRegs(cnt)
  spm.io.M.Data := newvalRegs(cnt)
  spm.io.M.ByteEn := bytenRegs(cnt)

  for (i <- 0 until corecnt) {

    val respReg = Reg(io.cores(i).S.Resp, OcpResp.NULL)
    io.cores(i).S.Resp := respReg
    io.cores(i).S.Data := spm.io.S.Data

    respReg := OcpResp.NULL
    cmdRegs(i) := OcpCmd.RD

    switch(states(i)) {
      is(sIdle) {
        when(io.cores(i).M.Cmd === OcpCmd.RD) {
          states(i) := sRead
          addrRegs(i) := io.cores(i).M.Addr
          bytenRegs(i) := io.cores(i).M.ByteEn
        }.elsewhen(io.cores(i).M.Cmd === OcpCmd.WR) {
          when(io.cores(i).M.Addr(2)) {
            newvalRegs(i) := io.cores(i).M.Data
            respReg := OcpResp.DVA
          }.otherwise {
            expvalRegs(i) := io.cores(i).M.Data
            respReg := OcpResp.DVA
          }
        }
      }
      is(sRead) {
        when(cnt === i.U && precnt === 0.U) {
          states(i) := sWrite
        }
      }
      is(sWrite) {
        when(spm.io.S.Data === expvalRegs(i)) {
          cmdRegs(i) := OcpCmd.WR
        }
        respReg := OcpResp.DVA
        states(i) := sIdle
      }
    }
  }
}

