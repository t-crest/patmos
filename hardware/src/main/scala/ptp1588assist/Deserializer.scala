package ptp1588assist

import chisel3._
import chisel3.util._

/*
 *  Deserialize$inputWidth to M-bit parallel
 */
class Deserializer(msbFirst: Boolean = false, inputWidth: Int = 4, outputWidth: Int = 8) extends Module{
    val io = IO(new Bundle() {
        val en = Input(Bool())
        val clr = Input(Bool())
        val shiftIn = Input(UInt(inputWidth.W))
        val shiftOut = Output(UInt(outputWidth.W))
        val done = Output(Bool())
    })

    val SHIFT_AMOUNT : UInt = (outputWidth / inputWidth).U - 1.U

    val shiftReg = RegInit(0.U(outputWidth.W))
    val countReg = RegInit(0.U((log2Floor(outputWidth/inputWidth)+1).W))
    val dataReg = RegInit(0.U(outputWidth.W))
    val doneReg = RegInit(false.B)

    // Shift-register
    when(io.clr) {
        shiftReg := 0.U
    } .elsewhen(io.en) {
        if (msbFirst) {
			      shiftReg := shiftReg(outputWidth-inputWidth-1,0) ## io.shiftIn
        } else {
            shiftReg := io.shiftIn ## shiftReg(outputWidth-1, inputWidth)
        }
    }

    // Shift Counter
    when(io.clr) {
        countReg := 0.U
    } .elsewhen(io.en) {
        when(countReg === SHIFT_AMOUNT) {
            countReg := 0.U
        }   .otherwise {
            countReg := countReg + 1.U
        }
    }

    // Flags
    when(countReg === SHIFT_AMOUNT && io.en){
        doneReg := true.B
    }.otherwise {
        doneReg := false.B
    }

    io.shiftOut := shiftReg
    io.done := doneReg
}