`timescale 1 ns / 100 ps

module Patmos_tb();

   wire [8:0]  io_led;

   wire [18:0] sram_addr;
   wire [3:0]  sram_nbw;
   wire [31:0] sram_dout;
   wire [31:0] sram_din;
   wire [31:0] sram_dinout;

   // Pull down parity bits to a defined logic level
   wire pull_down;
   assign pull_down = 1'b0;

   Patmos patmos(.clk(clk), .reset(reset),
                 .io_cpuInfoPins_id(32'h00000000),
                 .io_ledsPins_led(io_led),
                 .io_uartPins_tx(io_uartPins_tx),
                 .io_uartPins_rx(io_uartPins_rx),
                 .io_sSRam32CtrlPins_ramOut_addr(sram_addr),
                 .io_sSRam32CtrlPins_ramOut_doutEna(sram_doutEna),
                 .io_sSRam32CtrlPins_ramOut_nadsc(sram_nadsc),
                 .io_sSRam32CtrlPins_ramOut_noe(sram_noe),
                 .io_sSRam32CtrlPins_ramOut_nbwe(sram_nbwe),
                 .io_sSRam32CtrlPins_ramOut_nbw(sram_nbw[3:0]),
                 .io_sSRam32CtrlPins_ramOut_ngw(sram_ngw),
                 .io_sSRam32CtrlPins_ramOut_nce1(sram_nce1),
                 .io_sSRam32CtrlPins_ramOut_ce2(sram_ce2),
                 .io_sSRam32CtrlPins_ramOut_nce3(sram_nce3),
                 .io_sSRam32CtrlPins_ramOut_nadsp(sram_nadsp),
                 .io_sSRam32CtrlPins_ramOut_nadv(sram_nadv),
                 .io_sSRam32CtrlPins_ramOut_dout(sram_dout),
                 .io_sSRam32CtrlPins_ramIn_din(sram_din),
                 .io_comConf_S_Resp(2'b00),
                 .io_comSpm_S_Resp(2'b00));

   memory sram(.A0(sram_addr[0]),
               .A1(sram_addr[1]),
               .A2(sram_addr[2]),
               .A3(sram_addr[3]),
               .A4(sram_addr[4]),
               .A5(sram_addr[5]),
               .A6(sram_addr[6]),
               .A7(sram_addr[7]),
               .A8(sram_addr[8]),
               .A9(sram_addr[9]),
               .A10(sram_addr[10]),
               .A11(sram_addr[11]),
               .A12(sram_addr[12]),
               .A13(sram_addr[13]),
               .A14(sram_addr[14]),
               .A15(sram_addr[15]),
               .A16(sram_addr[16]),
               .A17(sram_addr[17]),
               .A18(sram_addr[18]),
               .DQA0(sram_dinout[0]),
               .DQA1(sram_dinout[1]),
               .DQA2(sram_dinout[2]),
               .DQA3(sram_dinout[3]),
               .DQA4(sram_dinout[4]),
               .DQA5(sram_dinout[5]),
               .DQA6(sram_dinout[6]),
               .DQA7(sram_dinout[7]),
               .DQB0(sram_dinout[8]),
               .DQB1(sram_dinout[9]),
               .DQB2(sram_dinout[10]),
               .DQB3(sram_dinout[11]),
               .DQB4(sram_dinout[12]),
               .DQB5(sram_dinout[13]),
               .DQB6(sram_dinout[14]),
               .DQB7(sram_dinout[15]),
               .DQC0(sram_dinout[16]),
               .DQC1(sram_dinout[17]),
               .DQC2(sram_dinout[18]),
               .DQC3(sram_dinout[19]),
               .DQC4(sram_dinout[20]),
               .DQC5(sram_dinout[21]),
               .DQC6(sram_dinout[22]),
               .DQC7(sram_dinout[23]),
               .DQD0(sram_dinout[24]),
               .DQD1(sram_dinout[25]),
               .DQD2(sram_dinout[26]),
               .DQD3(sram_dinout[27]),
               .DQD4(sram_dinout[28]),
               .DQD5(sram_dinout[29]),
               .DQD6(sram_dinout[30]),
               .DQD7(sram_dinout[31]),

               .DPA(pull_down),
               .DPB(pull_down),
               .DPC(pull_down),
               .DPD(pull_down),

               .BWENeg(sram_nbwe),
               .BWANeg(sram_nbw[0]),
               .BWBNeg(sram_nbw[1]),
               .BWCNeg(sram_nbw[2]),
               .BWDNeg(sram_nbw[3]),
               .GWNeg(sram_ngw),

               .CLK(sram_clk),
               .CE1Neg(sram_nce1),
               .CE2(sram_ce2),
               .CE3Neg(sram_nce3),
               .OENeg(sram_noe),
               .ADVNeg(sram_nadv),
               .ADSPNeg(sram_nadsp),
               .ADSCNeg(sram_nadsc),
               .MODE(1'b1),
               .ZZ(1'b0)
               );

   // Clock and reset handling
   reg clk_reg;
   reg reset_reg;
   assign clk = clk_reg;
   assign sram_clk = ~clk_reg;
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

   // Handle tri-state signals
   reg [31:0] sram_din_reg;
   reg [31:0] sram_din_reg2;
   assign sram_din = sram_din_reg2;
   assign sram_dinout = sram_doutEna ? sram_dout : 32'hZZZZZZZZ;
   always @(negedge clk)
     begin
        sram_din_reg = sram_dinout;
     end
   always @(posedge clk)
     begin
        sram_din_reg2 = sram_din_reg;
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
