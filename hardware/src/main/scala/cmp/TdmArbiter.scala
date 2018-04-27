/*
 * Copyright: 2018, Technical University of Denmark, DTU Compute
 * Author: Oktay Baris
 * License: Simplified BSD License
 *
 * A TDM arbiter to access on-chip SPM.
 * 
 *
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

class SpmBlock(MemSize : Int) extends Module {
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

class TdmArbiter(cnt:Int) extends Module {
  val io = new Bundle {
	val slave = Vec.fill(cnt) { new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH) } /*core=master for OCP*/
	val master = new OcpCoreMasterPort(ADDR_WIDTH, DATA_WIDTH)
  }

  // Counter Register
  val slotLen = 1 // 1 cc
  val period = cnt * slotLen
  val scheduleReg = Reg(init = UInt(0, log2Up(period)))

  scheduleReg := Mux(scheduleReg === UInt(period - 1), UInt(0), scheduleReg + UInt(1))

  // Memory 
  val spmBlock = Module(new SpmBlock(1024))

// Registers for buffering arbiter inputs
 /*val bufferBundle = new Bundle{
    val regCmd = UInt(3)
    val regAddr = UInt(ADDR_WIDTH)
    val regWrData = UInt(ADDR_WIDTH)
    val regRdData = UInt(DATA_WIDTH)
    val regMemResp = UInt(1) 
    val regResp = UInt(2)
 }*/

 //val inBuffer = Reg(Vec(cnt,bufferBundle))

    val regCmd = Vec.fill(cnt) { Reg(UInt(3)) }
    val regAddr = Vec.fill(cnt) { Reg(UInt(ADDR_WIDTH)) }
    val regWrData = Vec.fill(cnt) { Reg(UInt(ADDR_WIDTH)) }
    val regRdData = Vec.fill(cnt) { Reg(UInt(DATA_WIDTH)) }
    val regMemResp = Vec.fill(cnt) { Reg(UInt(1)) }
    val regResp = Vec.fill(cnt) { Reg(UInt(2)) }

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
    regMemResp(i) := UInt(0)
    regResp(i) := regMemResp(i)

            when(scheduleReg === UInt(i)){ 
                //write
                when (regCmd(i) === UInt(1)) {
                    spmBlock.io.Addr := regAddr(i) 
                    spmBlock.io.wrData:= regWrData(i)
                    spmBlock.io.Cmd := regCmd(i)
                    regCmd(i) :=UInt(0) // return back to idle state
	                  regMemResp(i):=UInt(1) 
                //read
                }.elsewhen (regCmd(i) ===UInt(2) ) {
                    spmBlock.io.Addr:= regAddr(i) 
                    spmBlock.io.Cmd:= regCmd(i) 
                    regCmd(i) :=UInt(0) // return back to idle state
                    regMemResp(i) :=UInt(1)               
                }
            }
                // FSM for buffering inputs
                when(io.slave(i).M.Cmd === OcpCmd.WR){
                //write
                    regCmd(i) := io.slave(i).M.Cmd
                    regAddr(i) := io.slave(i).M.Addr
                    regWrData(i) := io.slave(i).M.Data
              
                }.elsewhen(io.slave(i).M.Cmd === OcpCmd.RD){
                //read
                    regCmd(i) := io.slave(i).M.Cmd
                    regAddr(i) := io.slave(i).M.Addr
                }

           
           
          regRdData(i) := spmBlock.io.rdData
           
          //Connecting the read data to all requesters
          io.slave(i).S.Data := regRdData(i)
            
          //Acknowledment to the requester
          io.slave(i).S.Resp:= regResp(i)
  }
}

object TdmArbiterMain {
  def main(args: Array[String]): Unit = {

    val chiselArgs = args.slice(1, args.length)
    val cnt = args(0)

    chiselMain(chiselArgs, () => Module(new TdmArbiter(cnt.toInt)))
  }
}
