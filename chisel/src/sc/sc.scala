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


class SC(sc_size: Int, mm_size: Int) extends Component {
    val io = new Bundle {
    val wr_data		= Bits(INPUT, width = 32) // from CPU
    val wr_addr		= UFix(INPUT, width = log2Up(mm_size)) // from CPU
    val wr			= UFix(INPUT, 1) // from CPU
    val rd_data		= Bits(INPUT, width = 32) // to CPU
    val rd_addr		= UFix(OUTPUT, log2Up(mm_size)) // from CPU    
    val rd			= UFix(INPUT, 1) // from CPU
    val spill		= UFix(INPUT, 1)
    val fill		= UFix(INPUT, 1)
    val m_top		= UFix(INPUT, width = 32)
    val sc_top 		= UFix(INPUT, width = 32)
  }

    val sc_ram		= Mem(sc_size) {Bits(width = 32)}
  	val mem 		= Mem(mm_size) {Bits(width = 32)}
	val SC_MASK		= UFix()
	SC_MASK	:= UFix(sc_size) - Bits(1)
	// data exchange
	val sc_dout =   Reg() { Bits() }
	val mm_dout =   Reg() { Bits() }
	

	val sc_wr_addr = UFix()
	val sc_rd_addr = UFix()
	val sc_wr_data = Bits()
	val sc_rd_data = Bits()
	
//	val sc_top_reg 	= Reg(resetVal = UFix(0, 1))
//	val sc_top_reg 	= Reg(resetVal = UFix(0, 1))
	
//	sc_dout		:= sc(sc_rd_addr )
	mm_dout 	:= mem(io.m_top)
	
	sc_wr_addr	:= io.wr_addr
	sc_wr_data	:= io.wr_data
	when (io.fill === UFix(1)) { 
		sc_wr_addr	:= io.sc_top
		sc_wr_data	:= mem(io.m_top) // one clock delay to read from memory?
	}
	

	
	
	sc_rd_addr	:= io.rd_addr
	when (io.spill === UFix(1)) {
		sc_rd_addr	:= io.sc_top
	}
	
	io.rd_data := Bits(0)
	when (io.wr === UFix(1) || io.fill === UFix(1) ) { sc_ram(sc_wr_addr & SC_MASK) := sc_wr_data }
	when (io.rd === UFix(1) || io.spill === UFix(1) ) { sc_dout := sc_ram(sc_rd_addr & SC_MASK )
	  io.rd_data := sc_ram(sc_rd_addr )}
	

	//main memory
	
}
  


