/*
 * Arbiter for OCP burst slaves.
 * Pseudo round robin arbitration. Each turn for a non-requesting master costs 1 clock cycle.
 *
 * Author: Martin Schoeberl (martin@jopdesign.com) David Chong (davidchong99@gmail.com)
 *
 */

package ocp

import Chisel._

class TdmArbiter(cnt: Int, addrWidth : Int, dataWidth : Int, burstLen : Int) extends ArbiterType(cnt, dataWidth, dataWidth, burstLen) {

  val cntReg = Reg(init = UInt(0, log2Up(cnt*(burstLen + 2))))
  // slot length = burst size + 2 
  val burstCntReg = Reg(init = UInt(0, log2Up(burstLen)))
  val period = cnt * (burstLen + 2)
  val slotLen = burstLen + 2
  val cpuSlot = RegInit(Vec.fill(cnt){UInt(0, width=1)})

  val sIdle :: sRead :: sWrite :: Nil = Enum(UInt(), 3)
  val stateReg = RegInit(Vec.fill(cnt){sIdle})

  cntReg := Mux(cntReg === UInt(period - 1), UInt(0), cntReg + UInt(1))

  // Generater the slot Table for the whole period
  def slotTable(i: Int): UInt = {
    (cntReg === UInt(i*slotLen)).asUInt
  }

  for(i <- 0 until cnt) {
    cpuSlot(i) := slotTable(i)
  }

  // Initialize data to zero when cpuSlot is not enabled
  io.slave.M.Addr       := UInt(0)
  io.slave.M.Cmd        := UInt(0)
  io.slave.M.DataByteEn := UInt(0)
  io.slave.M.DataValid  := UInt(0)
  io.slave.M.Data       := UInt(0)
  
  // Initialize slave data to zero
  for (i <- 0 to cnt - 1) {
    io.master(i).S.CmdAccept := UInt(0)
    io.master(i).S.DataAccept := UInt(0)
    io.master(i).S.Resp := OcpResp.NULL
    io.master(i).S.Data := UInt(0) 
  }
    
  // Temporarily assigned to master 0
  //val masterIdReg = Reg(init = UInt(0, log2Up(cnt)))

    for (i <- 0 to cnt-1) {

      when (stateReg(i) === sIdle) {
        when (cpuSlot(i) === UInt(1)) {
          
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
              burstCntReg := UInt(0)
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
           burstCntReg := burstCntReg + UInt(1)
             when (burstCntReg === UInt(burstLen) - UInt(1)) {
               stateReg := sIdle
             }
           }
        }
    }
}


