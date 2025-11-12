/*
 * Arbiter for OCP burst slaves.
 * Pseudo round robin arbitration. Each turn for a non-requesting master costs 1 clock cycle.
 *
 * Author: Martin Schoeberl (martin@jopdesign.com) David Chong (davidchong99@gmail.com)
 *
 */

package ocp

import chisel3._
import chisel3.util._

class TdmArbiter(cnt: Int, addrWidth : Int, dataWidth : Int, burstLen : Int) extends ArbiterType(cnt, dataWidth, dataWidth, burstLen) {

  val cntReg = RegInit(init = 0.U(log2Up(cnt*(burstLen + 2)).W))
  // slot length = burst size + 2 
  val burstCntReg = RegInit(init = 0.U(log2Up(burstLen).W))
  val period = cnt * (burstLen + 2)
  val slotLen = burstLen + 2
  val cpuSlot = RegInit(VecInit(Seq.fill(cnt)((0.U(1.W)))))

  val sIdle :: sRead :: sWrite :: Nil = Enum(3)
  val stateReg = RegInit(VecInit(Seq.fill(cnt)(sIdle)))

  cntReg := Mux(cntReg === (period - 1).U, 0.U, cntReg + 1.U)

  // Generater the slot Table for the whole period
  def slotTable(i: Int): UInt = {
    (cntReg === (i*slotLen).U).asUInt
  }

  for(i <- 0 until cnt) {
    cpuSlot(i) := slotTable(i)
  }

  // Initialize data to zero when cpuSlot is not enabled
  io.slave.M.Addr       := 0.U
  io.slave.M.Cmd        := 0.U
  io.slave.M.DataByteEn := 0.U
  io.slave.M.DataValid  := 0.U
  io.slave.M.Data       := 0.U
  
  // Initialize slave data to zero
  for (i <- 0 to cnt - 1) {
    io.master(i).S.CmdAccept := 0.U
    io.master(i).S.DataAccept := 0.U
    io.master(i).S.Resp := OcpResp.NULL
    io.master(i).S.Data := 0.U
  }
    
  // Temporarily assigned to master 0
  //val masterIdReg = Reg(init = 0.U(log2Up(cnt).W))

    for (i <- 0 to cnt-1) {

      when (stateReg(i) === sIdle) {
        when (cpuSlot(i) === 1.U) {
          
          when (io.master(i).M.Cmd =/= OcpCmd.IDLE){
            when (io.master(i).M.Cmd === OcpCmd.RD) {
              io.slave.M := io.master(i).M
              io.master(i).S := io.slave.S
              stateReg(i) := sRead
            }
            when (io.master(i).M.Cmd === OcpCmd.WR) {
              io.slave.M := io.master(i).M
              io.master(i).S := io.slave.S
              stateReg(i) := sWrite
              burstCntReg := 0.U
            }
          }
        }
      }

       when (stateReg(i) === sWrite){
         io.slave.M := io.master(i).M
         io.master(i).S := io.slave.S 
         // Wait on DVA
         when(io.slave.S.Resp === OcpResp.DVA){
           stateReg(i) := sIdle
         }
       }

       when (stateReg(i) === sRead){
         io.slave.M := io.master(i).M
         io.master(i).S := io.slave.S
         when (io.slave.S.Resp === OcpResp.DVA) {
           burstCntReg := burstCntReg + 1.U
             when (burstCntReg === burstLen.U - 1.U) {
               stateReg := sIdle
             }
           }
        }
    }
}


