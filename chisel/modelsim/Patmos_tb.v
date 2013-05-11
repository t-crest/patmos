`timescale 1 ns / 100 ps

module Patmos_tb();
   Patmos patmos(.clk(clk), .reset(reset),
				 .io_dummy(io_dummy),
				 .io_led(io_led),
				 .io_uart_address(io_uart_address),
				 .io_uart_wr_data(io_uart_wr_data),
				 .io_uart_rd(io_uart_rd),
				 .io_uart_wr(io_uart_wr),
				 .io_uart_rd_data(io_uart_rd_data));

   reg clk_reg;
   reg reset_reg;   
   assign clk = clk_reg;
   assign reset = reset_reg;
   
   initial
	 begin
		$display($time, " << Starting the Simulation >>");
		clk_reg = 1'b0;
		reset_reg = 1'b1;
		#100 reset_reg = 1'b0;
	 end
   always
	 #10 clk_reg = ~clk_reg;
   
endmodule