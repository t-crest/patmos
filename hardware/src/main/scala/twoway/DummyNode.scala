
package twoway

import Chisel._
import s4noc_twoway._


class Node(n: Int, nodeIndex: Int, size: Int) extends Module{
    val nrChannels = n*n-1
    val memoryWidth = log2Down(size)
    val blockAddrWidth = log2Down(size/nrChannels)

    val io = new Bundle{
        val local = new RwChannel(memoryWidth)
        val output = Bool().asOutput
        val internal = UInt().asOutput


        val rwp = Input(Bool())
        val validp = Bool().asInput
        val addressp = UInt(width=10).asInput
        val datap = UInt(width=32).asInput
    }

    

    //io.local.out.rw := io.rwp
    io.local.out.valid := io.validp
    io.local.out.address := io.addressp
    io.local.out.data := io.datap
    io.output := Bool(false)
    io.internal := UInt(0,32)
    




/*
    println(size)
    println(memoryWidth)
    println(blockAddrWidth)
    io.local.out.rw := Bool(true)
    io.local.out.valid := Bool(true)
    if(nodeIndex == (n*n-1)){
    io.local.out.address := Cat(UInt(0, memoryWidth - blockAddrWidth), UInt(1,blockAddrWidth)) 
    }else {
    io.local.out.address := Cat(UInt(nodeIndex + 1, memoryWidth - blockAddrWidth), UInt(1,blockAddrWidth)) 
    }
    io.local.out.data := UInt(42 + nodeIndex) 
    io.output := Bool(false)*/
}