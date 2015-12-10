package patmos 

import Chisel._
import Node._
import PrefetchCons._
import PIConstants._
import Constants._
import scala.math._

class PFSMDM extends Module {
  val io = new PrefetcherIO()
	
  val pc_address_even = io.feicache.addrEven(TAG_HIGH, INDEX_LOW)
  val pc_address_odd = io.feicache.addrOdd(TAG_HIGH, INDEX_LOW)
  val prefTrig = io.ctrlpref.prefTrig
  val pc_address = io.ctrlpref.ctrlprefAddr

  //RPT ROM generation
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
  val previous_addrs_even_R = Reg(init = Bits(0, width = (TAG_SIZE + INDEX_SIZE))) 
  val previous_addrs_odd_R = Reg(init = Bits(0, width = (TAG_SIZE + INDEX_SIZE))) 
  
  val index_R = Reg(init = UInt(0, width = INDEX_REG_WIDTH))
  val sign_ext_R = Reg(init = Bits(0, width = (EXTMEM_ADDR_WIDTH - TAG_SIZE - INDEX_SIZE))) 
  val sp_R = Reg(init = UInt(1, width = log2Up(MAX_CALLS)))
  val small_l_count_R = Reg(init = UInt(0, width = MAX_SMALL_LOOP_WIDTH))
  val small_l_addr_R = Reg(init = Bits(0, width = (TAG_SIZE + INDEX_SIZE))) 	
  val iteration_inner_R = Reg(init = UInt(0, width = MAX_LOOP_ITER_WIDTH))
  val status_R = Vec.fill(MAX_DEPTH){Reg(init = UInt(0, width = MAX_DEPTH_WIDTH))}
  val iteration_outer_R = Vec.fill(MAX_DEPTH){Reg(init = UInt(0, width = MAX_ITERATION_WIDTH))}

//  val cache_line_id_address = pc_address(TAG_HIGH, INDEX_LOW)
  
  val cache_line_id_address = Reg(init = UInt(0, width = (TAG_SIZE + INDEX_SIZE)))
  val output = Reg(init = Bits(0, width = EXTMEM_ADDR_WIDTH))
  val en_seq = Reg(init = Bool(false))
  val change_state = Reg(init = Bool(false))

  // Reset the index when cache is flushed
  when (io.invalidate) {
    index_R := UInt(0)
  }
  
  // Input address selection
  when (pc_address_even != previous_addrs_even_R) {
    previous_addrs_even_R := pc_address_even
    when (pc_address_even != cache_line_id_address) {
      cache_line_id_address := pc_address_even
    }
  }
  .elsewhen (pc_address_odd != previous_addrs_odd_R) {
    previous_addrs_odd_R := pc_address_odd
    when (pc_address_odd != cache_line_id_address) {
      cache_line_id_address := pc_address_odd
    }
  }

  // State_machine
  val trigger :: small_loop :: Nil = Enum(UInt(), 2)
  val state = Reg(init = trigger)

  switch (state) {
    is(trigger) {
      when ((cache_line_id_address != previous_addrs_R) || en_seq)  { //prefetch
	previous_addrs_R := cache_line_id_address
	en_seq := Bool(false)
	when (change_state) {
	  state := small_loop
	  change_state := Bool(false)
	}
        .elsewhen (cache_line_id_address != trigger_rom(index_R)) { //no matching - next line prefetching
	  output := Cat((cache_line_id_address + UInt(1)), sign_ext_R) 
          state := trigger
        }
        .otherwise { //matching with rpt table entry
          when (type_rom(index_R) === UInt(0)) {  //call type
            output := Cat(destination_rom(index_R), sign_ext_R)
            stackAddrs(sp_R) := retdes_rom(index_R)  
            stackIndex(sp_R) := index_R + UInt(1)
            sp_R := sp_R + UInt(1)
            index_R := next_rom(index_R)
            state := trigger
          }
          .elsewhen (type_rom(index_R) === UInt(1)) { // return type
            state := trigger
            output := Cat(stackAddrs(sp_R - UInt(1)), sign_ext_R)
	    when (stackAddrs(sp_R - UInt(1)) === UInt(0)) {
	      index_R := UInt(0)
	      sp_R := UInt(1)
	    }
	    .otherwise {  
              index_R := stackIndex(sp_R - UInt(1))
              sp_R := sp_R - UInt(1) 
	    }
          }		 
          .elsewhen (type_rom(index_R) === UInt(3)) { // small_loop
	    output := Cat((cache_line_id_address + UInt(1)), sign_ext_R) 	
            index_R := index_R + UInt(1)
            when (count_rom(index_R) > UInt(1)) {
	      small_l_addr_R := cache_line_id_address + UInt(2) 
              small_l_count_R := count_rom(index_R) - UInt(1)
	      change_state := Bool(true)
            }
            .otherwise {
              state := trigger
            }
          } 
          .otherwise { //loop
            state := trigger
            when (status_R(depth_rom(index_R)) === UInt(0)) {//entring first time
              output := Cat(destination_rom(index_R), sign_ext_R)
              index_R := next_rom(index_R)
              when ((iteration_rom(index_R)) === UInt(1)) { // only one iteration
         	status_R(depth_rom(index_R)) := UInt(2) //change status to "exhausted" 
	      }
	      .otherwise {
	      	iteration_outer_R(depth_rom(index_R)) := iteration_rom(index_R) - UInt(1)
                status_R(depth_rom(index_R)) := UInt(1) //change status to "live"
              }
            }  
            .elsewhen (status_R(depth_rom(index_R)) === UInt(1)) {// next iteration of the outer loop
	      output := Cat(destination_rom(index_R), sign_ext_R)
              index_R := next_rom(index_R)
	      iteration_outer_R(depth_rom(index_R)) := iteration_outer_R(depth_rom(index_R)) - UInt(1)
	      when (iteration_outer_R(depth_rom(index_R)) === UInt(1)) { // last iteration
	      	status_R(depth_rom(index_R)) := UInt(2) // change status to "exhausted"
	      }
	    } 
            .elsewhen (status_R(depth_rom(index_R)) === UInt(2)) { // loop is already "exhausted"
              status_R(depth_rom(index_R)) := UInt(0) // reset the status
              index_R := index_R + UInt(1)
	      when (trigger_rom(index_R) === trigger_rom(index_R + UInt(1))) {  //next trigger is on the same cache line
                en_seq := Bool(true)
              }
	      .otherwise {
	          output := Cat((cache_line_id_address + UInt(1)), sign_ext_R) 	
              }
	    } 
	  }
        }  
      }  
    }
    is(small_loop) { //more than one prefetching 
      when (small_l_addr_R === cache_line_id_address) {
        state := trigger
      }
      .elsewhen (prefTrig) {
        when(small_l_count_R > UInt(0)) {
          output := Cat(small_l_addr_R, sign_ext_R)
          small_l_count_R := small_l_count_R - UInt(1)
	  small_l_addr_R := small_l_addr_R + UInt(1)
	  state := small_loop
        }
        .elsewhen (small_l_count_R === UInt(0)) {
      	  state := trigger
	}
      }
    } 
    io.prefrepl.prefAddr := output
  }
}    
