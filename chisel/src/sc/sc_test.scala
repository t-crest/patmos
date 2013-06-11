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
  	
	val sc_ex 		= new SC_ex(8)
	val sc_mem 		= new SC_mem(8, 10)
	
	sc_ex.io.spill 	<> sc_mem.io.spill
	sc_ex.io.fill 	<> sc_mem.io.fill
	sc_ex.io.m_top  <> sc_mem.io.m_top
	sc_ex.io.predicate := UFix(1)
	sc_ex.io.n_spill <> sc_mem.io.n_spill
	sc_ex.io.n_fill <> sc_mem.io.n_fill
	sc_ex.io.stall <> sc_mem.io.stall
	
	func_gen 		:= func_gen + UFix(1)
	
	sc_ex.io.sc_func_type		:= UFix(3) //none
	sc_ex.io.imm				:= UFix(256) //none
	
	when (func_gen === UFix(1)){ // first reserve
	  sc_ex.io.sc_func_type 	:= UFix(0)
	  sc_ex.io.imm				:= UFix(256)
	}
	
	when (func_gen === UFix(2)){ // second reserve
	  sc_ex.io.sc_func_type 	:= UFix(0)
	  sc_ex.io.imm				:= UFix(10)
	}
	
	when (func_gen === UFix(3)){ // random instruction after two reserve
	  sc_ex.io.sc_func_type 	:= UFix(3)
	  sc_ex.io.imm				:= UFix(10)
	}
	
	when (func_gen >= UFix(4) && func_gen <= UFix(13)){  // wait until required number of cycles in counting for spill
	  sc_ex.io.sc_func_type 	:= UFix(3)
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

