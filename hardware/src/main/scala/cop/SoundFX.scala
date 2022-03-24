/*
 * SoundFX Coprocessor
 *
 * Authors: Clemens Pircher (clemens.lukas@gmx.at)
 *
 * Uses the building blocks of the soundbytes repository (https://github.com/schoeberl/soundbytes).
 * Current status is a fixed function pipeline consisting of:
 *  > Distortion + Output Gain Stage [0, 1[
 *  > Delay + Output Gain Stage [0, 2[
 */

package cop

import chisel3._
import chisel3.util._

import soundbytes._

import patmos.Constants._
import ocp._

object SoundFX extends CoprocessorObject {

  def init(params: Map[String, String]) = {}

  def create(params: Map[String, String]): SoundFX = Module(new SoundFX())
}


class SoundFX() extends CoprocessorMemoryAccess() {
  val SAMPLE_WIDTH = 16
  val GAIN_WIDTH = 6
  val PTR_WIDTH = 12
  val MIX_WIDTH = 6
  val FB_WIDTH = 6
  
  val DATA_DENSITY = (DATA_WIDTH / SAMPLE_WIDTH)
  val DELAY_PACKET_SIZE = BURST_LENGTH * DATA_DENSITY
  val DELAY_COUNTER_WIDTH = log2Ceil(DELAY_PACKET_SIZE)

  // Coprocessor Functions.
  val FUNC_SAMPLE = 0.U(5.W)        // read/write sample
  
  val FUNC_MOD_EN = 1.U(5.W)        // module enable bits [1: Delay, 0: Distortion]
  
  val FUNC_DIST_GAIN = 2.U(5.W)     // distortion gain
  val FUNC_DIST_OUTGAIN = 3.U(5.W)  // distortion output gain
 
  val FUNC_DEL_BUF = 4.U(5.W)       // delay buffer address
  val FUNC_DEL_LEN = 5.U(5.W)       // delay length
  val FUNC_DEL_MAXLEN = 6.U(5.W)    // delay max length
  val FUNC_DEL_MIX = 7.U(5.W)       // delay mix
  val FUNC_DEL_FB = 8.U(5.W)        // delay feedback
  val FUNC_DEL_OUTGAIN = 9.U(5.W)   // delay output gain
  
  // Effects Modules.
  val distortion = Module(new Distortion(SAMPLE_WIDTH, GAIN_WIDTH, 10, 350.0f, false))
  val distOutGain = Module(new Gain(SAMPLE_WIDTH, GAIN_WIDTH, SAMPLE_WIDTH))
  val delay = Module(new Delay(SAMPLE_WIDTH, PTR_WIDTH, DELAY_COUNTER_WIDTH, MIX_WIDTH, FB_WIDTH))
  val delOutGain = Module(new Gain(SAMPLE_WIDTH, GAIN_WIDTH, SAMPLE_WIDTH + 1))
  
  // Configuration Registers.
  val regDistEn = RegInit(true.B)
  val regDelEn = RegInit(true.B)
  
  val regDistGain = RegInit((1 << (GAIN_WIDTH - 2)).U(GAIN_WIDTH.W))
  val regDistOutGain = RegInit((1 << (GAIN_WIDTH - 1)).U(GAIN_WIDTH.W))
  
  val regDelBuf = RegInit(0.U(EXTMEM_ADDR_WIDTH.W))
  val regDelLen = RegInit((1 << (PTR_WIDTH - 1)).U(PTR_WIDTH.W))
  val regDelMaxLen = RegInit(((1 << PTR_WIDTH) - 1).U(PTR_WIDTH.W))
  val regDelMix = RegInit((1 << (MIX_WIDTH - 2)).U(MIX_WIDTH.W))
  val regDelFB = RegInit((1 << (FB_WIDTH - 2)).U(FB_WIDTH.W))
  val regDelOutGain = RegInit((1 << (GAIN_WIDTH - 1)).U(GAIN_WIDTH.W))
  
  distortion.io.gain := regDistGain
  distOutGain.io.gain := regDistOutGain
  
  delay.io.delayLength := regDelLen
  delay.io.delayMaxLength := regDelMaxLen
  delay.io.mix := regDelMix
  delay.io.feedback := regDelFB
  delOutGain.io.gain := regDelOutGain
  
  // Memory FSM.
  val memIdle :: memRd :: memRdWait :: memWrReq :: memWr :: memWrWait :: Nil = Enum(6)
  val regMemState = RegInit(memIdle)
  val memCounter = Counter(BURST_LENGTH)

  val regDelIn = RegInit(VecInit(Seq.fill(BURST_LENGTH)(0.U(DATA_WIDTH.W))))
  val regDelOut = RegInit(VecInit(Seq.fill(DELAY_PACKET_SIZE)(0.S(SAMPLE_WIDTH.W))))
  val regDelPtr = RegInit(0.U(PTR_WIDTH.W))
  
  // Data <-> Sample Plumbing.
  val delIn = VecInit(Seq.fill(BURST_LENGTH)(0.U(DATA_WIDTH.W)))
  val delOut = VecInit(Seq.fill(BURST_LENGTH)(0.U(DATA_WIDTH.W)))
  
  delIn := regDelIn
  when(delay.io.delayOut.valid) {
    regDelOut := delay.io.delayOut.bits
    regDelPtr := delay.io.delayOutPtr
  }

  for (i <- 0 until BURST_LENGTH) {
    var invar = delIn(i)
    for (j <- 0 until DATA_DENSITY) {
      delay.io.delayInRsp.bits(i * DATA_DENSITY + j) := invar.head(SAMPLE_WIDTH).asSInt
      invar = invar.tail(SAMPLE_WIDTH)
    }
  }

  for (i <- 0 until BURST_LENGTH) {
    var outvar = delay.io.delayOut.bits(i * DATA_DENSITY).asUInt
    for (j <- 1 until DATA_DENSITY) {
      outvar = Cat(outvar, delay.io.delayOut.bits(i * DATA_DENSITY + j).asUInt)
    }
    delOut(i) := outvar
  }
  
  // Input.
  val regSample = RegInit(0.S(SAMPLE_WIDTH.W))
  val regSampleValid = RegInit(false.B)
  
  // Distortion Plumbing.
  val distDemux = Module(new DecoupledDemux(SInt(SAMPLE_WIDTH.W)))
  val distMux = Module(new DecoupledMux(SInt(SAMPLE_WIDTH.W)))
  
  distDemux.io.sel := regDistEn
  distMux.io.sel := regDistEn
  
  distDemux.io.in.bits := regSample
  distDemux.io.in.valid := regSampleValid
  when (distDemux.io.in.ready) {
    regSampleValid := false.B
  }
  
  distDemux.io.out1 <> distortion.io.in
  distortion.io.out <> distOutGain.io.in
  distOutGain.io.out <> distMux.io.in1

  distDemux.io.out2 <> distMux.io.in2
    
  // Delay Plumbing.
  val delDemux = Module(new DecoupledDemux(SInt(SAMPLE_WIDTH.W)))
  val delMux = Module(new DecoupledMux(SInt(SAMPLE_WIDTH.W)))
  
  val delInBuffer = Module(new DeserializationBuffer(SInt(SAMPLE_WIDTH.W), DELAY_PACKET_SIZE))
  val delOutBuffer = Module(new SerializationBuffer(SInt(SAMPLE_WIDTH.W), DELAY_PACKET_SIZE))
  
  delDemux.io.sel := regDelEn
  delMux.io.sel := regDelEn
  
  delDemux.io.out1 <> delInBuffer.io.in
  delInBuffer.io.out <> delay.io.signalIn
  delay.io.signalOut <> delOutBuffer.io.in
  delOutBuffer.io.out <> delOutGain.io.in
  delOutGain.io.out <> delMux.io.in1
  delMux.io.in1.bits := delOutGain.io.out.bits.min(((1 << (SAMPLE_WIDTH - 1)) - 1).S).max((-(1 << (SAMPLE_WIDTH - 1))).S)  // override data assignment with clipping due to possible gain > 1
  
  delDemux.io.out2 <> delMux.io.in2
    
  // Distortion -> Delay.
  val distDelQueue = Module(new Queue(SInt(SAMPLE_WIDTH.W), DELAY_PACKET_SIZE))
  distDelQueue.io.enq <> distMux.io.out
  distDelQueue.io.deq <> delDemux.io.in
  
  // OutputQueue.
  val outQueue = Module(new Queue(SInt(SAMPLE_WIDTH.W), DELAY_PACKET_SIZE))
  outQueue.io.enq <> delMux.io.out
    
  // COP access logic.  
  io.copOut.ena_out := false.B
  io.copOut.result := 0.U
    
  outQueue.io.deq.ready := false.B
  
  val regTrigger = RegInit(false.B)
  
  when((io.copIn.trigger || regTrigger) & io.copIn.ena_in) {
    when(io.copIn.isCustom) {
      // Nothing to do here
    }.elsewhen(io.copIn.read) {
      switch(io.copIn.funcId) {
        is (FUNC_SAMPLE) {
          when (outQueue.io.deq.valid) {
            outQueue.io.deq.ready := true.B
            io.copOut.result := outQueue.io.deq.bits.asUInt
            
            io.copOut.ena_out := true.B
            regTrigger := false.B
          }.otherwise {
            regTrigger := true.B
          }
        }
      }
    }.otherwise {
      switch(io.copIn.funcId) {
        is (FUNC_SAMPLE) {
          when (!regSampleValid) {
            regSampleValid := true.B
            regSample := io.copIn.opData(0).asSInt
            
            io.copOut.ena_out := true.B
            regTrigger := false.B
          }.otherwise {
            regTrigger := true.B
          }
        }
        is (FUNC_MOD_EN) {
          regDistEn := io.copIn.opData(0)(0)
          regDelEn := io.copIn.opData(0)(1)
        }
        is (FUNC_DIST_GAIN) {
          regDistGain := io.copIn.opData(0)
        }
        is (FUNC_DIST_OUTGAIN) {
          regDistOutGain := io.copIn.opData(0)
        }
        is (FUNC_DEL_BUF) {
          regDelBuf := io.copIn.opData(0)
        }
        is (FUNC_DEL_LEN) {
          regDelLen := io.copIn.opData(0)
        }
        is (FUNC_DEL_MAXLEN) {
          regDelMaxLen := io.copIn.opData(0)
        }
        is (FUNC_DEL_MIX) {
          regDelMix := io.copIn.opData(0)
        }
        is (FUNC_DEL_FB) {
          regDelFB := io.copIn.opData(0)
        }
        is (FUNC_DEL_OUTGAIN) {
          regDelOutGain := io.copIn.opData(0)
        }
      }
    }
  }  
  
  // FSM.
  delay.io.delayInRsp.valid := false.B
  delay.io.delayOut.ready := false.B


  io.memPort.M.Cmd := OcpCmd.IDLE
  io.memPort.M.Addr := 0.U
  io.memPort.M.Data := 0.U
  io.memPort.M.DataValid := 0.U
  io.memPort.M.DataByteEn := "b1111".U
  
  switch(regMemState) {
    is(memIdle) {
      when (delay.io.delayInReq.valid) {
        io.memPort.M.Cmd := OcpCmd.RD
        io.memPort.M.Addr := regDelBuf + (delay.io.delayInReq.bits << log2Floor(BURST_LENGTH))
        when (io.memPort.S.CmdAccept === 1.U) {
          regMemState := memRd
          memCounter.reset()
        }
      }
      
      when (delay.io.delayOut.valid) {
        // Do not apply data yet because the regDelOut has to fetch it first.
        delay.io.delayOut.ready := true.B
        regMemState := memWrReq
        memCounter.reset()
      }
    }
    is(memRd) {
      when (io.memPort.S.Resp === OcpResp.DVA) {
        regDelIn(memCounter.value) := io.memPort.S.Data
        when (memCounter.inc()) {
          regMemState := memRdWait
        }
      }
    }
    is(memRdWait) {
      delay.io.delayInRsp.valid := true.B
      regMemState := memIdle
    }
    is(memWrReq) {
      io.memPort.M.Cmd := OcpCmd.WR
      io.memPort.M.Addr := regDelBuf + (regDelPtr << log2Floor(BURST_LENGTH))
      io.memPort.M.Data := delOut(memCounter.value)
      io.memPort.M.DataValid := 1.U
      
      when (io.memPort.S.CmdAccept === 1.U) {
        regMemState := memWr
      }
      when (io.memPort.S.DataAccept === 1.U) {
        memCounter.inc()
      }
    }
    is(memWr) {
      io.memPort.M.Data := delOut(memCounter.value)
      io.memPort.M.DataValid := 1.U
      
      when (io.memPort.S.DataAccept === 1.U) {
        when (memCounter.inc()) {
          regMemState := memWrWait
        }
      }
    }
    is(memWrWait) {
      when (io.memPort.S.Resp === OcpResp.DVA) {
        regMemState := memIdle
      }
    }
  }
  
}
