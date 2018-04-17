
package twoway

import Chisel._
import s4noc_twoway._


class Node(n: Int, nodeIndex: Int, size: Int) extends Module{
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