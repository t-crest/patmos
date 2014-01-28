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
 * Connection definitions for the pipe stages.
 *
 * Authors: Martin Schoeberl (martin@jopdesign.com)
 *          Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package patmos

import Chisel._
import Node._

import Constants._

import ocp._

class FeDec() extends Bundle() {
  val instr_a = Bits(width = INSTR_WIDTH)
  val instr_b = Bits(width = INSTR_WIDTH)
  val pc = UInt(width = PC_SIZE)
  val reloc = UInt(width = ADDR_WIDTH)
}

object FeDecResetVal extends FeDec {
  instr_a := Bits(0)
  instr_b := Bits(0)
  pc := UInt(0)
  reloc := UInt(0)
}

class AluOp() extends Bundle() {
  val func = Bits(width = 4)
  val isMul = Bool()
  val isCmp = Bool()
  val isPred = Bool()
  val isMTS = Bool()
  val isMFS = Bool()
  val isSTC = Bool()
}

object AluOpResetVal extends AluOp {
  func := Bits(0)
  isMul := Bool(false)
  isCmp := Bool(false)
  isPred := Bool(false)
  isMTS := Bool(false)
  isMFS := Bool(false)
  isSTC := Bool(false)
}

class PredOp() extends Bundle() {
  val func = Bits(width = 2) // as they have a strange encoding
  val dest = Bits(width = PRED_BITS)
  val s1Addr = Bits(width = PRED_BITS+1)
  val s2Addr = Bits(width = PRED_BITS+1)
}

object PredOpResetVal extends PredOp {
  func := Bits(0)
  dest := Bits(0)
  s1Addr := Bits(0)
  s2Addr := Bits(0)
}

class JmpOp() extends Bundle() {
  val branch = Bool()
  val target = UInt(width = PC_SIZE)
  val reloc = UInt(width = ADDR_WIDTH)
}

object JmpOpResetVal extends JmpOp {
  branch := Bool(false)
  target := UInt(0)
  reloc := UInt(0)
}

class MemOp() extends Bundle() {
  val load = Bool()
  val store = Bool()
  val hword = Bool()
  val byte = Bool()
  val zext = Bool()
  val typ  = Bits(width = 2)
}

object MemOpResetVal extends MemOp {
  load := Bool(false)
  store := Bool(false)
  hword := Bool(false)
  byte := Bool(false)
  zext := Bool(false)
  typ := Bits(0)
}

class DecEx() extends Bundle() {
  val pc = UInt(width = PC_SIZE)
  val pred =  Vec.fill(PIPE_COUNT) { Bits(width = PRED_BITS+1) }
  val aluOp = Vec.fill(PIPE_COUNT) { new AluOp() }
  val predOp = Vec.fill(PIPE_COUNT) { new PredOp() }
  val jmpOp = new JmpOp()
  val memOp = new MemOp()

  // the register fields are very similar to RegFileRead
  // maybe join the structures
  val rsAddr = Vec.fill(2*PIPE_COUNT) { Bits(width = REG_BITS) }
  val rsData = Vec.fill(2*PIPE_COUNT) { Bits(width = DATA_WIDTH) }
  val rdAddr = Vec.fill(PIPE_COUNT) { Bits(width = REG_BITS) }
  val immVal = Vec.fill(PIPE_COUNT) { Bits(width = DATA_WIDTH) }
  val immOp  = Vec.fill(PIPE_COUNT) { Bool() }
  // maybe we should have similar structure as the Result one here
  val wrRd  = Vec.fill(PIPE_COUNT) { Bool() }

  val callAddr = UInt(width = DATA_WIDTH)
  val brcfAddr = UInt(width = DATA_WIDTH)
  val call = Bool()
  val ret = Bool()
  val brcf = Bool()
}

object DecExResetVal extends DecEx {
  pc := UInt(0)
  pred := Vec.fill(PIPE_COUNT) { Bits(0) }
  aluOp :=  Vec.fill(PIPE_COUNT) { AluOpResetVal }
  predOp := Vec.fill(PIPE_COUNT) { PredOpResetVal }
  jmpOp := JmpOpResetVal
  memOp := MemOpResetVal
  rsAddr := Vec.fill(2*PIPE_COUNT) { Bits(0) }
  rsData := Vec.fill(2*PIPE_COUNT) { Bits(0) }
  rdAddr := Vec.fill(PIPE_COUNT) { Bits(0) }
  immVal := Vec.fill(PIPE_COUNT) { Bits(0) }
  immOp := Vec.fill(PIPE_COUNT) { Bool(false) }
  wrRd := Vec.fill(PIPE_COUNT) { Bool(false) }
  callAddr := UInt(0)
  brcfAddr := UInt(0)
  call := Bool(false)
  ret := Bool(false)
  brcf := Bool(false)
}

class Result() extends Bundle() {
  val addr = Bits(width = REG_BITS)
  val data = Bits(width = DATA_WIDTH)
  val valid = Bool()
}

object ResultResetVal extends Result {
  addr := Bits(0)
  data := Bits(0)
  valid := Bool(false)
}

class MemIn() extends Bundle() {
  val load = Bool()
  val store = Bool()
  val hword = Bool()
  val byte = Bool()
  val zext = Bool()
  val typ = Bits(width = 2)
  val addr = Bits(width = DATA_WIDTH)
  val data = Bits(width = DATA_WIDTH)
  val call = Bool()
  val ret = Bool()
  val brcf = Bool()
  val callRetAddr = UInt(width = DATA_WIDTH)
  val callRetBase = UInt(width = DATA_WIDTH)
}

object MemInResetVal extends MemIn {
  load := Bool(false)
  store := Bool(false)
  hword := Bool(false)
  byte := Bool(false)
  zext := Bool(false)
  typ := Bits(0)
  addr := Bits(0)
  data := Bits(0)
  call := Bool(false)
  ret := Bool(false)
  brcf := Bool(false)
  callRetAddr := UInt(0)
  callRetBase := UInt(0)
}

class ExDec() extends Bundle() {
  val sp = UInt(width = DATA_WIDTH)
}

class ExMem() extends Bundle() {
  val rd = Vec.fill(PIPE_COUNT) { new Result() }
  val mem = new MemIn()
  val pc = UInt(width = PC_SIZE)
}

object ExMemResetVal extends ExMem {
  rd := Vec.fill(PIPE_COUNT) { ResultResetVal }
  mem := MemInResetVal
  pc := UInt(0)
}

class ExFe() extends Bundle() {
  val doBranch = Bool()
  val branchPc = UInt(width = PC_SIZE)
}

class MemFe() extends Bundle() {
  val doCallRet = Bool()
  val callRetPc = UInt(width = PC_SIZE)
  val callRetBase = UInt(width = PC_SIZE)
  // for ISPM write
  val store = Bool()
  val addr = Bits(width = DATA_WIDTH)
  val data = Bits(width = DATA_WIDTH)
}

class FeMem() extends Bundle() {
  val pc = UInt(width = MAX_OFF_WIDTH)
}

class MemWb() extends Bundle() {
  val rd = Vec.fill(PIPE_COUNT) { new Result() }
  // do we need this? probably not.
  // maybe drop unused pc fields
  // maybe nice for debugging?
  val pc = UInt(width = PC_SIZE)
}

class RegFileRead() extends Bundle() {
  // first two are for pipeline A, second two for pipeline B
  val rsAddr = Vec.fill(2*PIPE_COUNT) { Bits(INPUT, REG_BITS) }
  val rsData = Vec.fill(2*PIPE_COUNT) { Bits(OUTPUT, DATA_WIDTH) }
}

class RegFileIO() extends Bundle() {
  val ena = Bool(INPUT)
  val rfRead = new RegFileRead()
  val rfWrite = Vec.fill(PIPE_COUNT) { new Result().asInput }
}

class FetchIO extends Bundle() {
  val ena = Bool(INPUT)
  val fedec = new FeDec().asOutput
  // PC for returns
  val femem = new FeMem().asOutput
  // branch from EX
  val exfe = new ExFe().asInput
  // call from MEM
  val memfe = new MemFe().asInput
  //connections to mcache
  val femcache = new FeMCache().asOutput
  val mcachefe = new MCacheFe().asInput
}

class DecodeIO() extends Bundle() {
  val ena = Bool(INPUT)
  val fedec = new FeDec().asInput
  val decex = new DecEx().asOutput
  val exdec = new ExDec().asInput
  val rfWrite =  Vec.fill(PIPE_COUNT) { new Result().asInput }
}

class ExecuteIO() extends Bundle() {
  val ena = Bool(INPUT)
  val decex = new DecEx().asInput
  val exdec = new ExDec().asOutput
  val exmem = new ExMem().asOutput
  val exmcache = new ExMCache().asOutput
  // forwarding inputs
  val exResult = Vec.fill(PIPE_COUNT) { new Result().asInput }
  val memResult = Vec.fill(PIPE_COUNT) { new Result().asInput }
  // branch for FE
  val exfe = new ExFe().asOutput
}

class InOutIO() extends Bundle() {
  val memInOut = new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH)
  val comConf = new OcpIOMasterPort(ADDR_WIDTH, DATA_WIDTH)
  val comSpm = new OcpCoreMasterPort(ADDR_WIDTH, DATA_WIDTH)
}

class BootMemIO() extends Bundle() {
  val memInOut = new OcpCacheSlavePort(ADDR_WIDTH, DATA_WIDTH)
  val extMem = new OcpCacheMasterPort(ADDR_WIDTH, DATA_WIDTH)
}

class MemoryIO() extends Bundle() {
  val ena_out = Bool(OUTPUT)
  val ena_in = Bool(INPUT)
  val exmem = new ExMem().asInput
  val memwb = new MemWb().asOutput
  val memfe = new MemFe().asOutput
  val femem = new FeMem().asInput
  // for result forwarding
  val exResult = Vec.fill(PIPE_COUNT) { new Result().asOutput }
  // local and global accesses
  val localInOut = new OcpCoreMasterPort(ADDR_WIDTH, DATA_WIDTH)
  val globalInOut = new OcpCacheMasterPort(ADDR_WIDTH, DATA_WIDTH)
}

class WriteBackIO() extends Bundle() {
  val ena = Bool(INPUT)
  val memwb = new MemWb().asInput
  // wb result (unregistered)
  val rfWrite = Vec.fill(PIPE_COUNT) { new Result().asOutput }
  // for result forwarding (register)
  val memResult =  Vec.fill(PIPE_COUNT) { new Result().asOutput }
}

class PatmosCoreIO() extends Bundle() {
  val comConf = new OcpIOMasterPort(ADDR_WIDTH, DATA_WIDTH)
  val comSpm = new OcpCoreMasterPort(ADDR_WIDTH, DATA_WIDTH)
  val memPort = new OcpBurstMasterPort(EXTMEM_ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH)
}

class PatmosIO() extends Bundle() {
  val comConf = new OcpIOMasterPort(ADDR_WIDTH, DATA_WIDTH)
  val comSpm = new OcpCoreMasterPort(ADDR_WIDTH, DATA_WIDTH)
  //val mem_interface = new OcpBurstMasterPort(EXTMEM_ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH)
}
