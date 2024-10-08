package twoway

import chisel3._
import s4noc_twoway._

class TwoWayBoard(n: Int, memSize: Int) extends Module {


    val io = new Bundle{
        val led = Output(Vec(n*n,Bool()))
    }

    //Generate network
    val network = Module(new TwoWayMem(n, memSize))

    //Generate processing nodes
    val nodearray = VecInit {
      for (i <- 0 until n * n) yield {
        val node = Module(new NodeRead(n, i, memSize))
        // any wiring or other logic can go here
        node.io
      }
    }

    //Connect nodes to network
    for(i <- 0 until n*n){
        network.io.nodearray(i) <> nodearray(i).local
    }

    //Connect leds to nodes
    for(i <- 0 until n*n){
        io.led(i) := nodearray(i).led
    }


}


object TwoWayBoard {
  def main(args: Array[String]): Unit = {
    emitVerilog(new TwoWayBoard(2, 1024), Array("-td", "generated"))
  }
}


/*commented out Chisel3 tester has changed see https://github.com/schoeberl/chisel-examples/blob/master/TowardsChisel3.md 
class TestBoard(dut: TwoWayBoard) extends Tester(dut) {
    step(100000)
}

object TwoWayBoardTester {
  def main(args: Array[String]): Unit = {
    chiselMainTest(Array("--genHarness", "--test", "--backend", "c",
      "--compile", "--vcd", "--targetDir", "generated","--debug"),
      () => Module(new TwoWayBoard(2, 1024))) {
        c => new TestBoard(c)
      }
  }
}*/