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
 * UART module in chisel.
 * 
 * Author: Sahar Abbaspour (sabb@dtu.com)
 * 
 */


package hello

import Chisel._
import Node._


class UART(clk_freq: UFix, baud_rate: UFix) extends Component {
	val io = new Bundle {
    val address = UFix(INPUT, 1)
	val data_in = UFix(INPUT, 8)
	val wr = UFix(INPUT, 1) 
	val rd = UFix(INPUT, 1)
	val tx = UFix(OUTPUT, 1)
	val rx = UFix(INPUT, 1)
	val rd_data = UFix(OUTPUT, 8)
  }

  	val tx_divide = clk_freq / baud_rate
  
  	val accept_reg = Reg(resetVal = UFix(0, 1))
  	val tx_reg = Reg(resetVal = UFix(1, 1))
  	val data_counter = Reg(resetVal = UFix(0, 4))

  	
  	
  	val reset_state :: idle :: send:: Nil  = Enum(3){ UFix() } 
	val state = Reg(resetVal = reset_state)
	val baud_counter = Reg(resetVal = UFix(0, 10))
	val baud_tick = Reg(resetVal = UFix(0, 1))
	

	val data = Reg(resetVal = UFix(0, 10))//io.data_in
	
	//
	val s_baud_counter = Reg(resetVal = UFix(0, 10))
	val s_baud_tick = Reg(resetVal = UFix(0, 1))
	val data_buffer = Reg(resetVal = UFix(0, 8))	
	val rxd_reg0     = Reg(resetVal = UFix(1, 1))
	val rxd_reg1     = Reg(resetVal = UFix(1, 1))
	val rxd_reg2     = Reg(resetVal = UFix(1, 1))
//	val test     = Reg(resetVal = UFix(1, 1))
	val r_data = Reg(resetVal = UFix(0, 1))
	val r_data_counter = Reg(resetVal = UFix(0, 3)) 	
  	val r_reset_state :: r_idle  :: r_receive_half :: r_receive_data :: r_stop_bit :: Nil  = Enum(5){ UFix() }
	val r_state = Reg(resetVal = r_reset_state)
	val rx_clk_ena = Reg(resetVal = UFix(0, 1))
	val count_wait = Reg(resetVal = UFix(0, 2))
	//
	
	io.rd_data := Mux(io.address === UFix(0), Cat(UFix(0, width = 6), r_data, accept_reg), data_buffer)
	
	
	rxd_reg0 := io.rx;
	rxd_reg1 := rxd_reg0;
	rxd_reg2 := rxd_reg1
//	io.rd_data := data_buffer
	// Baud generator
	when (baud_counter === tx_divide)
	{
	  baud_counter := UFix(0)
	  baud_tick := UFix(1)
	}
	.otherwise
	{
	  baud_counter := baud_counter + UFix(1)
	  baud_tick := UFix(0)
	}
	
  // Count data bits
	when (baud_tick === UFix(1))
	{
	  when (state === send)
	  {
	    data_counter := data_counter + UFix(1)
	    accept_reg := UFix(0)
	  }
	  .otherwise
	  {
	    data_counter := UFix(0)
	  }
	}
    

    
    // TX state machine
	when (state === reset_state) {
		accept_reg	:= UFix(0)
		tx_reg      := UFix(1)
		state       := idle  
	}
	
  	when (state === idle) {
  		accept_reg	:= UFix(1)
  		tx_reg		:= UFix(1)
  		
  		when (io.wr === UFix(1)){
  		  state       := send
  		  data := Cat (UFix(1), io.data_in, UFix(0)) // latch input data
  		  
  		  }
 		when (accept_reg === UFix(0) || io.wr === UFix(0)){
 		  state       := idle
 		  }
  	}
  	
	when (state === send)
	  	{
		  	  accept_reg  := UFix(0)
	  	
		  	  when (baud_tick === UFix(1)){
		  	  	  state := Mux(data_counter === UFix(10), reset_state, send)
			  	  tx_reg	  := Mux(data_counter === UFix(10), UFix(1), data(0))
			  	  data := Cat (UFix(0), data (9, 1))
			  	  data_counter := Mux(data_counter === UFix(10), UFix(0), data_counter + UFix(1)) //  
		  	}
	  	}
  	
	// high speed baud counter
	when (s_baud_counter === (UFix(50000000) / UFix(115200)) / UFix(2))
	{	 
	  s_baud_tick := UFix(1)
	  s_baud_counter := s_baud_counter + UFix(1)  
	}
	.otherwise
	{
	  when (s_baud_counter === tx_divide)
	  {
	    s_baud_counter := UFix(0)
	  }
	  .otherwise 
	  {
	    when (rx_clk_ena === UFix(1)){
	    	s_baud_counter := s_baud_counter + UFix(1)
	    }
	  }
	  s_baud_tick := UFix(0)
	}
	//
	
		// RX state machine
		
		when (r_state === r_reset_state) {
			
			r_state       	:= r_idle  
		}
	  	when (r_state === r_idle) {
	  	   r_data := UFix(0)
//	  		when (s_baud_tick === UFix(1)){
		  		when (rxd_reg2 === UFix(0))
		  		{
		  		  r_state := r_receive_half
		  		  rx_clk_ena := UFix(1);
		  		}
	  	   		.otherwise {rx_clk_ena := UFix(0)}
//	  		}
	  	}
	  	
	  	when (r_state === r_receive_half){
	  	  when (s_baud_tick === UFix(1)){
	  	    count_wait := count_wait + UFix(1) // drop the first rx_clk ...
	  	    when (count_wait === UFix(1)){
	  		  r_state := r_receive_data
	  	    }

	  	  }
	  	}
		
	  	when (r_state === r_receive_data) {

		  		when (s_baud_tick === UFix(1)){
		  		  
		  			data_buffer := Cat(rxd_reg2, data_buffer(7, 1))
		  			r_data_counter := Mux(r_data_counter === UFix(7), UFix(0), r_data_counter + UFix(1)) //
		  	  	    r_state := Mux(r_data_counter === UFix(7), r_stop_bit, r_receive_data)
		  		}
		  	}
		when (r_state === r_stop_bit) 
		  	{
		  	 
				when (s_baud_tick === UFix(1)){
			  	 	when (rxd_reg2 === UFix(1))		
					{
			  	 	  r_state := r_reset_state
			  	 	  r_data := UFix(1)
			  	 	  rx_clk_ena := UFix(0)
					}
			  	 	.otherwise
			  	 	{
			  	 	  r_state := r_stop_bit // wait until receiving stop bit?
			  	 	}
		  	 	}	
		  	}
	
	io.tx := tx_reg
 //   io.data_out_r := data_buffer
}
