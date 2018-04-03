package twoway

import Chisel._
import s4noc_twoway._

/**
 * One NoC node, connected to the router, containing the memory,
 * providing the NI machinery.
 */

class NI(n: Int, nodeIndex : Int, size: Int) extends Module {
  val nrChannels = n * n - 1
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
    val memPort = new TrueDualPortMemory(size)
  }

  // Write NOC
  val st = Schedule.getSchedule(n)
  val scheduleLength = st._1.length
  val writeNocTab = Vec(st._2.map(Bool(_)))

  // TDM counter - same counter is used for both NoCs
  val regTdmCounter = Reg(init = UInt(0, log2Up(scheduleLength)))
  val end = regTdmCounter === UInt(scheduleLength - 1)
  regTdmCounter := Mux(end, UInt(0), regTdmCounter + UInt(1))

  // Memory initialization
  println(blockAddrWidth.toShort)
  println("Memory block size: " + scala.math.pow(2, blockAddrWidth).toInt)
  val memory = Module(new DualPortMemory(blockAddrWidth))

  // Default to not write to local memory
  io.memPort.io.portA.wrEna := Bool(false)
  io.memPort.io.portB.wrEna := Bool(false)

  // Decode memory request from LOCAL Node - use memory port A
  val upperAddr = io.memReq.in.address >> blockAddrWidth; // Target node
  val lowerAddr = io.memReq.in.address && (2^blockAddrWidth - 1) // Block address
  when(io.memReq.out.valid){
    when(upperAddr === nodeIndex){
      // LOCAL NODE -> LOCAL MEMORY
      io.memPort.io.portA.wrEna := io.memReq.out.rw
      io.memReq.in.valid := Bool(true)
      when(io.memReq.out.rw){
        // LOCAL MEMORY WRITE
        io.memPort.io.portA.wrAddr := lowerAddr
        io.memPort.io.portA.wrData := io.memReq.in.data
      } elsewhen {
        // LOCAL MEMORY READ
        io.memPort.io.portA.rdAddr := lowerAddr
        io.memReq.data := io.memPort.io.portA.rdData
      }
    } .otherwise {
      // LOCAL NODE -> EXTERNAL MEMORY
      io.memReq.in.valid := Bool(false)
      io.writeChannel.out.address := lowerAddr
      io.writeChannel.out.data := io.memReq.out.data
      io.writeChannel.out.rw := io.memReq.in.rw
    }
  }

  // writeNoc transmission
  val valid = writeNocTab(regTdmCounter) === upperAddr
  when(valid && io.memReq.in.valid && upperAddr != nodeIndex) {
    // Transmit outgoing memory read request/write when TDM reaches target node and request is not in local memory
    io.writeChannel.out.valid := Bool(true);
    when(io.memReq.out.rw){
      // external write has been transmitted, the node is allowed to continue execution
      io.memReq.in.valid := io.memReq.out.rw
    }
  }

  // ReadBack NoC variables
  val gotValue = Bool(false)
  val readBackValue = UInt(width = 32, init= UInt(0)) 

  // writeNoc reception - use memory port B
  when(io.writeChannel.in.valid) {
    val rxLowerAddr = io.writeChannel.in.address // Block address
    when(io.writeChannel.in.rw){
      // LOCAL MEMORY WRITE
      io.memPort.io.portB.wrAddr := rxLowerAddr
      io.memPort.io.portB.wrData := io.writeChannel.data
      io.memPort.io.portB.valid := io.writeChannel.in.rw
    } .otherwise {
      // LOCAL MEMORY READ
      io.memPort.io.portB.rdAddr := rxLowerAddr
      readBackValue := io.memPort.io.portB.rdData
      gotValue := Bool(true)
    }
  }

  // ReadBack NoC transmission
  io.readBackChannel.out.valid := Bool(false)
  when(gotValue){
    // Transmit read value on readBack NoC - no validTab here, since the constant delay time of 
    // accessing the memory is factored into the readBack schedule
    io.readBackChannel.out.valid := gotValue
    io.readBackChannel.out.data  := readBackValue
    gotValue := Bool(false)
  }

  // ReadBack NoC reception
  when(io.readBackChannel.in.valid){
    // Node should be waiting for the valid signal to be asserted, to indicate that data is available
    io.memReq.in.data := io.readBackChannel.data
    io.memReq.in.valid := io.readBackChannel.in.valid
  }
}
