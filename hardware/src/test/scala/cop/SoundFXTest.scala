package cop.test

import chisel3._
import chiseltest._
import org.scalatest.flatspec.AnyFlatSpec

import cop._
import ocp._
import soundbytes.Sounds._

class SoundFXTest extends AnyFlatSpec with ChiselScalatestTester {

  behavior of "SoundFX"

  it should "play" in {
    test(new SoundFX()).withAnnotations(Seq(VerilatorBackendAnnotation, WriteVcdAnnotation)) { dut =>
      val samples = getFileSamples("src/main/scala/soundbytes/sample.wav")
      val outSamples = new Array[Short](samples.length)
      val delSamples = Array.fill[Int]((1 << 12) * 4)(0)

      var finished = false
      
      // no timeout, as a bunch of 0 samples would lead to a timeout.
      dut.clock.setTimeout(0)      
      
      // Write the samples
      val th = fork {
        dut.io.copIn.trigger.poke(false.B)
        dut.io.copIn.ena_in.poke(true.B)
        dut.io.copIn.isCustom.poke(false.B)
        dut.io.copIn.read.poke(false.B)

        dut.io.copIn.funcId.poke(0.asUInt)
        dut.io.copIn.opAddr(0).poke(0.asUInt)
        dut.io.copIn.opAddr(1).poke(0.asUInt)
        dut.io.copIn.opData(0).poke(0.asUInt)
        dut.io.copIn.opData(1).poke(0.asUInt)
        dut.io.copIn.opAddrCop(0).poke(false.B)
        dut.io.copIn.opAddrCop(1).poke(false.B)
                
        for (s <- 0 until samples.length / 8) {
          
          // write 8 samples
          dut.io.copIn.read.poke(false.B)
          for (i <- 0 until 8) {
            dut.io.copIn.trigger.poke(true.B)
            dut.io.copIn.opData(0).poke((samples(s * 8 + i) & 0xFFFF).asUInt)
 
            if (dut.io.copOut.ena_out.peek.litToBoolean) {
              dut.clock.step()
              dut.io.copIn.trigger.poke(false.B)
            } else {
              dut.clock.step()
              dut.io.copIn.trigger.poke(false.B)
            
              while (!dut.io.copOut.ena_out.peek.litToBoolean)
              {
                dut.clock.step()
              }
              dut.clock.step()
            }
          }
          
          // read 8 samples
          dut.io.copIn.read.poke(true.B)
          for (i <- 0 until 8) {
            dut.io.copIn.trigger.poke(true.B)
 
            if (dut.io.copOut.ena_out.peek.litToBoolean) {
              outSamples(s * 8 + i) = dut.io.copOut.result.peek.litValue.toShort
              dut.clock.step()
              dut.io.copIn.trigger.poke(false.B)
            } else {
              dut.clock.step()
              dut.io.copIn.trigger.poke(false.B)
            
              while (!dut.io.copOut.ena_out.peek.litToBoolean)
              {
                dut.clock.step()
              }
              outSamples(s * 8 + i) = dut.io.copOut.result.peek.litValue.toShort
              dut.clock.step()
            }
          }
          
        }
        finished = true
      }

      // Write the delay
      val th2 = fork {
        dut.io.memPort.S.Resp.poke(OcpResp.NULL)
        dut.io.memPort.S.Data.poke(0.asUInt)
        dut.io.memPort.S.CmdAccept.poke(0.asUInt)
        dut.io.memPort.S.DataAccept.poke(0.asUInt)
        
        while(!finished) {

          if (dut.io.memPort.M.Cmd.peek.litValue.toInt == OcpCmd.RD.litValue.toInt) {
            val addr = dut.io.memPort.M.Addr.peek.litValue.toInt >> 2
            dut.io.memPort.S.CmdAccept.poke(1.asUInt)
            dut.clock.step()
            
            dut.io.memPort.S.CmdAccept.poke(0.asUInt)
            dut.io.memPort.S.Resp.poke(OcpResp.DVA)
            for (i <- 0 until 4) {
              dut.io.memPort.S.Data.poke((delSamples(addr + i) & 0xFFFFFFFFL).asUInt)
              dut.clock.step()
            }
            dut.io.memPort.S.Resp.poke(OcpResp.NULL)
            dut.clock.step()
          }
          else if (dut.io.memPort.M.Cmd.peek.litValue.toInt == OcpCmd.WR.litValue.toInt && dut.io.memPort.M.DataValid.peek.litValue.toInt == 1) {
            val addr = dut.io.memPort.M.Addr.peek.litValue.toInt >> 2
            dut.io.memPort.S.CmdAccept.poke(1.asUInt)
            dut.io.memPort.S.DataAccept.poke(1.asUInt)
            delSamples(addr) = dut.io.memPort.M.Data.peek.litValue.toInt
            dut.clock.step()
            
            dut.io.memPort.S.CmdAccept.poke(0.asUInt)
            for (i <- 1 until 4) {
              while (dut.io.memPort.M.DataValid.peek.litValue.toInt == 0) {
                dut.clock.step()
              }
              delSamples(addr + i) = dut.io.memPort.M.Data.peek.litValue.toInt
              dut.clock.step()
            }           
            dut.io.memPort.S.Resp.poke(OcpResp.DVA)
            dut.clock.step()
            dut.io.memPort.S.Resp.poke(OcpResp.NULL)
          }
          else {
            dut.clock.step()
          }
        }
      }

      th.join()
      th2.join()

      saveArray(outSamples, "src/main/scala/soundbytes/sample_out.wav")

    }
  }
}
