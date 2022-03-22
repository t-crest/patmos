package soundbytes

import scala.math._
import chisel3._
import chisel3.util._

/**
 * Delay unit which is based on a read and write pointer for the delay buffer.
 * Supports dragging/boosting with half/twice the playback speed on changes of the delay time.
 *
 * Possible Improvements:
 * Using half/twice the playback speed on delay may be a bit drastic.
 * Essentially, it lowers/increases the perceived pitch by an entire octave (or multiple with sufficient feedback and delayLength delta).
 * However, it is convenient and has low hardware footprint.
 * Maybe it would be good to supports different mechanisms for this.
 */
class Delay(dataWidth: Int = 16, ptrWidth: Int = 12, counterBits: Int = 3, mixWidth: Int = 6, feedbackWidth: Int = 6) extends Module {
  val packetSize = 1 << counterBits
  
  val io = IO(new Bundle {
    val signalIn = Flipped(Decoupled(Vec(packetSize, SInt(dataWidth.W))))
    val signalOut = Decoupled(Vec(packetSize, SInt(dataWidth.W)))
    
    val delayInReq = Valid(UInt(ptrWidth.W))
    val delayInRsp = Flipped(Valid(Vec(packetSize, SInt(dataWidth.W))))
    val delayOut = Decoupled(Vec(packetSize, SInt(dataWidth.W)))
    val delayOutPtr = Output(UInt(ptrWidth.W))
    
    val delayLength = Input(UInt(ptrWidth.W))
    val delayMaxLength = Input(UInt(ptrWidth.W))
    val mix = Input(UInt(mixWidth.W))
    val feedback = Input(UInt(feedbackWidth.W))
  })
    
  // Actual Delay Engine.
  val engine = Module(new DelayEngine(dataWidth, mixWidth, feedbackWidth))
  engine.io.in.valid := false.B
  engine.io.out.ready := false.B
  engine.io.mix := io.mix
  engine.io.feedback := io.feedback
  engine.io.in.bits(0) := 0.S
  engine.io.in.bits(1) := 0.S
  
  // Delay Sample Pointers.
  val regDelayLength = RegInit(8.U(ptrWidth.W))
  val rdPtr = WireDefault(0.U(ptrWidth.W))
  val wrPtr = RegInit(0.U(ptrWidth.W))
  when (wrPtr < regDelayLength) {
    rdPtr := wrPtr - regDelayLength + io.delayMaxLength + 1.U
  }.otherwise {
    rdPtr := wrPtr - regDelayLength
  }
  
  // State Variables.
  // Special computeDrag/computeBoost states are used when increasing/decreasing delayLength.
  // Prefetch and Interpolate is required to keep interpolation state for next operation during dragging.
  // SecondRead is required to keep track during boosting.
  val idle :: readDelay :: compute :: computeDrag :: computeBoost :: writeDelay :: hasValue :: Nil = Enum(7)
  val regState = RegInit(idle)
  
  val regPrefetch = RegInit(false.B)
  val regInterpolate = RegInit(0.S(dataWidth.W))
  val regSecondRead = RegInit(false.B)
  
  val computeCounter = Counter(packetSize)
  val dragCounter = (!regPrefetch) ## computeCounter.value.head(counterBits - 1)  // pattern is 0 0 1 1 2 2 ...
  val boostCounter = computeCounter.value.tail(1) ## 0.U(1.W)                     // pattern is 0 2 4 ... 0 2 4 ...
  
  val regSigInVals = RegInit(VecInit(Seq.fill(packetSize)(0.S(dataWidth.W))))
  val regDelInVals = RegInit(VecInit(Seq.fill(packetSize)(0.S(dataWidth.W))))
  val regSigOutVals = RegInit(VecInit(Seq.fill(packetSize)(0.S(dataWidth.W))))
  val regDelOutVals = RegInit(VecInit(Seq.fill(packetSize)(0.S(dataWidth.W))))
  
  // FSM.
  io.signalOut.bits := regSigOutVals
  io.delayInReq.valid := false.B
  io.delayInReq.bits := rdPtr
  io.delayOut.valid := false.B
  io.delayOutPtr := wrPtr
  io.delayOut.bits := regDelOutVals

  io.signalIn.ready := regState === idle
  io.signalOut.valid := regState === hasValue
  switch (regState) {
    is (idle) {
      when (io.signalIn.valid) {
        regSigInVals := io.signalIn.bits
        when (regPrefetch) {
          regPrefetch := false.B
          regState := computeDrag
        }.otherwise {
          regState := readDelay
        }
      }
    }
    is (readDelay) {
      io.delayInReq.valid := true.B

      when (io.delayInRsp.valid) {
        regDelInVals := io.delayInRsp.bits
        
        when (regDelayLength === io.delayLength & !regSecondRead) {
          regState := compute
        }.elsewhen (regDelayLength < io.delayLength) {
          regInterpolate := regDelInVals(packetSize - 1)
          regPrefetch := true.B
          regState := computeDrag
        }.otherwise {
          regState := computeBoost
        }
      }
    }
    // Delay can stay constant.
    is (compute) {
      when (engine.io.in.ready) {
        engine.io.in.valid := true.B
        engine.io.in.bits(0) := regSigInVals(computeCounter.value)
        engine.io.in.bits(1) := regDelInVals(computeCounter.value)
      }

      when (engine.io.out.valid) {
        engine.io.out.ready := true.B
        regSigOutVals(computeCounter.value) := engine.io.out.bits(0)
        regDelOutVals(computeCounter.value) := engine.io.out.bits(1)
        
        when (computeCounter.inc()) {
          regState := writeDelay
        }
      }    
    }
    // Delay must be increased.
    is (computeDrag) {
      when (engine.io.in.ready) {
        engine.io.in.valid := true.B
        engine.io.in.bits(0) := regSigInVals(computeCounter.value)

        // Multiplex delay vs interpolated delay.
        when (!computeCounter.value(0)) {
          engine.io.in.bits(1) := (regInterpolate +& regDelInVals(dragCounter)) >> 1
        }.otherwise {
          engine.io.in.bits(1) := regDelInVals(dragCounter)
        }
      }

      when (engine.io.out.valid) {
        engine.io.out.ready := true.B
        regSigOutVals(computeCounter.value) := engine.io.out.bits(0)
        regDelOutVals(computeCounter.value) := engine.io.out.bits(1)
        
        regInterpolate := regDelInVals(dragCounter)

        when (computeCounter.inc()) {
          regState := writeDelay
          when (!regPrefetch) {
            regDelayLength := regDelayLength + 1.U
          }
        }
      }
    }
    // Delay must be reduced.
    is (computeBoost) {
      when (engine.io.in.ready) {
        engine.io.in.valid := true.B
        engine.io.in.bits(0) := regSigInVals(computeCounter.value)
        engine.io.in.bits(1) := regDelInVals(boostCounter)
      }

      when (engine.io.out.valid) {
        engine.io.out.ready := true.B
        regSigOutVals(computeCounter.value) := engine.io.out.bits(0)
        regDelOutVals(computeCounter.value) := engine.io.out.bits(1)
        
        when (computeCounter.inc()) {
          regState := writeDelay
          regSecondRead := false.B
        }
        
        when (computeCounter.value === ((packetSize / 2) - 1).U) {
          regState := readDelay
          regDelayLength := regDelayLength - 1.U
          regSecondRead := true.B
        }
      }
    }
    is (writeDelay) {
      io.delayOut.valid := true.B

      when (io.delayOut.ready) {       
        when (wrPtr >= io.delayMaxLength) {
          wrPtr := 0.U
        }.otherwise {
          wrPtr := wrPtr + 1.U
        }
        
        regState := hasValue
      }
    }
    is (hasValue) {
      when (io.signalOut.ready) {
        regState := idle
      }
    }
  }
  
}
