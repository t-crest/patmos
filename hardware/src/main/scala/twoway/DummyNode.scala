
package twoway

import chisel3._
import chisel3.util._
import s4noc_twoway._


class TestNode(n: Int, nodeIndex: Int, size: Int) extends Module{
    val nrChannels = n*n-1
    val memoryWidth = log2Down(size)
    val blockAddrWidth = log2Down(size/nrChannels)
    println(size)
    println(memoryWidth)
    val io = new Bundle{
        val local = new RwChannel(memoryWidth)
        val output = Output(Bool())

        val test = Flipped(new RwChannel(memoryWidth))


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

        val led = Output(Bool())


        // Create interface for tester -> network interface accessS

    }

    io.local.out.rw := false.B

    io.local.out.address := 0.U
    io.local.out.data := 0.U
    io.local.out.valid := false.B

    val sReset :: sRead :: sSendPacket :: sLight :: Nil = Enum(4)
    val state = RegInit(init = sReset)
    val stateLast = RegNext(state, init = sReset)


    when(state === sLight){
        io.led := true.B
    }.otherwise{
        io.led := false.B
    }

    val counter = RegInit(init = 0.U)


    val m =( if(nodeIndex + 1 == n * n) 0 else nodeIndex + 1)



    switch (state) {
        is (sReset) {
            counter := 0.U
            io.local.out.rw := false.B
            io.local.out.address := 0.U//0x42+ 0x100 * nodeIndex)
            io.local.out.data := 0.U
            io.local.out.valid := false.B
            if (nodeIndex == 0) {
                state := sSendPacket
            }else {
                state := sRead
            }
        }

        is (sRead) {
            io.local.out.rw := false.B
            io.local.out.address := (0x042 + 0x100 * nodeIndex).U
            io.local.out.valid := true.B
            when (io.local.in.valid && stateLast === sRead) {
                when(io.local.in.data === 0x42.U){
                    state := sLight
                }
            }
        }
        is (sSendPacket) {
            io.local.out.rw := true.B
            io.local.out.address := (0x042 + 0x100 * m).U
            io.local.out.data := 0x42.U
            io.local.out.valid := true.B
            when(io.local.in.valid && stateLast === sSendPacket){
                state := sRead
                io.local.out.valid := false.B
            }
        }
        is(sLight) {
            counter := counter + 1.U(32.W)
            when(counter === 0.U){
                io.local.out.address :=  (0x042 + 0x100 * nodeIndex).U
                io.local.out.rw := true.B
                io.local.out.valid := true.B
                io.local.out.data := 0.U
            }.otherwise{
                io.local.out.valid := false.B
            }

            when(counter === 100.U(32.W)){
                state := sSendPacket
                counter := 0.U
            }
        }
    }
}