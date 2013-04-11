/*
	TX module
 */



package hello

import Chisel._
import Node._


class TX() extends Component {
  val io = new Bundle {
//    val reset = Bool(dir = INPUT)
	val data_in = UFix(INPUT, 8)
	val in_valid = UFix(INPUT, 1) 
	val tx = UFix(OUTPUT, 1)
	val accept_in = UFix(OUTPUT, 1)
  }

  	val accept_reg = Reg(resetVal = UFix(0, 1))
  	val tx_reg = Reg(resetVal = UFix(0, 1))
  	val data_counter = Reg(resetVal = UFix(0, 3))
 // 	val baud_counter = Reg(resetVal = UFix(0))
  	
  	
  	val reset_state :: idle :: start_bit :: send_data :: stop_bit :: Nil  = Enum(5){ UFix() } 
	val state = Reg(resetVal = reset_state)
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
	
  // Change state on baud tick
	when (baud_tick === UFix(1))
	{
	  when (state === send_data)
	  {
	    data_counter := data_counter + UFix(1)
	  }
	  .otherwise
	  {
	    data_counter := UFix(0)
	  }
	}
	
    // TX state machine
when (baud_tick === UFix(1)){	
	when (state === reset_state) {
		accept_reg	:= UFix(0)
		tx_reg      := UFix(1)
		state       := idle  
	}
  	when (state === idle) {
  		accept_reg	:= UFix(1)
  		tx_reg		:= UFix(1)
  		
  		when (io.in_valid === UFix(1)){
  		  state       := start_bit
  		  }
 		when (accept_reg === UFix(0)){
 		  state       := idle
 		  }
  	}
  	when (state === start_bit) {
  		accept_reg  := UFix(0)
  		tx_reg		:= UFix(0)
  		state       := send_data
  	}
  	
  	when (state === send_data) 
  	{
  	  accept_reg := UFix(0)
  	  tx_reg := io.data_in(data_counter)
  	  
  	  when (data_counter === UFix(7))
  	  {
  	    state := stop_bit
  	  }
  	  .otherwise { state := send_data}
  	}
  	when (state === stop_bit)
  	{
  	  accept_reg := UFix(0)
  	  tx_reg     := UFix(1)
  	  state      := reset_state
  	}
  	

}
//	io.rdy := (state === s_ok)
	io.tx := tx_reg
	io.accept_in := accept_reg
}
  



// Generate the Verlog code by invoking chiselMain() in our main()
//object HelloMain {
//  def main(args: Array[String]): Unit = { 
//    chiselMain( args, () => new TX())
//  }
//}
