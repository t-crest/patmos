/*
 * Synchronous SRAM memory controller for the GS832018/32/36AGT.
 *
 * Author: Christos Gkiokas (gkiokasc@gmail.com)
 *
 */

package io


import Chisel._
import scala.math._
import patmos.Constants._
import ocp._

import patmos.MemBlock
import patmos.MemBlockIO


/*
 Connections to the SRAM
*/
class RamInPins extends Bundle() {
  val din = Bits(INPUT, width = 32)
}
class RamOutPins() extends Bundle() {
  val addr = Bits(OUTPUT, width = 20)
  val doutEna = Bits(OUTPUT, width = 1)     //needed to drive tristate in top level
  val zz = Bits(OUTPUT, width = 1)          // Sleep mode
  val ft = Bits(OUTPUT, width = 1)          // Output data register enable
  val lbo = Bits(OUTPUT, width = 1)         // 0: Linear Burst Mode, 1: Interleaved Burst Mode
  val adsp = Bits(OUTPUT, width = 1)        // Burst write config: 1 for burst off
  val adsc = Bits(OUTPUT, width = 1)        // Burst write config: 0 for burst off
  val adv = Bits(OUTPUT, width = 1)         // Burst counter advance
  val bw = Bits(OUTPUT, width = 1)          // Global byte write
  val bwe = Bits(OUTPUT, width = 4)       // Bank byte enables
  val e1 = Bits(OUTPUT, width = 1)          // Chip enable active low
  val e2 = Bits(OUTPUT, width = 1)          // Chip enable active high
  val e3 = Bits(OUTPUT, width = 1)          // Chip enable active low
  val gw = Bits(OUTPUT, width = 1)          // Global write
  val g = Bits(OUTPUT, width = 1)           // Output enable (read)
  val stateLeds = Bits(OUTPUT,width = 3)
 
  val dout = Bits(OUTPUT,width = 32)
}

object SSramGSICtrl extends DeviceObject {
  var addrBits = -1

  def init(params: Map[String, String]) = {
    addrBits = getPosIntParam(params, "ocpAddrWidth")
  }

  def create(params: Map[String, String]) : SSramGSICtrl = {
    Module(new SSramGSICtrl(addrBits = addrBits))
  }

}


class SSramGSICtrl (
   addrBits : Int = 20,
   ramWsRd : Int = 1,
   ramWsWr : Int = 0,
   burstLen : Int = 4
) extends BurstDevice(addrBits) {
   // Override
    override val io = new BurstDeviceIO(addrBits) with patmos.HasPins {
    override val pins = new Bundle() {
      val ramOut = new RamOutPins().asOutput()
      val ramIn = new RamInPins().asInput()
    }
  }
  def log2upNew(in: Int) = ceil(log(in)/log(2)).toInt
  val BYTE_WIDTH = 8
  val BYTES_PER_WORD = io.ocp.M.Data.getWidth / BYTE_WIDTH

  val idle :: rd :: rd1 :: rd2 :: rd3 :: rd4 :: rd5 :: rd6 :: rd7 :: rdf :: wr1 :: Nil = Enum(UInt(), 11)
  val ssramState = Reg(init = idle)
  val address = Reg(Bits(width = addrBits))
  val rdDataEna = Reg(Bits(width = 1))
  val rdData = Reg(Bits(width = 32))
  val resp = Reg(Bits(width = 2))
  val ramDout = Reg(Bits(width = 32))
  val doutEna = Reg(Bits(width = 1))
  val waitState = Reg(UInt(width = 4))
  val burstCnt = Reg(UInt(width = log2Up(burstLen)))


  val zz = Reg(Bits(width = 1))              //Sleep
  val ft = Reg(Bits(width = 1))              //Flow-through or Pipeline mode -> set as flow-through
  val lbo = Reg(Bits(width = 1))             //Linear burst order ->
  val adsp = Reg(Bits(width = 1))            //Address strobe controls
  val adsc = Reg(Bits(width = 1))            //Address strobe controls
  val adv = Wire(Bits(width = 1))             //Burst address counter advance
  val bw = Wire(Bits(width = 1))             //Byte write all bytes
  val bwe = Wire(Bits(width = 4))            //Byte write per byte
  val e1 = Reg(Bits(width = 1))              //Chip enable active low
  val e2 = Reg(Bits(width = 1))              //Chip enable active high
  val e3 = Reg(Bits(width = 1))              //Chip enable active low
  val gw = Reg(Bits(width = 1))              //Global write
  val g = Reg(Bits(width = 1))               //Read
  

  val cmdAccept = Reg(Bits(width = 1))
  val dataAccept = Reg(Bits(width = 1))
  
 //init default register values
  rdDataEna := Bits(0)
  doutEna := Bits(0)
  adsc := Bits(1)
  e1 := Bits(1)
  //bwe := Bits("b1111")
  bwe := ~(io.ocp.M.DataByteEn)

  bw := Bits(1)
  gw := Bits(1)
  g := Bits(1)

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
 
  when (rdDataEna === Bits(1)) {
    io.ocp.S.Data := io.pins.ramIn.din
    rdData := io.pins.ramIn.din //read data can be used depending how the top-level keeps register of input or not
  }.otherwise
  {
    io.ocp.S.Data := rdData
  }

  when (ssramState === rd) {
    ssramState := rd1
    adsc := Bits(0)
    e1 := Bits(0)
    g := Bits(0)
    rdDataEna := Bits(1)
    cmdAccept := Bits(0)
    }

  when (ssramState === rd1) {
    e1 := Bits(0)
    g := Bits(0)
    cmdAccept := Bits(1)      //read 1st addr - DVA1
    rdDataEna := Bits(1)      //burstCount here
    burstCnt := burstCnt + UInt(1)
    resp := OcpResp.DVA

    ssramState := rd2
    
  }

  when (ssramState === rd2)
  {
    e1 := Bits(0)
    g := Bits(0)
    cmdAccept := Bits(1)    //read 2nd addr -- DVA2
    rdDataEna := Bits(1)
    burstCnt := burstCnt + UInt(1)
    resp := OcpResp.DVA

    ssramState := rd3
  }
  
  when (ssramState === rd3) 
  {
    e1 := Bits(0)
    g := Bits(0)
    cmdAccept := Bits(1)  //read 3d addr --- DVA3
    rdDataEna := Bits(1)  //burstCount here
    burstCnt := burstCnt + UInt(1) 
    resp := OcpResp.DVA

    ssramState := rd4
  }

  when (ssramState === rd4) 
  {
    e1 := Bits(0)
    g := Bits(0)
    cmdAccept := Bits(1) //read 4th addr ---DVA4 
    rdDataEna := Bits(1)
    burstCnt := burstCnt + UInt(1)
    resp := OcpResp.DVA

    ssramState := rdf   //go to RDF
  }
  
  when (ssramState === rd5) 
  {
        e1 := Bits(0)
    g := Bits(0)
    cmdAccept := Bits(1)
    rdDataEna := Bits(1)
    burstCnt := burstCnt 
    resp := OcpResp.DVA

    ssramState := rd6

  }
  when (ssramState === rd6) 
  {
        e1 := Bits(0)
    g := Bits(0)
    cmdAccept := Bits(1)
    rdDataEna := Bits(1)
    burstCnt := burstCnt + UInt(1)
    resp := OcpResp.DVA

    ssramState := rd7

  }
    when (ssramState === rd7) 
  {
    e1 := Bits(0)
    g := Bits(0)
    cmdAccept := Bits(1)
    rdDataEna := Bits(0)
    burstCnt := burstCnt 
    resp := OcpResp.DVA

    ssramState := rdf
  }

  when(ssramState === rdf)
  {
    burstCnt := UInt(0)
    g := Bits(1)
    e1 := Bits(0)
    rdDataEna := Bits(0)
    cmdAccept := Bits(1)
    resp := OcpResp.NULL

    ssramState := idle
  }

  when (ssramState === wr1) {
    cmdAccept := Bits(1)
    when (waitState <= UInt(1)) {
      when (burstCnt === UInt(burstLen-1)) {
        burstCnt := UInt(0)
        bw := Bits(1)
        resp := OcpResp.DVA
        cmdAccept := Bits(1)
        ssramState := idle
      }
      when (io.ocp.M.DataValid === Bits(1)) {
        dataAccept := Bits(1)
        burstCnt := burstCnt + UInt(1)
        adsc := Bits(1)
        bw := Bits(0)
  
        doutEna := Bits(1)
      }
    }
  }

  when (io.ocp.M.Cmd === OcpCmd.RD) {
    ssramState := rd
    //adsc := Bits(0)
    //e1 := Bits(0)
    g := Bits(0)
    //rdDataEna := Bits(1)
    cmdAccept := Bits(0)
  
  }
  .elsewhen(io.ocp.M.Cmd === OcpCmd.WR && io.ocp.M.DataValid === Bits(1)) {
    cmdAccept := Bits(1)
    dataAccept := Bits(1)
    ssramState := wr1
    adsc := Bits(0)
    e1 := Bits(0)
    //bw := Bits(0)
 
    doutEna := Bits(1)
  }

  //counter till output is ready
  when (waitState =/= UInt(0)) {
    waitState := waitState - UInt(1)
  }
  //set wait state after incoming request
  when (io.ocp.M.Cmd === OcpCmd.RD) {
    waitState := UInt(ramWsRd + 1)
  }
  when (io.ocp.M.Cmd === OcpCmd.WR) {
    waitState := UInt(ramWsWr + 1)
  }

  io.pins.ramOut.dout := io.ocp.M.Data
  when (doutEna === Bits(1)) {
    io.pins.ramOut.dout := ramDout
  }

  //output registers
  io.pins.ramOut.addr := Cat(address(addrBits-3, log2upNew(burstLen)), burstCnt)
  //io.pins.ramOut.addr := address
   //output registers
  io.pins.ramOut.adsc := Bits(0)            //Address strobe controls
  //io.pins.ramOut.addr := Cat(address(addrBits-3, log2Up(burstLen)), burstCnt)
  io.pins.ramOut.e1 := Bits(0)              //Chip enable active low

  io.pins.ramOut.doutEna := doutEna        //Tristate control
  
  io.pins.ramOut.gw := Bits(1)              //Global write
  //output to master
  io.ocp.S.Resp := resp
  io.ocp.S.DataAccept := dataAccept
  io.ocp.S.CmdAccept := cmdAccept

    //Fixed signals setup
  io.pins.ramOut.bw := bw              //Byte write all bytes
  io.pins.ramOut.bwe := ~(io.ocp.M.DataByteEn)  //Byte write per byte
  io.pins.ramOut.adv := Bits(1)             //Burst address counter advance
  io.pins.ramOut.g :=  g              //Read
  io.pins.ramOut.lbo := Bits(0)             //Linear burst order ->
  io.pins.ramOut.zz := Bits(0)              //Sleep
  io.pins.ramOut.ft := Bits(1)               //Flow-through or Pipeline mode -> set as flow-through
  
  io.pins.ramOut.e2 := Bits(1)              //Chip enable active high
  io.pins.ramOut.e3 := Bits(0)              //Chip enable active low
  io.pins.ramOut.adsp := Bits(1)            //Address strobe controls

  //Debug
  io.pins.ramOut.stateLeds := Bits(0)
}