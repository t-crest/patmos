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
 * Stack cache memory
 *
 * Author: Sahar Abbaspour (sabb@dtu.dk)
 *         Florian Brandner (florian.brandner@ensta-paristech.fr)
 *
 */

package stackcache

import Chisel._
import Node._

import scala.math

import ocp._
import patmos._
import patmos.Constants._

class StackCache() extends Module {
  val io = new StackCacheIO() {
    // slave to cpu
    val fromCPU = new OcpCoreSlavePort(EXTMEM_ADDR_WIDTH, DATA_WIDTH)
    // master to memory
    val toMemory = new OcpBurstMasterPort(EXTMEM_ADDR_WIDTH, DATA_WIDTH,
      BURST_LENGTH)
  }

  // number of bits needed to address the bytes of a word
  val wordBits = Chisel.log2Up(BYTES_PER_WORD)

  // number of bits needed to address the bytes of a burst
  val burstBits = Chisel.log2Up(BURST_LENGTH * BYTES_PER_WORD)

  // number of bits to address the stack cache's memory
  val scSizeBits = Chisel.log2Up(SCACHE_SIZE / BYTES_PER_WORD)

  // stateReg machine to manage spilling and filling
  val idleState :: fillState :: waitFillState :: spillState :: holdSpillState :: waitSpillState :: Nil = Enum(UInt(), 6)
  val stateReg = Reg(init = idleState)

  // stack top pointer
  val stackTopReg = Reg(init = UInt(0, DATA_WIDTH))

  // memory top pointer
  val memTopReg = Reg(init = UInt(0, DATA_WIDTH))

  // temporary address used during filling/spilling 
  val transferAddrReg = Reg(init = UInt(0, DATA_WIDTH))

  // temporary address used during filling
  val requiredMemTopReg = Reg(init = UInt(0, DATA_WIDTH))

  // the actual memory of the stack cache
  val memoryBlock = new Array[MemBlockIO](BYTES_PER_WORD)
  for (i <- 0 until BYTES_PER_WORD) {
    memoryBlock(i) = MemBlock(SCACHE_SIZE / BYTES_PER_WORD, BYTE_WIDTH).io
  }

  val mb_rdAddr = UInt(width = ADDR_WIDTH)
  val mb_rdData = memoryBlock(3).rdData ## memoryBlock(2).rdData ## memoryBlock(1).rdData ## memoryBlock(0).rdData
  val mb_wrAddr = UInt(width = ADDR_WIDTH)
  val mb_wrEna = UInt(width = BYTE_WIDTH)
  val mb_wrData = UInt(width = DATA_WIDTH)

  // register addr for MemBlock
  val rdAddrReg = Reg(next = memoryBlock(0).rdAddr)

  // response to CPU for read/write requests
  val responseToCPUReg = Reg(init = OcpResp.NULL)

  // default OCP "request"
  io.stall := stateReg != idleState
  io.toMemory.M.Cmd := OcpCmd.IDLE
  io.toMemory.M.Addr := UInt(0)
  io.toMemory.M.Data := UInt(0)
  io.toMemory.M.DataValid := UInt(0)
  io.toMemory.M.DataByteEn := UInt("b1111")

  // default signals for the stack cache's memory
  mb_rdAddr := UInt(0)
  mb_wrAddr := UInt(0)
  mb_wrEna := UInt(0)
  mb_wrData := UInt(0)

  // signals to execute stage
  io.scex.stackTop := stackTopReg
  io.scex.memTop := memTopReg

  // extract current burst counter
  val burstCounter = transferAddrReg.apply(burstBits - 1, wordBits)

  // reset response to CPU
  responseToCPUReg := OcpResp.NULL
  
  // chose between sres and sspill, sres 0, sspill 1 
  val resSpillOpReg = Reg(init = Bool(false))

  //////////////////////////////////////////////////////////////////////////////
  // Stack Control Interface (mfs, sres, sens, sfree)
  //////////////////////////////////////////////////////////////////////////////
  switch(stateReg) {
    is(idleState) {
      // perform operation requested by pipeline
      stateReg := idleState

      when (io.ena_in) {
        switch(io.exsc.op) {
          is(sc_OP_NONE) {
            // don't do anything
          }
          is(sc_OP_SET_ST) {
            // assign the operation's operand to the stack top pointer
            stackTopReg := io.exsc.opData
          }
          is(sc_OP_SET_MT) {
            // assign the operation's operand to the mem top pointer
            memTopReg := io.exsc.opData
          }
          is(sc_OP_ENS) {
            // compute required mem top pointer, and check if filling is needed
            val nextRequiredMemTop = stackTopReg + io.exsc.opOff

            // start transfer from the current memory top pointer on
            transferAddrReg := memTopReg.apply(ADDR_WIDTH - 1, burstBits) ##
              Fill(burstBits, UInt("b0"))

            // check if filling is needed
            val needsFill = memTopReg < nextRequiredMemTop

            // update memory top pointer if needed
            requiredMemTopReg := nextRequiredMemTop

            // start actual filling if needed
            stateReg := Mux(needsFill, fillState, idleState)
          }
          is(sc_OP_FREE) {
            // move stack top pointer upwards
            val nextStackTop = stackTopReg + io.exsc.opOff
            stackTopReg := nextStackTop

            // ensure that mem top pointer is above stack top
            when(nextStackTop > memTopReg) {
              memTopReg := nextStackTop
            }
          }
        }
        
        when(io.exsc.op === sc_OP_RES || io.exsc.op === sc_OP_SPILL) {
          // register type of instruction
          val resSpillOp = Mux(io.exsc.op === sc_OP_RES, Bool(false), Bool(true))
          resSpillOpReg := resSpillOp
            
          // start address of spilling for sspill
          val spillAddress = memTopReg - io.exsc.opOff
            
          // decrement the stack top pointer
          val nextStackTop = stackTopReg - io.exsc.opOff
          stackTopReg := Mux(resSpillOp, stackTopReg, nextStackTop)

          // start transfer from the current stack pointer + SCACHE_SIZE on
          val nextTransferAddr = Mux(resSpillOp, (nextStackTop + UInt(SCACHE_SIZE)).apply(
                                       ADDR_WIDTH-1, 
                                       burstBits) ## Fill(burstBits, UInt("b0")),
                                       spillAddress(ADDR_WIDTH - 1, burstBits) ## Fill(burstBits, UInt("b0")))      
                

          // start reading from the stack cache's memory
          mb_rdAddr := nextTransferAddr.apply(scSizeBits + wordBits - 1, wordBits)

          // store transfer address in a register
          transferAddrReg := nextTransferAddr

          // check if spilling is actually needed
          val needsSpill = Mux(resSpillOp, Bool(true), (memTopReg - nextStackTop) > UInt(SCACHE_SIZE))
          stateReg := Mux(needsSpill, holdSpillState, idleState)
        }
      }
    }

    // // // // // // // // // // // // // // // // // // // // // // // // //
    // SPILLING
    // // // // // // // // // // // // // // // // // // // // // // // // //
    is(holdSpillState) {

      val nextTransferAddr = transferAddrReg + UInt(BYTES_PER_WORD)

      // only write the data that actually needs spilling
      val writeEnable = Mux(resSpillOpReg, (transferAddrReg < memTopReg), ((stackTopReg + UInt(SCACHE_SIZE)) <= transferAddrReg) &
        (transferAddrReg < memTopReg))

      // generate an OCP write request
      io.toMemory.M.Cmd := OcpCmd.WR
      io.toMemory.M.Addr := transferAddrReg
      io.toMemory.M.Data := mb_rdData
      io.toMemory.M.DataValid := UInt(1)
      io.toMemory.M.DataByteEn := Fill(4, writeEnable)

      // check if command has been accepted
      val accepted = io.toMemory.S.CmdAccept === UInt(1)

      // read next data element once accepted, otherwise hold
      mb_rdAddr := Mux(accepted, nextTransferAddr.apply(scSizeBits + wordBits - 1,
        wordBits),

        rdAddrReg)

      // increment transfer address if accepted
      transferAddrReg := Mux(accepted, nextTransferAddr, transferAddrReg)

      // advance stateReg if accepted
      stateReg := Mux(accepted, spillState, holdSpillState)
    }

    is(spillState) {
      val nextTransferAddr = transferAddrReg + UInt(BYTES_PER_WORD)

      // only write the data that actually needs spilling
      val writeEnable = Mux(resSpillOpReg, transferAddrReg < memTopReg, 
          ((stackTopReg + UInt(SCACHE_SIZE)) <= transferAddrReg) &
        (transferAddrReg < memTopReg))

      // read next data element from the stack cache's memory
      mb_rdAddr := nextTransferAddr.apply(scSizeBits + wordBits - 1, wordBits)

      // hand current data element over to the OCP bus
      io.toMemory.M.Data := mb_rdData
      io.toMemory.M.DataValid := UInt(1)
      io.toMemory.M.DataByteEn := Fill(4, writeEnable)

      // increment transfer address and advance stateReg
      transferAddrReg := nextTransferAddr

      stateReg := Mux(burstCounter === UInt(BURST_LENGTH - 1),
        waitSpillState, spillState)
    }

    is(waitSpillState) {
      // check whether all data has been spilled
      val spillingDone = memTopReg <= transferAddrReg

      // wait for a response from the memory, if all data has been transfered
      // return to the IDLE stateReg
      stateReg := Mux(io.toMemory.S.Resp === OcpResp.DVA,
        Mux(spillingDone, idleState, holdSpillState),
        waitSpillState)

      // done? finally compute the new memory top pointer
      memTopReg := Mux(resSpillOpReg,  Mux(spillingDone, transferAddrReg, memTopReg), Mux(spillingDone, stackTopReg + UInt(SCACHE_SIZE), memTopReg))

      // if more spilling is needed preserve the stack cache's read address
      mb_rdAddr := Mux(spillingDone, UInt(0), rdAddrReg)

    }

    // // // // // // // // // // // // // // // // // // // // // // // // //
    // FILLING
    // // // // // // // // // // // // // // // // // // // // // // // // //
    is(fillState) {
      // generate an OCP read request and wait that it is accepted
      io.toMemory.M.Cmd := OcpCmd.RD
      io.toMemory.M.Addr := transferAddrReg

      // go to next stateReg
      stateReg := Mux(io.toMemory.S.CmdAccept === Bits(1),
      waitFillState, fillState)
    }

    is(waitFillState) {
      io.toMemory.M.Addr := transferAddrReg

      when(io.toMemory.S.Resp === OcpResp.DVA) {
        // check whether all data has been filled
        val fillingDone = requiredMemTopReg <= transferAddrReg

        // check whether data should be written to the stack cache's memory
        val writeEnable = !fillingDone && memTopReg <= transferAddrReg

        // write to the stack cache's memory
        mb_wrEna := Fill(4, writeEnable)
        mb_wrData := io.toMemory.S.Data
        mb_wrAddr := transferAddrReg.apply(scSizeBits + wordBits - 1, wordBits)

        // increment transfer address
        transferAddrReg := transferAddrReg + UInt(BYTES_PER_WORD)

        // go to next stateReg
        stateReg := Mux(burstCounter === UInt(BURST_LENGTH - 1),
          Mux(fillingDone, idleState, fillState),
          waitFillState)

        // done? finally compute the new memory top pointer
        memTopReg := Mux(fillingDone, requiredMemTopReg, memTopReg)
      }
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  // Stack Cache Memory Interface (loads/stores)
  //////////////////////////////////////////////////////////////////////////////

  // send response and (potential) read-data to CPU
  io.fromCPU.S.Resp := responseToCPUReg
  io.fromCPU.S.Data := mb_rdData

  // handle read/write requests from CPU
  when(io.fromCPU.M.Cmd === OcpCmd.WR) {
    // write to the stack cache's memory
    mb_wrEna := io.fromCPU.M.ByteEn
    mb_wrData := io.fromCPU.M.Data
    mb_wrAddr := (io.fromCPU.M.Addr + stackTopReg).apply(scSizeBits + wordBits - 1,
      wordBits)

    // generate response that indicates that the write has completed 
    responseToCPUReg := OcpResp.DVA
  }
    .elsewhen(io.fromCPU.M.Cmd === OcpCmd.RD) {
      // Read from the stack cache's memory
      mb_rdAddr := (io.fromCPU.M.Addr + stackTopReg).apply(scSizeBits + wordBits - 1,
        wordBits)

      // generate response that indicates that the write has completed 
      responseToCPUReg := OcpResp.DVA
    }

  //////////////////////////////////////////////////////////////////////////////
  // Fiddle with signals to/from memory
  //////////////////////////////////////////////////////////////////////////////

  for (i <- 0 until BYTES_PER_WORD) {
    memoryBlock(i) <= (mb_wrEna.apply(i), mb_wrAddr,
      mb_wrData(BYTE_WIDTH * (i + 1) - 1, BYTE_WIDTH * i))
    memoryBlock(i).rdAddr := mb_rdAddr
  }

  //////////////////////////////////////////////////////////////////////////////
  // preserve some signals for debugging
  //////////////////////////////////////////////////////////////////////////////

  debug(mb_rdAddr)
  debug(mb_rdData)
  debug(mb_wrAddr)
  debug(mb_wrData)
  debug(mb_wrEna)
}








