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
  Instruction Cache for Patmos
  Author: Philipp Degasperi (philipp.degasperi@gmail.com)
 */

package patmos

import Chisel._
import Node._
import IConstants._
import Constants._
import ocp._

import scala.collection.mutable.HashMap
import scala.util.Random
import scala.math

object IConstants {

  //address encoding for icache address space (=0x20000)
  val ICACHE_ONE_BIT = 17
  //default = 2KB I-Cache
  val ICACHE_SIZE = 2048
  //size of a block default = 16 words
  val WORD_COUNT = 16
  val ICACHE_WORD_SIZE = ICACHE_SIZE / 4
  val ICACHE_SIZE_WIDTH = log2Up(ICACHE_WORD_SIZE)
  val BLOCK_COUNT = ICACHE_WORD_SIZE / WORD_COUNT
  val WORD_COUNT_WIDTH = log2Up(WORD_COUNT)
  val BLOCK_COUNT_WIDTH = log2Up(BLOCK_COUNT)
  val VALIDBIT_FIELD_SIZE = 1  //could be enlarged into subblocks
  val TAG_FIELD_HIGH = EXTMEM_ADDR_WIDTH-1
  val TAG_FIELD_SIZE = (TAG_FIELD_HIGH + 1 - BLOCK_COUNT_WIDTH - WORD_COUNT_WIDTH)
  val TAG_FIELD_LOW = TAG_FIELD_HIGH - TAG_FIELD_SIZE + 1
  val INDEX_FIELD_HIGH = TAG_FIELD_LOW - 1
  val INDEX_FIELD_LOW = WORD_COUNT_WIDTH
  val INDEX_FIELD_SIZE = INDEX_FIELD_HIGH - INDEX_FIELD_LOW + 1
  val OFFSET_SIZE = 0 //could be added in case to address some subbytes in the block

}

class ICacheIO extends Bundle() {
  val ena_out = Bool(OUTPUT)
  val ena_in = Bool(INPUT)
  val femcache = new FeMCache().asInput
  val exmcache = new ExMCache().asInput
  val mcachefe = new MCacheFe().asOutput
  val ocp_port = new OcpBurstMasterPort(EXTMEM_ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH)
}
class ICacheCtrlIO extends Bundle() {
  val ena_in = Bool(INPUT)
  val fetch_ena = Bool(OUTPUT)
  val icache_ctrlrepl = new ICacheCtrlRepl().asOutput
  val icache_replctrl = new ICacheReplCtrl().asInput
  val feicache = new FeMCache().asInput
  val exicache = new ExMCache().asInput
  val ocp_port = new OcpBurstMasterPort(EXTMEM_ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH)
}
class ICacheReplIO extends Bundle() {
  val ena_in = Bool(INPUT)
  val exicache = new ExMCache().asInput
  val feicache = new FeMCache().asInput
  val icachefe = new MCacheFe().asOutput
  val icache_ctrlrepl = new ICacheCtrlRepl().asInput
  val icache_replctrl = new ICacheReplCtrl().asOutput
  val icachemem_in = new ICacheMemIn().asOutput
  val icachemem_out = new ICacheMemOut().asInput
}
class ICacheMemIO extends Bundle() {
  val icachemem_in = new ICacheMemIn().asInput
  val icachemem_out = new ICacheMemOut().asOutput
}
class ICacheCtrlRepl extends Bundle() {
  val wEna = Bool()
  val wData = Bits(width = INSTR_WIDTH)
  val wAddr = Bits(width = ADDR_WIDTH)
  val wTag = Bool()
}
class ICacheReplCtrl extends Bundle() {
  val hitEna = Bool()
  val fetchAddr = Bits(width = EXTMEM_ADDR_WIDTH)
  val selICache = Bool()
}
class ICacheMemIn extends Bundle() {
  val wEven = Bool()
  val wOdd = Bool()
  val wData = Bits(width = DATA_WIDTH)
  val wAddr = Bits(width = INDEX_FIELD_SIZE + WORD_COUNT_WIDTH)
  val addrOdd = Bits(width = INDEX_FIELD_SIZE + WORD_COUNT_WIDTH)
  val addrEven = Bits(width = INDEX_FIELD_SIZE + WORD_COUNT_WIDTH)
}
class ICacheMemOut extends Bundle() {
  val instrEven = Bits(width = INSTR_WIDTH)
  val instrOdd = Bits(width = INSTR_WIDTH)
}


/*
 ICache: Top Level Class for the Instruction Cache
 */
class ICache() extends Module {
  val io = new ICacheIO()
  //generate submodules of instruction cache
  val mcachectrl = Module(new ICacheCtrl())
  val mcacherepl = Module(new ICacheReplDm())
  val mcachemem = Module(new ICacheMem())
  //connect submodules of instruction cache
  mcachectrl.io.icache_ctrlrepl <> mcacherepl.io.icache_ctrlrepl
  mcachectrl.io.feicache <> io.femcache
  mcachectrl.io.exicache <> io.exmcache
  mcachectrl.io.ocp_port <> io.ocp_port
  //connect inputs to instruction cache repl unit
  mcacherepl.io.exicache <> io.exmcache
  mcacherepl.io.feicache <> io.femcache
  mcacherepl.io.icachefe <> io.mcachefe
  mcacherepl.io.icache_replctrl <> mcachectrl.io.icache_replctrl
  //connect repl unit to on chip memory
  mcacherepl.io.icachemem_in <> mcachemem.io.icachemem_in
  mcacherepl.io.icachemem_out <> mcachemem.io.icachemem_out
  //connect enables
  mcachectrl.io.ena_in <> io.ena_in
  mcacherepl.io.ena_in <> io.ena_in
  //output enable depending on hit/miss/fetch
  io.ena_out := mcachectrl.io.fetch_ena
}

/*
 ICacheMem Class: On-Chip Instruction Cache Memory
 */
class ICacheMem extends Module {
  val io = new ICacheMemIO()
 
  val icacheEven = MemBlock(ICACHE_WORD_SIZE / 2, INSTR_WIDTH)
  val icacheOdd = MemBlock(ICACHE_WORD_SIZE / 2, INSTR_WIDTH)

  icacheEven.io <= (io.icachemem_in.wEven, io.icachemem_in.wAddr,
                         io.icachemem_in.wData)

  icacheOdd.io <= (io.icachemem_in.wOdd, io.icachemem_in.wAddr,
                        io.icachemem_in.wData)

  io.icachemem_out.instrEven := icacheEven.io(io.icachemem_in.addrEven)
  io.icachemem_out.instrOdd := icacheOdd.io(io.icachemem_in.addrOdd)

}

/*
 Least Recently Used Replacement Class
 */
class ICacheReplLru extends Module {
  val io = new ICacheReplIO()

  /*
   add LRU replacement policy here 
   */

}

/*
 Direct Mapped Replacement Class
 */
class ICacheReplDm() extends Module {
  val io = new ICacheReplIO()

  //reserve memory for the instruction cache tag field containing valid bit and address tag
  val tagMemEven = MemBlock(BLOCK_COUNT / 2, TAG_FIELD_SIZE)
  val tagMemOdd = MemBlock(BLOCK_COUNT / 2, TAG_FIELD_SIZE)
  val validVec = Vec.fill(BLOCK_COUNT) { Reg(init = Bool(false))}

  //variables when call/return occurs
  val callRetBaseReg = Reg(init = UInt(1, DATA_WIDTH))
  val callAddrReg = Reg(init = UInt(1, DATA_WIDTH))
  val selIspmReg = Reg(init = Bool(false))
  val selICacheReg = Reg(init = Bool(false))

  val fetchAddr = Bits(width = EXTMEM_ADDR_WIDTH)
  val hitInstrEven = Bool()
  val hitInstrOdd = Bool()

  val relBase = Mux(selICacheReg,
                    callRetBaseReg(ICACHE_ADDR_OFFSET,0),
                    callRetBaseReg(ISPM_ONE_BIT-3, 0))
  val relPc = callAddrReg + relBase

  val reloc = Mux(selICacheReg,
                  UInt(0),
                  Mux(selIspmReg,
                      UInt(1 << (ISPM_ONE_BIT - 2)),
                      UInt(0)))

  when (io.exicache.doCallRet && io.ena_in) {
    callRetBaseReg := io.exicache.callRetBase
    callAddrReg := io.exicache.callRetAddr
    selIspmReg := io.exicache.callRetBase(EXTMEM_ADDR_WIDTH - 1,ISPM_ONE_BIT - 2) === Bits(0x1)
    selICacheReg := io.exicache.callRetBase(EXTMEM_ADDR_WIDTH - 1,15) >= Bits(0x1)
  }

  val addrIndexEven = (io.feicache.addrEven)(INDEX_FIELD_HIGH, INDEX_FIELD_LOW+1)
  val addrIndexOdd = io.feicache.addrOdd(INDEX_FIELD_HIGH, INDEX_FIELD_LOW+1)
  val addrTagEven = (io.feicache.addrEven)(TAG_FIELD_HIGH, TAG_FIELD_LOW)
  val addrTagOdd = io.feicache.addrOdd(TAG_FIELD_HIGH, TAG_FIELD_LOW)
  val blockParityEven = io.feicache.addrEven(INDEX_FIELD_LOW)
  val blockParityOdd = io.feicache.addrOdd(INDEX_FIELD_LOW)

  // Mux of tag memory input
  val addrBlockEven = Mux(blockParityEven, addrIndexOdd, addrIndexEven)
  val addrBlockOdd = Mux(blockParityEven, addrIndexEven, addrIndexOdd)
  val addrEvenReg = Reg(next = io.feicache.addrEven)
  val addrOddReg = Reg(next = io.feicache.addrOdd)
  val blockParityEvenReg = addrEvenReg(INDEX_FIELD_LOW)
  val blockParityOddReg = addrOddReg(INDEX_FIELD_LOW)
  val addrTagEvenReg = addrEvenReg(TAG_FIELD_HIGH, TAG_FIELD_LOW)
  val addrTagOddReg = addrOddReg(TAG_FIELD_HIGH, TAG_FIELD_LOW)
  val addrIndexEvenReg = addrEvenReg(INDEX_FIELD_HIGH, INDEX_FIELD_LOW)
  val addrIndexOddReg = addrOddReg(INDEX_FIELD_HIGH, INDEX_FIELD_LOW)
  val addrValidEven = io.feicache.addrEven(INDEX_FIELD_HIGH, INDEX_FIELD_LOW)
  val addrValidOdd = io.feicache.addrOdd(INDEX_FIELD_HIGH, INDEX_FIELD_LOW)

  //Mux at tag memory input
  val toutEven = tagMemEven.io(addrBlockEven)
  val toutOdd = tagMemOdd.io(addrBlockOdd)
  // Mux of tag memory output
  val tagEven = Mux(blockParityEvenReg, toutOdd, toutEven)
  val tagOdd = Mux(blockParityOddReg, toutOdd, toutEven)
  //valid tag
  val validTag = validVec(addrValidEven) && validVec(addrValidOdd)
  val validTagReg = Reg(next = validTag)

  //check for a hit of both instructions of the address bundle
  hitInstrEven := Bool(true)
  hitInstrOdd := Bool(true)
  when (tagEven != addrTagEvenReg) {
    hitInstrEven := Bool(false)
  }
  fetchAddr := addrEvenReg
  when (tagOdd != addrTagOddReg) {
    hitInstrOdd := Bool(false)
    fetchAddr := addrOddReg
  }
  //debug signals for emulator
  debug(hitInstrEven)
  debug(hitInstrOdd)

  val wrAddrTag = io.icache_ctrlrepl.wAddr(TAG_FIELD_HIGH,TAG_FIELD_LOW)
  //index for valid field
  val wrValidIndex = io.icache_ctrlrepl.wAddr(INDEX_FIELD_HIGH, INDEX_FIELD_LOW)
  //index for tag field even/odd
  val wrAddrIndex = io.icache_ctrlrepl.wAddr(INDEX_FIELD_HIGH, INDEX_FIELD_LOW+1)
  val wrAddrParity = io.icache_ctrlrepl.wAddr(INDEX_FIELD_LOW)
  //update tag field when new write occurs
  tagMemEven.io <= (io.icache_ctrlrepl.wTag && !wrAddrParity, wrAddrIndex, wrAddrTag)
  tagMemOdd.io <= (io.icache_ctrlrepl.wTag && wrAddrParity, wrAddrIndex, wrAddrTag)
  when (io.icache_ctrlrepl.wTag) {
    validVec(wrValidIndex) := Bool(true)
  }

  val wrParity = io.icache_ctrlrepl.wAddr(0)

  //outputs to icache memory
  io.icachemem_in.wEven := Mux(wrParity, Bool(false), io.icache_ctrlrepl.wEna)
  io.icachemem_in.wOdd := Mux(wrParity, io.icache_ctrlrepl.wEna, Bool(false))
  io.icachemem_in.wData := io.icache_ctrlrepl.wData
  io.icachemem_in.wAddr := (io.icache_ctrlrepl.wAddr)(INDEX_FIELD_HIGH,1)
  io.icachemem_in.addrOdd := (io.feicache.addrOdd)(INDEX_FIELD_HIGH,1)
  io.icachemem_in.addrEven := (io.feicache.addrEven)(INDEX_FIELD_HIGH,1)

  io.icachefe.instrEven := io.icachemem_out.instrEven
  io.icachefe.instrOdd := io.icachemem_out.instrOdd

  io.icachefe.relBase := relBase
  io.icachefe.relPc := relPc
  io.icachefe.reloc := reloc
  io.icachefe.memSel := Cat(selIspmReg, selICacheReg)
  //hit/miss return
  io.icache_replctrl.fetchAddr := fetchAddr
  io.icache_replctrl.hitEna := (hitInstrEven && hitInstrOdd && validTagReg)
  io.icache_replctrl.selICache := selICacheReg

}

/*
 Instruction Cache Control Class: handles block transfer from external Memory to the I-Cache
 */
class ICacheCtrl() extends Module {
  val io = new ICacheCtrlIO()

  //fsm state variables
  val initState :: idleState :: transferState :: waitState :: Nil = Enum(UInt(), 4)
  val icacheState = Reg(init = idleState)
  //signal for replacement unit
  val wData = Bits(width = DATA_WIDTH)
  val wTag = Bool()
  val wAddr = Bits(width = ADDR_WIDTH)
  val wEna = Bool()
  //signals for external memory
  val ocpCmd = Bits(width = 3)
  val ocpAddr = Bits(width = EXTMEM_ADDR_WIDTH)
  val fetchCnt = Reg(init = Bits(0, width = ICACHE_SIZE_WIDTH))
  val burstCnt = Reg(init = UInt(0, width = log2Up(BURST_LENGTH)))
  val fetchEna = Bool()
  //input output registers
  val addrReg = Reg(init = Bits(0, width = 32))
  val ocpSlaveReg = Reg(next = io.ocp_port.S)
  //address for the entire block
  val absFetchAddr = Cat(addrReg(EXTMEM_ADDR_WIDTH,WORD_COUNT_WIDTH), Bits(0)(WORD_COUNT_WIDTH-1,0))

  //init signals
  wData := Bits(0)
  wTag := Bool(false)
  wEna := Bool(false)
  wAddr := Bits(0)
  ocpCmd := OcpCmd.IDLE
  ocpAddr := Bits(0)
  fetchEna := Bool(true)

  //wait till ICache is the selected source
  when (icacheState === initState) {
    when (io.icache_replctrl.selICache) {
      icacheState := idleState
    }
  } 
  when (icacheState === idleState) {
    when (!io.icache_replctrl.hitEna) {
      fetchEna := Bool(false)
      addrReg := io.icache_replctrl.fetchAddr
      burstCnt := UInt(0)
      fetchCnt := UInt(0)
      //write new tag field memory
      wTag := Bool(true)
      wAddr := Cat(io.icache_replctrl.fetchAddr(EXTMEM_ADDR_WIDTH-1,WORD_COUNT_WIDTH), Bits(0)(WORD_COUNT_WIDTH-1,0))
      //check if command is accepted by the memory controller
      when (io.ocp_port.S.CmdAccept === Bits(1)) {
        ocpAddr := Cat(io.icache_replctrl.fetchAddr(EXTMEM_ADDR_WIDTH-1,WORD_COUNT_WIDTH), Bits(0)(WORD_COUNT_WIDTH-1,0))
        ocpCmd := OcpCmd.RD
        icacheState := transferState
      }
      .otherwise {
        icacheState := waitState
      }
    }
  }
  when (icacheState === waitState) {
    fetchEna := Bool(false)
    when (io.ocp_port.S.CmdAccept === Bits(1)) {
      ocpAddr := Cat(addrReg(EXTMEM_ADDR_WIDTH-1,WORD_COUNT_WIDTH), Bits(0)(WORD_COUNT_WIDTH-1,0))
      ocpCmd := OcpCmd.RD
    }
  }
  //transfer/fetch cache block
  when (icacheState === transferState) {
    fetchEna := Bool(false)
    when (fetchCnt < UInt(WORD_COUNT)) {
      when (ocpSlaveReg.Resp === OcpResp.DVA) {
        fetchCnt := fetchCnt + Bits(1)
        burstCnt := burstCnt + Bits(1)
        when(fetchCnt < UInt(WORD_COUNT-1)) {
          //fetch next address from external memory
          when (burstCnt >= UInt(BURST_LENGTH - 1)) {
            ocpCmd := OcpCmd.RD
            ocpAddr := Cat(addrReg(EXTMEM_ADDR_WIDTH,WORD_COUNT_WIDTH), Bits(0)(WORD_COUNT_WIDTH-1,0)) + fetchCnt + Bits(1)
            burstCnt := UInt(0)
          }
        }
        //write current address to icache memory
        wData := ocpSlaveReg.Data
        wEna := Bool(true)
      }
      wAddr := Cat(addrReg(EXTMEM_ADDR_WIDTH,WORD_COUNT_WIDTH), Bits(0)(WORD_COUNT_WIDTH-1,0)) + fetchCnt
    }
    //restart to idle state
    .otherwise {
      icacheState := idleState
    }
  }
  
  //outputs to instruction cache memory
  io.icache_ctrlrepl.wEna := wEna
  io.icache_ctrlrepl.wData := wData
  io.icache_ctrlrepl.wAddr := wAddr
  io.icache_ctrlrepl.wTag := wTag

  io.fetch_ena := fetchEna

  //output to external memory
  io.ocp_port.M.Addr := Cat(ocpAddr, Bits("b00"))
  io.ocp_port.M.Cmd := ocpCmd
  io.ocp_port.M.Data := Bits(0) //read-only
  io.ocp_port.M.DataByteEn := Bits("b1111") //read-only
  io.ocp_port.M.DataValid := Bits(0) //read-only

}
