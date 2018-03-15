/*
 * Copyright: 2015, Technical University of Denmark, DTU Compute
 * Author: Oktay Baris
 * License: Simplified BSD License
 */
package cmp

import Chisel._
import Node._

import patmos._
import patmos.Constants._

import ocp._

class SpmBlockPort(MemSize : Int) extends Bundle {
  val Addr   = UInt(INPUT, ADDR_WIDTH)
  val wrData = UInt(INPUT, ADDR_WIDTH)
  val rdData = UInt(OUTPUT, ADDR_WIDTH)
  val Cmd  = UInt(INPUT, 3)
  val Resp   = UInt(OUTPUT, 2)
 }


class spmBlock(MemSize : Int) extends Module {
  val io = new SpmBlockPort(MemSize)
  
  val mem = Mem(UInt(width = ADDR_WIDTH), MemSize)

  val rdDataReg = Reg(init = UInt(0, ADDR_WIDTH))
  
  //default values
  rdDataReg := Bits(0)
  io.Resp   := Bits(0)

      // write
      when(io.Cmd === Bits(1)) {
        mem(io.Addr) := io.wrData
        //io.Resp := UInt(1)
      }.elsewhen(io.Cmd === Bits(2)) { 
      // read
        rdDataReg := mem(io.Addr)
        //io.Resp := UInt(1)
      }
  io.rdData := rdDataReg

}


class TDM(cnt:Int) extends Module {
  val io = new Bundle {
	val slave = Vec.fill(cnt) { new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH) } /*core=master for OCP*/
	val master = new OcpCoreMasterPort(ADDR_WIDTH, DATA_WIDTH)
  }

  // Counter Register
  val slotLen = 1 // 1 cc
  val period = cnt * slotLen
  val cntReg = Reg(init = UInt(0, log2Up(slotLen)))
  cntReg := Mux(cntReg === UInt(slotLen-1), UInt(0), cntReg + UInt(1))


  val scheduleReg = Reg(init = UInt(0, log2Up(cnt)))
  scheduleReg := Mux(cntReg === UInt(slotLen-1), scheduleReg + UInt(1), scheduleReg)

//shared SPM

//val spm = Module(new Spm(1024))
 //io.master <> spm.io //OcpCoreMasterPort <> OcpCoreSlavePort

val spmBlock = Module(new spmBlock(1024))
//val spmBlock = Vec.fill(cnt) {Module(new spmBlock(1024))} // Pool of SPMs
//io.master <> spmBlock.io.slave


// Registers (buffering input)
  val CoreReg_Cmd = Vec.fill(cnt) { Reg(UInt(3)) }
  val CoreReg_Addr = Vec.fill(cnt) { Reg(UInt(ADDR_WIDTH)) }
  val CoreReg_wrData = Vec.fill(cnt) { Reg(UInt(ADDR_WIDTH)) }
  val CoreReg_rdData = Vec.fill(cnt) { Reg(UInt(DATA_WIDTH)) }
  val CoreReg_MemResp = Vec.fill(cnt) { Reg(UInt(1)) }
  val CoreReg_Resp = Vec.fill(cnt) { Reg(UInt(2)) }

//default values (idle state)

   spmBlock.io.Addr := Bits(0)
   spmBlock.io.wrData := Bits(0)
   spmBlock.io.Cmd    := Bits(0)


  // Initialize slave port data to zero
  for (i <- 0 to cnt - 1) {
    io.slave(i).S.Resp := Bits(0)
    io.slave(i).S.Data := Bits(0) 
  }
 
for (i <- 0 to cnt-1) {

      // default values
    CoreReg_MemResp(i) := UInt(0)
    CoreReg_Resp(i) := CoreReg_MemResp(i)

            when(scheduleReg === UInt(i)){ 
                //write
                when (CoreReg_Cmd(i) === UInt(1)) {
                    spmBlock.io.Addr := CoreReg_Addr(i) 
                    spmBlock.io.wrData:= CoreReg_wrData(i)
                    spmBlock.io.Cmd := CoreReg_Cmd(i)
                    CoreReg_Cmd(i) :=UInt(0) // return back to idle state
	                  CoreReg_MemResp(i):=UInt(1) 
                //read
                }.elsewhen (CoreReg_Cmd(i) ===UInt(2) ) {
                    spmBlock.io.Addr:=CoreReg_Addr(i) 
                    spmBlock.io.Cmd:=CoreReg_Cmd(i) 
                    CoreReg_Cmd(i) :=UInt(0) // return back to idle state
                    CoreReg_MemResp(i):=UInt(1)               
                }
            }
                // FSM for buffering inputs
                when(io.slave(i).M.Cmd === OcpCmd.WR){
                //write
                    CoreReg_Cmd(i) := io.slave(i).M.Cmd
                    CoreReg_Addr(i) := io.slave(i).M.Addr
                    CoreReg_wrData(i) := io.slave(i).M.Data
              
                }.elsewhen(io.slave(i).M.Cmd === OcpCmd.RD){
                //read
                    CoreReg_Cmd(i) := io.slave(i).M.Cmd
                    CoreReg_Addr(i) := io.slave(i).M.Addr
                }

           when(spmBlock.io.rdData =/= UInt(0)){
                CoreReg_rdData(i) := spmBlock.io.rdData
           }

          //Connecting the read data to all requesters
          io.slave(i).S.Data := CoreReg_rdData(i)
            
          //Acknowledment to the requester
          io.slave(i).S.Resp:= CoreReg_Resp(i)
  }
}

object TDMMain {
  def main(args: Array[String]): Unit = {

    val chiselArgs = args.slice(1, args.length)
    val cnt = args(0)

    chiselMain(chiselArgs, () => Module(new TDM(cnt.toInt)))
  }
}
