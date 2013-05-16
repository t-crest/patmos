/*
	Test UART module
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
  
  	val wr = Reg(resetVal = UFix(0, 1))
  	val rd = Reg(resetVal = UFix(0, 1))
 	val wr_data = Reg(resetVal = UFix(0, 8))
// 	val rd_data = Reg(resetVal = UFix(0, 8))
  	val address = Reg(resetVal = UFix(0, 1))
  	val read_st :: wait_st :: write_st:: Nil  = Enum(3){ UFix() } 
	val state = Reg(resetVal = read_st)
  
  	val CNT_MAX = UFix(50000000/2-1);
  	val r1 = Reg(resetVal = UFix(0, 25))
  	val blk = Reg(resetVal = UFix(0, 1))
  
//  	val rx = new RX()
  	val uart = new UART()
  	
 
// 	val wr_data = Mux(blk === UFix(0), UFix(40), UFix(41))
 	
	uart.io.tx <> io.tx
	uart.io.rx <> io.rx
	uart.io.data_in := wr_data// uart.io.rd_data //wr_data //

//	rx.io.rx  <> io.rx //:=blk//

	
 	r1 := r1 + UFix(1)
	when (r1 === CNT_MAX) {
  		 r1 := UFix(0)
  		 blk := ~blk
  		
	}
	
	when (state === read_st) {
	  wr 		:= UFix(0)
//	  wr_data 	:= UFix(0)
	  rd 		:= UFix(1)
	  address 	:= UFix(0)
	  state		:= wait_st
	}
	
	when (state === wait_st){
	  when (uart.io.rd_data === UFix(1)){
	  	state := write_st
	  	rd 	  := UFix(0)
	  }
	  .otherwise {
	  	state := wait_st
	  }
	}
	
	when (state === write_st){
	  wr 		:= UFix(1)
	  address 	:= UFix(1)
	  wr_data 	:= Mux(blk === UFix(0), UFix(40), UFix(41))
	  state 	:= read_st
	}
	
  	io.led := blk;

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


