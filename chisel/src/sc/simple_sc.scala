/*
   Copyright 2013 Technical University of Denmark, DTU Compute. 
   All rights reserved.
   
   This file is part of the time-predictable VLIW processor Patmos.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

      1. Redistributions of source code must retain the above copyright notice,
         this list of conditions and the following disclaimer.

      2. Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
   NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   The views and conclusions contained in the software and documentation are
   those of the authors and should not be interpreted as representing official
   policies, either expressed or implied, of the copyright holder.
 */

/*
 * Stack cache memory 
 * 
 * Author: Sahar Abbaspour (sabb@dtu.dk)
 * 
 */


package patmos

import Chisel._
import Node._



import scala.math

import ocp._
import Constants._ 

class StackCache(sc_size: Int, mem_size: Int, burstLen : Int) extends Component {
val io = new Bundle {
    val scCpuInOut = new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH) // slave to cpu
    val scMemInOut = new OcpBurstMasterPort(ADDR_WIDTH, DATA_WIDTH, burstLen) // master to memory
    
    val spill 		= UFix(INPUT, 1)
	val fill 		= UFix(INPUT, 1)
	val free 		= UFix(INPUT, 1) 
	val sc_top 		= UFix(INPUT, width = 32) 
	val m_top		= UFix(OUTPUT, width = 32)
	val n_spill		= UFix(INPUT, log2Up(sc_size))
    val n_fill		= UFix(INPUT, log2Up(sc_size))
    val stall		= UFix(OUTPUT, 1)
  }

    val sc0 = { Mem(sc_size / BYTES_PER_WORD, seqRead = true) { Bits(width = BYTE_WIDTH) } }
    val sc1 = { Mem(sc_size / BYTES_PER_WORD, seqRead = true) { Bits(width = BYTE_WIDTH) } }
    val sc2 = { Mem(sc_size / BYTES_PER_WORD, seqRead = true) { Bits(width = BYTE_WIDTH) } }
    val sc3 = { Mem(sc_size / BYTES_PER_WORD, seqRead = true) { Bits(width = BYTE_WIDTH) } }
  	
    val init_st :: spill_st :: fill_st :: free_st :: spill_wait_st :: fill_wait_st :: Nil  = Enum(6){ UFix() } 
	val state 		= Reg(resetVal = init_st)
	val n_spill 	= Reg(resetVal = Fix(0, width = log2Up(sc_size)))
	val n_fill 		= Reg(resetVal = Fix(0, width = log2Up(sc_size)))
	val m_top 		= Reg(resetVal = UFix(512, width = 32))
	val spill = Reg(resetVal = UFix(0, 1))
	val fill = Reg(resetVal = UFix(0, 1))
	val fill_en = Mux(state === fill_st, Bits("b1111"),  Bits("b0000"))
    
	val addrBits = log2Up(mem_size)
    val sc_en = Mux(io.scCpuInOut.M.Cmd === OcpCmd.WR, io.scCpuInOut.M.ByteEn,  Bits("b0000"))
	
    val SC_MASK		= UFix(255)
    
    val cpu_addr_masked = io.scCpuInOut.M.Addr & SC_MASK
    val mem_addr_masked = (m_top - UFix(1)) & SC_MASK
    
    val burst_count = Reg(resetVal = Fix(3, 3))
    
	when(sc_en(0)) { sc0(cpu_addr_masked ) := io.scCpuInOut.M.Data(BYTE_WIDTH-1, 0) }
	when(sc_en(1)) { sc1(cpu_addr_masked ) := io.scCpuInOut.M.Data(2*BYTE_WIDTH-1, BYTE_WIDTH) }
	when(sc_en(2)) { sc2(cpu_addr_masked ) := io.scCpuInOut.M.Data(3*BYTE_WIDTH-1, 2*BYTE_WIDTH) }
	when(sc_en(3)) { sc3(cpu_addr_masked ) := io.scCpuInOut.M.Data(DATA_WIDTH-1, 3*BYTE_WIDTH) }
	

	
	
	val rdDataSpill = Cat(sc3(mem_addr_masked), 
			  			sc2(mem_addr_masked), 
			  			sc1(mem_addr_masked),
			  			sc0(mem_addr_masked))
			  			
    // load
    val rdData = Cat(sc3(cpu_addr_masked),
		  			sc2(cpu_addr_masked),
		  			sc1(cpu_addr_masked),
		  			sc0(cpu_addr_masked ))


	io.scMemInOut.M.Data := rdDataSpill  			
	io.scCpuInOut.S.Data := rdData
	io.scCpuInOut.S.Resp := Mux(io.scCpuInOut.M.Cmd === OcpCmd.WRNP || io.scCpuInOut.M.Cmd === OcpCmd.RD, OcpResp.DVA, OcpResp.NULL)
	
	io.scMemInOut.M.Cmd := OcpCmd.IDLE
	io.scMemInOut.M.Addr := UFix(0)
	io.scMemInOut.M.Data := Bits(0)
	io.scMemInOut.M.DataByteEn := Bits("b0000")
	io.scMemInOut.M.DataValid := Bits(0)
	
	
	val mem_addr_masked_reg = Reg(resetVal = Bits(0, width = log2Up(sc_size)))
	mem_addr_masked_reg := mem_addr_masked
	
	val mem_addr_reg = Reg(resetVal = Bits(0, width = ADDR_WIDTH))
	mem_addr_reg := m_top - UFix(1)
	

	io.stall		:= UFix(0)
	
	when (state === init_st){
	  // spill
	  when (io.spill === UFix(1)){
	    spill := UFix(1)
	    n_spill := io.n_spill
	    io.stall	:= UFix(1)
	    state := spill_st
	    io.scMemInOut.M.Cmd	:= OcpCmd.WRNP
    	io.scMemInOut.M.Addr := mem_addr_reg  
    	io.scMemInOut.M.Data := rdDataSpill
	  } 
	  // fill
	  .elsewhen (io.fill === UFix(1)){
	    fill := UFix(1)
	    state := fill_st
	    io.stall := UFix(1)
	    io.scMemInOut.M.Cmd	:= OcpCmd.RD
	    io.scMemInOut.M.Addr := mem_addr_reg
	  }
	  // free
	  .elsewhen (io.free === UFix(1)){
	    state 		:= free_st
	    io.stall	:= UFix(0)
	  }
	}
	
	when (state === spill_wait_st) { // wait for the slave to response
		spill := UFix(1)
	    io.stall	:= UFix(1)
	    io.scMemInOut.M.Cmd	:= OcpCmd.WRNP
    	io.scMemInOut.M.Addr := mem_addr_reg
    	io.scMemInOut.M.Data := rdDataSpill
    	io.scMemInOut.M.DataValid := Bits(1)
		when (io.scMemInOut.S.DataAccept === Bits(1) ) {
			state := spill_st
			m_top  		:= m_top - UFix(1)
			n_spill 	:= n_spill - UFix(1)
		}
		
	}
	
	when (state === fill_wait_st) { // 
		fill := UFix(1)
	    io.stall	:= UFix(1)
	    io.scMemInOut.M.Cmd	:= OcpCmd.RD
    	io.scMemInOut.M.Addr := m_top // mem_addr_reg + UFix(1) // m_top
    	io.scMemInOut.M.DataValid := Bits(0)
	    when (io.scMemInOut.S.Resp === OcpResp.DVA) {
			m_top  		:= m_top + UFix(1)
			n_fill 	:= n_fill - UFix(1)
			state := fill_st
		}
	}
	
	when (state === spill_st){
		io.stall := UFix(1)
	  	io.scMemInOut.M.Cmd	:= OcpCmd.IDLE
    	io.scMemInOut.M.Addr := mem_addr_reg
    	io.scMemInOut.M.Data := rdDataSpill
	  when ( io.scMemInOut.S.DataAccept === Bits(0)) { // wait for the acc signal from slave
	    	state 		:= spill_wait_st  	
	    }
	  .elsewhen (io.scMemInOut.S.DataAccept === Bits(1)){ //start transfer
		  when (n_spill - Fix(burstLen) >= UFix(0)) {io.scMemInOut.M.DataByteEn := Bits("b1111")} 
		  .elsewhen (n_spill - UFix(burstLen - 1) === UFix(0)) {io.scMemInOut.M.DataByteEn := Bits("b1110")} 
		  .elsewhen (n_spill - UFix(burstLen - 2) === UFix(0)) {io.scMemInOut.M.DataByteEn := Bits("b1100")}
		  .elsewhen (n_spill - UFix(burstLen - 3) === UFix(0)) {io.scMemInOut.M.DataByteEn := Bits("b1000")}
		  when ((n_spill - Fix(1)) >= Fix(0)){
		    m_top  		:= m_top - UFix(1)
		    n_spill 	:= n_spill - UFix(1)
		    io.stall 	:= UFix(1)
		    io.scMemInOut.M.Addr := m_top - UFix(1)
		   
		       
		    io.scMemInOut.M.Cmd := OcpCmd.IDLE
		    io.scMemInOut.M.DataValid := Bits(1)
		    io.scMemInOut.M.Data := rdDataSpill
		  }
		  
		  .otherwise {
			io.scMemInOut.M.Cmd := OcpCmd.IDLE
		    state 		:= init_st
		    io.stall 	:= UFix(0)
		  }
	   }
	}
	
	when (state === fill_st){
	  when ( io.scMemInOut.S.Resp === OcpResp.DVA && (burst_count - Fix(1)) >= Fix(0)) { // 
		  burst_count := burst_count - Fix(1)
		  io.stall := UFix(1)
		  io.scMemInOut.M.Cmd	:= OcpCmd.IDLE
		  when (fill_en(0)) {sc0(mem_addr_masked) := io.scMemInOut.S.Data(BYTE_WIDTH-1, 0)}
		  when (fill_en(1)) {sc1(mem_addr_masked) := io.scMemInOut.S.Data(2*BYTE_WIDTH-1, BYTE_WIDTH)}
		  when (fill_en(2)) {sc2(mem_addr_masked) := io.scMemInOut.S.Data(3*BYTE_WIDTH-1, 2*BYTE_WIDTH)}
		  when (fill_en(3)) {sc3(mem_addr_masked) := io.scMemInOut.S.Data(DATA_WIDTH-1, 3*BYTE_WIDTH)}
		  
		  when (n_fill - Fix(1) >= Fix(0)){
		    m_top 		:= m_top + UFix(1)
		    n_fill 		:= n_fill - UFix(1)
		    io.stall 	:= UFix(1)
		    io.scMemInOut.M.Cmd := OcpCmd.IDLE
		  }
		  .otherwise {
		    io.scMemInOut.M.Cmd := OcpCmd.IDLE
		    state 		:= init_st
		    io.stall 	:= UFix(0)
		  }
	  }
	  .otherwise {
		  burst_count := UFix(3)
		  state 		:= fill_wait_st  	
	  }
	  
	  
	}
	
	when (state === free_st){
	  when (io.sc_top > m_top){
	    m_top 		:= io.sc_top
	  }
	   io.stall 	:= UFix(0)
	   when (io.fill === UFix(1)) {
		  when ( io.scMemInOut.S.Resp === OcpResp.DVA) {
	    	state 		:= fill_st 
	    	n_fill		:= io.n_fill
	    	io.scMemInOut.M.Cmd	:= OcpCmd.RD
		  }
	     .otherwise { state := fill_wait_st}
	     
		  n_fill		:= io.n_fill
		  io.stall := UFix(1)
	   }
	   .elsewhen (io.spill === UFix(1)) { 
		   when (io.scMemInOut.S.DataAccept === Bits(1)) {
			   state := spill_st
			   n_spill := io.n_spill
			   io.stall := UFix(1)
		   }
		   .otherwise { state := spill_wait_st}
		}
	   .otherwise {state := init_st}
	}
	
	io.m_top		:= m_top
	
}
  

