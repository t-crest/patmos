package mcache

import Chisel._
import Node._

import scala.collection.mutable.HashMap

class TopIO extends Bundle() {
  //val fedec = new FeDec().asOutput
  //val exfe = new ExFe().asInput
  val mcache_in = new MCacheIn().asInput
  val mcache_out = new MCacheOut().asOutput
  val led = Bits(OUTPUT, width = 8)
}

class Top(filename: String) extends Component {
  val io = new TopIO()
  val mcache = new MCache()
  val mcachemem = new MCacheMem()
  val extmemrom = new ExtMemROM(filename)

  //val fetch = new Fetch()
  mcache.io.mcachemem_in <> mcachemem.io.mcachemem_in
  mcache.io.mcachemem_out <> mcachemem.io.mcachemem_out
  mcache.io.extmem_in <> extmemrom.io.extmem_in
  mcache.io.extmem_out <> extmemrom.io.extmem_out

  //connect mcache with top for debugging
  io.mcache_in <> mcache.io.mcache_in
  io.mcache_out <> mcache.io.mcache_out

  //connect to fetch stage
  //mcache.io.mcache_in <> fetch.io.mcache_in
  //mcache.io.mcache_out <> fetch.io.mcache_out

  //connect top with fetch for debugging
  //fetch.io.exfe <> io.exfe
  //fetch.io.fedec <> io.fedec

  //for debugging a led counter
  val led_counter = Reg(resetVal = UFix(0, 32))
  val CNT_MAX = UFix(4)
  val led_output = Reg(resetVal = UFix(0, 1))    
  led_counter := led_counter + UFix(1)
  when (led_counter === CNT_MAX) {
    led_counter := UFix(0)
    led_output := ~led_output
  }
  io.led := led_output

}

/*
 test mcache connected to fetch stage
*/
/*
class TopTests(c: Top) extends Tester(c, Array(c.io)) {
  defTests {
    var allGood = true
    val vars = new HashMap[Node, Node]()
    val ovars = new HashMap[Node, Node]()
    var init = false
    var end_simulation = false

    vars.clear()
    ovars.clear()
    while (end_simulation != true) {
      if (init == false) {
        for (i <- 0 until 4) {
          println("INIT")
        }
        init = true
      }
      else {
        println("EXEC")
        for (i <- 0 until 1000) {
          vars(c.io.exfe.doBranch) = Bits(0)
          vars(c.io.exfe.branchPc) = Bits(0)
          allGood = step(vars, ovars) && allGood
        }
        end_simulation = true
      }    
    }
    allGood
  }
}
 */

/*
 test pattern only for MCache.scala without MC_Fetch.scala
 */
class TopCacheTests(c: Top) extends Tester(c, Array(c.io)) {
  defTests {
    var allGood = true
    val vars = new HashMap[Node, Node]()
    val ovars = new HashMap[Node, Node]()
    var init = false
    var end_simulation = false
    var j = 0
    vars.clear()
    ovars.clear()
    while (end_simulation != true) {
      if (init == false) {
        vars(c.io.mcache_in.address) = Bits("h0000000000000000",32)
        vars(c.io.mcache_in.request) = Bits(0)
        allGood = step(vars, ovars) && allGood
        init = true
      }
      else {
        val pc =  Bits(12)
        vars(c.io.mcache_in.address) = pc
        vars(c.io.mcache_in.request) = Bits(1)
        allGood = step(vars, ovars) && allGood
        for (i <- 0 until 1000) {
          var hit = ovars(c.io.mcache_out.ena).litValue()
          println("HIT:" + ovars(c.io.mcache_out.ena).litValue())
          if (hit == 1) {
            println("ENTERED HIT STATE... fetch next address!")
            j = j + 1
            vars(c.io.mcache_in.address) = (pc + (Bits(j)))
          }
          else {
            println("ENTERED NULL STATE... wait for hit!")
          }
          allGood = step(vars, ovars) && allGood
        }     
        end_simulation = true
      }
    }
    allGood
  }
}

//main function calling the top class for chisel main and test
object TopMain {
  def main(args: Array[String]) : Unit = {
    // Use first argument for the program name (.bin file)
    val chiselArgs = args.slice(1, args.length)
    val file = args(0) + ".bin"
    chiselMainTest(chiselArgs, () => new Top(file)) {
      c => new TopCacheTests(c)
    }
  }
}
