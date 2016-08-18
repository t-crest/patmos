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

  //RPT ROM generation
  val trigger_rom = trigger_f()
  val trigger_rom_d = trigger_f()
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
  val small_l_stop_R = Reg(init = Bits(0, width = (TAG_SIZE + INDEX_SIZE))) 	
  val status_R = Vec.fill(MAX_DEPTH){Reg(init = UInt(0, width = MAX_DEPTH_WIDTH))}
  val iteration_outer_R = Vec.fill(MAX_DEPTH){Reg(init = UInt(0, width = MAX_ITERATION_WIDTH))}
  val cache_line_id_address_R = Reg(init = UInt(0, width = (TAG_SIZE + INDEX_SIZE)))
  val output = Reg(init = Bits(0, width = EXTMEM_ADDR_WIDTH))
  val en_seq = Reg(init = Bool(false))
  val change_state = Reg(init = Bool(false))

  // Reset the index when cache is flushed
  when (io.invalidate) {
    index_R := UInt(0)
  }
  
  // Input address selection
  val cache_line_id_address = Mux((pc_address_even != previous_addrs_even_R), pc_address_even, Mux((pc_address_odd != previous_addrs_odd_R), pc_address_odd, cache_line_id_address_R))
  
  when (pc_address_even != previous_addrs_even_R) {
    previous_addrs_even_R := pc_address_even
    cache_line_id_address_R := pc_address_even
  }
  .elsewhen (pc_address_odd != previous_addrs_odd_R) {
    previous_addrs_odd_R := pc_address_odd
      cache_line_id_address_R := pc_address_odd
  }

  // State_machine
  val trigger :: small_loop :: Nil = Enum(UInt(), 2)
  val state = Reg(init = trigger)
  val line_change = ((cache_line_id_address != previous_addrs_R) || en_seq)
  val index_match = (cache_line_id_address === trigger_rom(index_R))
  val call = (type_rom(index_R) === UInt(0))
  val ret = (type_rom(index_R) === UInt(1))
  val cont_loop = (type_rom(index_R) === UInt(3))
  val loops = (type_rom(index_R) === UInt(2))
  val loop_entry = ((status_R(depth_rom(index_R))) === UInt(0))  
  val loop_iteration = ((status_R(depth_rom(index_R))) === UInt(1))  
  val loop_exit = ((status_R(depth_rom(index_R))) === UInt(2))  

  switch (state) {
    is(trigger) {
      when (line_change)  { //trigger prefetching
        previous_addrs_R := cache_line_id_address
        en_seq := Bool(false)
      }
      when (change_state && line_change) {
	  state := small_loop
	  change_state := Bool(false)
      }
      when ((!index_match) && (!change_state) && line_change) { //no matching - next line prefetching
	  output := Cat((cache_line_id_address + UInt(1)), sign_ext_R) 
      }
      //matching with rpt table entry
      when (call && (!change_state) && index_match && line_change)  {  //call type
          output := Cat(destination_rom(index_R), sign_ext_R)
          stackAddrs(sp_R) := retdes_rom(index_R)  
          stackIndex(sp_R) := index_R + UInt(1)
          sp_R := sp_R + UInt(1)
          index_R := next_rom(index_R)
	  en_seq := Mux((trigger_rom(index_R) === destination_rom(index_R)), Bool(true), Bool(false))  // case call and destination are on the same cache line
      }
      when (ret && (!change_state) && index_match && line_change) { // return type
          output := Cat(stackAddrs(sp_R - UInt(1)), sign_ext_R)
	  index_R := Mux((stackAddrs(sp_R - UInt(1)) === UInt(0)), UInt(0), stackIndex(sp_R - UInt(1))) 
	  sp_R := Mux((stackAddrs(sp_R - UInt(1)) === UInt(0)), UInt(1), (sp_R - UInt(1))) 
      }		 
      when (cont_loop && (!change_state) && index_match && line_change) { // small_loop
	  output := Mux((count_rom(index_R) === UInt(0)), Cat(cache_line_id_address, sign_ext_R), Cat((cache_line_id_address + UInt(1)), sign_ext_R))	 	
          index_R := index_R + UInt(1)
	  en_seq := Mux((count_rom(index_R) === UInt(0)), Bool(true), Bool(false))
	  change_state := Mux((count_rom(index_R) > UInt(1)), Bool(true), Bool(false)) 
	  small_l_addr_R := cache_line_id_address + UInt(2) 
          small_l_stop_R := cache_line_id_address + UInt(1)
	  small_l_count_R := count_rom(index_R) - UInt(1)
      } 
      when (loop_entry && loops && (!change_state) && index_match && line_change) { //loop
        //entring first time
        output := Cat(destination_rom(index_R), sign_ext_R)
        index_R := next_rom(index_R)
        status_R(depth_rom(index_R)) := Mux(((iteration_rom(index_R)) === UInt(1)), UInt(2), UInt(1)) 
	iteration_outer_R(depth_rom(index_R)) := iteration_rom(index_R) - UInt(1)
      } 
      when (loop_iteration && loops && (!change_state) && index_match && line_change) { //loop
        // next iteration of the outer loop
	output := Cat(destination_rom(index_R), sign_ext_R)
        index_R := next_rom(index_R)
	iteration_outer_R(depth_rom(index_R)) := iteration_outer_R(depth_rom(index_R)) - UInt(1)
	status_R(depth_rom(index_R)) := Mux((iteration_outer_R(depth_rom(index_R)) === UInt(1)), UInt(2), UInt(1)) // if last iteration, change to exhausted
      }
      when (loop_exit && loops && (!change_state) && index_match && line_change) { //loop
        // loop is already "exhausted"
        status_R(depth_rom(index_R)) := UInt(0) // reset the status
        index_R := index_R + UInt(1)
	when (trigger_rom(index_R) === trigger_rom_d(index_R + UInt(1))) {  //next trigger is on the same cache line
          en_seq := Bool(true)
        }
	.otherwise {
	  output := Cat((cache_line_id_address + UInt(1)), sign_ext_R) 
	}
      }    
    }
    is(small_loop) { //more than one prefetching 
      when (small_l_stop_R === cache_line_id_address) {
        state := trigger
      }
      .elsewhen (prefTrig) {
	state := Mux((small_l_count_R != UInt(0)), small_loop, trigger)    
        output := Cat(small_l_addr_R, sign_ext_R)
        small_l_count_R := small_l_count_R - UInt(1)
	small_l_addr_R := small_l_addr_R + UInt(1)
	state := small_loop
      }
    } 
    io.prefrepl.prefAddr := output
  }
}    
