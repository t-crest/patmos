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
 * Stack cache test
 * 
 * Author: Sahar Abbaspour (sabb@dtu.dk)
 * 
 */

package sc

import Chisel._
import Node._


class Test() extends Component {
  val io = new Bundle {
	val led = UFix(OUTPUT, 1) 
  }
  
  	val func_gen	= Reg(resetVal = UFix(0, 5))
  	
	val sc_ex 		= new SC_ex(256)
	val sc_mem 		= new SC_mem(256)
	val sc			= new SC(256, 1024)
  	
  	
  	
	sc_ex.io.spill 	<> sc_mem.io.spill
	sc_ex.io.fill 	<> sc_mem.io.fill
	sc_ex.io.m_top  <> sc_mem.io.m_top
	sc_ex.io.predicate := UFix(1)
	sc_ex.io.n_spill <> sc_mem.io.n_spill
	sc_ex.io.n_fill <> sc_mem.io.n_fill
	sc_ex.io.stall <> sc_mem.io.stall
	sc_ex.io.sc_top <> sc_mem.io.sc_top
	sc_ex.io.free <> sc_mem.io.free
	
	val wr = Reg(resetVal = UFix(0))
	val rd = Reg(resetVal = UFix(0))
	val wr_data = Reg(resetVal = Bits(0, 32))
	val rd_data = Reg(resetVal = Bits(0, 32))
	val wr_addr = Reg(resetVal = UFix(0, 10))
	val rd_addr = Reg(resetVal = UFix(0, 10))
	
	sc.io.wr	:= wr
//	sc.io.rd	:= rd
	sc.io.wr_data	:= wr_data
	sc.io.wr_addr	:= wr_addr
	sc.io.spill	<> sc_ex.io.spill
	sc.io.fill <> sc_ex.io.fill
	sc.io.m_top  <> sc_mem.io.m_top
	sc.io.sc_top  <> sc_ex.io.sc_top
//	sc.io.rd_addr := rd_addr
	rd_data		:= sc.io.rd_data
	
	
	
	func_gen 		:= func_gen + UFix(1)
	
	sc_ex.io.sc_func_type		:= UFix(3) //none
	sc_ex.io.imm				:= UFix(256) //none
	wr := UFix(0)
	rd := UFix(0)
	wr_data := Bits(0)
	rd_data := Bits(0)
	wr_addr := UFix(0)
	rd_addr := UFix(0)
	sc.io.rd := UFix(1)
	sc.io.rd_addr := UFix(15)
	when (func_gen === UFix(1)){ // first reserve
	  sc_ex.io.sc_func_type 	:= UFix(0)
	  sc_ex.io.imm				:= UFix(256)
	}
	
	when (func_gen === UFix(2)){ // second reserve
	  sc_ex.io.sc_func_type 	:= UFix(0)
	  sc_ex.io.imm				:= UFix(10)
	}
	
	when (func_gen === UFix(3)){ // random instruction after two reserve, store to stack cache
	  sc_ex.io.sc_func_type 	:= UFix(3)
	  sc_ex.io.imm				:= UFix(10)
	  wr := UFix(1)
	  wr_addr := UFix(15)
	  wr_data := Bits(15)
	}
	
	when (func_gen >= UFix(4) && func_gen <= UFix(13)){  // wait until required number of cycles in counting for spill
	  sc_ex.io.sc_func_type 	:= UFix(3)
	  wr := UFix(0)

	}
	
	when (func_gen === UFix(14)){ // free 10 blocks // there could be some extra non-stack cycles in between
	  sc_ex.io.sc_func_type 	:= UFix(2)
	  sc_ex.io.imm				:= UFix(10)

	}
	
	when (func_gen === UFix(15)){ // ensure 256 blocks
	  sc_ex.io.sc_func_type 	:= UFix(1)
	   sc_ex.io.imm				:= UFix(256)
	} 
	
	io.led 			:= UFix(1)
	
}
  



// Generate the Verlog code by invoking chiselMain() in our main()
object HelloMain {
  def main(args: Array[String]): Unit = { 
    chiselMain( args, () => new Test())
  }
}

