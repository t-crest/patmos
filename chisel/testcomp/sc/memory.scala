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
 * Main memory model
 * 
 * Author: Sahar Abbaspour (sabb@dtu.dk)
 * 
 */


package patmos

import Chisel._
import Node._


import ocp._
import Constants._ 
import scala.math


class MainMem(mm_size: Int, burstLen : Int) extends Component {
    val io = new Bundle {
    val mmInOut = new OcpBurstSlavePort(ADDR_WIDTH, DATA_WIDTH, burstLen) // slave to cache

  }
    
    val mem 			= Mem(mm_size, seqRead = true) {Bits(width = DATA_WIDTH)}	
	val init			= Reg(resetVal = UFix(1, 1))
	
	val cmd 			= Reg(resetVal = Bits(0, 3))
	val valid			= Reg(resetVal = Bits(0, 1))
	
	when (init === UFix(1)) { // initialize memory, for simulation

		
		mem(UFix(3071)) := UFix(1020)
		mem(UFix(3067)) := UFix(1016)
		mem(UFix(3063)) := UFix(1012)
		mem(UFix(3059)) := UFix(1008)
		mem(UFix(3055)) := UFix(1004)
		mem(UFix(3051)) := UFix(1000)
		mem(UFix(3047)) := UFix(996)
		mem(UFix(3043)) := UFix(992)
		mem(UFix(3039)) := UFix(988)
		mem(UFix(3035)) := UFix(984)

		
	  	init := UFix(0)
  	}
	
	val init_st :: wr_st :: rd_st :: wr_wait_st :: rd_wait_st :: Nil  = Enum(5){ UFix() } 
	val state 		= Reg(resetVal = init_st)
	val mem_delay = Reg(resetVal = UFix(0, 1))
    val mem_delay_cnt = Reg(resetVal = UFix(0, 3))
	val burst_count = Reg(resetVal = UFix(0, log2Up(burstLen)))
    val burst_start = Reg(resetVal = UFix(0, 1))
    
    val rd_addr = Reg(resetVal = Bits(0, ADDR_WIDTH))
    
	//defaults for slave signals
	io.mmInOut.S.Resp := OcpResp.NULL
	io.mmInOut.S.Data := UFix(0)
	io.mmInOut.S.DataAccept := UFix(0) // not accepting 
	io.mmInOut.S.CmdAccept := UFix(0)
   
	cmd := io.mmInOut.M.Cmd
	valid := io.mmInOut.M.DataValid 
	
	when (mem_delay === UFix(1)) {
    	mem_delay_cnt := mem_delay_cnt + UFix(1)
    }
	
	when (state === init_st) {
		io.mmInOut.S.Resp := OcpResp.NULL
		io.mmInOut.S.Data := UFix(0)
		io.mmInOut.S.DataAccept := UFix(0) // not accepting 
		mem_delay_cnt := UFix(0)
		when (cmd === OcpCmd.WR ) {
			state := wr_wait_st
			mem_delay := UFix(1)
		}
		when (cmd === OcpCmd.RD) {
			state := rd_wait_st
			mem_delay := UFix(1)
		}
	}
	
	when (state === wr_wait_st) {
	  
		
		when (mem_delay_cnt === UFix(7)) {
				state := wr_st
				
				io.mmInOut.S.DataAccept := UFix(1)
				io.mmInOut.S.CmdAccept := UFix(1)
				//mem_delay_cnt := UFix(0)
		}
	}
	
	when (state === rd_wait_st) {
		
		io.mmInOut.S.Data := mem(io.mmInOut.M.Addr) // read the first data
		
		when (mem_delay_cnt === UFix(6)) {
			io.mmInOut.S.CmdAccept := UFix(1)
			rd_addr := io.mmInOut.M.Addr // register the address since master will remove it later
		}
		when (mem_delay_cnt === UFix(7)) {
				io.mmInOut.S.Data := mem(rd_addr)
				rd_addr := rd_addr + UFix(4)
				io.mmInOut.S.Resp := OcpResp.DVA
				io.mmInOut.S.CmdAccept := UFix(0)
				io.mmInOut.S.DataAccept := UFix(0)
				io.mmInOut.S.CmdAccept := UFix(0)
				state := rd_st	
		}
	}
	
	when (state === wr_st) {
		io.mmInOut.S.DataAccept := UFix(1)
		burst_count := burst_count + UFix(1)
		when (burst_count === UFix(burstLen - 1)) { 
			io.mmInOut.S.Resp := OcpResp.DVA
			burst_count := UFix(0)
			burst_start := UFix(1)
			mem_delay_cnt := UFix(0)
			mem_delay := UFix(1)
			io.mmInOut.S.DataAccept := UFix(0)
			state := init_st
		}
	}
	
	when (state === rd_st) {
		rd_addr := rd_addr + UFix(4) // slaves increments the address itself
		io.mmInOut.S.Resp := OcpResp.DVA
		io.mmInOut.S.Data := mem(rd_addr)
		io.mmInOut.S.DataAccept := UFix(0)
		burst_count := burst_count + UFix(1)
		when (burst_count === UFix(burstLen - 2)) { 
			io.mmInOut.S.Resp := OcpResp.DVA
			burst_count := UFix(0)
			burst_start := UFix(1)
			mem_delay_cnt := UFix(0)
			mem_delay := UFix(1)
			io.mmInOut.S.DataAccept := UFix(0)
			state := init_st
		}
	}
	
 
    
    
}
  
//object stackMain {
//  def main(args: Array[String]): Unit = { 
//    chiselMain( args, () => new MainMemory())
//  }
//}



