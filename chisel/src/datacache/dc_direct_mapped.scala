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
 * direct mapped data cache
 * 
 * Author: Sahar Abbaspour (sabb@dtu.dk)
 * 
 */

// TODO: burst + stall

package dc

import Chisel._
import Node._


import scala.math




class DMCache(associativity: Int, num_blocks: Int) extends Bundle() {
  	      val valid 		= Mem(num_blocks / associativity) {Bits(width = 1)}
  	      val tag 			= Mem(num_blocks / associativity) {Bits(width = 30 - log2Up(num_blocks / associativity))}
  	      val data 			= Mem(num_blocks / associativity) {Bits(width = 32)}
  	}

class DC_1_way(associativity: Int, num_blocks: Int, word_length: Int) extends Component {
    val io = new Bundle {
  
    val rd				= UFix(INPUT, 1) // CPU read, load
    val wr				= UFix(INPUT, 1) // CPU write, store
    val data_in			= Bits(INPUT, width = 32) // data from CPU, write, store
    val data_out		= Bits(OUTPUT, width = 32) // data to CPU, read, load
    val mem_data_in		= Bits(INPUT, width = 32) // 
    val mem_data_out	= Bits(OUTPUT, width = 32) // 
    val address			= Bits(INPUT, width = 32) //
    
  } 
 
    val index_number 		= io.address(log2Up(num_blocks) + 1, 2)
    val dm_cache 			=  new DMCache(1, 1024)
    
    val is_hit = Reg(resetVal = UFix(0, 1))
    
  	when (dm_cache.valid(index_number) && dm_cache.tag(index_number) === io.address(word_length - 1, log2Up(num_blocks) + 2)) {
    	is_hit 	:= UFix(1)
    }
    .otherwise {
    	is_hit 	:= UFix(0)
    }
    
  	
  	val data_out = Reg(resetVal = Bits(0, 32))
  	val read_data = Reg(resetVal = Bits(10, 32))
  	
  	io.data_out		:= Bits(0)

  	val rd_reg			= Reg(resetVal = UFix(0, 1)) // register to sync rd with hit detection
  	
  	rd_reg				:= io.rd
  	// read the data from the cache
	when (rd_reg === UFix(1)) {
  	  
	  when (is_hit === UFix(1)) { // read hit
		  read_data	:= dm_cache.data(index_number)
	  }
	  
	  .otherwise { // read miss
		  dm_cache.data(index_number) := io.mem_data_in
		  dm_cache.valid(index_number) := UFix(1)
		  dm_cache.tag(index_number)	:= io.address(word_length - 1, log2Up(num_blocks) + 2)// update the tag
		  read_data	:= io.mem_data_in
	  }
	}

	//write
	when (io.wr === UFix(1)) {
	  
	  when (is_hit === UFix(1)) { // write hit
		  dm_cache.data(index_number) := io.data_in
		 
	  }
	  
	  .otherwise { // miss
		   dm_cache.data(index_number) := io.data_in
		   data_out := io.data_in
		   dm_cache.valid(index_number) := UFix(1)// update the valid bit
		   dm_cache.tag(index_number)	:= io.address(word_length - 1, log2Up(num_blocks) + 2)// update the tag
		   
	  }
	}

	io.mem_data_out	:= data_out
	io.data_out		:= read_data
}
  

