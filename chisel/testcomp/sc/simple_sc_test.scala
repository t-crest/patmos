/*
   Copyright 2013 Technical University of Denmark, DTU Compute. 
   All rights reserved.
   
   This file is part of the time-predictable VLIW processor Patmos.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

      1. Redistributions of source code must retain the above copyright notice,
         this list of conditions and the following disclaimer.

      2. Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
   NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   The views and conclusions contained in the software and documentation are
   those of the authors and should not be interpreted as representing official
   policies, either expressed or implied, of the copyright holder.
 */

/*
 * Test for simple stack cache with OCP
 * 
 * Author: Sahar Abbaspour (sabb@dtu.dk)
 * 
 */


package patmos

import Chisel._
import Node._



import scala.math
import ocp._

class StackCacheTest() extends Component {
    val io = new Bundle {
    val led = Bits(OUTPUT, 1)
  }
    
	val sc_init			= Reg(resetVal = UFix(1, 1))
	val mm = new MainMem(512, 4)
	
    val sc_simple = new StackCache(256, 512, 4)
    
	mm.io.mmInOut.M.Cmd := sc_simple.io.scMemInOut.M.Cmd
	mm.io.mmInOut.M.Addr := sc_simple.io.scMemInOut.M.Addr
	mm.io.mmInOut.M.Data := sc_simple.io.scMemInOut.M.Data
	mm.io.mmInOut.M.DataByteEn := sc_simple.io.scMemInOut.M.DataByteEn
	mm.io.mmInOut.M.DataValid := sc_simple.io.scMemInOut.M.DataValid
	
	
	
    sc_simple.io.scCpuInOut.M.Addr := UFix(0)
	sc_simple.io.scCpuInOut.M.Data := UFix(0)
    sc_simple.io.scCpuInOut.M.ByteEn := Bits(0)
    sc_simple.io.scCpuInOut.M.Cmd := OcpCmd.IDLE
    
    sc_simple.io.scMemInOut.S.Resp := mm.io.mmInOut.S.Resp
	sc_simple.io.scMemInOut.S.Data := mm.io.mmInOut.S.Data
	sc_simple.io.scMemInOut.S.DataAccept := mm.io.mmInOut.S.DataAccept
	sc_simple.io.scMemInOut.S.CmdAccept := mm.io.mmInOut.S.CmdAccept
	
	val mem_delay = Reg(resetVal = UFix(0, 1))
    val mem_delay_cnt = Reg(resetVal = UFix(0, 3))
    
    io.led := sc_simple.io.scCpuInOut.S.Data(0)
    
    when (sc_simple.io.scMemInOut.M.Cmd === OcpCmd.RD) {
    	mem_delay := UFix(1)
    }
    
    when (mem_delay === UFix(1)) {
    	mem_delay_cnt := mem_delay_cnt + UFix(1)
    //	mem_delay := UFix(0)
    }
    
    when (mem_delay_cnt === UFix(7)) {
    	io.led := UFix(1)
    //	sc_simple.io.scMemInOut.S.Resp := OcpResp.DVA
    //	mem_delay_cnt := UFix(0)
    }


    
    val sc_ex 		= new SC_ex(256)
    
    sc_simple.io.spill	<> sc_ex.io.spill
	sc_simple.io.fill <> sc_ex.io.fill
	sc_simple.io.m_top  <> sc_ex.io.m_top
	sc_simple.io.sc_top  <> sc_ex.io.sc_top	
	sc_ex.io.n_spill <> sc_simple.io.n_spill
	sc_ex.io.n_fill <> sc_simple.io.n_fill
	
	sc_ex.io.free <> sc_simple.io.free

    
    val func_gen	= Reg(resetVal = UFix(0, 5))
    
    sc_ex.io.stall <> sc_simple.io.stall
    
    when (sc_simple.io.stall === UFix(0) && sc_init === UFix(0) ) {
	  func_gen 		:= func_gen + UFix(1)
	}
    
	
	sc_ex.io.sc_func_type		:= UFix(3) //none
	sc_ex.io.imm				:= UFix(256) //none
    
	val count	= Reg(resetVal = UFix(0, 4))
	count := count + UFix(1)
	when (count <= UFix(10) && sc_init === UFix(1)) { // store to stack cache
		sc_simple.io.scCpuInOut.M.Addr := UFix(511) - count
		sc_simple.io.scCpuInOut.M.Data := UFix(511) - count
		sc_simple.io.scCpuInOut.M.ByteEn := Bits(15)
		sc_simple.io.scCpuInOut.M.Cmd := OcpCmd.WRNP
	}
	.otherwise {
		sc_init := UFix(0)
	}


      when (func_gen === UFix(1)){ // first reserve
		  sc_ex.io.sc_func_type 	:= UFix(0)
		  sc_ex.io.imm				:= UFix(256)
		}
		
      
      when (func_gen === UFix(2)){ // second reserve, spill
		  sc_ex.io.sc_func_type 	:= UFix(0)
		  sc_ex.io.imm				:= UFix(10)
      }
		
      when (func_gen === UFix(3)){ // store to stack cache
		  sc_ex.io.sc_func_type 	:= UFix(3)
		  sc_ex.io.imm				:= UFix(10)
		  sc_simple.io.scCpuInOut.M.Addr := UFix(503)
		  sc_simple.io.scCpuInOut.M.Data := UFix(305)
		  sc_simple.io.scCpuInOut.M.ByteEn := Bits(15)
		  sc_simple.io.scCpuInOut.M.Cmd := OcpCmd.WRNP
      }

      when (func_gen === UFix(4)) { // wait
    	  sc_simple.io.scCpuInOut.M.ByteEn := Bits(0)
    	  sc_simple.io.scCpuInOut.M.Cmd := OcpCmd.IDLE
      }

      when (func_gen === UFix(5)) { // load from stack cache
    	  sc_simple.io.scCpuInOut.M.Addr := UFix(503)
    	  sc_simple.io.scCpuInOut.M.Cmd := OcpCmd.RD
      }
	
	
      when (func_gen === UFix(6)){ // free 10 blocks 
    	  sc_simple.io.scCpuInOut.M.Cmd := OcpCmd.IDLE
    	  sc_ex.io.sc_func_type 	:= UFix(2)
		  sc_ex.io.imm				:= UFix(10)

      }
	
	  when (func_gen === UFix(7)){ // ensure 256 blocks
		  sc_simple.io.scCpuInOut.M.Cmd := OcpCmd.IDLE
		  
		  sc_ex.io.sc_func_type 	:= UFix(1)
		  sc_ex.io.imm				:= UFix(256)
	  } 
	
	  when (func_gen === UFix(8)) { // load from stack cache
    	  sc_simple.io.scCpuInOut.M.Addr := UFix(503)
    	  sc_simple.io.scCpuInOut.M.Cmd := OcpCmd.RD
      }
	
	
    
    
    
}
  
object stackMain {
  def main(args: Array[String]): Unit = { 
    chiselMain( args, () => new StackCacheTest())
  }
}


