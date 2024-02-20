/*
 * SSRAM connection to memory bus (f.e. DE2-70 board)
 *
 * Author: Philipp Degasperi (philipp.degasperi@gmail.com)
 *
 */

package io

import Chisel._
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
  override val io = new BurstDeviceIO(addrBits) with patmos.HasPins {
    override val pins = new Bundle() {
      val ramOut = new RamOutType(addrBits-2).asOutput
      val ramIn = Input(new RamInType())
    }
  }

  val idle :: rd1 :: wr1 :: Nil = Enum(UInt(), 3)
  val ssramState = Reg(init = idle)
  val waitState = Reg(UInt(4.W))
  val burstCnt = Reg(UInt(log2Up(burstLen).W))
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
  burstCnt := 0.U
  dataAccept := Bits(0)
  cmdAccept := Bits(1)

  //catch inputs
  when (io.ocp.M.Cmd === OcpCmd.RD || io.ocp.M.Cmd === OcpCmd.WR) {
    address := io.ocp.M.Addr(addrBits-1, 2)
  }

  //following helps to output only when output data is valid
  io.ocp.S.Data := rdData
  when (rdDataEna === Bits(1)) {
    io.ocp.S.Data := io.pins.ramIn.din
    rdData := io.pins.ramIn.din //read data can be used depending how the top-level keeps register of input or not
  }

  when (ssramState === rd1) {
    noe := Bits(0)
    nadv := Bits(0)
    cmdAccept := Bits(0)
    when (waitState <= 1.U) {
      rdDataEna := Bits(1)
      burstCnt := burstCnt + 1.U
      resp := OcpResp.DVA
      when (burstCnt === (burstLen-1).U) {
        burstCnt := 0.U
        nadv := Bits(1)
        noe := Bits(1)
        cmdAccept := Bits(1)
        ssramState := idle
      }
    }
  }
  when (ssramState === wr1) {
    cmdAccept := Bits(0)
    when (waitState <= 1.U) {
      when (burstCnt === (burstLen-1).U) {
        burstCnt := 0.U
        resp := OcpResp.DVA
        cmdAccept := Bits(1)
        ssramState := idle
      }
      when (io.ocp.M.DataValid === Bits(1)) {
        dataAccept := Bits(1)
        burstCnt := burstCnt + 1.U
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
  when (waitState =/= 0.U) {
    waitState := waitState - 1.U
  }
  //set wait state after incoming request
  when (io.ocp.M.Cmd === OcpCmd.RD) {
    waitState := (ramWsRd + 1).U
  }
  when (io.ocp.M.Cmd === OcpCmd.WR) {
    waitState := (ramWsWr + 1).U
  }

  io.pins.ramOut.dout := io.ocp.M.Data
  when (doutEna === Bits(1)) {
    io.pins.ramOut.dout := ramDout
  }

  //output registers
  io.pins.ramOut.nadsc := nadsc
  io.pins.ramOut.noe := noe
  io.pins.ramOut.nbwe := nbwe
  io.pins.ramOut.nbw := nbw
  io.pins.ramOut.nadv := nadv
  io.pins.ramOut.doutEna := doutEna
  io.pins.ramOut.addr := Cat(address(addrBits-3, log2Up(burstLen)), burstCnt)
  //output to master
  io.ocp.S.Resp := resp
  io.ocp.S.DataAccept := dataAccept
  io.ocp.S.CmdAccept := cmdAccept
  //output fixed signals
  io.pins.ramOut.ngw := Bits(1)
  io.pins.ramOut.nce1 := Bits(0)
  io.pins.ramOut.ce2 := Bits(1)
  io.pins.ramOut.nce3 := Bits(0)
  io.pins.ramOut.nadsp := Bits(1)
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
    val ramIn = Output(new RamInType())
  }

  //on chip memory instance
  val ssram_extmem = Mem(Bits(width = 32), 2 * ICACHE_SIZE) //bus width = 32

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
    for (x <- byteArray.length / 4 until ICACHE_SIZE * 2)
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

