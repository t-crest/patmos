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
 * Data cache test
 * 
 * Author: Sahar Abbaspour (sabb@dtu.dk)
 * 
 */

package dc

import Chisel._
import Node._


class Test_dc() extends Component {
  val io = new Bundle {
	val led = Bits(OUTPUT, 1) 
  }
  
	val dc	 		= new DC_1_way(1, 1024, 32, 5)
		val mem 			= Mem(150000) {Bits(width = 32)}
	
	val init			= Reg(resetVal = UFix(1, 1))
	
	when (init === UFix(1)) { // initialize memory, for simulation
		mem(Bits(110)) := Bits(120)
		
		mem(Bits(16484)) := Bits(130)
		
		mem(Bits(154)) := Bits(154)
		mem(Bits(158)) := Bits(158)
		mem(Bits(162)) := Bits(162)
		mem(Bits(166)) := Bits(166)
		mem(Bits(170)) := Bits(170)
		mem(Bits(174)) := Bits(174)
		
	  	init := UFix(0)
  	}
	

	val wr = Reg(resetVal = UFix(0, 1))
  	val rd = Reg(resetVal = UFix(0, 1))
 	val data_in = Reg(resetVal = Bits(0, 32))
  	val address = Reg(resetVal = Bits(0, 32))
  	val idle :: read :: read_miss :: read_hit :: write1 :: write2 :: write_hit:: wait_st :: read1 :: Nil  = Enum(9){ UFix() } 
	val state = Reg(resetVal = idle)
	

	when (state === idle) {
		wr			:= UFix(0)
		address		:= UFix(0)
		data_in		:= Bits(0)
		state 		:= write1
	}
	
	when (state === write1) {
		wr			:= UFix(1)
		address		:= UFix(100)
		data_in		:= Bits(100)
		state 		:= write2
	}
	
	when (state === write2) {
		wr			:= UFix(1)
		address		:= UFix(110)
		data_in		:= Bits(110)
		state 		:= read_hit
	}
	when (state === read_hit) {
		rd			:= UFix(1)
		wr			:= UFix(0)
		address		:= UFix(100)
		state 		:= wait_st
	}
	
	when (state === wait_st) {
		rd			:= UFix(0)
		wr			:= UFix(0)
		state 		:= read_miss
	}
	when (state === read_miss) {
		rd			:= UFix(1) // rd goes low after one clock?
		address		:= UFix(150)
		when (dc.io.stall === UFix(0)) {
			state 		:= read1
		}
	}
	
	when (state === read1) {
		rd			:= UFix(1)
		address		:= UFix(162)
		state 		:= write_hit
	}
	

	io.led 				:= UFix(1)
	dc.io.address		:= address
	dc.io.mem_data_in	:= mem(address)
	dc.io.wr			:= wr
	dc.io.rd			:= rd
	dc.io.data_in		:= data_in

}
  



// Generate the Verlog code by invoking chiselMain() in our main()
object HelloMain {
  def main(args: Array[String]): Unit = { 
    chiselMain( args, () => new Test_dc())
  }
}


