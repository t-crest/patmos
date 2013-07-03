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


class UART(clk_freq: Int, baud_rate: Int) extends Component {
  val io = new Bundle {
    val address = UFix(INPUT, 1)
	val data_in = UFix(INPUT, 8)
	val wr = UFix(INPUT, 1) 
	val rd = UFix(INPUT, 1)
	val tx = UFix(OUTPUT, 1)
	val rx = UFix(INPUT, 1)
	val rd_data = UFix(OUTPUT, 8)
  }

  	
  	val c_tx_divider_val 	= clk_freq/baud_rate	
  	val tx_baud_counter 	= Reg(resetVal = UFix(0, log2Up(clk_freq/baud_rate)))
	val tx_baud_tick 		= Reg(resetVal = UFix(0, 1))
	
	val tx_reset_state :: tx_send:: Nil  = Enum(2){ UFix() } 
	val tx_state 			= Reg(resetVal = tx_reset_state)
	val tx_empty 			= Reg(resetVal = UFix(0, 1))
  	val tx_reg 				= Reg(resetVal = UFix(1, 1))
  	val tx_counter 			= Reg(resetVal = UFix(0, 4))
//	val tx_buff				= Reg(resetVal = UFix(0, 10))
	
	
	val rxd_reg0 			= Reg(resetVal = UFix(1, 1))
	val rxd_reg1 			= Reg(resetVal = UFix(1, 1))
	val rxd_reg2 			= Reg(resetVal = UFix(1, 1))
	
	val rx_baud_counter 	= Reg(resetVal = UFix(0, log2Up(clk_freq/baud_rate)))
	val rx_baud_tick 		= Reg(resetVal = UFix(0, 1))
	val rx_enable	 		= Reg(resetVal = UFix(0, 1))
	
	val rx_data_buffer 		= Reg(resetVal = UFix(0, 8))	
	val rx_full = Reg(resetVal = UFix(0, 1))
	val rx_counter = Reg(resetVal = UFix(0, 3)) 	
  	val rx_idle  :: rx_start :: rx_receive_data :: rx_stop_bit :: Nil  = Enum(4){ UFix() }
	val rx_state 			= Reg(resetVal = rx_idle)
	
	// UART TX clk
	when (tx_baud_counter === UFix(clk_freq/baud_rate)){
  	  	tx_baud_counter		:= UFix(0)
  	  	tx_baud_tick			:= UFix(1)
  	}
  	.otherwise {
  		tx_baud_counter		:= tx_baud_counter + UFix(1)
  		tx_baud_tick			:= UFix(0)
  	}

  	val tx_buff1 = Reg(resetVal = UFix(0, 10))
	val tx_buff2 = Reg(resetVal = UFix(0, 10))
	val tx_e1 = Reg(resetVal = UFix(1, 1))
	val tx_e2 = Reg(resetVal = UFix(1, 1))
	

    
    // Send data	
  	
  	when (tx_state === tx_reset_state) {
		tx_empty			:= UFix(1)
		tx_counter			:= UFix(0)
		tx_reg      		:= UFix(1)
		tx_e1 		:= UFix(1)
		
		when (io.wr === UFix(1)) {
			//tx_buff			:= Cat (UFix(1), io.data_in, UFix(0)) // keep the input
			when (tx_e1 === UFix(1)){
			  tx_buff1 := Mux(tx_e2 === UFix(1), Cat (UFix(1), io.data_in, UFix(0)), tx_buff2) 
			  tx_e1 := UFix(0)
			}
			.otherwise{
			  when (tx_e2 === UFix(1)){
			    tx_buff2 := Cat(UFix(1), io.data_in, UFix(0))
			    tx_e2 := UFix(0)
			  }
			}
			
			tx_state       	:= tx_send	  
		}
	}
	
  	
	when (tx_state === tx_send){
		tx_empty  			:= UFix(0)
	  	
		when (tx_baud_tick === UFix(1)){
		  	tx_state 		:= Mux(tx_counter === UFix(10), tx_reset_state, tx_send)
			tx_reg	  		:= Mux(tx_counter === UFix(10), UFix(1), tx_buff1(0))
			tx_buff1 		:= Cat (UFix(0), tx_buff1 (9, 1))
			tx_counter 		:= Mux(tx_counter === UFix(10), UFix(0), tx_counter + UFix(1)) // 
		}
	}
  	
	
	
	// UART TX clk
	when (rx_enable) {
		when (rx_baud_counter === UFix(clk_freq/baud_rate)){
  	  	rx_baud_counter		:= UFix(0)
  	  	rx_baud_tick		:= UFix(1)
		}
	  	.otherwise {
	  		rx_baud_counter		:= rx_baud_counter + UFix(1)
	  		rx_baud_tick		:= UFix(0)
	  	}
	}
	
	
  	// Receive data
  	
	rxd_reg0 				:= io.rx;
	rxd_reg1 				:= rxd_reg0;
	rxd_reg2 				:= rxd_reg1
	
	
	// RX shift in
	when (rx_state === rx_idle) {
		rx_full				:= UFix(0)
  		when (rxd_reg2 === UFix(0)){
  		   rx_state 		:= rx_start
  		   rx_baud_counter	:= UFix(clk_freq/baud_rate) / UFix(2)
  		   rx_enable 		:= UFix(1)
  		}
	}
	
	when (rx_state === rx_start){
		when (rx_baud_tick === UFix(1)) {
			when (rxd_reg2 != UFix(0)) {
				rx_state 		:= rx_idle
			}
			.otherwise{
				rx_state		:= rx_receive_data
			}
		}
	}
	
	when (rx_state === rx_receive_data) {
		when (rx_baud_tick === UFix(1)){
			rx_data_buffer := Cat(rxd_reg2, rx_data_buffer(7, 1))
		  	rx_counter := Mux(rx_counter === UFix(7), UFix(0), rx_counter + UFix(1)) //
		  	rx_state := Mux(rx_counter === UFix(7), rx_stop_bit, rx_receive_data)
		}
	}
 	
	when (rx_state === rx_stop_bit) {
		when (rx_baud_tick === UFix(1)){
			when (rxd_reg2 === UFix(1)) {
				rx_state := rx_idle
				rx_enable		:= UFix(0)
				rx_full			:= UFix(1)
			}
			
		}
	}
	
	io.rd_data := Mux(io.address === UFix(0), Cat(UFix(0, width = 6), rx_full, tx_empty), rx_data_buffer)
	io.tx := tx_reg
