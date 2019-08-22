package ptp1588assist

import Chisel._

/*
 *  Deserialize$inputWidth to M-bit parallel
 */
class Deserializer(msbFirst: Boolean = false, inputWidth: Int = 4, outputWidth: Int = 8) extends Module{
    val io = new Bundle() {
        val en = Bool(INPUT)
        val clr = Bool(INPUT)
        val shiftIn = Bits(INPUT, width = inputWidth)
        val shiftOut = Bits(OUTPUT, width = outputWidth)
        val done = Bool(OUTPUT)
    }

    val SHIFT_AMOUNT = (outputWidth / inputWidth).U - 1.U

    val shiftReg = Reg(init = Bits(0, width = outputWidth))
    val countReg = Reg(init = UInt(0, width=log2Floor(outputWidth/inputWidth)+1))
    val dataReg = Reg(init = Bits(0, width = outputWidth))
    val doneReg = Reg(init = Bool(false))

    // Shift-register
    when(io.clr) {
        shiftReg := 0.U
    } .elsewhen(io.en) {
        if (msbFirst) {
            shiftReg(inputWidth-1, 0) := io.shiftIn
			      shiftReg(outputWidth-1, inputWidth) := shiftReg(outputWidth-inputWidth-1,0)
        } else {
            shiftReg(outputWidth-inputWidth-1,0) := shiftReg(outputWidth-1, inputWidth)
            shiftReg(outputWidth-1, outputWidth-inputWidth) := io.shiftIn
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