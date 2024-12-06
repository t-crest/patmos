package patmos


import scala.io.Source
import scala.util.control.Breaks._
import java.io.PrintWriter
import java.io.File



object MakeRams{
	def main(): Unit = {

		val filename = new File("build/Patmos.v")

		val tempFile = new File("build/PatmosTemp.v")
		val w = new PrintWriter(tempFile)

		var found = false
		var start = 0


		val lines = Source.fromFile(filename).getLines().toArray

		var stop = lines.length

		breakable { for (line <- 0 until lines.length){
			if(lines(line).contains("module TrueDualPortMemory(input clk, input reset,")){
				found = true
				start = line


				


				break()
			} else {
				w.println(lines(line))
			}
		} }

		if(found){
			found = false

			breakable { for (line <- start until lines.length){
				if(lines(line).contains("endmodule")){
					found = true
					stop = line + 1

					var size = 0

					var tmp = lines(start + 1)

					tmp = tmp.substring(tmp.indexOf('[')+1, tmp.indexOf(':'))

					writeRam(w, tmp.toInt)



					break()
				}
			} }

			for(line <- stop until lines.length){
					w.println(lines(line))
			}
			
		}

		w.close()

		tempFile.renameTo(filename)



	}

	def writeRam(w: PrintWriter, width : Int): Unit ={

		w.println(s"""
module TrueDualPortMemory(input clk, input reset,
    input [${width}:0] io_portA_addr,
    input [31:0] io_portA_wrData,
    output[31:0] io_portA_rdData,
    input  io_portA_wrEna,
    input [${width}:0] io_portB_addr,
    input [31:0] io_portB_wrData,
    output[31:0] io_portB_rdData,
    input  io_portB_wrEna
);

  reg [31:0] T0;
  reg [31:0] T1 [${Math.pow(2,width+1).toInt} - 1:0];
  wire[31:0] T2;
  wire[31:0] T3;
  wire[${width}:0] R4;
  wire[${width}:0] T11;
  wire[${width}:0] T5;
  wire T6;
  reg [31:0] T7;
  wire[${width}:0] R8;
  wire[${width}:0] T12;
  wire[${width}:0] T9;
  wire T10;

`ifndef SYNTHESIS
// synthesis translate_off
  integer initvar;
  initial begin
    #0.002;
    for (initvar = 0; initvar < ${Math.pow(2,width+1).toInt}; initvar = initvar+1)""")
w.println("""
      T1[initvar] = {1{$random}};
    R4 = {1{$random}};
    R8 = {1{$random}};
  end
// synthesis translate_on
`endif

  assign io_portB_rdData = T0;

  assign T11 = reset ? 7'h0 : T5;
  assign T5 = T6 ? io_portB_addr : R4;
  assign T6 = io_portB_wrEna ^ 1'h1;
  assign io_portA_rdData = T7;

  assign T12 = reset ? 7'h0 : T9;
  assign T9 = T10 ? io_portA_addr : R8;
  assign T10 = io_portA_wrEna ^ 1'h1;

  assign R8 = io_portA_addr;
  assign R4 = io_portB_addr;

  always @(posedge clk) begin
    T0 <= T1[R4];

    if (io_portA_wrEna)
      T1[io_portA_addr] <= io_portA_wrData;
  end

  always @(posedge clk) begin
    T7 <= T1[R8];

    if (io_portB_wrEna)
      T1[io_portB_addr] <= io_portB_wrData;
  end
endmodule
	""")

	}
	
}
