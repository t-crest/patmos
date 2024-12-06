/*
 * SSRAM connection to memory bus (f.e. DE2-70 board)
 *
 * Author: Philipp Degasperi (philipp.degasperi@gmail.com)
 *
 */

package io

import chisel3._
import chisel3.util._
import patmos.Constants._
import ocp._

import scala.collection.mutable.HashMap

/*
 Connections to the SRAM
*/
class RamInType extends Bundle() {
  val din = UInt(32.W)
}
class RamOutType(addrBits: Int) extends Bundle() {
  val addr = UInt(addrBits.W)
  val doutEna = UInt(1.W) //needed to drive tristate in top level
  val nadsc = UInt(1.W)
  val noe = UInt(1.W)
  val nbwe = UInt(1.W)
  val nbw = UInt(4.W)
  val ngw = UInt(1.W)
  val nce1 = UInt(1.W)
  val ce2 = UInt(1.W)
  val nce3 = UInt(1.W)
  val nadsp = UInt(1.W)
  val nadv = UInt(1.W)
  val dout = UInt(32.W)
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
    val pins: Bundle {
      val ramOut: RamOutType
      val ramIn: RamInType
    } = new Bundle() {
      val ramOut = Output(new RamOutType(addrBits-2))
      val ramIn = Input(new RamInType())
    }
  }

  val idle :: rd1 :: wr1 :: Nil = Enum(3)
  val ssramState = RegInit(init = idle)
  val waitState = Reg(UInt(4.W))
  val burstCnt = Reg(UInt(log2Up(burstLen).W))
  val rdDataEna = Reg(UInt(1.W))
  val rdData = Reg(UInt(32.W))
  val resp = Reg(UInt(2.W))
  val ramDout = Reg(UInt(32.W))
  val address = Reg(UInt((addrBits-2).W))
  val doutEna = Reg(UInt(1.W))
  val nadsc = Reg(UInt(1.W))
  val noe = Reg(UInt(1.W))
  val nbwe = Reg(UInt(1.W))
  val nbw = Reg(UInt(4.W))
  val nadv = Reg(UInt(1.W))
  val cmdAccept = UInt(1.W)
  val dataAccept = UInt(1.W)

  //init default register values
  rdDataEna := 0.U
  doutEna := 0.U
  nadsc := 1.U
  noe := 1.U
  nbwe := 1.U
  nbw := "b1111".U
  nadv := 1.U
  resp := OcpResp.NULL
  ramDout := io.ocp.M.Data
  burstCnt := 0.U
  dataAccept := 0.U
  cmdAccept := 1.U

  //catch inputs
  when (io.ocp.M.Cmd === OcpCmd.RD || io.ocp.M.Cmd === OcpCmd.WR) {
    address := io.ocp.M.Addr(addrBits-1, 2)
  }

  //following helps to output only when output data is valid
  io.ocp.S.Data := rdData
  when (rdDataEna === 1.U) {
    io.ocp.S.Data := io.pins.ramIn.din
    rdData := io.pins.ramIn.din //read data can be used depending how the top-level keeps register of input or not
  }

  when (ssramState === rd1) {
    noe := 0.U
    nadv := 0.U
    cmdAccept := 0.U
    when (waitState <= 1.U) {
      rdDataEna := 1.U
      burstCnt := burstCnt + 1.U
      resp := OcpResp.DVA
      when (burstCnt === (burstLen-1).U) {
        burstCnt := 0.U
        nadv := 1.U
        noe := 1.U
        cmdAccept := 1.U
        ssramState := idle
      }
    }
  }
  when (ssramState === wr1) {
    cmdAccept := 0.U
    when (waitState <= 1.U) {
      when (burstCnt === (burstLen-1).U) {
        burstCnt := 0.U
        resp := OcpResp.DVA
        cmdAccept := 1.U
        ssramState := idle
      }
      when (io.ocp.M.DataValid === 1.U) {
        dataAccept := 1.U
        burstCnt := burstCnt + 1.U
        nadsc := 0.U
        nbwe := 0.U
        nbw := ~(io.ocp.M.DataByteEn)
        doutEna := 1.U
      }
    }
  }

  when (io.ocp.M.Cmd === OcpCmd.RD) {
    ssramState := rd1
    nadsc := 0.U
    noe := 0.U
  }
  .elsewhen(io.ocp.M.Cmd === OcpCmd.WR && io.ocp.M.DataValid === 1.U) {
    dataAccept := 1.U
    ssramState := wr1
    nadsc := 0.U
    nbwe := 0.U
    nbw := ~(io.ocp.M.DataByteEn)
    doutEna := 1.U
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
  when (doutEna === 1.U) {
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
  io.pins.ramOut.ngw := 1.U
  io.pins.ramOut.nce1 := 0.U
  io.pins.ramOut.ce2 := 1.U
  io.pins.ramOut.nce3 := 0.U
  io.pins.ramOut.nadsp := 1.U
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

    emitVerilog(new SSRam32Ctrl(addrBits), chiselArgs)
  }
}

/*
 External Memory, only to simulate a SSRAM in Chisel as a on-chip memory implementation
 and reading some data from binary to memory vector

 >>> This is handled by the emulator now! <<<
*/
class ExtSsram(addrBits : Int, fileName : String) extends Module {
  val io = new Bundle() {
    val ramOut = Input(new RamOutType(addrBits))
    val ramIn = Output(new RamInType())
  }

  //on chip memory instance
  val ssram_extmem = Mem(2 * ICACHE_SIZE, UInt(32.W)) //bus width = 32

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
      printf("%08x\n", word.U)
      // mmh, width is needed to keep bit 31
      ssram_extmem(i) := word.U(32.W)
    }
    // generate some dummy data to fill the table and make Bit 31 test happy
    for (x <- byteArray.length / 4 until ICACHE_SIZE * 2)
      ssram_extmem(x) := "h80000000".U
    ssram_extmem
  }

  //initSsram(fileName)
  val address = RegInit(init = 0.U(19.W))
  val dout = RegInit(init = 0.U(32.W))
  val nadv = RegInit(init = 0.U(1.W))

  nadv := io.ramOut.nadv
  when (io.ramOut.nadsc === 0.U) {
    address := io.ramOut.addr
  }
  .elsewhen (nadv === 0.U) {
    address := address + 1.U
  }
  when (io.ramOut.noe === 0.U) {
    dout := ssram_extmem(address)
  }
  io.ramIn.din := dout
}

