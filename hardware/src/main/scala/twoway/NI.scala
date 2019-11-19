package twoway

import Chisel._
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
  
    val testSignal = UInt(width=32).asOutput
    
  }
 
  // Set default values for memReq
  io.memReq.out.data := UInt(0)
  //io.memReq.in.valid := Bool(false)

  // Write NOC
  val st = Schedule.getSchedule(n, false, nodeIndex)
  val scheduleLength = st._1.length
  val timeslotToNode = Vec(st._3.map(UInt(_))) //Converts scala generated array to ROM
  

  // TDM counter - same counter is used for both NoCs
  val regTdmCounter = Reg(init = UInt(0, log2Up(scheduleLength)))
  val end = regTdmCounter === UInt(scheduleLength - 1)
  regTdmCounter := Mux(end, UInt(0), regTdmCounter + UInt(1))

  // Readback NOC:
  val stback = Schedule.getSchedule(n, false, nodeIndex)
  val scheduleLengthback = st._1.length
  val readBackValid = Vec(stback._2.map(Bool(_)))


  // Decode memory request from LOCAL Node - use memory port A
  val upperAddr = UInt(width = log2Up(nrChannels))
  upperAddr := io.memReq.in.address >> blockAddrWidth; // Target node
  
  val lowerAddr = UInt(width = blockAddrWidth)
  lowerAddr := io.memReq.in.address(blockAddrWidth, 0) // Block address

  // TDM schedule starts one cycles later for read data delay
  val regDelay = RegNext(regTdmCounter, init=UInt(0))


  // Set default values for readBackChannel
  io.readBackChannel.out.data := UInt(0)

  // Set default values for writeChannel
  io.writeChannel.out.rw := UInt(0)
  io.writeChannel.out.address := UInt(0)
  io.writeChannel.out.data := UInt(0)
  io.writeChannel.out.valid := Bool(false)


  // Set default vaues for memPort
  io.memPort.io.portB.addr := UInt(0)
  io.memPort.io.portB.wrData := UInt(0)
  io.memPort.io.portA.addr := UInt(0)
  io.memPort.io.portA.wrData := UInt(0)


  io.memReq.out.valid := Bool(false)

  // Default to not write to local memory
  io.memPort.io.portA.wrEna := Bool(false)
  io.memPort.io.portB.wrEna := Bool(false)

  val delayValid = Reg(init = Bool(false), next = Bool(false))

  io.memReq.out.valid := delayValid  //Changed to register


  //Delay data is used to choose between ports.
  val delayData = Reg(init = UInt(0));


  //Register unsuring only having valid high to write network for one cycle.
  val transmitted = Reg(init = Bool(false))
  transmitted := transmitted

  when(delayData === UInt(1)){
    io.memReq.out.data :=  io.memPort.io.portA.rdData
  }.otherwise{
    when(delayData === UInt(2)){

    io.memReq.out.data := io.readBackChannel.in.data

    }.otherwise{
      io.memReq.out.data := UInt(0);
    }
  }


      delayData := UInt(0)
  //Registers for requests that takes multiple cycles, where the data is only valid one cycle.
  val notProcessed = Reg(init = Bool(false))
  val inDataReg = Reg(init = UInt(0))
  val inAddressReg = Reg(init = UInt(0))
  val inRwReg = Reg(init = Bool(false))



  val valid = Bool()
  valid := (timeslotToNode(Mux(notProcessed,inAddressReg >> blockAddrWidth, upperAddr)) === regDelay)

 
  //This when handles requests if they are immediate.
  when(io.memReq.in.valid){
    when(upperAddr === UInt(nodeIndex)){  //Is this right? When valid it should alwayws be for the node.
      // LOCAL NODE -> LOCAL MEMORY
      io.memPort.io.portA.wrEna := io.memReq.in.rw

      delayData := UInt(1)

      //When it is local it always takes a single cycle.
      notProcessed := Bool(false)

      io.memPort.io.portA.addr := lowerAddr

      io.memPort.io.portA.wrData := io.memReq.in.data 


      //Read request needs one cycle delay. Also for write request.
      //Valid will go high next cycle, where the data will be stored/retrieved.
      
      delayValid := Bool(true)


      //We don't use the write channel here.
      //io.writeChannel.out.address := lowerAddr    


    } .otherwise {
      // LOCAL NODE -> EXTERNAL MEMORY

      //Assume the data has not been processed.
      notProcessed := Bool(true)

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
          notProcessed := Bool(false)
          delayValid := Bool(true)

          // Transmit outgoing memory read request/write when TDM reaches target node and request is not in local memory
          io.writeChannel.out.valid := Bool(true);
          when(io.memReq.in.rw){
          // external write has been transmitted, the node is allowed to continue execution
          //io.memReq.out.valid := io.memReq.in.rw  //Multiple writes to valid.
          }
        }
      }.otherwise{
        //Read request

        when((valid) && !transmitted ) {
          transmitted := Bool(true)
          delayValid := Bool(false)
          notProcessed := Bool(false)

          // Transmit outgoing memory read request/write when TDM reaches target node and request is not in local memory
          io.writeChannel.out.valid := Bool(true);
        }
      }
    }
  }.otherwise{
    when(notProcessed){
    //If notProcessed is high, this process takes over. This iis if a request takes multiple cycles.

      // LOCAL NODE -> EXTERNAL MEMORY

      //Assume the data has not been processed.
      notProcessed := Bool(true)


      //We always use the delayValid
      //io.memReq.out.valid := delayValid
      io.writeChannel.out.address := inAddressReg(blockAddrWidth, 0) // Block address
      io.writeChannel.out.data := inDataReg
      io.writeChannel.out.rw := inRwReg

      when(inRwReg){
        //When it is a write

        //When the target is correct, we set valid high next time.
        when((valid) ) {
          delayValid := Bool(true)
          notProcessed := Bool(false)
          // Transmit outgoing memory read request/write when TDM reaches target node and request is not in local memory
          io.writeChannel.out.valid := Bool(true);
          when(inRwReg){
          // external write has been transmitted, the node is allowed to continue execution
          //io.memReq.out.valid := io.memReq.in.rw  //Multiple writes to valid.
          }
        }
      }.otherwise{
        //read request

        when((valid) && !transmitted ) {
          transmitted := Bool(true)
          delayValid := Bool(false)
          notProcessed := Bool(false)

          // Transmit outgoing memory read request/write when TDM reaches target node and request is not in local memory
          io.writeChannel.out.valid := Bool(true);
        }
      }
    }
  }




  // ReadBack NoC variables
  val gotValue = Reg(init = Bool(false))
  val readbackValueDelayed = Reg(init= UInt(0,32))  // a 1-cycle buffer is needed on the read value for transmitting readback requests when a blank in the cycle has occured
  readbackValueDelayed := io.memPort.io.portB.rdData

  val rbDelayArray = st._5
  val rbDelayROM = Vec(rbDelayArray.map(SInt(_)))
  val nrOfFIFORegs = rbDelayArray.reduceLeft(_ max _) // finds the greates number in the array which corrosponds to the number of registers needed.
  val rbFIFO = RegInit(Vec(Seq.fill(nrOfFIFORegs)(new SingleChannel()))) // generate the rbFIFO
  val localValidTable = Vec(st._6.map(Bool(_)))//used to check wether an insertion should be preformed
  

  // TDM counter - 1 clk cycle delayed, such that the 1 cycle read time is accounded for, one cycle for the router to NI and one unknown...
  val FIFOTdmCounter = Reg(init = UInt(scheduleLength - 2, log2Up(scheduleLength)))
  val endTDMFIFO = FIFOTdmCounter === UInt(scheduleLength - 1)
  FIFOTdmCounter := Mux(endTDMFIFO, UInt(0), FIFOTdmCounter + UInt(1))
  

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
      gotValue := Bool(true)
    }
  }.otherwise{
    io.memPort.io.portB.wrEna := Bool(false)
  }

  debug(io.testSignal)

  io.testSignal := readBackValid(regDelay)

  // ReadBack NoC transmission


  // TDM counter - 1
  val shiftedTdmCounter = Reg(init = UInt(st._4, log2Up(scheduleLength)))
  val end2 = shiftedTdmCounter === UInt(scheduleLength - 1)
  shiftedTdmCounter := Mux(end2, UInt(0), shiftedTdmCounter + UInt(1))
  val regShiftedTdmCounter = RegNext(shiftedTdmCounter, init=UInt(0))//This works for all route 2x2 and 3x3 and all route except the 4'th indexed ass 3 in 4x4
  
  shiftedTdmCounter := Mux(end2, UInt(0), shiftedTdmCounter + UInt(1))  
  
  
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
      gotValue := Bool(true)
    }.otherwise{
      gotValue := Bool(false)
    }
    /*for(i <-1 until rbFIFO.length){ 
      rbFIFO(i) := rbFIFO(i - 1)
    }*/
  }.otherwise{
    gotValue := Bool(false)
  }


  
  //Multiplexer that pics out the appropriate data in the FIFO for the readback network.
  var muxReadDataChannel = new SingleChannel()
  //val gotValueRb = Wire(init = Bool(false))
  val lookupvalue = rbDelayROM(regShiftedTdmCounter)
  

  muxReadDataChannel.data := UInt(0)
  muxReadDataChannel.valid := Bool(false)

  when(lookupvalue === SInt(0)){
    muxReadDataChannel.data := io.memPort.io.portB.rdData
    muxReadDataChannel.valid := gotValue
  }.elsewhen(lookupvalue === SInt(-1)){
    muxReadDataChannel.data := io.memPort.io.portB.rdData
    muxReadDataChannel.valid := Bool(false)
    //gotValueRb := Bool(false)
  }.otherwise{

    muxReadDataChannel := rbFIFO(lookupvalue.asUInt() - UInt(1) )

  }

 
  io.readBackChannel.out.valid := muxReadDataChannel.valid
  io.readBackChannel.out.data := muxReadDataChannel.data
        // ReadBack NoC reception
        when(io.readBackChannel.in.valid){
          // Node should be waiting for the valid signal to be asserted, to indicate that data is available
          transmitted := Bool(false)
          delayData := UInt(2)
          io.memReq.out.data := io.readBackChannel.in.data
          io.memReq.out.valid := io.readBackChannel.in.valid
        }.otherwise{
          io.memReq.out.valid := delayValid
        }

}