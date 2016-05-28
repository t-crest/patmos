/*
   Copyright 2016 Technical University of Denmark, DTU Compute.
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
 * VGA controller
 *
 * Authors: Alessandro Tontini (alessandro.tontini@hotmail.it)
						Martina Valente (martina.valente.mail@gmail.com)
						Maja Lund 
 *
 */

package io

import Chisel._
import Node._
import ocp._
import patmos.Constants._
import patmos.MemBlock
import patmos.MemBlockIO

class Vga extends Module {
  
  val io = new Bundle {
    
    //VGA control signals:
    
    val vga_r       = UInt (OUTPUT, 8) 
    val vga_g       = UInt (OUTPUT, 8) 
    val vga_b       = UInt (OUTPUT, 8) 
    val vga_clk     = UInt (OUTPUT, 1) 
    val vga_sync_n  = UInt (OUTPUT, 1)
    val vga_blank_n = UInt (OUTPUT, 1)
    val vga_vs      = UInt (OUTPUT, 1)
    val vga_hs      = UInt (OUTPUT, 1)

	  //memory signals:
		
		val memPort     = new OcpBurstMasterPort(EXTMEM_ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH)
    
  }
  
  val s_h :: bp_h :: d_h :: fp_h :: Nil = Enum(UInt(), 4) //states for horizontal synch
  val s_v :: bp_v :: d_v :: fp_v :: Nil = Enum(UInt(), 4) //states for vertical synch
	val s_idle :: s_1 :: s_2  :: s_3 :: s_4  :: s_5 :: Nil = Enum(UInt(), 6) //states for memory read and display
 
  val state_h = Reg(init = s_h)
  val state_v = Reg(init = s_v)
	val state_m = Reg(init = s_idle)
  
  //rising edge detector
 
  def risingEdge(x: UInt) = ((x === UInt(1)) && (Reg(next = x) === UInt(0)))

  //constant values for 800x600@60Hz: 
  
  val SYNCH    = UInt (128,9) //clock cycles (40 MHz)
  val BPH      = UInt (88,8) //clock cycles (40 MHz)
  val HRES     = UInt (800,11) //clock cycles (40 MHz)
  val FPH      = UInt (40,7) //clock cycles (40 MHz) 
  
  val SYNCV    = UInt (4) //lines
  val BPV      = UInt (23) //lines
  val VRES     = UInt (600,11) //lines
  val FPV      = UInt (1) //lines

  val POLH     = UInt (0) //0=negative pol, 1=positive pol
  val POLV     = UInt (0)

	//CONSTANTS:

	val OCM_LOC_NR        = 100 //Nr of onchip memory locations
	val OCM_LOC_WIDTH     = 32  //Width of each onchip memory location (4 pixels @ 8bpp)

	val SRAM_DEFAULT_ADDR = Bits("h00000000")  //default SRAM address 
	val SRAM_BEGIN_ADDR   = Bits("h1E2B40",32) //starting point for SRAM reading

	val PHYSICAL_LINES    = Bits(300) 
	val SRAM_WORDS2READ   = UInt(100) //nr of SRAM words to read for each line
  
  //control signals for VGA:
   
  val vga_hs     = Reg(init = UInt(1,1)) 
  val vga_vs     = Reg(init = UInt(1,1))

	val vga_r      = Reg(init = UInt(0,8))
	val vga_g      = Reg(init = UInt(0,8))
	val vga_b      = Reg(init = UInt(0,8))
  
  val h_count    = Reg(init = UInt(0, 12)) 
  val v_count    = Reg(init = UInt(0, 11)) 
  
  val disp_h     = Reg(init = UInt(0,1))
  val disp_v     = Reg(init = UInt(0,1))
   
  val vga_clk    = Reg(init = UInt(0,1))
	val extVga_clk = Reg(init = UInt(0,1))
	
	//control signals for SRAM read:
	
	val memSel      = Reg(init = UInt(0,1))  //on chip memory selector
	val wordCountRD = Reg(init = Bits(0,15)) //word counter (display)
	val wordCountWR = Reg(init = Bits(0,8))	 //word counter (read from SRAM)

	//On-chip memories definition

	val ocMem0 = MemBlock(OCM_LOC_NR, OCM_LOC_WIDTH) 
	val ocMem1 = MemBlock(OCM_LOC_NR, OCM_LOC_WIDTH)

	val wen0   = Bits(width=1)
	val wen1   = Bits(width=1)		
	

//Internal VGA clock generation (used to generate control signals):
  
  val cnt = Reg(init = UInt(0,1))
  
  when(cnt < UInt(1)){
      vga_clk := UInt(0)
      cnt := cnt + UInt(1)
   }.otherwise{
      vga_clk := UInt(1)
      cnt := cnt + UInt(1)
   }


//External VGA clock generation (output to VGA):
  
  val cnt1 = Reg(init = UInt(1,1))
  
  when(cnt1 < UInt(1)){
      extVga_clk := UInt(0)
      cnt1 := cnt1 + UInt(1)
   }.otherwise{
      extVga_clk := UInt(1)
      cnt1 := cnt1 + UInt(1)
   }	


//RGB output control logic:

	val dispEn   = Reg(init=UInt(0,1))   //start VGA signals only when first ocMem is filled	
	val pixelSel = Reg(init=UInt(3,2))	 //select one pixel within a word
	
  //from 400x300 (memory) to 800x600 (display)
  val pixelCnt  = Reg(init=UInt(0,1)) //keep visualizing the same pixel twice
  val lineCnt   = Reg(init=UInt(0,1))	//keep visualizing the same row twice
	
	val risingVgaClk = Reg(init = UInt(0,1))
	risingVgaClk := risingEdge(vga_clk)

	val pixel_1 = UInt(width=8)
  pixel_1 := UInt(0)	

	val pixel_0 = UInt(width=8)
  pixel_0 := UInt(0)	

	vga_r := vga_r
	vga_g := vga_g
	vga_b := vga_b

  //read from on chip memory
	val rdata1 = ocMem1.io(wordCountRD) 	
  val rdata0 = ocMem0.io(wordCountRD)

	when(dispEn === UInt(1)){

		when(((disp_h & disp_v) === UInt(1)) & (risingVgaClk === UInt(1))){ //Display interval

			when(memSel === UInt(0)){ //ocMem0 is selected

				when(pixelSel === UInt(3)){
					pixel_1 := rdata1(31,24)
				}.elsewhen(pixelSel === UInt(2)){
					pixel_1 := rdata1(23,16)
				}.elsewhen(pixelSel === UInt(1)){
					pixel_1 := rdata1(15,8)
				}.otherwise{
					pixel_1 := rdata1(7,0)
				}

				vga_r := pixel_1(7,5) << UInt(5)  
				vga_g := pixel_1(4,2) << UInt(5)
				vga_b := pixel_1(1,0) << UInt(6)

			}.otherwise{ //ocMem1 is selected

				when(pixelSel === UInt(3)){
					pixel_0 := rdata0(31,24)
				}.elsewhen(pixelSel === UInt(2)){
					pixel_0 := rdata0(23,16)
				}.elsewhen(pixelSel === UInt(1)){
					pixel_0 := rdata0(15,8)
				}.otherwise{
					pixel_0 := rdata0(7,0)
				}

				vga_r := pixel_0(7,5) << UInt(5)  
				vga_g := pixel_0(4,2) << UInt(5)
				vga_b := pixel_0(1,0) << UInt(6)
			}
			
			pixelCnt := pixelCnt + UInt(1)

			when(pixelCnt === UInt(1)){
				pixelSel := pixelSel - UInt(1)
				when(pixelSel === UInt(0)){
					wordCountRD := wordCountRD + Bits(1)
				}
			}

			when(wordCountRD === Bits(OCM_LOC_NR)){
        wordCountRD := Bits(0)
        lineCnt := lineCnt+UInt(1)
		    when(lineCnt===UInt(1)){
		      memSel := ~memSel 
		    }
      }
			
		}
	}
	

//SRAM memory FSM (read one frame)

	val memAddrRD = Reg(init = SRAM_BEGIN_ADDR) //SRAM address of the next word to read
	val vertCount = Reg(init = Bits(0,10))      //line counter (up to PHYSICAL_LINES)
  val lineCntRD = Reg(init=UInt(0,1))         //read line from memory every second display portion

	io.memPort.M.Cmd  := OcpCmd.IDLE	
	io.memPort.M.Addr := SRAM_DEFAULT_ADDR
	
	wen0 := Bits(0)
	wen1 := Bits(0)
	
	//write to on chip memory
	ocMem0.io <= (wen0, wordCountWR, io.memPort.S.Data)					
	ocMem1.io <= (wen1, wordCountWR, io.memPort.S.Data)			

	switch(state_m){

		is(s_idle){
			when(risingEdge(disp_h & disp_v) | (dispEn === UInt(0))){ //beginning of display portion
        lineCntRD := lineCntRD + UInt(1)
				when(lineCntRD === UInt(0)){
					when(vertCount === PHYSICAL_LINES){
						memAddrRD := SRAM_BEGIN_ADDR
						vertCount := Bits(0)
					}
					state_m := s_1
        }.otherwise{
					state_m := s_idle
				}
			}.otherwise{
				state_m := s_idle
			}
		}		

		is(s_1){ //perform a read command
        when(wordCountWR < SRAM_WORDS2READ){ 
					io.memPort.M.Cmd := OcpCmd.RD
					io.memPort.M.Addr := memAddrRD;
					memAddrRD := memAddrRD + Bits("h10");
					when(io.memPort.S.CmdAccept === UInt(1)){
						state_m := s_2
					}.otherwise{
						state_m := s_1
					}
				}.otherwise{
          wordCountWR := Bits(0)
					vertCount := vertCount + Bits(1)
					state_m   := s_idle
					dispEn   := UInt(1)
				}
		}
	
		is(s_2){ //read first word
			when(io.memPort.S.Resp === OcpResp.DVA){
				when(memSel === UInt(0)){
					wen0   := Bits(1)				
				}.otherwise{
					wen1   := Bits(1)					
				}
				wordCountWR := wordCountWR + Bits(1)
				state_m := s_3
			}.otherwise{
				state_m := s_2
			}	
		}

		is(s_3){//read second word
			when(io.memPort.S.Resp === OcpResp.DVA){
				when(memSel === UInt(0)){
					wen0   := Bits(1)				
				}.otherwise{
					wen1   := Bits(1)					
				}
				wordCountWR := wordCountWR + Bits(1)
				state_m := s_4
			}.otherwise{
				state_m := s_3
			}	
		}

		is(s_4){//read third word
			when(io.memPort.S.Resp === OcpResp.DVA){
				when(memSel === UInt(0)){
					wen0   := Bits(1)				
				}.otherwise{
					wen1   := Bits(1)					
				}
				wordCountWR := wordCountWR + Bits(1)
				state_m := s_5
			}.otherwise{
				state_m := s_4
			}			
		}

		is(s_5){//read fourth word
			when(io.memPort.S.Resp === OcpResp.DVA){
				when(memSel === UInt(0)){
					wen0   := Bits(1)					
				}.otherwise{
					wen1   := Bits(1)					
				}
				wordCountWR := wordCountWR + Bits(1)
				state_m := s_1
			}.otherwise{	
				state_m := s_5
			}			
		}

	} 
	

//VGA synch signals generation

  h_count := h_count
  disp_h  := disp_h
  disp_v  := disp_v
  vga_hs  := vga_hs 
  vga_vs  := vga_vs
  
  when(dispEn === UInt(1)){
		when(risingVgaClk === UInt(1)){
		  
		  //horizontal synchronization:
		  
		  h_count := h_count + UInt(1)
		  
		  disp_h := UInt(0)
		  
		  switch(state_h){
		    is (s_h){
		      
		      when(h_count === SYNCH){ state_h := bp_h }.otherwise{ state_h := s_h }
		      
		    }
		    is (bp_h){ 
		   
		      when(h_count === (SYNCH + BPH)){ state_h := d_h }.otherwise{ state_h := bp_h }
		      
		    }
		    is (d_h){
		      
		      disp_h  := UInt(1)
		      when(h_count === (SYNCH + BPH + HRES)){ state_h := fp_h }.otherwise{ state_h := d_h }
		      
		    }
		    is (fp_h){  
		      
		      when(h_count === (SYNCH + BPH + HRES + FPH)){ 
		        state_h := s_h 
		        v_count := v_count + UInt(1)
		        h_count := UInt(0)
		      }.otherwise{ state_h := fp_h }
		      
		    }
		  }
		
		  vga_hs := !(state_h === s_h)
		  
		  
		  //vertical synchronization:
		  
		  vga_vs := UInt(1) 
		  
		  disp_v := UInt(0)
		  
		  switch(state_v){
		    is (s_v){
		      
		      vga_vs  := UInt(0)
		      when(v_count === SYNCV){ state_v := bp_v }.otherwise{ state_v := s_v }
		      
		    }
		    is (bp_v){ 
		      
		      when(v_count === (SYNCV + BPV)){ state_v := d_v }.otherwise{ state_v := bp_v }
		      
		    }
		    is (d_v){
		      
		      disp_v  := UInt(1)
		      when(v_count === (SYNCV + BPV + VRES)){ state_v := fp_v }.otherwise{ state_v := d_v }
		      
		    }
		    is (fp_v){  
		      
		      when(v_count === (SYNCV + BPV + VRES + FPV)){ 
		        state_v := s_v
		        v_count := UInt(0)
		      }.otherwise{ 
		        state_v := fp_v 
		      }
		      
		    }
		  }
		 
		}
  }


//Output update
  
  io.vga_blank_n := (disp_h & disp_v)
  io.vga_sync_n  := UInt(0) 
  io.vga_clk     := extVga_clk 
  io.vga_hs      := vga_hs ^ POLH 
  io.vga_vs      := vga_vs ^ POLV
	io.vga_r       := vga_r
	io.vga_g       := vga_g
	io.vga_b       := vga_b

}

object VGAMain {
  def main(args: Array[String]): Unit = {
    chiselMain(Array[String]("--backend", "v", "--targetDir", "generated"),() => Module(new Vga()))
  }
}

