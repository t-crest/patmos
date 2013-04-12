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
  	val data_counter = Reg(resetVal = UFix(0, 4))
 // 	val baud_counter = Reg(resetVal = UFix(0))
  	
  	
  	val reset_state :: idle :: send:: Nil  = Enum(3){ UFix() } 
	val state = Reg(resetVal = reset_state)
	val baud_counter = Reg(resetVal = UFix(0, 10))
	val baud_tick = Reg(resetVal = UFix(0, 1))
	

	val data = Reg(resetVal = UFix(0, 10))//io.data_in

	
	
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
when (baud_tick === UFix(1)){	
	when (state === reset_state) {
		accept_reg	:= UFix(0)
		tx_reg      := UFix(1)
		state       := idle  
	}
  	when (state === idle) {
  		accept_reg	:= UFix(1)
  		tx_reg		:= UFix(1)
  		
  		when (accept_reg === UFix(1) && io.in_valid === UFix(1)){
  		  state       := send
  		  data := Cat (UFix(1), io.data_in, UFix(0))
  		  }
 		when (accept_reg === UFix(0) || io.in_valid === UFix(0)){
 		  state       := idle
 		  }
  	}
  	
  	when (state === send)
  	{
  	  
  	  accept_reg  := UFix(0)
  	  tx_reg	  := data(0)
  	  data := Cat (UFix(0), data (9, 1))
  	  when (data_counter === UFix(9))
  	  {
  	    accept_reg := UFix(0)
  	    state := reset_state
  	  }
  	  .otherwise {state := send}
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

