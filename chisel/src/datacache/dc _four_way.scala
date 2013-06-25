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
 * 4 way set associative data cache
 * 
 * Author: Sahar Abbaspour (sabb@dtu.dk)
 * 
 */


package dc

import Chisel._
import Node._


import scala.math




class Cache(associativity: Int, num_blocks: Int) extends Bundle() {
  	      val valid 		= Mem(num_blocks / associativity) {Bits(width = 1)}
  	      val tag 			= Mem(num_blocks / associativity) {Bits(width = 30 - log2Up(num_blocks / associativity))}
  	      val data 			= Mem(num_blocks / associativity) {Bits(width = 32)}
  	}

class DC_4_way(associativity: Int, num_blocks: Int, word_length: Int) extends Component {
    val io = new Bundle {
  
    val rd				= UFix(OUTPUT, 1) // CPU read, load
    val wr				= UFix(OUTPUT, 1) // CPU write, store
    val data_in			= Bits(INPUT, width = 32) // data from CPU, write, store
    val data_out		= Bits(OUTPUT, width = 32) // data to CPU, read, load
    val mem_data_in		= Bits(INPUT, width = 32) // 
    val mem_data_out	= Bits(OUTPUT, width = 32) // 
    val address			= UFix(INPUT, width = 32) //
    
  }
 

    val set0 =  new Cache(4, 1024)
    val set1 =  new Cache(4, 1024)
    val set2 =  new Cache(4, 1024)
    val set3 =  new Cache(4, 1024)

    
    val index_number 		= num_blocks / associativity
    
    val age			= Mem(index_number, seqRead = true) {Bits(width = 2 * associativity)} // to keep track of ages for a four way, 2 bits per each block in set
  	val set_age		= age(UFix(index_number)) // read the ages in accessed set // independent of read/write hit/miss
  	
  	val new_set_age	= Reg(resetVal = Bits(255, width = associativity * 2)) // new ages of the sets
  	



//	val tag_line	= tag(io.address(log2Up(index_number) + 1, 2)) // tag(index)
//	val valid_line	= valid(io.address(log2Up(index_number) + 1, 2))
//	val data_line	= Reg(resetVal = Bits(0, word_length * associativity))
//	val tag_offset	= tag_array_length / 4
//	val data_offset = associativity / 4
	val hit			= Bits()

	
	hit				:= Cat(set0.valid(Bits(index_number)) && set0.tag(Bits(index_number)) === io.address(word_length - 1, log2Up(index_number) + 2),
	    		  			set1.valid(Bits(index_number)) && set1.tag(Bits(index_number)) === io.address(word_length - 1, log2Up(index_number) + 2),	
	    		  			set2.valid(Bits(index_number)) && set2.tag(Bits(index_number)) === io.address(word_length - 1, log2Up(index_number) + 2),
	    		  			set3.valid(Bits(index_number)) && set3.tag(Bits(index_number)) === io.address(word_length - 1, log2Up(index_number) + 2))
	  
	val is_hit 		= Reg(resetVal = UFix(0, 1))
	// find the LRU block
	val b0			= Mux(set_age(7, 6) < set_age(5, 4), UFix(2), UFix(3))
	val b1			= Mux(set_age(3, 2) < set_age(1, 0), UFix(1), UFix(0))
	val b2			= Mux(b1 < b0, b0, b1)
	
 

	io.data_out		:= Bits(0)
	io.mem_data_out	:= Bits(0)
  	// check if the accessed content is in the cache
  	  when (hit(0) || hit(1) || hit(2) || hit(3)){
  	    is_hit 		:= UFix(1)
  	  }
  	
  	// read the data from the cache
	when (io.rd === UFix(1)) {
	  when (is_hit === UFix(1)) { // read hit
	   
	    
	    
	    when (hit(0) === UFix(1)) { // mux to read data
	      io.data_out	:= set0.data(Bits(index_number)) // read the data
	      
	      new_set_age := Cat (Bits(0), Mux(set_age(3, 2) < set_age(1, 0), set_age(3, 2) + Bits(1), set_age(3, 2)), 
	    		   			Mux(set_age(5, 4) < set_age(1, 0), set_age(5, 4) + Bits(1), set_age(5, 4)), 
	    		   			Mux(set_age(7, 6) < set_age(1, 0), set_age(7, 6) + Bits(1), set_age(7, 6)))

	    }
	    .elsewhen (hit(1) === UFix(1)) { 
	      io.data_out	:= set1.data(Bits(index_number))
	      
	      new_set_age := Cat (Mux(set_age(1, 0) < set_age(3, 2), set_age(1, 0) + Bits(1), set_age(3, 2)), Bits(0),
	    		   			Mux(set_age(5, 4) < set_age(3, 2), set_age(5, 4) + Bits(1), set_age(5, 4)), 
	    		   			Mux(set_age(7, 6) < set_age(3, 2), set_age(7, 6) + Bits(1), set_age(7, 6)))
	    }
	    .elsewhen (hit(2) === UFix(1)) { // mux to read data
	      io.data_out	:= set2.data(Bits(index_number)) // read the data
	      
	      new_set_age := Cat (Mux(set_age(1, 0) < set_age(5, 4), set_age(1, 0) + Bits(1), set_age(5, 4)), 
	    		  			Mux(set_age(3, 2) < set_age(5, 4), set_age(3, 2) + Bits(1), set_age(5, 4)), 
	    		  			Bits(0),
	    		   			Mux(set_age(7, 6) < set_age(5, 4), set_age(7, 6) + Bits(1), set_age(5, 4)))
	    }
	    .elsewhen (hit(3) === UFix(1)) { 
	      io.data_out	:= set3.data(Bits(index_number))
	      
	      new_set_age := Cat (Mux(set_age(1, 0) < set_age(7, 6), set_age(1, 0) + Bits(1), set_age(7, 6)),
	    		  			Mux(set_age(3, 2) < set_age(7, 6), set_age(3, 2) + Bits(1), set_age(7, 6)),
	    		   			Mux(set_age(5, 4) < set_age(7, 6), set_age(5, 4) + Bits(1), set_age(7, 6)), 
	    		   			Bits(0))
	    }
	    
	     // update ages, write to the age memory
	    age(Bits(index_number)) := new_set_age
	   
	    //When a block is accessed that is currently in the cache with
	    //age a, its age is reset to 0, all blocks younger than a age by
	    //1, while blocks older than a are not affected.
	  
	    
	  }
	  
	  when (is_hit === UFix(0)){ // read miss
//		In case of a cache miss, the accessed block is put into the
//		cache with age 0, all blocks in the cache age by 1, and the
//	    block with age (A - 1) (oldest, if any) is removed from the cache.

	    // load from main memory to LRU block, update valid and tag bit, update LRU
	    when (b2 === UFix(0)){
	      set0.data(Bits(index_number)) := io.mem_data_in
	      set0.tag(Bits(index_number))	:= io.address(word_length - 1, log2Up(index_number) + 2)
	      set0.valid(Bits(index_number)):= UFix(1)
	      
	      new_set_age := Cat (Bits(0), set_age(3, 2) + Bits(1), set_age(5, 4) + Bits(1), set_age(7, 6) + Bits(1))
	    }
	    .elsewhen (b2 === UFix(1)){
	      set1.data(Bits(index_number)) := io.mem_data_in
	      set1.tag(Bits(index_number))	:= io.address(word_length - 1, log2Up(index_number) + 2)
	      set1.valid(Bits(index_number)):= UFix(1)
	      
	      new_set_age := Cat ( set_age(1, 0) + Bits(1), Bits(0), set_age(5, 4) + Bits(1), set_age(7, 6) + Bits(1))
	    }
	    .elsewhen (b2 === UFix(2)){
	      set2.data(Bits(index_number)) := io.mem_data_in
	      set2.tag(Bits(index_number))	:= io.address(word_length - 1, log2Up(index_number) + 2)
	      set2.valid(Bits(index_number)):= UFix(1)
	      
	      new_set_age := Cat (set_age(1, 0) + Bits(1), set_age(3, 2) + Bits(1), Bits(0), set_age(7, 6) + Bits(1))
	    }
	    .elsewhen (b2 === UFix(3)){
	      set3.data(Bits(index_number)) := io.mem_data_in
	      set2.tag(Bits(index_number))	:= io.address(word_length - 1, log2Up(index_number) + 2)
	      set3.valid(Bits(index_number)):= UFix(1)
	      
	      new_set_age := Cat (set_age(1, 0) + Bits(1), set_age(3, 2) + Bits(1), set_age(5, 4) + Bits(1), Bits(0))
	    }
	    
	    age(Bits(index_number)) := new_set_age
	  }
	}

	//write
	when (io.wr === UFix(1)) {
	  when (is_hit === UFix(1)) { // write hit
	   
        // also write to memory, write_through
	 //   io.mem_data_out := io.data_in  
	    
	    when (hit(0) === UFix(1)) { // mux to write data
	       set0.data(Bits(index_number)) := io.data_in

	       // update the ages
	       new_set_age := Cat (Bits(0), Mux(set_age(3, 2) < set_age(1, 0), set_age(3, 2) + Bits(1), set_age(3, 2)), 
	    		   			Mux(set_age(5, 4) < set_age(1, 0), set_age(5, 4) + Bits(1), set_age(5, 4)), 
	    		   			Mux(set_age(7, 6) < set_age(1, 0), set_age(7, 6) + Bits(1), set_age(7, 6)))

	    }
	    .elsewhen (hit(1) === UFix(1)) { 
	      
	      set1.data(Bits(index_number)) := io.data_in
	    
	      new_set_age := Cat (Mux(set_age(1, 0) < set_age(3, 2), set_age(1, 0) + Bits(1), set_age(3, 2)), Bits(0),
	    		   			Mux(set_age(5, 4) < set_age(3, 2), set_age(5, 4) + Bits(1), set_age(5, 4)), 
	    		   			Mux(set_age(7, 6) < set_age(3, 2), set_age(7, 6) + Bits(1), set_age(7, 6)))
	    }
	    .elsewhen (hit(2) === UFix(1)) { // mux to read data
	      set2.data(Bits(index_number)) := io.data_in
	      
	      new_set_age := Cat (Mux(set_age(1, 0) < set_age(5, 4), set_age(1, 0) + Bits(1), set_age(5, 4)), 
	    		  			Mux(set_age(3, 2) < set_age(5, 4), set_age(3, 2) + Bits(1), set_age(5, 4)), 
	    		  			Bits(0),
	    		   			Mux(set_age(7, 6) < set_age(5, 4), set_age(7, 6) + Bits(1), set_age(5, 4)))
	    }
	    .elsewhen (hit(3) === UFix(1)) { 
	      set3.data(Bits(index_number)) := io.data_in
	      
	      new_set_age := Cat (Mux(set_age(1, 0) < set_age(7, 6), set_age(1, 0) + Bits(1), set_age(7, 6)),
	    		  			Mux(set_age(3, 2) < set_age(7, 6), set_age(3, 2) + Bits(1), set_age(7, 6)),
	    		   			Mux(set_age(5, 4) < set_age(7, 6), set_age(5, 4) + Bits(1), set_age(7, 6)), 
	    		   			Bits(0))
	    }
	    
	    age(Bits(index_number)) := new_set_age

	  }
	  
	  when (is_hit === UFix(0)){ // write miss, also write to main memory
	    
	    when (b2 === UFix(0)){
	      set0.data(Bits(index_number)) := io.data_in
	      set0.tag(Bits(index_number))	:= io.address(word_length - 1, log2Up(index_number) + 2)
	      set0.valid(Bits(index_number)):= UFix(1)
	      
	      new_set_age := Cat (Bits(0), set_age(3, 2) + Bits(1), set_age(5, 4) + Bits(1), set_age(7, 6) + Bits(1))
	    }
	    .elsewhen (b2 === UFix(1)){
	      set1.data(Bits(index_number)) := io.data_in
	      set1.tag(Bits(index_number))	:= io.address(word_length - 1, log2Up(index_number) + 2)
	      set1.valid(Bits(index_number)):= UFix(1)
	      
	      new_set_age := Cat ( set_age(1, 0) + Bits(1), Bits(0), set_age(5, 4) + Bits(1), set_age(7, 6) + Bits(1))
	    }
	    .elsewhen (b2 === UFix(2)){
	      set2.data(Bits(index_number)) := io.data_in
	      set2.tag(Bits(index_number))	:= io.address(word_length - 1, log2Up(index_number) + 2)
	      set2.valid(Bits(index_number)):= UFix(1)
	      
	      new_set_age := Cat (set_age(1, 0) + Bits(1), set_age(3, 2) + Bits(1), Bits(0), set_age(7, 6) + Bits(1))
	    }
	    .elsewhen (b2 === UFix(3)){
	      set3.data(Bits(index_number)) := io.data_in
	      set2.tag(Bits(index_number))	:= io.address(word_length - 1, log2Up(index_number) + 2)
	      set3.valid(Bits(index_number)):= UFix(1)
	      
	      new_set_age := Cat (set_age(1, 0) + Bits(1), set_age(3, 2) + Bits(1), set_age(5, 4) + Bits(1), Bits(0))
	    }
	    
	    age(Bits(index_number)) := new_set_age
	  }
	}

}
  

