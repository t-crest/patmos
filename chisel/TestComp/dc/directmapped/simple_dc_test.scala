/*
   Copyright 2013 Technical University of Denmark, DTU Compute. 
   All rights reserved.
   
   This file is part of the time-predictable VLIW processor Patmos.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

      1. Redistributions of source code must retain the above copyright notice,
         this list of conditions and the following didclaimer.

      2. Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following didclaimer in the
         documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DIdcLAIMED. IN
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
 * Test Direct Mapped cache 
 * 
 * Author: Sahar Abbaspour (sabb@dtu.dk)
 * 
 */


package patmos

import Chisel._
import Node._



import scala.math
import ocp._

class DirectMappedCacheTest() extends Component {
    val io = new Bundle {
    val led = Bits(OUTPUT, 1)
  }

    val dc_simple = new DirectMappedCache(1024, 512, 4)
	val idle :: write1 :: write2 :: wait_st :: read_hit :: read_miss :: end :: Nil  = Enum(7){ UFix() } 
	val state = Reg(resetVal = idle)
	
	val mm = new MainMem(512, 4)
	
	mm.io.mmInOut.M.Cmd := dc_simple.io.dcMemInOut.M.Cmd
	mm.io.mmInOut.M.Addr := dc_simple.io.dcMemInOut.M.Addr
	mm.io.mmInOut.M.Data := dc_simple.io.dcMemInOut.M.Data
	mm.io.mmInOut.M.DataByteEn := dc_simple.io.dcMemInOut.M.DataByteEn
	mm.io.mmInOut.M.DataValid := dc_simple.io.dcMemInOut.M.DataValid
	
	dc_simple.io.dcMemInOut.S.Resp := mm.io.mmInOut.S.Resp
	dc_simple.io.dcMemInOut.S.Data := mm.io.mmInOut.S.Data
	dc_simple.io.dcMemInOut.S.DataAccept := mm.io.mmInOut.S.DataAccept
	dc_simple.io.dcMemInOut.S.CmdAccept := mm.io.mmInOut.S.CmdAccept
	
	//	dc_simple.io.dcMemInOut.S.Resp := OcpResp.NULL  

	
	
	dc_simple.io.dcCpuInOut.M.Addr := UFix(0)
	dc_simple.io.dcCpuInOut.M.Data := UFix(0)
    dc_simple.io.dcCpuInOut.M.ByteEn := Bits(0)
    dc_simple.io.dcCpuInOut.M.Cmd := OcpCmd.IDLE

	when (state === idle) {
		dc_simple.io.dcCpuInOut.M.Addr := UFix(0)
		dc_simple.io.dcCpuInOut.M.Data := UFix(0)
    	dc_simple.io.dcCpuInOut.M.ByteEn := Bits(0)
    	dc_simple.io.dcCpuInOut.M.Cmd := OcpCmd.IDLE
		state 		:= write1
	}
    
    // test write
	when (state === write1) { //write miss
		dc_simple.io.dcCpuInOut.M.Addr := UFix(100)
		dc_simple.io.dcCpuInOut.M.Data := UFix(100)
		dc_simple.io.dcCpuInOut.M.ByteEn := Bits(15)
		dc_simple.io.dcCpuInOut.M.Cmd := OcpCmd.WRNP
		
		when (dc_simple.io.stall === UFix(0)) { state := write2 }
	}
    
	when (state === write2) {
	    dc_simple.io.dcCpuInOut.M.Addr := UFix(110)
		dc_simple.io.dcCpuInOut.M.Data := UFix(110)
		dc_simple.io.dcCpuInOut.M.ByteEn := Bits(15)
		dc_simple.io.dcCpuInOut.M.Cmd := OcpCmd.WRNP
		when (dc_simple.io.stall === UFix(0)) { state := read_hit }
	}
//    
	when (state === read_hit) {
	    dc_simple.io.dcCpuInOut.M.Addr := UFix(100)
		dc_simple.io.dcCpuInOut.M.Cmd := OcpCmd.RD
		when (dc_simple.io.dcCpuInOut.S.Resp != OcpResp.NULL) { state := wait_st }
	}
	
	when (state === wait_st) { // random wait state
		dc_simple.io.dcCpuInOut.M.Addr := UFix(0)
		dc_simple.io.dcCpuInOut.M.Data := UFix(0)
	    dc_simple.io.dcCpuInOut.M.ByteEn := Bits(0)
	    dc_simple.io.dcCpuInOut.M.Cmd := OcpCmd.IDLE
	    state := read_miss
	}
	
	when (state === read_miss) {
		
	}
//	
//	
//    io.led := dc_simple.io.dcCpuInOut.S.Data(0)
    
    

}
  
object directmappedMain {
  def main(args: Array[String]): Unit = { 
    chiselMain( args, () => new DirectMappedCacheTest())
  }
}

