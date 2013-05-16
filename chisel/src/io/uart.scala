/*
	uart module
 */



package hello

import Chisel._
import Node._


class UART() extends Component {
  val io = new Bundle {
    val address = UFix(INPUT, 1)
	val data_in = UFix(INPUT, 8)
	val wr = UFix(INPUT, 1) 
	val rd = UFix(INPUT, 1)
	val tx = UFix(OUTPUT, 1)
	val rx = UFix(INPUT, 1)
	val rd_data = UFix(OUTPUT, 8)
  }

  
 	val accept_reg = Reg(resetVal = UFix(0, 1))
 	val r_data = Reg(resetVal = UFix(0, 1))
  	val tx_reg = Reg(resetVal = UFix(0, 1))
  	val data_counter = Reg(resetVal = UFix(0, 4))  	
  	val reset_state :: idle :: send:: Nil  = Enum(3){ UFix() } 
	val state = Reg(resetVal = reset_state)
	val baud_counter = Reg(resetVal = UFix(0, 10))
	val baud_tick = Reg(resetVal = UFix(0, 1))
	val data = Reg(resetVal = UFix(0, 10))//io.data_in

	//*****************//
	val data_buffer = Reg(resetVal = UFix(0, 8))	
	val s_baud_counter = Reg(resetVal = UFix(0, 10))
	val s_baud_tick = Reg(resetVal = UFix(0, 1))
  	val r_data_counter = Reg(resetVal = UFix(0, 3))
  	
  	val r_reset_state :: r_idle  :: r_receive_data :: r_stop_bit :: Nil  = Enum(4){ UFix() }
	val r_state = Reg(resetVal = r_reset_state)
	
//	data_buffer := UFix(0)
	
	
	
	// address decoding
	io.rd_data := Mux(io.address === UFix(0), Cat(UFix(0, width = 6), r_data, accept_reg), data_buffer)
	
	
	
	
	// Baud generator
	when (baud_counter === (UFix(50000000) / UFix(115200)))
	{
	  baud_counter := UFix(0)
	  baud_tick := UFix(1)
	}
	.otherwise
	{
	  baud_counter := baud_counter + UFix(1)
	  baud_tick := UFix(0)
	}

    // high speed baud counter
	when (s_baud_counter === Bits("b0011011001"))//(UFix(50000000) / UFix(115200)) / UFix(2))
	{	 
	  s_baud_tick := UFix(1)
	  s_baud_counter := s_baud_counter + UFix(1)  
	}
	.otherwise
	{
	  when (s_baud_counter === (UFix(50000000) / UFix(115200)))
	  {
	    s_baud_counter := UFix(0)
	  }
	  .otherwise 
	  {
	    s_baud_counter := s_baud_counter + UFix(1)
	  }
	  s_baud_tick := UFix(0)
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
		  	  
			  	  tx_reg	  := data(0)
			  	  data := Cat (UFix(0), data (9, 1))
			  	  data_counter := Mux(data_counter === UFix(10), UFix(0), data_counter + UFix(1)) //
		  	  	  state := Mux(data_counter === UFix(10), idle, send)	
	
		  	}
	  	}
  	
	  	
   // RX state machine	  	
	when (r_state === r_reset_state) {
		//	out_valid_reg	:= UFix(0)
			r_state       	:= r_idle  
			r_data  := UFix(0)
		}
	when (r_state === r_idle) {
	  		//out_valid_reg	:= UFix(0)
	  		when (s_baud_tick === UFix(1)){
		  		when (io.rx === UFix(0) && io.rd === UFix(1))
		  		{
		  		  r_state := r_receive_data
		  		}
	  		}
	  	}
	when (r_state === r_receive_data) {
		  	  
		  		//out_valid_reg  := UFix(0)
		  		
		  		when (s_baud_tick === UFix(1)){
		  		  
		  			data_buffer := Cat(io.rx, data_buffer(7, 1))
		  			r_data_counter := Mux(data_counter === UFix(7), UFix(0), data_counter + UFix(1)) //
		  	  	    r_state := Mux(data_counter === UFix(7), r_stop_bit, r_receive_data)
		  		}
		  	}
	when (r_state === r_stop_bit) 
		  	{
		  	 //	out_valid_reg  := UFix(1)
				when (s_baud_tick === UFix(1)){
			  	 	when (io.rx === UFix(1))		
					{
			  	 	  r_state := r_idle
			  	 	  r_data  := UFix(1)
					}
			  	 	.otherwise
			  	 	{
			  	 	  r_state := r_stop_bit
			  	 	}
		  	 	}	
		  	}
	
	  	
	  	
	io.rd_data := data_buffer  	
	io.tx := tx_reg

}
  



// Generate the Verlog code by invoking chiselMain() in our main()
//object HelloMain {
//  def main(args: Array[String]): Unit = { 
//    chiselMain( args, () => new TX())
//  }
//}


