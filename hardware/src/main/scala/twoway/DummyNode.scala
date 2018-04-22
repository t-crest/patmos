
package twoway

import Chisel._
import s4noc_twoway._


class TestNode(n: Int, nodeIndex: Int, size: Int) extends Module{
    val nrChannels = n*n-1
    val memoryWidth = log2Down(size)
    val blockAddrWidth = log2Down(size/nrChannels)
    println(size)
    println(memoryWidth)
    val io = new Bundle{
        val local = new RwChannel(memoryWidth)
        val output = Bool().asOutput

        val test = new RwChannel(memoryWidth).flip

 
        // Create interface for tester -> network interface accessS

    }
    
    io.local <> io.test
}

class NodeRead(n: Int, nodeIndex: Int, size: Int) extends Module{
    val nrChannels = n*n-1
    val memoryWidth = log2Down(size)
    val blockAddrWidth = log2Down(size/nrChannels)
  

    val io = new Bundle{
        val local = new RwChannel(memoryWidth)

        val led = Bool().asOutput

 
        // Create interface for tester -> network interface accessS

    }

    io.local.out.rw := Bool(false)
    io.local.out.address := UInt(0)
    io.local.out.data := UInt(0)
    io.local.out.valid := Bool(false)

    val sReset :: sRead :: sSendPacket :: sLight :: Nil = Enum(UInt(),4)
    val state = Reg(init = sReset)
    val stateLast = RegNext(state)


    when(state === sLight){
        io.led := Bool(true)
    }.otherwise{
        io.led := Bool(false)
    }

    val counter = Reg(UInt(0))


    val m =( if(nodeIndex + 1 == n * n) 0 else nodeIndex + 1)



    switch (state) {
    is (sReset) {
        counter := UInt(0)
        io.local.out.rw := Bool(false)
        io.local.out.address := UInt(0)//0x42+ 0x100 * nodeIndex)
        io.local.out.data := UInt(0)
        io.local.out.valid := Bool(false)
      if (nodeIndex == 0) {
        state := sSendPacket
      }else {
        state := sRead
      }
    }

    is (sRead) {
        io.local.out.rw := Bool(false)
        io.local.out.address := UInt(0x042 + 0x100 * nodeIndex)
        io.local.out.valid := Bool(true)
      when (io.local.in.valid && stateLast === sRead) {
        when(io.local.in.data === UInt(0x42)){
            state := sLight
        }
      }
    }
    is (sSendPacket) {
        io.local.out.rw := Bool(true)
        io.local.out.address := UInt(0x042 + 0x100 * m)
        io.local.out.data := UInt(0x42)
        io.local.out.valid := Bool(true)
        when(io.local.in.valid && stateLast === sSendPacket){
            state := sRead
        }
    }
    is(sLight) {
        counter := counter + UInt(1,32)
        when(counter === UInt(0)){
            io.local.out.address :=  UInt(0x042 + 0x100 * nodeIndex)
            io.local.out.rw := Bool(true)
            io.local.out.valid := Bool(true)
            io.local.out.data := UInt(0)
        }.otherwise{
            io.local.out.valid := Bool(false)
        }

        when(counter === UInt(100,32)){
            state := sSendPacket
            counter := UInt(0)
        }
    }
  }
}