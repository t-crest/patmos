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
 * 2 way set associative data cache
 * 
 * Author: Sahar Abbaspour (sabb@dtu.dk)
 * 
 */


package dc

import Chisel._
import Node._


import scala.math






class DC_2_way(associativity: Int, num_blocks: Int, word_length: Int) extends Component {
    val io = new Bundle {
  
    val rd				= UFix(INPUT, 1) // CPU read, load
    val wr				= UFix(INPUT, 1) // CPU write, store
    val data_in			= Bits(INPUT, width = 32) // data from CPU, write, store
    val data_out		= Bits(OUTPUT, width = 32) // data to CPU, read, load
    val mem_data_in		= Bits(INPUT, width = 32) // 
    val mem_data_out	= Bits(OUTPUT, width = 32) // 
    val address			= Bits(INPUT, width = 32) //
    
  }
 class Cache(associativity: Int, num_blocks: Int) extends Bundle() {
  	      val valid 		= Mem(num_blocks / associativity) {Bits(width = 1)}
  	      val tag 			= Mem(num_blocks / associativity) {Bits(width = 30 - log2Up(num_blocks / associativity))}
  	      val data 			= Mem(num_blocks / associativity) {Bits(width = 32)}
  	}

    val way0 =  new Cache(2, 1024)
    val way1 =  new Cache(2, 1024)

    val lru 			= Mem(num_blocks / associativity) {Bits(width = 1)}
    
    val index_number 	= io.address(log2Up(num_blocks) + 1, 2)
  	
  	val init			= Reg(resetVal = UFix(1, 1))
  	
  	when (init === UFix(1)) { // initialize memory, for simulation
	  	way0.valid (Bits(25)) := Bits(0)
	  	way0.valid (Bits(27)) := Bits(0)
	  	way0.valid (Bits(37)) := Bits(0) // address == 150
	  	way0.tag(Bits(25)) := Bits(10)
	  	way0.tag(Bits(27)) := Bits(10)
	  	way0.tag(Bits(37)) := Bits(10)
	
	  	way1.valid (Bits(25)) := Bits(0)
	  	way1.valid (Bits(27)) := Bits(0)
	  	way1.valid (Bits(37)) := Bits(0) // address == 150
	  	way1.tag(Bits(25)) := Bits(10)
	  	way1.tag(Bits(27)) := Bits(10)
	  	way1.tag(Bits(37)) := Bits(10)
	  	
	  	lru(Bits(25)) := Bits(0)
	  	lru(Bits(27)) := Bits(0)
	  	lru(Bits(37)) := Bits(0)
	  	
	  	init := UFix(0)
  	}
    
    val mem_data_in_reg = Reg() {Bits()}
  	mem_data_in_reg		:= io.mem_data_in
  	
  	val address_reg		= Reg() { Bits() } 
  	address_reg			:= io.address
  	
  	val wr_reg		= Reg() { UFix() } 
  	wr_reg			:= io.wr
  	
  	val rd_reg		= Reg() { UFix() } 
  	rd_reg			:= io.rd
  	
  	val data_in_reg		= Reg() { Bits() } 
  	data_in_reg			:= io.data_in
  	
  	val index_number_reg	= Reg() { Bits() } 
  	index_number_reg		:= index_number
  	
  	val valid_dout0 =   Reg() { Bits() }
  	val tag_dout0 =  Reg() { Bits() }
 	val data_dout0 = Reg() { Bits() }
 	
 	val valid_dout1 =   Reg() { Bits() }
  	val tag_dout1 =  Reg() { Bits() }
 	val data_dout1 = Reg() { Bits() }
 	
 	
 	val is_hit0 		= UFix()
    val is_hit1 		= UFix()

    
    is_hit0				:= UFix(0)
    is_hit1				:= UFix(0)
    
    
    when (io.rd === UFix(1) || io.wr === UFix(1)) { // on a read/write, read the tag and valid
		valid_dout0 := way0.valid(index_number) 
		tag_dout0 := way0.tag(index_number) 
		
		valid_dout1 := way1.valid(index_number) 
		tag_dout1 := way1.tag(index_number) 
	} 

	when (io.rd === UFix(1)) { // read the data on a load
		data_dout0	:= way0.data(index_number)
		data_dout1	:= way1.data(index_number)
	}
    
    io.data_out := Bits(0)
 	io.mem_data_out := Bits(1)
    
    when (valid_dout0 === Bits(1) && address_reg(word_length - 1, log2Up(num_blocks) + 2) === tag_dout0) {
    	is_hit0			:= UFix(1)
 	}
 	.elsewhen (valid_dout0 != Bits(1) || address_reg(word_length - 1, log2Up(num_blocks) + 2) != tag_dout0) {
 		is_hit0			:= UFix(0)
 	}
 	
 	when  (valid_dout1 === Bits(1) && address_reg(word_length - 1, log2Up(num_blocks) + 2) === tag_dout1) {
    	is_hit1			:= UFix(1)
 	}
 	
 	.elsewhen (valid_dout1 != Bits(1) || address_reg(word_length - 1, log2Up(num_blocks) + 2) != tag_dout1) {
 		is_hit1			:= UFix(0)
 	}
 	
 	
	when (rd_reg === UFix(1)) {
		  when (is_hit0 === UFix(1)) { // read hit
			  io.data_out := data_dout0
			  lru(index_number_reg) 		:= UFix(1)
		  }
		  
		  .elsewhen (is_hit1 === UFix(1)) { // read hit
			  	io.data_out := data_dout1
			  	lru(index_number_reg)  		:= UFix(0)
		  }
		  
		  .elsewhen (is_hit0 === UFix(0) &&  is_hit0 === UFix(0)) { // read miss
			    when (lru(index_number_reg)  === UFix(0)) {
			    	way0.data(index_number_reg) := mem_data_in_reg // read data and write it to cache
		  			way0.valid(index_number_reg) := Bits(1)// update the valid bit
					way0.tag(index_number_reg)	:= address_reg(word_length - 1, log2Up(num_blocks) + 2)// update the tag
					lru(index_number_reg) 	:= UFix(1)
			    }
			    .elsewhen (lru(index_number_reg)  === UFix(1)) {
			    	way1.data(index_number_reg) := mem_data_in_reg // read data and write it to cache
		  			way1.valid(index_number_reg) := Bits(1)// update the valid bit
					way1.tag(index_number_reg)	:= address_reg(word_length - 1, log2Up(num_blocks) + 2)// update the tag
					lru(index_number_reg) 	:= UFix(0)
					
			    }
			    io.data_out :=  mem_data_in_reg// on a miss, it reads again, this is for sim
		  }
	}
	
	when (wr_reg === UFix(1)) {
		io.mem_data_out	:= data_in_reg // to memory
		 when (is_hit0 === UFix(1)) { // write hit
			way0.data(index_number_reg)  := data_in_reg
			lru(index_number_reg)  		:= UFix(1) // no eviction
		  }
		  
		  .elsewhen (is_hit1 === UFix(1)) { // write hit
			way1.data(index_number_reg)  := data_in_reg
			lru(index_number_reg)  		:= UFix(0)
		  }
		  
		  .elsewhen (is_hit0 === UFix(0) &&  is_hit0 === UFix(0)) { // write miss
			  when (lru(index_number_reg)  === UFix(0)) {
				  	way0.valid(index_number_reg) := Bits(1)// update the valid bit
					way0.tag(index_number_reg)	:= address_reg(word_length - 1, log2Up(num_blocks) + 2)// update the tag
					way0.data(index_number_reg)  := data_in_reg
					lru(index_number_reg) 	:= UFix(1)
			  }
		    
			  when (lru(index_number_reg)  === UFix(1)) {
				  	way1.valid(index_number_reg) := Bits(1)// update the valid bit
					way1.tag(index_number_reg)	:= address_reg(word_length - 1, log2Up(num_blocks) + 2)// update the tag
					way1.data(index_number_reg)  := data_in_reg
					lru(index_number_reg) 	:= UFix(0)
			  }
		  }
	}
	

}
  

