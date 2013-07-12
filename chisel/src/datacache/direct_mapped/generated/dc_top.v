

module dc_top(output led);

  wire h_io_led;
  reg clk, reset;
  initial begin
	clk = 1; reset = 0;
	 #5  reset = 1;    // Assert the reset
	#25  reset = 0; 
  end

  always begin
   #10  clk = ~clk; // Toggle clock every 5 ticks
  end
	
  assign led = h_io_led;

  
  //assign res = 1'h0;
  
  
  Test_dc dc_t(.clk(clk), .reset(reset), .io_led( h_io_led ));
endmodule



