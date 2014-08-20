// Test bench with Qdr memory
// Requires CY7C2263KV18 model, available from Cypress Semiconductor

`timescale 1 ns / 100 ps

module Patmos_tb();

   wire [8:0]  io_led;

   // QDR-II+ signals
   reg         ram_clk;
   wire [18:0] ram_addr;
   wire [1:0]  ram_nbws_0, ram_nbws_1;
   reg  [1:0]  ram_nbws;
   reg  [15:0] ram_din_0, ram_din_1, ram_din_0_buf;
   wire [17:0] ram_din;
   wire [15:0] ram_dout_0, ram_dout_1;
   reg  [17:0] ram_dout;
   wire        ram_odt, ram_ndoff;
   wire        ram_cq, ram_ncq;
   // JTAG signals for QDR-II+ memory
   reg         ram_tck, ram_tms, ram_tdi;
   wire        ram_tdo;
            
   // Pull down parity bits to a defined logic level
   wire pull_down;
   assign pull_down = 1'b0;

   Patmos patmos(.clk(clk), .reset(reset),
                 .io_cpuInfoPins_id(32'h00000000),
                 .io_ledsPins_led(io_led),
                 .io_uartPins_tx(io_uartPins_tx),
                 .io_uartPins_rx(io_uartPins_rx),

                 .io_qdrIIplusCtrlPins_addr(ram_addr),
                 .io_qdrIIplusCtrlPins_nrps(ram_nrps),
                 .io_qdrIIplusCtrlPins_nwps(ram_nwps),
                 .io_qdrIIplusCtrlPins_nbws_0(ram_nbws_0),
                 .io_qdrIIplusCtrlPins_nbws_1(ram_nbws_1),
                 .io_qdrIIplusCtrlPins_dout_0(ram_dout_0),
                 .io_qdrIIplusCtrlPins_dout_1(ram_dout_1),
                 .io_qdrIIplusCtrlPins_din_0(ram_din_0),
                 .io_qdrIIplusCtrlPins_din_1(ram_din_1),
                 .io_qdrIIplusCtrlPins_odt(ram_odt),
                 .io_qdrIIplusCtrlPins_ndoff(ram_ndoff),
                 .io_qdrIIplusCtrlPins_qvld(ram_qvld),

                 .io_comConf_S_Resp(2'b00),
                 .io_comSpm_S_Resp(2'b00));

   cyqdr2_b4 ram(ram_tck, ram_tms, ram_tdi, ram_tdo,
                 ram_dout, ram_din, ram_addr,
                 ram_clk, ~ram_clk,
                 ram_nrps, ram_nwps, ram_nbws[0], ram_nbws[1],
                 ram_cq, ram_ncq, 1'b1, ram_ndoff, ram_qvld, ram_odt);

   // Clock and reset handling
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
     #6.25 clk_reg = ~clk_reg;

   // QDR memory handling
   initial
     begin
	    ram_tck = 1;
	    ram_tms = 1;
	    ram_tdi = 1'bx;
     end

   // generate clock for QDR memory
   always @(clk)
     begin
        ram_clk = #3.12 clk;
     end

   // dual-edge input
   always @(posedge clk)
     begin
        ram_din_0_buf = ram_din[15:0];
     end
   always @(negedge clk)
     begin
        ram_din_0 = ram_din_0_buf;        
        ram_din_1 = ram_din[15:0];
     end
   // dual-edge output
   always @(clk, ram_nbws_0, ram_nbws_1, ram_dout_0, ram_dout_1)
     begin
        ram_nbws = clk ? ram_nbws_0 : ram_nbws_1;
        ram_dout = { 2'b00, clk ? ram_dout_0 : ram_dout_1 };
     end
   
   // Generate input for serial line
   reg [9:0] data;
   integer i, k;
   reg rx;
   assign io_uartPins_rx = rx;

   always
     begin

        rx = 1'b1;

        #500000;

        // Send minimal valid packet "08 65-22-df-69 00-00-00-00 00-00-00-00"

        // Size 0x08
        data = 10'b0000100001; // 0x08
        for (i = 9; i >= 0; i = i-1)
          begin
             #8681 rx = data[i];
          end
        // CRC 0x6522df69
        data = 10'b0101001101; // 0x65
        for (i = 9; i >= 0; i = i-1)
          begin
             #8681 rx = data[i];
          end
        data = 10'b0010001001; // 0x22
        for (i = 9; i >= 0; i = i-1)
          begin
             #8681 rx = data[i];
          end
        data = 10'b0111110111; // 0xdf
        for (i = 9; i >= 0; i = i-1)
          begin
             #8681 rx = data[i];
          end
        data = 10'b0100101101; // 0x69
        for (i = 9; i >= 0; i = i-1)
          begin
             #8681 rx = data[i];
          end

        // Entry point 0x00000000
        // Use DEBUG mode in boot loader and set entry point there!
        for (k = 3; k >= 0; k = k-1)
          begin
             data = 10'b0000000001;
             for (i = 9; i >= 0; i = i-1)
               begin
                  #8681 rx = data[i];
               end
          end
        // Segment count 0x00000000
        for (k = 3; k >= 0; k = k-1)
          begin
             data = 10'b0000000001;
             for (i = 9; i >= 0; i = i-1)
               begin
                  #8681 rx = data[i];
               end
          end

        #1000000000;

     end

endmodule // Patmos_tb
