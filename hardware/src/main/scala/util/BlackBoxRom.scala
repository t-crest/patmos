/*
 * Black Box ROM to fix Chisel 2 to Chisel 3 migration issue in patmos Fetch stage
 * based on https://www.chisel-lang.org/chisel3/docs/explanations/blackboxes.html
 *
 * Author: Bosse Bandowski (bosse.bandowski@outlook.com)
 *
 */

package util


import chisel3._
import chisel3.util.HasBlackBoxInline
import patmos.Constants._

class BlackBoxRom(romContents : (Array[BigInt], Array[BigInt]), addrWidth : Int) extends BlackBox with HasBlackBoxInline {
    
    val io = IO(new Bundle {
        val addressEven = Input(UInt(addrWidth.W))
        val addressOdd = Input(UInt(addrWidth.W))
        val instructionEven = Output(UInt(INSTR_WIDTH.W))
        val instructionOdd = Output(UInt(INSTR_WIDTH.W))
    })

    val romLinePatternEven = "\t|\t%d: instructionEven = %d'h%x;\n"
    val romLinePatternOdd = "\t|\t%d: instructionOdd = %d'h%x;\n"


    val HEADER =    """module %s
        |#(
        |    parameter  addrWidth = %d,
        |               instrWidth = %d
        |)
        |(
        |    input wire [addrWidth-1:0] addressEven,
        |    input wire [addrWidth-1:0] addressOdd,
        |    output reg [instrWidth-1:0] instructionEven,
        |    output reg [instrWidth-1:0] instructionOdd
        |);
        """.format(name, addrWidth, INSTR_WIDTH)

    val FOOTER_EVEN =    """
        |    default: begin
        |        instructionEven = %d'bx;
        |        `ifndef SYNTHESIS
        |            // synthesis translate_off
        |            instructionEven = {1{$random}};
        |            // synthesis translate_on
        |        `endif
        |    end
        |endcase
        """.format(INSTR_WIDTH)

    val FOOTER_ODD =    """
        |    default: begin
        |        instructionOdd = %d'bx;
        |        `ifndef SYNTHESIS
        |            // synthesis translate_off
        |            instructionOdd = {1{$random}};
        |            // synthesis translate_on
        |        `endif
        |    end
        |endcase
        """.format(INSTR_WIDTH)

    val FOOTER_MODULE = "\n|endmodule"


    var BODY_EVEN = "\n|always @(*) case (addressEven)\n"
    var BODY_ODD = "\n|always @(*) case (addressOdd)\n"

    for (id <- 0 to romContents._1.length - 1) {
        BODY_EVEN = BODY_EVEN + romLinePatternEven.format(id, INSTR_WIDTH, romContents._1(id))
        BODY_ODD = BODY_ODD + romLinePatternOdd.format(id, INSTR_WIDTH, romContents._2(id))
    }

    setInline(name + ".v", (HEADER + BODY_EVEN + FOOTER_EVEN + BODY_ODD + FOOTER_ODD + FOOTER_MODULE).stripMargin)
}