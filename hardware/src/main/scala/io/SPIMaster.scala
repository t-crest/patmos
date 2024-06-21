/*
 * SPI master in chisel.
 *
 * Author: Chris Gkiokas (gkiokasc@gmail.com)
 *    
 *
 */

package io

import chisel3._
import chisel3.util._

import patmos.Constants._

import ocp._



object SPIMaster extends DeviceObject {
    var slaveCount = 1
    var sclkHz = 25000000
    var fifoDepth = -1
    var wordLen = 8;

  def init(params: Map[String, String]) = {
    slaveCount = getPosIntParam(params, "slaveCount")  //TODO
    sclkHz = getPosIntParam(params, "sclk_scale")
    fifoDepth = getPosIntParam(params, "fifoDepth")
    wordLen = getPosIntParam(params, "wordLength")

  }

  def create(params: Map[String, String]) : SPIMaster = {
    Module(new SPIMaster(CLOCK_FREQ, slaveCount, sclkHz, fifoDepth, wordLen))
  }
}

class SPIMaster(clkFreq : Int, slaveCount : Int, sclkHz : Int, fifoDepth : Int, wordLen : Int) extends CoreDevice() {
    
    override val io = new CoreDeviceIO() with patmos.HasPins {
        override val pins = new Bundle() {
            val sclk = Output(UInt(1.W))
            val mosi = Output(UInt(1.W))
            val miso = Input(UInt(1.W))
            val nSS = Output(UInt(slaveCount.W))
            }
        }

    //Sclk generation
    var sclkCounterN = log2Up(CLOCK_FREQ/sclkHz)

    val sclkCounterReg = RegInit(init = 0.U(32.W))
    val tick = sclkCounterReg === (sclkCounterN-1).U

    sclkCounterReg := sclkCounterReg + 1.U
    when (tick) {
      sclkCounterReg := 0.U
    }

    // send sm
    val idle :: send :: waitOne :: Nil  = Enum(3)
    val state = RegInit(init = idle)

    // Tx duration count
    val wordCounterReg = RegInit(init = 0.U(32.W))
    val wordDone = wordCounterReg === wordLen.U

    // IO Signal registers
    val sclkReg = RegInit(init = false.B)

    //val prevSclkReg = Reg(init = 0.U(1.W))


    val sclkEdge = sclkReg && !RegNext(sclkReg)
    val sclkFall = !sclkReg && RegNext(sclkReg)

    val mosiReg = RegInit(init = 0.U(1.W))
    

    val misoReg = RegInit(init = 0.U(1.W))
    val nSSReg = RegInit(init = 0.U(1.W))


    //Serial-in parallel out register for miso
    //val misoRxReg = Reg(init = 0.U(wordLen.W))
    val misoRxReg = Reg(Vec(wordLen, 0.U(1.W)))

    // Queue of received messages 
    val rxQueue = Module(new Queue(UInt(wordLen.W), fifoDepth))
    rxQueue.io.enq.bits     := misoRxReg.asUInt
    rxQueue.io.enq.valid    := false.B
    rxQueue.io.deq.ready    := false.B

    // Queue of messages to be sent
    val txQueue = Module(new Queue(UInt(wordLen.W), fifoDepth))
    txQueue.io.enq.bits     := io.ocp.M.Data(wordLen-1, 0)
    txQueue.io.enq.valid    := false.B
    txQueue.io.deq.ready    := false.B

    //Serial-out register for mosi
    val loadToSend = RegInit(init = false.B)
    loadToSend := false.B //Default value
    val mosiTxReg = RegInit(init = 0.U(wordLen.W))
    when (loadToSend) {
      txQueue.io.deq.ready := true.B
      mosiTxReg := txQueue.io.deq.bits
    } otherwise {
      //mosiTxReg := txQueue.io.deq.bits
    }

    // Default response
    val respReg = RegInit(init = OcpResp.NULL)
    respReg := OcpResp.NULL

    //Read response data register
    val rdDataReg = RegInit(init = 0.U(wordLen.W))
  
    // Connections to master
    io.ocp.S.Resp := respReg
    io.ocp.S.Data := rdDataReg
    

    //Read any stored data in miso queue. 
    when(io.ocp.M.Cmd === OcpCmd.RD) {
      when(rxQueue.io.count > 0.U)
      {
        rxQueue.io.deq.ready := true.B
        rdDataReg := rxQueue.io.deq.bits
        respReg := OcpResp.DVA
      }
    }

    //Activate master send
    when (io.ocp.M.Cmd === OcpCmd.WR) {
      // loadToSend := true.B
      respReg := OcpResp.DVA
      txQueue.io.enq.bits := io.ocp.M.Data(wordLen-1, 0)
      txQueue.io.enq.valid := true.B
    }



    when (state === idle)
    {
      nSSReg := 1.U
      wordCounterReg := 0.U
      mosiReg := 0.U
      sclkReg := false.B
      //When TX queue has data send
      when (txQueue.io.count > 0.U )
      {
        loadToSend := true.B
        state := send 
      }
      
    }
    when (state === send)
    {
      //Toggle sclk
      when(tick){
          sclkReg := ~sclkReg;
      }
      // Shift out the bits in the tx register

      when(sclkReg)
      {
        mosiReg := mosiTxReg(wordCounterReg)
        
        //misoRxReg := Cat(misoReg , misoRxReg (wordLen-1, 1))
        misoRxReg(wordCounterReg) := misoReg
      }

      when(sclkFall){
        wordCounterReg := wordCounterReg + 1.U
      }

      // Pull slave select low TODO:multiple slaves?
      nSSReg := 0.U

      
      // When a word length is sent close the transmission 
      // and write to the rx queue any incoming messages from the slave
      when(wordDone)
      {
        rxQueue.io.enq.bits     := misoRxReg.asUInt
        rxQueue.io.enq.valid    := true.B
        state := idle
      }
    }

    //Pin connections
    io.pins.sclk := sclkReg
  	io.pins.mosi := mosiReg
    misoReg := io.pins.miso 
    io.pins.nSS := nSSReg
}