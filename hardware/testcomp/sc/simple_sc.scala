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
	val sc_top 		= UFix(INPUT, width = ADDR_WIDTH) 
	val m_top		= UFix(OUTPUT, width = ADDR_WIDTH)
	val n_spill		= UFix(INPUT, log2Up(sc_size))
    val n_fill		= UFix(INPUT, log2Up(sc_size))
    val stall		= UFix(OUTPUT, 1)
  }

    val sc0 = { Mem(sc_size, seqRead = true) { Bits(width = BYTE_WIDTH) } }
    val sc1 = { Mem(sc_size, seqRead = true) { Bits(width = BYTE_WIDTH) } }
    val sc2 = { Mem(sc_size, seqRead = true) { Bits(width = BYTE_WIDTH) } }
    val sc3 = { Mem(sc_size, seqRead = true) { Bits(width = BYTE_WIDTH) } }
  	
    val init_st :: spill_st :: fill_st :: free_st :: Nil  = Enum(4){ UFix() } 
	val state 		= Reg(resetVal = init_st)
	val n_spill 	= Reg(resetVal = Fix(0, width = log2Up(sc_size)))
	val n_fill 		= Reg(resetVal = Fix(0, width = log2Up(sc_size)))
	val m_top 		= Reg(resetVal = UFix(3075, width = ADDR_WIDTH)) // one higher than stack area
	val spill = Reg(resetVal = UFix(0, 1))
	val fill = Reg(resetVal = UFix(0, 1))
	val fill_en = Mux(state === fill_st, UFix(15),  UFix(0))
    
	val addrUFix = log2Up(mem_size)
    val sc_en = Mux(io.scCpuInOut.M.Cmd === OcpCmd.WR, io.scCpuInOut.M.ByteEn,  UFix(0))
	
    val SC_MASK		= UFix(width = ADDR_WIDTH)
    SC_MASK			:= (UFix(sc_size) << UFix(2)) - UFix(1)
    
//    val first_addr = Reg(resetVal = Bits(0, width = ADDR_WIDTH))
//    val first_cmd = Reg(resetVal = Bits(0, 3))
//    val first_data = Reg(resetVal = Bits(0, width = DATA_WIDTH))

  
    val scLdStAddr			= UFix(width = ADDR_WIDTH)
    scLdStAddr				:= io.scCpuInOut.M.Addr + io.sc_top
    val cpu_addr_masked		= UFix(width = ADDR_WIDTH)
    cpu_addr_masked			:=  (scLdStAddr & SC_MASK)(addrUFix + 1, 2)
   // val cpu_addr_masked = scLdStAddr(addrUFix + 1, 2)
    



    val spillEnCnt = Reg(resetVal = Fix(0, width = log2Up(sc_size)))
    val fillEnCnt = Reg(resetVal = Fix(0, width = log2Up(sc_size)))
    

	

	val rdData = Reg() { Bits() }
		
	val sResp = Reg(resetVal = Bits(0, 2))
	
	
	val mTopAlign = m_top - UFix(3) 
	val FillAlignCnt = UFix(burstLen) - mTopAlign(burstLen -1, log2Up(burstLen))
	val alignedFillAddr = m_top -  (FillAlignCnt << UFix(2))// alignment of m_top for fill
	
	val fillCnt	= Fix(width = ADDR_WIDTH)
	fillCnt := io.n_fill
	val alignFillCnt = Fix(width = ADDR_WIDTH)
	alignFillCnt := fillCnt + FillAlignCnt
	
	
	
	val mTopDec		= m_top - UFix(4) // --m_top
	val mTopDecAlign = mTopDec + UFix(1) 
	val alignCnt = UFix(burstLen) - mTopDec(burstLen -1, log2Up(burstLen))
	val alignedAddr = mTopDec +  (alignCnt << UFix(2))// alignment of m_top for spill
	
	val spillCnt	= Fix(width = ADDR_WIDTH)
	spillCnt := io.n_spill
	val alignSpillCnt = Fix(width = ADDR_WIDTH)
	alignSpillCnt := spillCnt + alignCnt
	
	val memAddrReg = Reg(resetVal = Bits(0, width = ADDR_WIDTH))
	memAddrReg := m_top
	
	
	val spillAddr	=	Mux(io.scMemInOut.S.DataAccept === UFix(1), (m_top - UFix(4))& SC_MASK, m_top & SC_MASK)
	val mem_addr_masked = spillAddr(addrUFix + 1, 2) // load from stack cache when spilling
	
	

	
	val ldAddress	 = Mux(spill === UFix(1) || io.spill === UFix(1), mem_addr_masked, cpu_addr_masked) 
    rdData := Cat(sc3(ldAddress),
		  			sc2(ldAddress),
		  			sc1(ldAddress),
		  			sc0(ldAddress ))
		  			
	
	io.scMemInOut.M.Data := rdData  			
	io.scCpuInOut.S.Data := rdData
	
	
	sResp := Mux(io.scCpuInOut.M.Cmd === OcpCmd.WR || io.scCpuInOut.M.Cmd === OcpCmd.RD, OcpResp.DVA, OcpResp.NULL)
	io.scCpuInOut.S.Resp := sResp
	
	io.scMemInOut.M.Cmd := OcpCmd.IDLE
	io.scMemInOut.M.Addr := UFix(0)
	io.scMemInOut.M.Data := UFix(0)
	io.scMemInOut.M.DataByteEn := UFix(0)
	io.scMemInOut.M.DataValid := UFix(0)
	

	io.stall		:= UFix(0)
	io.m_top		:= m_top
	
	when (state === init_st){
	  // spill
	  when (io.spill === UFix(1)){
	    io.stall	:= UFix(1)
	    spill := UFix(1)
	    n_spill := alignSpillCnt
    	spillEnCnt := Fix(burstLen) - spillCnt(log2Up(burstLen) - 1, 0) 
    	m_top := alignedAddr
    	state := spill_st
	  } 
	  
	  // fill
	  .elsewhen (io.fill === UFix(1)){
	    io.stall	:= UFix(1)
	    fill := UFix(1)
	    m_top := alignedFillAddr
	    n_fill	:= alignFillCnt
	    fillEnCnt := Fix(burstLen) - fillCnt(log2Up(burstLen) - 1, 0) 
	    state := fill_st
	  }
	  // free
	  .elsewhen (io.free === UFix(1)){
	    state 		:= free_st
	    io.stall	:= UFix(0)
	  }
	}
	
	
	when (state === spill_st){
	  io.stall := UFix(1)
	  
	  io.scMemInOut.M.DataByteEn := UFix(15)
	  io.scMemInOut.M.Cmd	:= OcpCmd.WR
	  io.scMemInOut.M.Addr := m_top
	  io.scMemInOut.M.Data := rdData 
	  	 
	 
	  when (io.scMemInOut.S.DataAccept === UFix(1)) { //start transfer
		 when (n_spill - Fix(2) >= Fix(0))  {
		   io.scMemInOut.M.DataValid := Mux(spillEnCnt === Fix(0), UFix(1), UFix(0))
	       m_top := m_top - UFix(4) // --m_top
	       n_spill := n_spill - UFix(1)
	       spillEnCnt := Mux(spillEnCnt === Fix(0), spillEnCnt, spillEnCnt - Fix(1))
	       state := spill_st
		}
		.otherwise {
			io.scMemInOut.M.Cmd	:= OcpCmd.IDLE
			state := init_st
			spill := UFix(0)
		}
	
		
	 }
	  
	}
	
	val fillEn	 = Mux(fillEnCnt === Fix(0), UFix(1), UFix(0))
	val stData	 = Mux(state === fill_st, io.scMemInOut.S.Data, io.scCpuInOut.M.Data) 
	val stAddr	 = Mux(fill === UFix(1) || io.fill === UFix(1), (m_top & SC_MASK)(addrUFix + 1, 2), cpu_addr_masked)
	val scEn	 = Mux(state === fill_st, fillEn, sc_en)
	
	when (state === fill_st){
	  io.stall := UFix(1)
	  io.scMemInOut.M.Cmd	:= OcpCmd.RD
	  io.scMemInOut.M.Addr	:= m_top

	  when ( io.scMemInOut.S.Resp === OcpResp.DVA) { // 
		  
		  when ((n_fill - Fix(2)) >= Fix(0)){
				io.scMemInOut.M.Cmd	:= OcpCmd.IDLE
		   		
		   		m_top  	:= m_top + UFix(4)
		   		n_fill 	:= n_fill - UFix(1)
		   		when (fillEnCnt - Fix(1) >= Fix(0)) {fillEnCnt := fillEnCnt - Fix(1)}
		   	}
		  .otherwise {
			io.scMemInOut.M.Cmd := OcpCmd.IDLE
			fill		:= UFix(0)
		    state 		:= init_st
		    io.stall 	:= UFix(0)
		   
		  }		
	  } 
	}

	
	
	when(scEn(0)) {sc0(stAddr)	:= stData(BYTE_WIDTH-1, 0) }
	when(scEn(1)) {sc1(stAddr) := stData(2*BYTE_WIDTH-1, BYTE_WIDTH)}
	when(scEn(2)) {sc2(stAddr) := stData(3*BYTE_WIDTH-1, 2*BYTE_WIDTH)}
	when(scEn(3)) {sc3(stAddr) := stData(DATA_WIDTH-1, 3*BYTE_WIDTH)}
	
	when (state === free_st){
	  when (io.sc_top > m_top){
	    m_top 		:= io.sc_top
	  }
	   io.stall 	:= UFix(0)
	   when (io.fill === UFix(1)) {
	    	io.stall	:= UFix(1)
	    			fill := UFix(1)
	    m_top := alignedFillAddr
	    n_fill	:= alignFillCnt
	    fillEnCnt := Fix(burstLen) - fillCnt(log2Up(burstLen) - 1, 0) 
	    io.scMemInOut.M.Cmd	:= OcpCmd.RD
	  io.scMemInOut.M.Addr	:= m_top
	    state := fill_st	
	   }
	   .elsewhen (io.spill === UFix(1)) { 
	
			   state := spill_st
		}
	   .otherwise {state := init_st}
	}
	
	
	
}
  


