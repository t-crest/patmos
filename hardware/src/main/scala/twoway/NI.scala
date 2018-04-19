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
  val writeNocTab = Vec(st._2.map(Bool(_)))
  val timeslotToNode = Vec(st._3.map(UInt(_))) //Not sure but my hope is that this converts the array to a ROM, that can be used as a look 
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

  // writeNoc transmission, Can we write something?
  //val valid = writeNocTab(regTdmCounter) // CHANGE HERE TO SAY IF WE ARE IN CORRECT SLOT
  //val valid = writeNocTab && timeslotToNode(upperAddr) === regtdmCounter
  //val valid = Bool()

  // TDM schedule starts one cycles later for read data delay
  val regDelay = RegNext(regTdmCounter, init=UInt(0))
  val valid = Bool(timeslotToNode(upperAddr) === regDelay)


 

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

  io.memReq.out.valid := delayValid  //Change to register


  //Register to only have valid high to write network for one cycle.
  val transmitted = Reg(init = Bool(false))
  transmitted := transmitted

  when(io.memReq.in.valid){
    println("Test")
    println(upperAddr)
    println(nodeIndex)
    when(Bool(upperAddr === UInt(nodeIndex))){  //Is this right? When valid it should alwayws be for the node.
      // LOCAL NODE -> LOCAL MEMORY
      io.memPort.io.portA.wrEna := io.memReq.in.rw


      //io.memReq.out.valid := Bool(true)  //Change to register
      io.memPort.io.portA.addr := lowerAddr

      io.memPort.io.portA.wrData := io.memReq.in.data 
      io.memReq.out.data := io.memPort.io.portA.rdData

      //Read request needs a one cycle delay.
      delayValid := Bool(false)

      when(io.memReq.in.rw === Bool(false)){
 
        delayValid := Bool(true)
   
      }.otherwise{
        io.memReq.out.valid := Bool(true)  //Change to register
      }
      io.writeChannel.out.address := lowerAddr    



    } .otherwise {
      // LOCAL NODE -> EXTERNAL MEMORY
      io.memReq.out.valid := delayValid
      io.writeChannel.out.address := lowerAddr        
      io.writeChannel.out.data := io.memReq.in.data
      io.writeChannel.out.rw := io.memReq.in.rw

      when(io.memReq.in.rw){

        //Change valid to "Is target correct"
        when((valid) ) {
          delayValid := Bool(true)
  
          // Transmit outgoing memory read request/write when TDM reaches target node and request is not in local memory
          io.writeChannel.out.valid := Bool(true);
          when(io.memReq.in.rw){
          // external write has been transmitted, the node is allowed to continue execution
          //io.memReq.out.valid := io.memReq.in.rw  //Multiple writes to valid.
          }
        }
      }.otherwise{
        //Write request

        when((valid) && !transmitted ) {
          transmitted := Bool(true)
          delayValid := Bool(false)

          // Transmit outgoing memory read request/write when TDM reaches target node and request is not in local memory
          io.writeChannel.out.valid := Bool(true);
        }

        // ReadBack NoC reception
        when(io.readBackChannel.in.valid){
          // Node should be waiting for the valid signal to be asserted, to indicate that data is available
          transmitted := Bool(false)
          io.memReq.out.data := io.readBackChannel.in.data
          io.memReq.out.valid := io.readBackChannel.in.valid
        }
      }
    }
  }



  // ReadBack NoC variables
  val gotValue = Reg(init = Bool(false))
  val readbackValueDelayed = Reg(init= UInt(0,32))  // a 1-cycle buffer is needed on the read value for transmitting readback requests when a blank in the cycle has occured
  readbackValueDelayed := io.memPort.io.portB.rdData

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
  val shiftedTdmCounter = Reg(init = UInt(st._4 - 1 , log2Up(scheduleLength)))
  val end2 = shiftedTdmCounter === UInt(scheduleLength - 1)
  shiftedTdmCounter := Mux(end2, UInt(0), shiftedTdmCounter + UInt(1))

  // After the first blank has been encountered in the schedule, all subsequent values must be delayed by
  // a single clockcycle. This flag must be reset upon the end of the schedule
  val readbackDelayFlag = Reg(init = Bool(false))
  when(!readbackDelayFlag && !writeNocTab(shiftedTdmCounter)){
    // the first blank has been encountered in the writeNocTab
    readbackDelayFlag := Bool(true)
  }
  
  when(shiftedTdmCounter === UInt(0) && readbackDelayFlag){
    readbackDelayFlag := Bool(false)
  }

  // When readbackDelayFlag is set, we take the 1-cycle delayed input
  val transmittedValue = Mux(readbackDelayFlag, readbackValueDelayed, io.memPort.io.portB.rdData)
    
  // If flag has been set, gotValid must be delayed once
  
  val gotValueDelayed = Reg(init = Bool(false))
  gotValueDelayed := gotValue
  val gotValueMux = Mux(readbackDelayFlag, gotValueDelayed, gotValue)

  io.readBackChannel.out.valid := Bool(false)
  when(gotValueMux){
    // Transmit read value on readBack NoC - no validTab here, since the constant delay time of 
    // accessing the memory is factored into the readBack schedule.

    //Though, if the validtab is low, it needs to transmit in the next cycle.
    when(readBackValid(regTdmCounter)){
      io.readBackChannel.out.valid := gotValueMux
      io.readBackChannel.out.data  := transmittedValue
      gotValue := Bool(false)
    }.otherwise{
      io.memPort.io.portB.addr := RegNext(rxLowerAddr)
    }
  }


}
