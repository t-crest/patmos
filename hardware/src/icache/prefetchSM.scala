package patmos 

import Chisel._
import Node._
import PrefetchCons2._
import PIConstants2._
import Constants._
import scala.math._

class PFSM extends Module {
  val io = new Prefetcher2IO()
	
  val pc_address = Bits(INPUT, width = EXTMEM_ADDR_WIDTH)
  pc_address := io.feicache.addrEven

  //RPT ROM generation
  val index_rom = index_f()
  val trigger_rom = trigger_f()
  val type_rom = type_f()
  val destination_rom = destination_f() 
  val iteration_rom = iteration_f()
  val next_rom = next_f()
  val count_rom = count_f()
  val depth_rom = depth_f()
  val retdes_rom = retdes_f() 
  
  //Stack for CALL and RETURN
  val stackAddrs = Mem(Bits(width = (TAG_SIZE + INDEX_SIZE)), MAX_CALLS) 
  val stackIndex = Mem(Bits(width = (TAG_SIZE + INDEX_SIZE)), MAX_CALLS)

  // Registers generation
  val previous_addrs_R = Reg(init = Bits(0, width = (TAG_SIZE + INDEX_SIZE))) 
  val index_R = Reg(init = UInt(0, width = INDEX_REG_WIDTH))
  val sign_ext_R = Reg(init = Bits(0, width = (EXTMEM_ADDR_WIDTH - TAG_SIZE - INDEX_SIZE))) 
  val sp_R = Reg(init = UInt(0, width = log2Up(MAX_CALLS)))
  val small_l_count_R = Reg(init = UInt(0, width = MAX_SMALL_LOOP_WIDTH))
  val small_l_addr_R = Reg(init = Bits(0, width = (TAG_SIZE + INDEX_SIZE))) 	
  val iteration_inner_R = Reg(init = UInt(0, width = MAX_LOOP_ITER_WIDTH))
  val status_R = Vec.fill(MAX_DEPTH){Reg(init = UInt(0, width = MAX_DEPTH_WIDTH))}
  val iteration_outer_R = Vec.fill(MAX_DEPTH){Reg(init = UInt(0, width = MAX_ITERATION_WIDTH))}
  val cache_line_id_address = pc_address(TAG_HIGH, INDEX_LOW)
  val output = Reg(init = Bits(0, width = EXTMEM_ADDR_WIDTH))
  val en_pr = Reg(init = Bool(false))
  val en_seq = Reg(init = Bool(true))
 
  // Reset the index when cache is flushed
  when (io.invalidate) {
    index_R := UInt(0)
  }

  // State_machine
  val trigger :: small_loop :: Nil = Enum(UInt(), 2)
  val state = Reg(init = trigger)

  switch (state) {
    is(trigger) {
      when ((cache_line_id_address === previous_addrs_R) && en_seq && io.ena_in) { //the next prefetching is done for this cache line
        en_pr := Bool(false)
        state := trigger 
      }
      .elsewhen (io.ena_in) { //prefetch
        previous_addrs_R := cache_line_id_address
        en_seq := Bool(true)
        en_pr := Bool(true)
        when (cache_line_id_address != trigger_rom(index_R)) { //no matching - next line prefetching
          output := Cat((cache_line_id_address + Bits(1)), sign_ext_R)
          state := trigger
        }
        .otherwise { //matching with rpt table entry
          when (type_rom(index_R) === UInt(0)) {  //call type
            output := Cat(destination_rom(index_R), sign_ext_R).toBits
            stackAddrs(sp_R) := retdes_rom(index_R)  
            stackIndex(sp_R) := index_R + UInt(1)
            sp_R := sp_R + UInt(1)
            index_R := next_rom(index_R)
            state := trigger
          }
          .elsewhen (type_rom(index_R) === UInt(1)) { // return type
            output := Cat(stackAddrs(sp_R - UInt(1)), sign_ext_R).toUInt
            index_R := stackIndex(sp_R - UInt(1))
            sp_R := sp_R - UInt(1) 
            state := trigger
          } 
          .elsewhen (type_rom(index_R) === UInt(3)) { // small_loop
            output := Cat((cache_line_id_address + UInt(1)), sign_ext_R).toBits
            index_R := index_R + UInt(1)
            when (count_rom(index_R) > UInt(1)) {
              small_l_addr_R := cache_line_id_address + UInt(2) 
              small_l_count_R := count_rom(index_R) - UInt(1)
              state := small_loop
            }
            .otherwise {
              state := trigger
            }
          } 
          .otherwise { //loop
            when (index_R === next_rom(index_R)) { //inner loop
              output := Cat(destination_rom(index_R), sign_ext_R).toBits
              when (iteration_inner_R === UInt(0)) { //first entry in the inner loop
                when(iteration_rom(index_R) ===  UInt(1)) { //number of iteration is one
                  index_R := index_R + UInt(1)
                  state := trigger
                }  
                .otherwise {
                  iteration_inner_R := iteration_rom(index_R) - UInt(1)
                  state := trigger	
                }
              } 
              .elsewhen (iteration_inner_R != UInt(0)) { //next entry in the inner loop
                when(iteration_inner_R === UInt(1)) {
                  iteration_inner_R := UInt(0)
                  index_R := index_R + UInt(1)	
                  state := trigger
                }
                .otherwise {
                  iteration_inner_R := iteration_inner_R - UInt(1)
                  state := trigger
                }
              }		
            }		
            .otherwise {  //outer loop
              when (status_R(depth_rom(index_R)) === UInt(0)) {//entring first time
                output := Cat(destination_rom(index_R), sign_ext_R).toBits
                when (iteration_outer_R(depth_rom(index_R)) === UInt(1)) { // only one iteration
	          status_R(depth_rom(index_R)) := UInt(2) //change status to "exhausted" 
                  index_R := next_rom(index_R)
	      	  state := trigger
	      	}
	      	.otherwise {
	      	  iteration_outer_R(depth_rom(index_R)) := iteration_rom(index_R) - UInt(1)
                  status_R(depth_rom(index_R)) := UInt(1) //change status to "live"
		  index_R := next_rom(index_R)
		  state := trigger
                }
             }
             .elsewhen (status_R(depth_rom(index_R)) === UInt(1)) {// next iteration of the outer loop
	      	output := Cat(destination_rom(index_R), sign_ext_R).toBits
                iteration_outer_R(depth_rom(index_R)) := iteration_outer_R(depth_rom(index_R)) - UInt(1)	
	      	when (iteration_outer_R(depth_rom(index_R)) === UInt(1)) { // last iteration
	      	  status_R(depth_rom(index_R)) := UInt(2) // change status to "exhausted"
	      	  iteration_outer_R(depth_rom(index_R)) := UInt(0)
                  index_R := next_rom(index_R)
	      	  state := trigger
	        }
	  	.otherwise {
	     	  iteration_outer_R(depth_rom(index_R)) := iteration_outer_R(depth_rom(index_R)) - UInt(1)
                  index_R := next_rom(index_R)
	     	  state := trigger
	        }
	      } 
              .elsewhen (status_R(depth_rom(index_R)) === UInt(2)) { // loop is already "exhausted"
                 status_R(depth_rom(index_R)) := UInt(0) // reset the status
	      	 when (trigger_rom(index_R) === trigger_rom(index_R + UInt(1))) {  //next trigger is on the same cache line
                   en_seq := Bool(false)
                 }
	       	 en_pr := Bool(false)
                 index_R := index_R + UInt(1)
      	         state := trigger
	      }
	    }
	  }
        }   
      }  
    }
    is(small_loop) { //more than one prefetching 
      output := Cat(small_l_addr_R, sign_ext_R)
      en_pr := Bool(true)
      when(small_l_count_R > UInt(1)) {
        small_l_count_R := small_l_count_R - UInt(1)
	small_l_addr_R := small_l_addr_R + UInt(1)
	state := small_loop
      }
      .otherwise {
        small_l_count_R := UInt(0)
        previous_addrs_R := cache_line_id_address
	state := trigger 
     }
  } 
  io.prefrepl.prefAddr := output
  io.prefrepl.pref_en := en_pr
  }
}

//class PFSMTest(c:PFSM) extends Tester(c)
//{
//  for(pc_addr <- 10490 until 10498) { 
//    poke(c.io.pc_address, pc_addr << 4)
//    step(1)
//    if (pc_addr == 10491) {
//	    expect(c.io.prefetch_address, 23302 << 4)
//    } else if (pc_addr == 10495) {
//      expect(c.io.prefetch_address, 23510 << 4)
//    } else {
//    expect(c.io.prefetch_address, (pc_addr + 1) << 4)
//    }
//  }
//}

//object test1 {
//  def main(args:Array[String]):Unit = {
//    chiselMainTest(args, () => Module(new PFSM())) {
//      c => new PFSMTest(c)
//    }
//  }
//} 			
