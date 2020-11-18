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
    val idle :: fibonacci_running :: Nil = Enum(2)
    val op_state = Reg(init = idle)
    val read_idle :: fibonacci_read_request :: Nil = Enum(2) 
    val read_state = Reg(init = read_idle)
    val current_value = Reg(UInt(width = 32))
    val last_value = Reg(UInt(width = 32))
    val next_value = Wire(UInt(width = 32))
    val iterations = Reg(UInt(width = 32))
    val current_iteration = Reg(UInt(width = 32))

    io.copOut.result := UInt(0)
    io.copOut.ena_out := Bool(false)

    // start operation 
    when(io.copIn.trigger & io.copIn.ena_in) {
        
        when(io.copIn.read) {
            when(op_state === idle) {
                // fibonacci calculation
                when(io.copIn.funcId === FUNC_FIBONACCI) {
                    io.copOut.result    := current_value
                    io.copOut.ena_out   := Bool(true)
                }
            }
            .otherwise
            {
                 // fibonacci calculation
                when(io.copIn.funcId === FUNC_FIBONACCI) {
                    read_state := fibonacci_read_request
                }
            }
        }
        .otherwise
        {
            when(op_state === idle) {
                // fibonacci calculation
                when(io.copIn.funcId === FUNC_FIBONACCI) {
                    current_value       := UInt(1)
                    last_value          := UInt(1)
                    op_state            := fibonacci_running
                    current_iteration   := UInt(1)
                    iterations          := io.copIn.opData(0)
                    io.copOut.ena_out   := Bool(true)
                }
            }
            .otherwise
            {
                io.copOut.ena_out   := Bool(true)
            }
            
        }
    }

    // output logic for 1-latency add
    when(op_state === fibonacci_running) {
        
        when (iterations <= current_iteration) {
            op_state := idle
        }
        .otherwise
        {
            next_value          := last_value + current_value
            last_value          := current_value
            current_value       := next_value
            current_iteration   := current_iteration + UInt(1)
        }
    }

    when (read_state === fibonacci_read_request & op_state === idle)
    {
        io.copOut.result    := current_value
        io.copOut.ena_out   := Bool(true)
        read_state          := read_idle
    }

}