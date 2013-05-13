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
  
  	val CNT_MAX = UFix(50000000/2-1);
  	val r1 = Reg(resetVal = UFix(0, 25))
  	val blk = Reg(resetVal = UFix(0, 1))
  
//  	val rx = new RX()
  	val uart = new UART()
  	
 
 	val wr_data = Mux(blk === UFix(0), UFix(40), UFix(41))
 	
	uart.io.tx <> io.tx
	uart.io.rx <> io.rx
	uart.io.data_in := uart.io.rd_data //wr_data //

//	rx.io.rx  <> io.rx //:=blk//

	
 	r1 := r1 + UFix(1)
	when (r1 === CNT_MAX) {
  		 r1 := UFix(0)
  		 blk := ~blk
  		
	}
	
	
  	io.led := blk;

	uart.io.wr := blk//UFix(1); 
	uart.io.rd := blk
}
  


// Generate the Verlog code by invoking chiselMain() in our main()
object HelloMain {
  def main(args: Array[String]): Unit = { 
    chiselMain( args, () => new Test())
  }
}


