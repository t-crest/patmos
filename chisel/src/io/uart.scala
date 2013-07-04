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


package io

import Chisel._
import Node._

import patmos.UartIO
import patmos.UartPinIO

class UART(clk_freq: Int, baud_rate: Int) extends Component {
  	val io = new UartIO()

  	val c_tx_divider_val 	= clk_freq/baud_rate	
  	val tx_baud_counter 	= Reg(resetVal = UFix(0, log2Up(clk_freq/baud_rate)))
	val tx_baud_tick 		= Reg(resetVal = UFix(0, 1))
	
	val tx_idle :: tx_send :: Nil  = Enum(2){ UFix() } 
	val tx_state 			= Reg(resetVal = tx_idle)
	val tx_empty 			= Reg(resetVal = UFix(1, 1))
	val tx_data 			= Reg(resetVal = UFix(0, 8))
	val tx_buff				= Reg(resetVal = UFix(0, 10))
  	val tx_reg 				= Reg(resetVal = UFix(1, 1))
  	val tx_counter 			= Reg(resetVal = UFix(0, 4))
		
	val rxd_reg0 			= Reg(resetVal = UFix(1, 1))
	val rxd_reg1 			= Reg(resetVal = UFix(1, 1))
	val rxd_reg2 			= Reg(resetVal = UFix(1, 1))
	
	val rx_baud_counter 	= Reg(resetVal = UFix(0, log2Up(clk_freq/baud_rate)))
	val rx_baud_tick 		= Reg(resetVal = UFix(0, 1))
	val rx_enable	 		= Reg(resetVal = UFix(0, 1))
	
	val rx_data 			= Reg(resetVal = UFix(0, 8))	
	val rx_buff 			= Reg(resetVal = UFix(0, 8))	
	val rx_full 			= Reg(resetVal = UFix(0, 1))
	val rx_counter			= Reg(resetVal = UFix(0, 3)) 	
  	val rx_idle  :: rx_start :: rx_receive_data :: rx_stop_bit :: Nil  = Enum(4){ UFix() }
	val rx_state 			= Reg(resetVal = rx_idle)
	
	// Write to UART
	when (io.wr === UFix(1)) {
		tx_data := io.data_in
	    tx_empty := UFix(0)
	}

	// Read data
	val rdDataReg = Reg(resetVal = UFix(0, width = 8))
	when(io.rd === UFix(1)) {
		rdDataReg := Mux(io.address === UFix(0),
						 Cat(UFix(0, width = 6), rx_full, tx_empty),
						 rx_data)
		rx_full := Mux(io.address === UFix(0), rx_full, UFix(0))
	}

	// UART TX clk
	when (tx_baud_counter === UFix(clk_freq/baud_rate)){
  	  	tx_baud_counter		:= UFix(0)
  	  	tx_baud_tick		:= UFix(1)
  	}
  	.otherwise {
  		tx_baud_counter		:= tx_baud_counter + UFix(1)
  		tx_baud_tick		:= UFix(0)
  	}

    // Send data	
  	
  	when (tx_state === tx_idle) {
		when (tx_empty === UFix(0)) {
		  tx_empty			:= UFix(1)
		  tx_buff			:= Cat(UFix(1), tx_data, UFix(0))
		  tx_state			:= tx_send
		}
	}	
  	
	when (tx_state === tx_send) {
		when (tx_baud_tick === UFix(1)){
			tx_buff			:= Cat (UFix(0), tx_buff (9, 1))
			tx_reg	  		:= tx_buff(0)
			tx_counter 		:= Mux(tx_counter === UFix(10), UFix(0), tx_counter + UFix(1))

		    when (tx_counter === UFix(10)) {
			  when (tx_empty === UFix(0)) {
				tx_empty		:= UFix(1)
				tx_buff			:= Cat(UFix(1), tx_data)
				tx_reg  		:= UFix(0)
				tx_counter		:= UFix(1)
			  }
			  .otherwise {
				tx_reg  		:= UFix(1)
				tx_counter		:= UFix(0)
		  		tx_state 		:= tx_idle
			  }
			}
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
  	
	rxd_reg0 				:= io.pins.rx;
	rxd_reg1 				:= rxd_reg0;
	rxd_reg2 				:= rxd_reg1
	
	
	// RX shift in
	when (rx_state === rx_idle) {
  		when (rxd_reg2 === UFix(0)){
  		   rx_state 		:= rx_start
  		   rx_baud_counter	:= UFix(clk_freq/baud_rate) / UFix(2)
  		   rx_enable 		:= UFix(1)
  		}
	}
	
	when (rx_state === rx_start){
		when (rx_baud_tick === UFix(1)) {
			when (rxd_reg2 === UFix(0)) {
				rx_state		:= rx_receive_data
			}
			.otherwise{
				rx_state 		:= rx_idle
			}
		}
	}
	
	when (rx_state === rx_receive_data) {
		when (rx_baud_tick === UFix(1)){
		  	rx_state := Mux(rx_counter === UFix(7), rx_stop_bit, rx_receive_data)
		  	rx_counter := Mux(rx_counter === UFix(7), UFix(0), rx_counter + UFix(1))
			rx_buff :=  Cat(rxd_reg2, rx_buff(7, 1))
		}
	}
 	
	when (rx_state === rx_stop_bit) {
		when (rx_baud_tick === UFix(1)){
			when (rxd_reg2 === UFix(1)) {
				rx_state 		:= rx_idle
				rx_enable		:= UFix(0)
				rx_data 		:= rx_buff
				rx_full			:= UFix(1)
			}
			.otherwise{
				rx_state 		:= rx_idle
				rx_enable		:= UFix(0)
			}
		}
	}

	io.rd_data := rdDataReg
	io.pins.tx := tx_reg
}
