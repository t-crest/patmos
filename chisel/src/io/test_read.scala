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
 * Test UART module in chisel.
 * 
 * Author: Sahar Abbaspour (sabb@dtu.com)
 * 
 */


package hello

import Chisel._
import Node._


class Test() extends Component {
  val io = new Bundle {
//    val reset = Bool(dir = INPUT)
	val rx = UFix(INPUT, 1)
	val tx = UFix(OUTPUT, 1)
	val led = UFix(OUTPUT, 1) 
  }
  
  	val CNT_MAX = UFix(16000000/2-1);
  	val r1 = Reg(resetVal = UFix(0, 25))
  	val blk = Reg(resetVal = UFix(0, 1))
  	
  	val wr = Reg(resetVal = UFix(0, 1))
  	val rd = Reg(resetVal = UFix(0, 1))
 	val wr_data = Reg(resetVal = UFix(0, 8))
// 	val rd_data = Reg(resetVal = UFix(0, 8))
  	val address = Reg(resetVal = UFix(0, 1))
  	val r_read :: r_wait_st :: read_st :: data_read_st :: wait_st :: write_st:: Nil  = Enum(6){ UFix() } 
	val state = Reg(resetVal = r_read)

  	val uart = new UART()
 // 	val rx = new RX()
  	
  	
	uart.io.clk_freq := UFix(50000000)
	uart.io.baud_rate := UFix(115200)
// 	val wr_data = Mux(blk === UFix(0), UFix(40), UFix(41))
 	
	uart.io.tx <> io.tx
	uart.io.rx <> io.rx
//	rx.io.rx <> io.rx


//	rx.io.rx  <> io.rx //:=blk//
//	when (uart.io.rd_data === UFix(49)){
//	  blk := UFix(1)
//	}
	
 	r1 := r1 + UFix(1)
	when (r1 === CNT_MAX) {
  		 r1 := UFix(0)
  		// blk := ~blk
  		
	}
	
	//************//
	when (state === r_read) {
	  wr 		:= UFix(0)
	  address 	:= UFix(0)
	  state		:= r_wait_st
	}
	
	when (state === r_wait_st) {
		when (uart.io.rd_data(1) === UFix(1)){
		    address 	:= UFix(1) // read data from uart
		  	state := data_read_st
		  	blk := UFix(1)
		}
		.otherwise{
			state := r_wait_st
		}
	}
	when (state === data_read_st){
	  wr_data :=  uart.io.rd_data //uart.io.data_out_r//wr_data //rx.io.data_out//
	  state   := read_st
	}
	//************//
	when (state === read_st) {
	  wr 		:= UFix(0)
//	  wr_data 	:= UFix(0)
	  rd 		:= UFix(1)
	  address 	:= UFix(0)
	  state		:= wait_st
	}
	
	when (state === wait_st){
	  when (uart.io.rd_data(0) === UFix(1)){
	  	state := write_st
	  	rd 	  := UFix(0)
	  	wr 		:= UFix(1)
	    address 	:= UFix(1)
	  }
	  .otherwise {
	  	state := r_read
	  }
	}
	
	when (state === write_st){
	 wr 		:= UFix(0)
//	  wr_data 	:= Mux(blk === UFix(0), UFix(40), UFix(41))

	  state 	:= r_read
	}
	
	uart.io.address := address
  	io.led := blk;
	uart.io.data_in := wr_data  
  	//rd_data := uart.io.rd_data
	uart.io.wr := wr//UFix(1); 
	uart.io.rd := rd
	
}
  



// Generate the Verlog code by invoking chiselMain() in our main()
object HelloMain {
  def main(args: Array[String]): Unit = { 
    chiselMain( args, () => new Test())
  }
}

