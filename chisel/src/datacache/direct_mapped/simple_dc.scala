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
 * Direct Mapped cache memory 
 * 
 * Author: Sahar Abbaspour (sabb@dtu.dk)
 * 
 */


package patmos

import Chisel._
import Node._



import scala.math

import ocp._
import Constants._ 

class DirectMappedCache(dc_size: Int, mem_size: Int, burstLen : Int) extends Component {
val io = new Bundle {
    val dcCpuInOut = new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH) // slave to cpu
    val dcMemInOut = new OcpBurstMasterPort(ADDR_WIDTH, DATA_WIDTH, burstLen) // master to memory
    val stall		= UFix(OUTPUT, 1)
  }

    val dc0 = { Mem(dc_size / BYTES_PER_WORD, seqRead = true) { Bits(width = BYTE_WIDTH) } }
    val dc1 = { Mem(dc_size / BYTES_PER_WORD, seqRead = true) { Bits(width = BYTE_WIDTH) } }
    val dc2 = { Mem(dc_size / BYTES_PER_WORD, seqRead = true) { Bits(width = BYTE_WIDTH) } }
    val dc3 = { Mem(dc_size / BYTES_PER_WORD, seqRead = true) { Bits(width = BYTE_WIDTH) } }
  	
    val valid = Mem(dc_size) {Bits(width = 1)}
  	val tag  = Mem(dc_size) {Bits(width = BYTES_PER_WORD * BYTE_WIDTH - 1 - log2Up(dc_size))}
    
  	
  	val init			= Reg(resetVal = UFix(1, 1))
  	
  	when (init === UFix(1)) { // initialize memory, for simulation
	  	valid (Bits(25)) := Bits(0)
	  	valid (Bits(27)) := Bits(0)
	  	valid (Bits(37)) := Bits(0) // address == 150
	  	valid (Bits(38)) := Bits(0) // address == 150
	  	valid (Bits(39)) := Bits(0) // address == 150
	  	valid (Bits(40)) := Bits(0) // address == 150
	  	valid (Bits(41)) := Bits(0) // address == 150
	  	valid (Bits(42)) := Bits(0) // address == 150
	  	tag	(Bits(25)) := Bits(10)
	  	tag(Bits(27)) := Bits(10)
	  	tag(Bits(37)) := Bits(10) 
	  	tag(Bits(38)) := Bits(10) 
	  	tag(Bits(39)) := Bits(10) 
	  	tag(Bits(40)) := Bits(10) 
	  	tag(Bits(41)) := Bits(10) 
	  	tag(Bits(42)) := Bits(10)
	  	
	  	init := UFix(0)
  	}
  	
  	
  	
    val index_number 	= io.dcCpuInOut.M.Addr(log2Up(dc_size) + 1, 2)
  	
	val addrBits = log2Up(dc_size / BYTES_PER_WORD)
    val dc_en = Mux(io.dcCpuInOut.M.Cmd === OcpCmd.WRNP, io.dcCpuInOut.M.ByteEn,  Bits("b0000"))
    
    val dc_en_reg		= Reg() { Bits() } 
  	dc_en_reg			:= dc_en
  	
    val valid_dout = Reg() { Bits() }
  	val tag_dout =  Reg() { Bits() }
 	val data_dout = Reg() { Bits() }
 	
 	// register inputs
 	val address_reg		= Reg() { Bits() } 
  	address_reg			:= io.dcCpuInOut.M.Addr
  	
  	val wr_reg		= Reg() { UFix() } 
  	wr_reg			:= Mux(io.dcCpuInOut.M.Cmd === OcpCmd.WRNP, UFix(1), UFix(0))
  	
  	val rd_reg		= Reg() { UFix() } 
  	rd_reg			:= Mux(io.dcCpuInOut.M.Cmd === OcpCmd.RD, UFix(1), UFix(0))
  	
  	val data_in_reg		= Reg() { Bits() } 
  	data_in_reg			:= io.dcCpuInOut.M.Data
  	
  	val index_number_reg	= Reg() { Bits() } 
  	index_number_reg		:= index_number
  	//
  	val init_st :: write_hit_wait :: Nil  = Enum(2){ UFix() } 
	val state = Reg(resetVal = init_st)
 	
	
	val dcResp = Reg(resetVal = Bits(0, 2))
	//default outputs
	io.dcCpuInOut.S.Resp := OcpResp.NULL
	io.dcCpuInOut.S.Data := Bits(0)
	
	io.dcMemInOut.M.Cmd := OcpCmd.IDLE
	io.dcMemInOut.M.Addr := Bits(0)
	io.dcMemInOut.M.Data := Bits(0)
	io.dcMemInOut.M.DataByteEn := Bits("b0000")
	io.dcMemInOut.M.DataValid := Bits(0)
	io.stall := UFix(0)
	
    when (io.dcCpuInOut.M.Cmd === OcpCmd.RD || io.dcCpuInOut.M.Cmd === OcpCmd.WRNP) { // on a read/write, read the tag and valid
		valid_dout := valid(index_number) 
		tag_dout := tag(index_number) 
	} 
    

	data_dout	:= Cat(dc3(io.dcCpuInOut.M.Addr(addrBits + 1, log2Up(BYTES_PER_WORD))),
		  			dc2(io.dcCpuInOut.M.Addr(addrBits + 1, log2Up(BYTES_PER_WORD))),
		  			dc1(io.dcCpuInOut.M.Addr(addrBits + 1, log2Up(BYTES_PER_WORD))),
		  			dc0(io.dcCpuInOut.M.Addr(addrBits + 1, log2Up(BYTES_PER_WORD))))

    dcResp := Mux(wr_reg === UFix(1) || rd_reg === UFix(1), OcpResp.DVA, OcpResp.NULL)
	
    when ( valid_dout != Bits(1) || address_reg(BYTES_PER_WORD * BYTE_WIDTH - 1, log2Up(dc_size) + 1) != tag_dout){ //miss
  		
  		when (wr_reg === UFix(1)) {
	  		valid(index_number_reg) := Bits(1)// update the valid bit
			tag(index_number_reg)	:= address_reg(BYTES_PER_WORD * BYTE_WIDTH - 1, log2Up(dc_size) + 1)// update the tag
			//write to cache
			when(dc_en_reg(0)) { dc0(address_reg(addrBits + 1, 2)) := data_in_reg(BYTE_WIDTH-1, 0) }
			when(dc_en_reg(1)) { dc1(address_reg(addrBits + 1, 2)) := data_in_reg(2*BYTE_WIDTH-1, BYTE_WIDTH) }
			when(dc_en_reg(2)) { dc2(address_reg(addrBits + 1, 2)) := data_in_reg(3*BYTE_WIDTH-1, 2*BYTE_WIDTH) }
			when(dc_en_reg(3)) { dc3(address_reg(addrBits + 1, 2)) := data_in_reg(DATA_WIDTH-1, 3*BYTE_WIDTH) }
			
			io.dcCpuInOut.S.Resp := dcResp
			// write through to memory
			when ( io.dcMemInOut.S.DataAccept === Bits(0)) { // wait for the acc signal from slave
				io.stall := UFix(1)
				state := write_hit_wait
			}
			.elsewhen (io.dcMemInOut.S.DataAccept === Bits(1)){ //start transfer
				io.dcMemInOut.M.Data := data_in_reg
				io.dcMemInOut.M.Addr := address_reg
				io.dcMemInOut.M.Cmd := OcpCmd.WRNP
				io.dcMemInOut.M.DataValid := Bits(1) 
				io.dcMemInOut.M.DataByteEn := dc_en_reg
			}
  		}
  		
  		when (rd_reg === UFix(1)) {
//  			data(index_number_reg) := mem_data_in_reg // read data and write it to cache
  			valid(index_number_reg) := Bits(1)// update the valid bit
			tag(index_number_reg)	:= address_reg(BYTES_PER_WORD * BYTE_WIDTH - 1, log2Up(dc_size) + 1)// update the tag
//			io.data_out :=  mem_data_in_reg// on a miss, it reads again, this is for sim
				
			io.dcCpuInOut.S.Resp := OcpResp.NULL // tell cpu to wait
			
			// read a block from main memory	
			io.dcMemInOut.M.Addr := address_reg
			io.dcMemInOut.M.Cmd := OcpCmd.RD
			// wait for memory transfer
			//state := wait_mem
			
  		}

  	}

    
    .elsewhen (valid_dout === Bits(1) && address_reg(BYTES_PER_WORD * BYTE_WIDTH - 1, log2Up(dc_size) + 1) === tag_dout) { //hit
  		when (wr_reg === UFix(1)) {
			when(dc_en_reg(0)) { dc0(address_reg(addrBits + 1, log2Up(BYTES_PER_WORD))) := data_in_reg(BYTE_WIDTH-1, 0) }
			when(dc_en_reg(1)) { dc1(address_reg(addrBits + 1, log2Up(BYTES_PER_WORD))) := data_in_reg(2*BYTE_WIDTH-1, BYTE_WIDTH) }
			when(dc_en_reg(2)) { dc2(address_reg(addrBits + 1, log2Up(BYTES_PER_WORD))) := data_in_reg(3*BYTE_WIDTH-1, 2*BYTE_WIDTH) }
			when(dc_en_reg(3)) { dc3(address_reg(addrBits + 1, log2Up(BYTES_PER_WORD))) := data_in_reg(DATA_WIDTH-1, 3*BYTE_WIDTH) }
			
			io.dcMemInOut.M.Data := data_in_reg
			io.dcMemInOut.M.Addr := address_reg
			io.dcMemInOut.M.Cmd := OcpCmd.WRNP
			io.dcMemInOut.M.DataValid := Bits(1) 
			io.dcMemInOut.M.DataByteEn :=  dc_en_reg//Bits(8)
			when ( io.dcMemInOut.S.DataAccept === Bits(0)) { // wait for the acc signal from slave
				io.stall := UFix(1)
				state := write_hit_wait
			}

  		}
  		
  		when (rd_reg === UFix(1)) {
  			io.dcCpuInOut.S.Data := data_dout
  			io.dcCpuInOut.S.Resp := OcpResp.DVA
  		}
  	//	io.dcCpuInOut.S.Resp := Mux(io.dcCpuInOut.M.Cmd === OcpCmd.WRNP || io.dcCpuInOut.M.Cmd === OcpCmd.RD, OcpResp.DVA, OcpResp.NULL)
  	}
    
    		
//	when (state === wait_mem) {
//		when (io.dcMemInOut.S.Resp === OcpResp.DVA) { 
//			// write to cache
//			dc0(io.dcCpuInOut.M.Addr(addrBits + 1, 2)) := io.dcMemInOut.S.Data(BYTE_WIDTH-1, 0)
//			dc1(io.dcCpuInOut.M.Addr(addrBits + 1, 2)) := io.dcMemInOut.S.Data(2*BYTE_WIDTH-1, BYTE_WIDTH) 
//			dc2(io.dcCpuInOut.M.Addr(addrBits + 1, 2)) := io.dcMemInOut.S.Data(3*BYTE_WIDTH-1, 2*BYTE_WIDTH)
//			dc3(io.dcCpuInOut.M.Addr(addrBits + 1, 2)) := io.dcMemInOut.S.Data(DATA_WIDTH-1, 3*BYTE_WIDTH)
//			io.dcCpuInOut.S.Resp := OcpResp.DVA
//			state := idle
//		}
//		
//	}	
	
	when (state === write_hit_wait) {
		when ( io.dcMemInOut.S.DataAccept === Bits(0)) { // wait for the acc signal from slave
				io.stall := UFix(1)
				io.dcMemInOut.M.Data := data_in_reg
				io.dcMemInOut.M.Addr := address_reg
				io.dcMemInOut.M.Cmd := OcpCmd.WRNP
				io.dcMemInOut.M.DataValid := Bits(1) 
				io.dcMemInOut.M.DataByteEn := Bits(1) << Bits(burstLen) //Bits(8)
			}
			.elsewhen (io.dcMemInOut.S.DataAccept === Bits(1)){ //start transfer)
				io.dcMemInOut.M.Data := data_in_reg
				io.dcMemInOut.M.Addr := address_reg
				io.dcMemInOut.M.Cmd := OcpCmd.WRNP
				state := init_st
				io.stall := UFix(0)
			}
	}
 	
//  	when (state === transfer) {
//  		io.stall := UFix(1)
//	  	burst_count	:= burst_count - UFix(1)
//	  	data(index_number_reg) := mem_data_in_reg // read data and write it to cache
//	  	valid(index_number_reg) := Bits(1)// update the valid bit
//	  	tag(index_number_reg)	:= address_reg(word_length - 1, log2Up(num_blocks) + 2)// update the tag
//	  	io.data_out :=  mem_data_in_reg// on a miss, it reads again, this is for sim
//	  	index_number_reg := index_number_reg + Bits(1) //
//	  	//address_reg		:= address_reg + Bits(4) // next address
//	  	when (burst_count === UFix(0)) {
//	  		state := transfer_done
//	  		burst_count := UFix(burst_size)
//	  		io.stall := UFix(0)
//	  	}
//  	}
    


  // load
//  val rdData = Cat(dc3(io.dcCpuInOut.M.Addr(addrBits + 1, 2)),
//		  			dc2(io.dcCpuInOut.M.Addr(addrBits + 1, 2)),
//		  			dc1(io.dcCpuInOut.M.Addr(addrBits + 1, 2)),
//		  			dc0(io.dcCpuInOut.M.Addr(addrBits + 1, 2)))

//	val Cmd = Bits(width = 3)
//  val Addr = Bits(width = addrWidth)
//  val Data = Bits(width = dataWidth)
//  val ByteEn = Bits(width = dataWidth/8)
//	io.dcCpuInOut.S.Data := rdData

	
	

  
}
  

