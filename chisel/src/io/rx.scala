/*
	TX module
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
  }

  	val out_valid_reg = Reg(resetVal = UFix(0, 1))
  	//val tx_reg = Reg(resetVal = UFix(0, 1))
  	val data_counter = Reg(resetVal = UFix(0, 3))
  	
  	val reset_state :: idle :: start_bit :: receive_data :: stop_bit :: Nil  = Enum(5){ UFix() }
	val state = Reg(resetVal = reset_state)
	
//	val rx_filtered = Reg(resetVal = UFix(0, 1))
//	val rx_state = Reg(resetVal = UFix(3, 2))
	
//	val ticker = Reg(resetVal = UFix(0, 4))
	val data_buffer = Reg(resetVal = UFix(0, 8))
	val baud_counter = Reg(resetVal = UFix(0, 10))
	val baud_tick = Reg(resetVal = UFix(0, 1))
	
	
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
	
	
	
	// Sample input
	when (baud_tick === UFix(1))	
	{
		when (state === receive_data)
		{
			data_buffer := Cat(io.rx, data_buffer(7, 1))
			data_counter := data_counter + UFix(1)
		}
		.otherwise
		{
			data_buffer := data_buffer
			data_counter := UFix(0)
		}
	}
	.otherwise
	{
		data_buffer := data_buffer
	}
	

	// RX state machine
	when (baud_tick === UFix(1)){		
		when (state === reset_state) {
			out_valid_reg	:= UFix(0)
			state       	:= idle  
		}
	  	when (state === idle) {
	  		out_valid_reg	:= UFix(0)
	  		when (io.rx === UFix(0))
	  		{
	  		  state := receive_data
	  		}
	  		when (io.rx === UFix(1)) 
	  		{ 
	  			state := idle 
	  		}
	  	}
	  	
	  	when (state === receive_data) {
	  		out_valid_reg  := UFix(0)
	  		when (data_counter === UFix(7))
	  		{ 
	  		  state := stop_bit
	  		}
	  		.otherwise {state := receive_data}
	  	}
	  	
	  	when (state === stop_bit) 
	  	{
	  	 	out_valid_reg  := UFix(1)
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
	io.data_out := data_buffer
}




// Generate the Verlog code by invoking chiselMain() in our main()
/*object HelloMain {
  def main(args: Array[String]): Unit = { 
    chiselMain( args, () => new tx())
  }
}*/

