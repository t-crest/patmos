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
 * SSRAM connection to memory bus (f.e. DE2-70 board)
 *
 * Author: Philipp Degasperi (philipp.degasperi@gmail.com)
 *
 */

package io

import Chisel._
import Node._
import patmos.Constants._
import ocp._

import scala.collection.mutable.HashMap

/*
 Connections to the SRAM
*/
class RamInType extends Bundle() {
  val din = Bits(width = 32)
}
class RamOutType(addrBits: Int) extends Bundle() {
  val addr = Bits(width = addrBits)
  val doutEna = Bits(width = 1) //needed to drive tristate in top level
  val nadsc = Bits(width = 1)
  val noe = Bits(width = 1)
  val nbwe = Bits(width = 1)
  val nbw = Bits(width = 4)
  val ngw = Bits(width = 1)
  val nce1 = Bits(width = 1)
  val ce2 = Bits(width = 1)
  val nce3 = Bits(width = 1)
  val nadsp = Bits(width = 1)
  val nadv = Bits(width = 1)
  val dout = Bits(width = 32)
}

object SSRam32Ctrl extends DeviceObject {
  var addrBits = -1

  def init(params: Map[String, String]) = {
    addrBits = getPosIntParam(params, "ocpAddrWidth")
  }

  def create(params: Map[String, String]) : SSRam32Ctrl = {
    Module(new SSRam32Ctrl(addrBits = addrBits, burstLen = BURST_LENGTH))
  }

  trait Pins {
    val sSRam32CtrlPins = new Bundle() {
      val ramOut = new RamOutType(addrBits-2).asOutput
      val ramIn = new RamInType().asInput
    }
  }
}

/*
  SSRAM Controller for a Burst R/W access to the SSRAM
  Notes: Burst addresses in the used SSRAM are generated internally only for max. 4 addresses (= max. burst length)
  >> setting the mode to 0 a linear burst is possible, setting mode to 1 a interleaved burst is done by the SSRAM
*/
class SSRam32Ctrl (
   addrBits : Int,
   ramWsRd : Int = 2,
   ramWsWr : Int = 0,
   burstLen : Int = 4
) extends BurstDevice(addrBits) {
  override val io = new BurstDeviceIO(addrBits) with SSRam32Ctrl.Pins

  val idle :: rd1 :: wr1 :: Nil = Enum(UInt(), 3)
  val ssramState = Reg(init = idle)
  val waitState = Reg(UInt(width = 4))
  val burstCnt = Reg(UInt(width = log2Up(burstLen)))
  val rdDataEna = Reg(Bits(width = 1))
  val rdData = Reg(Bits(width = 32))
  val resp = Reg(Bits(width = 2))
  val ramDout = Reg(Bits(width = 32))
  val address = Reg(Bits(width = addrBits-2))
  val doutEna = Reg(Bits(width = 1))
  val nadsc = Reg(Bits(width = 1))
  val noe = Reg(Bits(width = 1))
  val nbwe = Reg(Bits(width = 1))
  val nbw = Reg(Bits(width = 4))
  val nadv = Reg(Bits(width = 1))
  val cmdAccept = Bits(width = 1)
  val dataAccept = Bits(width = 1)

  //init default register values
  rdDataEna := Bits(0)
  doutEna := Bits(0)
  nadsc := Bits(1)
  noe := Bits(1)
  nbwe := Bits(1)
  nbw := Bits("b1111")
  nadv := Bits(1)
  resp := OcpResp.NULL
  ramDout := io.ocp.M.Data
  burstCnt := UInt(0)
  dataAccept := Bits(0)
  cmdAccept := Bits(1)

  //catch inputs
  when (io.ocp.M.Cmd === OcpCmd.RD || io.ocp.M.Cmd === OcpCmd.WR) {
    address := io.ocp.M.Addr(addrBits-1, 2)
  }

  //following helps to output only when output data is valid
  io.ocp.S.Data := rdData
  when (rdDataEna === Bits(1)) {
    io.ocp.S.Data := io.sSRam32CtrlPins.ramIn.din
    rdData := io.sSRam32CtrlPins.ramIn.din //read data can be used depending how the top-level keeps register of input or not
  }

  when (ssramState === rd1) {
    noe := Bits(0)
    nadv := Bits(0)
    cmdAccept := Bits(0)
    when (waitState <= UInt(1)) {
      rdDataEna := Bits(1)
      burstCnt := burstCnt + UInt(1)
      resp := OcpResp.DVA
      when (burstCnt === UInt(burstLen-1)) {
        burstCnt := UInt(0)
        nadv := Bits(1)
        noe := Bits(1)
        cmdAccept := Bits(1)
        ssramState := idle
      }
    }
  }
  when (ssramState === wr1) {
    cmdAccept := Bits(0)
    when (waitState <= UInt(1)) {
      when (burstCnt === UInt(burstLen-1)) {
        burstCnt := UInt(0)
        resp := OcpResp.DVA
        cmdAccept := Bits(1)
        ssramState := idle
      }
      when (io.ocp.M.DataValid === Bits(1)) {
        dataAccept := Bits(1)
        burstCnt := burstCnt + UInt(1)
        nadsc := Bits(0)
        nbwe := Bits(0)
        nbw := ~(io.ocp.M.DataByteEn)
        doutEna := Bits(1)
      }
    }
  }

  when (io.ocp.M.Cmd === OcpCmd.RD) {
    ssramState := rd1
    nadsc := Bits(0)
    noe := Bits(0)
  }
  .elsewhen(io.ocp.M.Cmd === OcpCmd.WR && io.ocp.M.DataValid === Bits(1)) {
    dataAccept := Bits(1)
    ssramState := wr1
    nadsc := Bits(0)
    nbwe := Bits(0)
    nbw := ~(io.ocp.M.DataByteEn)
    doutEna := Bits(1)
  }

  //counter till output is ready
  when (waitState != UInt(0)) {
    waitState := waitState - UInt(1)
  }
  //set wait state after incoming request
  when (io.ocp.M.Cmd === OcpCmd.RD) {
    waitState := UInt(ramWsRd + 1)
  }
  when (io.ocp.M.Cmd === OcpCmd.WR) {
    waitState := UInt(ramWsWr + 1)
  }

  io.sSRam32CtrlPins.ramOut.dout := io.ocp.M.Data
  when (doutEna === Bits(1)) {
    io.sSRam32CtrlPins.ramOut.dout := ramDout
  }

  //output registers
  io.sSRam32CtrlPins.ramOut.nadsc := nadsc
  io.sSRam32CtrlPins.ramOut.noe := noe
  io.sSRam32CtrlPins.ramOut.nbwe := nbwe
  io.sSRam32CtrlPins.ramOut.nbw := nbw
  io.sSRam32CtrlPins.ramOut.nadv := nadv
  io.sSRam32CtrlPins.ramOut.doutEna := doutEna
  io.sSRam32CtrlPins.ramOut.addr := Cat(address(addrBits-3, log2Up(burstLen)), burstCnt)
  //output to master
  io.ocp.S.Resp := resp
  io.ocp.S.DataAccept := dataAccept
  io.ocp.S.CmdAccept := cmdAccept
  //output fixed signals
  io.sSRam32CtrlPins.ramOut.ngw := Bits(1)
  io.sSRam32CtrlPins.ramOut.nce1 := Bits(0)
  io.sSRam32CtrlPins.ramOut.ce2 := Bits(1)
  io.sSRam32CtrlPins.ramOut.nce3 := Bits(0)
  io.sSRam32CtrlPins.ramOut.nadsp := Bits(1)
}

/*
 Test Class for the SSRAM implementation

class SsramTest(c: SSRam32Ctrl) extends Tester(c) {
  println("RUN")
  for (i <- 0 until 100) {
    step(1)
  }
}
 */

/*
 Used to instantiate a single SSRAM control component
 */
object SSRam32Main {
  def main(args: Array[String]): Unit = {

    val chiselArgs = args.slice(1, args.length)
    val addrBits = args(0).toInt

    chiselMain(chiselArgs, () => Module(new SSRam32Ctrl(addrBits)))
  }
}

/*
 External Memory, only to simulate a SSRAM in Chisel as a on-chip memory implementation
 and reading some data from binary to memory vector

 >>> This is handled by the emulator now! <<<
*/
class ExtSsram(addrBits : Int, fileName : String) extends Module {
  val io = new Bundle() {
    val ramOut = new RamOutType(addrBits).asInput
    val ramIn = new RamInType().asOutput
  }

  //on chip memory instance
  val ssram_extmem = Mem(Bits(width = 32), 2 * MCACHE_SIZE) //bus width = 32

  def initSsram(fileName: String): Mem[UInt] = {
    println("Reading " + fileName)
    // an encodig to read a binary file? Strange new world.
    val source = scala.io.Source.fromFile(fileName)(scala.io.Codec.ISO8859)
    val byteArray = source.map(_.toByte).toArray
    source.close()
    for (i <- 0 until byteArray.length / 4) {
      var word = 0
      for (j <- 0 until 4) {
        word <<= 8
        word += byteArray(i * 4 + j).toInt & 0xff
      }
      printf("%08x\n", Bits(word))
      // mmh, width is needed to keep bit 31
      ssram_extmem(Bits(i)) := Bits(word, width=32)
    }
    // generate some dummy data to fill the table and make Bit 31 test happy
    for (x <- byteArray.length / 4 until MCACHE_SIZE * 2)
      ssram_extmem(Bits(x)) := Bits("h80000000")
    ssram_extmem
  }

  //initSsram(fileName)
  val address = Reg(init = Bits(0, width = 19))
  val dout = Reg(init = Bits(0, width = 32))
  val nadv = Reg(init = Bits(0, width = 1))

  nadv := io.ramOut.nadv
  when (io.ramOut.nadsc === Bits(0)) {
    address := io.ramOut.addr
  }
  .elsewhen (nadv === Bits(0)) {
    address := address + Bits(1)
  }
  when (io.ramOut.noe === Bits(0)) {
    dout := ssram_extmem(address)
  }
  io.ramIn.din := dout
}

