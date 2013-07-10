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
import scala.math

class Test_dc2way() extends Component {
  val io = new Bundle {
	val led = UFix(OUTPUT, 1) 
  }
  
	val dc_2way 	= new DC_2_way(2, 1024, 32)
	val mem 			= Mem(150000) {Bits(width = 32)}
	
	val init			= Reg(resetVal = UFix(1, 1))
	
	when (init === UFix(1)) { // initialize memory, for simulation
		mem(Bits(110)) := Bits(120)
		
		mem(Bits(16484)) := Bits(130)
	  	init := UFix(0)
  	}

	

	val wr = Reg(resetVal = UFix(0, 1))
  	val rd = Reg(resetVal = UFix(0, 1))

 	val data_in = Reg(resetVal = Bits(0, 32))
  	val address = Reg(resetVal = Bits(0, 32))
  	val idle :: read_hit :: read_miss :: read1_0 :: read1_1 :: read2_0 :: read2_1 :: write1_0 :: write1_1 :: write_hit :: write_miss  :: wait_st :: wait_st2 :: wait_st3 :: read_way0 :: no_state :: Nil  = Enum(16){ UFix() } 
	val state = Reg(resetVal = idle)

	val data_dout = Reg() { Bits() }
	data_dout	:= mem(address)
	
	when (state === idle) {
		wr			:= UFix(0)
		address		:= Bits(0)
		data_in		:= Bits(0)
		state 		:= write1_0
	}
	
	when (state === write1_0) { //write to way0
		wr			:= UFix(1)
		address		:= Bits(100)
		data_in		:= Bits(100)
		state 		:= write1_1
	}
	
	when (state === write1_1) {
		wr			:= UFix(1)
		address		:= Bits(16484) // same index as address 100
		data_in		:= Bits(101)
		state 		:= read1_0
	}
	
	when (state === read1_0) { // read way0
		rd			:= UFix(1)
		wr			:= UFix(0)
		address		:= Bits(100)
		state 		:= read1_1
	}
	
	when (state === read1_1) { // read way1
		rd			:= UFix(1)
		wr			:= UFix(0)
		address		:= Bits(16484)
		state 		:= write_miss
	}
	
	
	when (state === write_miss) { 
		wr			:= UFix(1)
		rd			:= UFix(0)
		address		:= Bits(49252) // same index as address 100, different tag, way0 is replaced since last accessed is way1
		data_in		:= Bits(110)
		state 		:= read2_1
	}
	
	when (state === read2_1) { // read way1, to check it is still there?
		rd			:= UFix(1)
		wr			:= UFix(0)
		address		:= Bits(16484)
		state 		:= read2_0
	}

	when (state === read2_0) {// read way0 for new value, ie. 110, 
		rd			:= UFix(1)
		wr			:= UFix(0)
		address		:= Bits(49252)
		state 		:= wait_st
	}
	
	when (state === wait_st) {
		rd			:= UFix(0)
		wr			:= UFix(0)
		state 		:= read_miss
	}

	when (state === read_miss) {// 
		rd			:= UFix(1)
		wr			:= UFix(0)
		address		:= Bits(110)
		state 		:= wait_st2
	}
	
	when (state === wait_st2) {
		rd			:= UFix(0)
		wr			:= UFix(0)
		state 		:= read_hit
	}
	
	when (state === read_hit) { // should read 120
		rd			:= UFix(1)
		wr			:= UFix(0)
		address		:= Bits(110)
		state 		:= write_hit 
	}
	
	when (state === write_hit) { // way1 is lru, replace it
		rd			:= UFix(0)
		wr			:= UFix(1)
		address		:= Bits(114788)
		data_in		:= Bits(150)
		state 		:= wait_st3
	}
	
	when (state === wait_st3) {
		rd			:= UFix(0)
		wr			:= UFix(0)
		state 		:= read_way0
	}
	
	when (state === read_way0) { //read way0
		rd			:= UFix(1)
		wr			:= UFix(0)
		address		:= Bits(49252) // read 110 from this address, it is same as before
		state 		:= no_state
	}
	
	//:= mem(address)
	io.led 					:= UFix(1)
	dc_2way.io.address		:= address
	dc_2way.io.mem_data_in	:= mem(address)
//	mem(address)			:= dc_2way.io.mem_data_out
	dc_2way.io.wr			:= wr
	dc_2way.io.rd			:= rd
	dc_2way.io.data_in		:= data_in
	
}
  



// Generate the Verlog code by invoking chiselMain() in our main()
object HelloMain2 {
  def main(args: Array[String]): Unit = { 
    chiselMain( args, () => new Test_dc2way())
  }
}


