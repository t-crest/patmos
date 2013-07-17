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
 * Stack cache memory stage
 * 
 * Author: Sahar Abbaspour (sabb@dtu.dk)
 * 
 */


package sc

import Chisel._
import Node._



import scala.math


class SC_mem(sc_size: Int) extends Component {
    val io = new Bundle {
    val spill 		= UFix(INPUT, 1)
	val fill 		= UFix(INPUT, 1)
	val free 		= UFix(INPUT, 1) 
	val sc_top 		= UFix(INPUT, width = 32) 
	val m_top		= UFix(OUTPUT, width = 32)
	val n_spill		= UFix(INPUT, log2Up(sc_size))
    val n_fill		= UFix(INPUT, log2Up(sc_size))
    val stall		= UFix(OUTPUT, 1)
  }

  	
  	val init_st :: spill_st :: fill_st :: free_st :: Nil  = Enum(4){ UFix() } 
	val state 		= Reg(resetVal = init_st)
	val n_spill 	= Reg(resetVal = Fix(0, width = log2Up(sc_size)))
	val n_fill 		= Reg(resetVal = Fix(0, width = log2Up(sc_size)))
	val m_top 		= Reg(resetVal = UFix(500, width = 32))
	val gm_spill 	= Reg(resetVal = UFix(0, 1))
	val sc_fill 	= Reg(resetVal = UFix(0, 1))
//	val stall 		= Reg(resetVal = UFix(0, 1))
	val sc_mask		= UFix(255)

	io.stall		:= UFix(0) 
	
	when (state === init_st){
	  when (io.spill === UFix(1)){
	    state 		:= spill_st
	    n_spill		:= io.n_spill
	    io.stall	:= UFix(1)
	  } 
	  .elsewhen (io.fill === UFix(1)){
	    state 		:= fill_st
	    n_fill		:= io.n_fill
	    io.stall	:= UFix(1)
	  }
	  .elsewhen (io.free === UFix(1)){
	    state 		:= free_st
	    io.stall	:= UFix(0)
	  }
	}
	
	when (state === spill_st){
	  when ((n_spill - Fix(1)) >= Fix(0)){
	    m_top  		:= m_top - UFix(1);
	    n_spill 	:= n_spill - UFix(1)
	    gm_spill 	:= UFix(1)
	    io.stall 	:= UFix(1)
	  }
	  
	  .otherwise{
	    gm_spill    := UFix(0);
	    state 		:= init_st
	    io.stall 	:= UFix(0)
	  }
	}
	
	when (state === fill_st){
	  when (n_fill - Fix(1) >= Fix(0)){
	    m_top 		:= m_top + UFix(1)
	    n_fill 		:= n_fill - UFix(1)
	    sc_fill 	:= UFix(1)
	    io.stall 	:= UFix(1)
	  }
	  .otherwise {
	    sc_fill 	:= UFix(0)
	    state 		:= init_st
	    io.stall 	:= UFix(0)
	  }
	}
	
	when (state === free_st){
	  when (io.sc_top > m_top){
	    m_top 		:= io.sc_top
	  }
	   io.stall 	:= UFix(0)
	}
	
	io.m_top		:= m_top
	
	// data exchange
//	mem(m_top) 		:= sc(m_top & sc_mask)

	
}
  


