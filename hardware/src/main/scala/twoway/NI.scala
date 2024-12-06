package twoway

import chisel3._
import chisel3.util._
import s4noc_twoway._

/**
 * One NoC node, connected to the router, containing the memory,
 * providing the NI machinery.
 */

class NI(n: Int, nodeIndex : Int, size: Int) extends Module {
  val nrChannels = n * n
  val memoryWidth = log2Down(size)
  val blockAddrWidth = log2Down(size/nrChannels)

  val io = new Bundle {
    // Everything is bus-widened to accomodate read/writes in a single cycle

    // MSB: 0 = read, 1 = write
    // memReq.in.valid is set to 1 when the request has been fulfilled (either a write has
    // been transmitted to the noc, or a read has been received from local/external memory)
    val memReq = new RwChannel(memoryWidth) // Memory requests from Node
    val readBackChannel = new Channel()
    // MSB: 0 = read request, 1 = write (to block in other node)
    val writeChannel = new RwChannel(blockAddrWidth)
    // Port A: Local node requests
    // Port B: External requests
    val memPort = Module(new TrueDualPortMemory(Math.pow(2,blockAddrWidth).toInt)) //adding module fixed a compile error. It should just make a mudule as you would in VHDL. Dont know why uou dont need ot for the channels for instance 
  
    val testSignal = Output(UInt(32.W))
    
  }
 
  // Set default values for memReq
  io.memReq.out.data := 0.U
  //io.memReq.in.valid := false.B

  // Write NOC
  val st = Schedule.getSchedule(n, false, nodeIndex)
  val scheduleLength = st._1.length
  val timeslotToNode = VecInit(st._3.toIndexedSeq.map(_.U)) //Converts scala generated array to ROM
  

  // TDM counter - same counter is used for both NoCs
  val regTdmCounter = RegInit(init = 0.U(log2Up(scheduleLength).W))
  val end = regTdmCounter === (scheduleLength - 1).U
  regTdmCounter := Mux(end, 0.U, regTdmCounter + 1.U)

  // Readback NOC:
  val stback = Schedule.getSchedule(n, false, nodeIndex)
  val scheduleLengthback = st._1.length
  val readBackValid = VecInit(stback._2.toIndexedSeq.map(_.B))


  // Decode memory request from LOCAL Node - use memory port A
  val upperAddr = UInt(log2Up(nrChannels).W)
  upperAddr := io.memReq.in.address >> blockAddrWidth; // Target node
  
  val lowerAddr = UInt(blockAddrWidth.W)
  lowerAddr := io.memReq.in.address(blockAddrWidth, 0) // Block address

  // TDM schedule starts one cycles later for read data delay
  val regDelay = RegNext(regTdmCounter, init=0.U)


  // Set default values for readBackChannel
  io.readBackChannel.out.data := 0.U

  // Set default values for writeChannel
  io.writeChannel.out.rw := 0.U
  io.writeChannel.out.address := 0.U
  io.writeChannel.out.data := 0.U
  io.writeChannel.out.valid := false.B


  // Set default vaues for memPort
  io.memPort.io.portB.addr := 0.U
  io.memPort.io.portB.wrData := 0.U
  io.memPort.io.portA.addr := 0.U
  io.memPort.io.portA.wrData := 0.U


  io.memReq.out.valid := false.B

  // Default to not write to local memory
  io.memPort.io.portA.wrEna := false.B
  io.memPort.io.portB.wrEna := false.B

  val delayValid = RegNext(init = false.B, next = false.B)

  io.memReq.out.valid := delayValid  //Changed to register


  //Delay data is used to choose between ports.
  val delayData = RegInit(init = 0.U);


  //Register unsuring only having valid high to write network for one cycle.
  val transmitted = RegInit(init = false.B)
  transmitted := transmitted

  when(delayData === 1.U){
    io.memReq.out.data :=  io.memPort.io.portA.rdData
  }.otherwise{
    when(delayData === 2.U){

    io.memReq.out.data := io.readBackChannel.in.data

    }.otherwise{
      io.memReq.out.data := 0.U;
    }
  }


      delayData := 0.U
  //Registers for requests that takes multiple cycles, where the data is only valid one cycle.
  val notProcessed = RegInit(init = false.B)
  val inDataReg = RegInit(init = 0.U)
  val inAddressReg = RegInit(init = 0.U)
  val inRwReg = RegInit(init = false.B)



  val valid = Bool()
  valid := (timeslotToNode(Mux(notProcessed, (inAddressReg >> blockAddrWidth).asUInt, upperAddr)) === regDelay)

 
  //This when handles requests if they are immediate.
  when(io.memReq.in.valid){
    when(upperAddr === nodeIndex.U){  //Is this right? When valid it should alwayws be for the node.
      // LOCAL NODE -> LOCAL MEMORY
      io.memPort.io.portA.wrEna := io.memReq.in.rw

      delayData := 1.U

      //When it is local it always takes a single cycle.
      notProcessed := false.B

      io.memPort.io.portA.addr := lowerAddr

      io.memPort.io.portA.wrData := io.memReq.in.data 


      //Read request needs one cycle delay. Also for write request.
      //Valid will go high next cycle, where the data will be stored/retrieved.
      
      delayValid := true.B


      //We don't use the write channel here.
      //io.writeChannel.out.address := lowerAddr    


    } .otherwise {
      // LOCAL NODE -> EXTERNAL MEMORY

      //Assume the data has not been processed.
      notProcessed := true.B

      //We sample the request the first time.
      inDataReg := io.memReq.in.data
      inAddressReg := io.memReq.in.address
      inRwReg := io.memReq.in.rw



      //We always use the delayValid
      //io.memReq.out.valid := delayValid
      io.writeChannel.out.address := lowerAddr        
      io.writeChannel.out.data := io.memReq.in.data
      io.writeChannel.out.rw := io.memReq.in.rw

      when(io.memReq.in.rw){
        //When it is a write

        //When the target is correct, we set valid high next time.
        when((valid) ) {
          notProcessed := false.B
          delayValid := true.B

          // Transmit outgoing memory read request/write when TDM reaches target node and request is not in local memory
          io.writeChannel.out.valid := true.B;
          when(io.memReq.in.rw){
          // external write has been transmitted, the node is allowed to continue execution
          //io.memReq.out.valid := io.memReq.in.rw  //Multiple writes to valid.
          }
        }
      }.otherwise{
        //Read request

        when((valid) && !transmitted ) {
          transmitted := true.B
          delayValid := false.B
          notProcessed := false.B

          // Transmit outgoing memory read request/write when TDM reaches target node and request is not in local memory
          io.writeChannel.out.valid := true.B;
        }
      }
    }
  }.otherwise{
    when(notProcessed){
    //If notProcessed is high, this process takes over. This iis if a request takes multiple cycles.

      // LOCAL NODE -> EXTERNAL MEMORY

      //Assume the data has not been processed.
      notProcessed := true.B


      //We always use the delayValid
      //io.memReq.out.valid := delayValid
      io.writeChannel.out.address := inAddressReg(blockAddrWidth, 0) // Block address
      io.writeChannel.out.data := inDataReg
      io.writeChannel.out.rw := inRwReg

      when(inRwReg){
        //When it is a write

        //When the target is correct, we set valid high next time.
        when((valid) ) {
          delayValid := true.B
          notProcessed := false.B
          // Transmit outgoing memory read request/write when TDM reaches target node and request is not in local memory
          io.writeChannel.out.valid := true.B;
          when(inRwReg){
          // external write has been transmitted, the node is allowed to continue execution
          //io.memReq.out.valid := io.memReq.in.rw  //Multiple writes to valid.
          }
        }
      }.otherwise{
        //read request

        when((valid) && !transmitted ) {
          transmitted := true.B
          delayValid := false.B
          notProcessed := false.B

          // Transmit outgoing memory read request/write when TDM reaches target node and request is not in local memory
          io.writeChannel.out.valid := true.B;
        }
      }
    }
  }




  // ReadBack NoC variables
  val gotValue = RegInit(init = false.B)
  val readbackValueDelayed = RegInit(init= 0.U(32.W))  // a 1-cycle buffer is needed on the read value for transmitting readback requests when a blank in the cycle has occured
  readbackValueDelayed := io.memPort.io.portB.rdData

  val rbDelayArray = st._5
  val rbDelayROM = VecInit(rbDelayArray.toIndexedSeq.map(_.S))
  val nrOfFIFORegs = rbDelayArray.reduceLeft(_ max _) // finds the greates number in the array which corrosponds to the number of registers needed.
  val rbFIFO = RegInit(VecInit(Seq.fill(nrOfFIFORegs)(new SingleChannel()))) // generate the rbFIFO
  val localValidTable = VecInit(st._6.toIndexedSeq.map(_.B))//used to check wether an insertion should be preformed
  

  // TDM counter - 1 clk cycle delayed, such that the 1 cycle read time is accounded for, one cycle for the router to NI and one unknown...
  val FIFOTdmCounter = RegInit(init = (scheduleLength - 2).U(log2Up(scheduleLength).W))
  val endTDMFIFO = FIFOTdmCounter === (scheduleLength - 1).U
  FIFOTdmCounter := Mux(endTDMFIFO, 0.U, FIFOTdmCounter + 1.U)
  

  // writeNoc reception - use memory port B
  val rxLowerAddr = io.writeChannel.in.address // Block address
  io.memPort.io.portB.addr := rxLowerAddr
  io.memPort.io.portB.wrData := io.writeChannel.in.data

  when(io.writeChannel.in.valid) {
    // LOCAL MEMORY WRITE
      
    io.memPort.io.portB.wrEna := io.writeChannel.in.rw
    when(io.writeChannel.in.rw){
    } .otherwise {
      // LOCAL MEMORY READ
      gotValue := true.B
    }
  }.otherwise{
    io.memPort.io.portB.wrEna := false.B
  }

  //debug(io.testSignal) does nothing in chisel3 (no proning in frontend of chisel3 anyway)

  io.testSignal := readBackValid(regDelay)

  // ReadBack NoC transmission


  // TDM counter - 1
  val shiftedTdmCounter = RegInit(init = st._4.U(log2Up(scheduleLength).W))
  val end2 = shiftedTdmCounter === (scheduleLength - 1).U
  shiftedTdmCounter := Mux(end2, 0.U, shiftedTdmCounter + 1.U)
  val regShiftedTdmCounter = RegNext(shiftedTdmCounter, init=0.U)//This works for all route 2x2 and 3x3 and all route except the 4'th indexed ass 3 in 4x4
  
  shiftedTdmCounter := Mux(end2, 0.U, shiftedTdmCounter + 1.U)
  
  
  //FIFO logic
  rbFIFO(0).data := io.memPort.io.portB.rdData
  rbFIFO(0).valid := gotValue
  for(i <-1 until rbFIFO.length){ 
    rbFIFO(i) := rbFIFO(i - 1)
  }
  when(localValidTable(FIFOTdmCounter)){
    //rbFIFO(0).data := io.memPort.io.portB.rdData
    //rbFIFO(0).valid := gotValue // is set high when data is ready in from the memory
    when(io.writeChannel.in.valid && !io.writeChannel.in.rw){ // if new data is available in the next cycle
      gotValue := true.B
    }.otherwise{
      gotValue := false.B
    }
    /*for(i <-1 until rbFIFO.length){ 
      rbFIFO(i) := rbFIFO(i - 1)
    }*/
  }.otherwise{
    gotValue := false.B
  }


  
  //Multiplexer that pics out the appropriate data in the FIFO for the readback network.
  var muxReadDataChannel = new SingleChannel()
  //val gotValueRb = Wire(init = false.B)
  val lookupvalue = rbDelayROM(regShiftedTdmCounter)
  

  muxReadDataChannel.data := 0.U
  muxReadDataChannel.valid := false.B

  when(lookupvalue === 0.S){
    muxReadDataChannel.data := io.memPort.io.portB.rdData
    muxReadDataChannel.valid := gotValue
  }.elsewhen(lookupvalue === -1.S){
    muxReadDataChannel.data := io.memPort.io.portB.rdData
    muxReadDataChannel.valid := false.B
    //gotValueRb := false.B
  }.otherwise{

    muxReadDataChannel := rbFIFO(lookupvalue.asUInt - 1.U )

  }

 
  io.readBackChannel.out.valid := muxReadDataChannel.valid
  io.readBackChannel.out.data := muxReadDataChannel.data
        // ReadBack NoC reception
        when(io.readBackChannel.in.valid){
          // Node should be waiting for the valid signal to be asserted, to indicate that data is available
          transmitted := false.B
          delayData := 2.U
          io.memReq.out.data := io.readBackChannel.in.data
          io.memReq.out.valid := io.readBackChannel.in.valid
        }.otherwise{
          io.memReq.out.valid := delayValid
        }

}