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
  
	val dc	 		= new DC_1_way(1, 1024, 32)
	val mem 		= Mem(2 ^ 32, seqRead = true) {Bits(width = 32)}
	
	// some initializiation of memory
	
//	mem(Bits(100)) := Bits(100)
//	mem(Bits(101)) := Bits(101)
	
	val test = Reg(resetVal = Bits(0, 32))
	test	:= mem(Bits(100))
	val wr = Reg(resetVal = UFix(0, 1))
  	val rd = Reg(resetVal = UFix(0, 1))
 //	val wr_data = Reg(resetVal = UFix(0, 8))
 	val data_in = Reg(resetVal = Bits(0, 32))
  	val address = Reg(resetVal = Bits(0, 32))
  	val idle :: read :: read_miss :: read_hit :: write1 :: write2 :: write_hit:: Nil  = Enum(7){ UFix() } 
	val state = Reg(resetVal = idle)
	
//	dc.io.data_in	:= Bits(0)
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
		state 		:= write_hit
	}
	
//	mem()
	io.led 				:= test(1)
	dc.io.address		:= address
	dc.io.mem_data_in	:= mem(address)
	mem(address)		:= dc.io.mem_data_out
//	dc.io.mem_data_out	:= 
	dc.io.wr			:= wr
	dc.io.rd			:= rd
	dc.io.data_in		:= data_in

}
  



// Generate the Verlog code by invoking chiselMain() in our main()
object HelloMain2 {
  def main(args: Array[String]): Unit = { 
    chiselMain( args, () => new Test_dc())
  }
}


