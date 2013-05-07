/*
	RX module
 */



package hello

import Chisel._
import Node._


class RX() extends Component {
  val io = new Bundle {
//    val reset = Bool(dir = INPUT)
	val rx = UFix(INPUT, 1)
	val data_out = UFix(OUTPUT, 8) 
	val out_valid = UFix(OUTPUT, 1)
	val rd = UFix(INPUT, 1)
	val out_t = UFix(OUTPUT, 1)
  }

  	val out_valid_reg = Reg(resetVal = UFix(0, 1))
  	//val tx_reg = Reg(resetVal = UFix(0, 1))
  	val data_counter = Reg(resetVal = UFix(0, 3))
  	
  	val reset_state :: idle  :: receive_data :: stop_bit :: Nil  = Enum(4){ UFix() }
	val state = Reg(resetVal = reset_state)
	
//	val rx_filtered = Reg(resetVal = UFix(0, 1))
//	val rx_state = Reg(resetVal = UFix(3, 2))
	
//	val ticker = Reg(resetVal = UFix(0, 1))
	val data_buffer = Reg(resetVal = UFix(0, 8))
	val baud_counter = Reg(resetVal = UFix(0, 10))
	val baud_tick = Reg(resetVal = UFix(0, 1))
	
	val s_baud_counter = Reg(resetVal = UFix(0, 10))
	val s_baud_tick = Reg(resetVal = UFix(0, 1))
	
	val test = Reg(resetVal = UFix(0, 1))
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
	
	when (baud_tick === UFix(1))
	{
	  test	:= UFix(1)
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
	
	// Sample input
//	when (s_baud_tick === UFix(1))	
//	{
//		when (state === receive_data)
//		{
//			data_buffer := Cat(io.rx, data_buffer(7, 1))
//			data_counter := data_counter + UFix(1)
//		}
//		.otherwise
//		{
//			data_buffer := data_buffer
//			data_counter := UFix(0)
//		}
//	}
//	.otherwise
//	{
//		data_buffer := data_buffer
//	}
	
	// RX state machine
		
		when (state === reset_state) {
			out_valid_reg	:= UFix(0)
			state       	:= idle  
		}
	  	when (state === idle) {
	  		out_valid_reg	:= UFix(0)
	  		when (s_baud_tick === UFix(1)){
	  		when (io.rx === UFix(0))
	  		{
	  		  state := receive_data
	  		}
	  		}
	  	}
	  	
	  
	  	  
		  	when (state === receive_data) {
		  	  
		  		out_valid_reg  := UFix(0)
		  		
		  		when (s_baud_tick === UFix(1)){
		  		  
		  			data_buffer := Cat(io.rx, data_buffer(7, 1))
		  			data_counter := Mux(data_counter === UFix(7), UFix(0), data_counter + UFix(1)) //
		  	  	    state := Mux(data_counter === UFix(7), stop_bit, receive_data)
		  		}
		  	}
		  	when (state === stop_bit) 
		  	{
		  	 	out_valid_reg  := UFix(1)
				when (s_baud_tick === UFix(1)){
			  	 	when (io.rx === UFix(1))		
					{
			  	 	  state := idle
					}
			  	 	.otherwise
			  	 	{
			  	 	  state := stop_bit
			  	 	}
		  	 	}	
		  	}
	io.out_valid := out_valid_reg
//	when (io.rd === UFix(1)){
		  		io.data_out := data_buffer

	
	
	io.out_t := test
}




// Generate the Verlog code by invoking chiselMain() in our main()
//object HelloMain {
//  def main(args: Array[String]): Unit = { 
//    chiselMain( args, () => new RX())
//  }
//}


