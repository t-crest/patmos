/*
 * Fibonacci Coprocessor to demo Split-Phase Cop-Write and Cop-Read with stalling
 *
 * Authors: Christoph Lehr (christoph.lehr@gmx.at)
 *          Alexander Baranyai (alexyai@gmx.at)
 *          Clemens Pircher (clemens.lukas@gmx.at)
 *
 */

package cop

import Chisel._

import patmos.Constants._
import util._
import ocp._

object Fibonacci extends CoprocessorObject {

  def init(params: Map[String, String]) = {}

  def create(params: Map[String, String]): Fibonacci = Module(new Fibonacci())
}

class Fibonacci() extends BaseCoprocessor() {
    //coprocessor definitions
    def FUNC_FIBONACCI = "b00010".U(5.W)

    //some coprocessor registers
    val idle :: fibonacciRunning :: Nil = Enum(2)
    val opStateReg = Reg(init = idle)
    val readIdle :: fibonacciReadRequest :: Nil = Enum(2) 
    val readStateReg = Reg(init = readIdle)
    val currentValueReg = Reg(UInt(32.W))
    val lastValueReg = Reg(UInt(32.W))
    val nextValue = Wire(UInt(32.W))
    val iterationsReg = Reg(UInt(32.W))
    val currentIterationReg = Reg(UInt(32.W))

    io.copOut.result := 0.U
    io.copOut.ena_out := false.B

    // start operation 
    when(io.copIn.trigger & io.copIn.ena_in) {
        
        when(io.copIn.read) {
            when(opStateReg === idle) {
                // fibonacci calculation
                when(io.copIn.funcId === FUNC_FIBONACCI) {
                    io.copOut.result    := currentValueReg
                    io.copOut.ena_out   := true.B
                }
            }
            .otherwise
            {
                 // fibonacci calculation
                when(io.copIn.funcId === FUNC_FIBONACCI) {
                    readStateReg := fibonacciReadRequest
                }
            }
        }
        .elsewhen(!io.copIn.isCustom)
        {
            when(opStateReg === idle) {
                // fibonacci calculation
                when(io.copIn.funcId === FUNC_FIBONACCI) {
                    currentValueReg       := 1.U
                    lastValueReg          := 1.U
                    opStateReg            := fibonacciRunning
                    currentIterationReg   := 1.U
                    iterationsReg          := io.copIn.opData(0)
                    io.copOut.ena_out   := true.B
                }
            }
            .otherwise
            {
                io.copOut.ena_out   := true.B
            }
            
        }
    }

    // output logic for 1-latency add
    when(opStateReg === fibonacciRunning) {
        
        when (iterationsReg <= currentIterationReg) {
            opStateReg := idle
        }
        .otherwise
        {
            nextValue          := lastValueReg + currentValueReg
            lastValueReg          := currentValueReg
            currentValueReg       := nextValue
            currentIterationReg   := currentIterationReg + 1.U
        }
    }

    when (readStateReg === fibonacciReadRequest & opStateReg === idle)
    {
        io.copOut.result    := currentValueReg
        io.copOut.ena_out   := true.B
        readStateReg          := readIdle
    }

}
