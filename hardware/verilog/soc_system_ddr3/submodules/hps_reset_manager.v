`timescale 1 ps / 1 ps
module hps_reset_manager (
        output wire  hps_warm_reset,   // hps_warm_reset.reset
        output wire  hps_cold_reset,   // hps_cold_reset.reset
        input  wire  hps_fpga_reset_n, // hps_fpga_reset.reset_n
        input  wire  clock_clk         //          clock.clk
    );

wire [1:0]  hps_reset_req;

// Source/Probe megawizard instance
hps_reset hps_reset_inst (
  .source_clk (clock_clk),
  .source     (hps_reset_req)
);

altera_edge_detector pulse_cold_reset (
  .clk       (clock_clk),
  .rst_n     (hps_fpga_reset_n),
  .signal_in (hps_reset_req[0]),
  .pulse_out (hps_cold_reset)
);
  defparam pulse_cold_reset.PULSE_EXT = 6;
  defparam pulse_cold_reset.EDGE_TYPE = 1;
  defparam pulse_cold_reset.IGNORE_RST_WHILE_BUSY = 1;

altera_edge_detector pulse_warm_reset (
  .clk       (clock_clk),
  .rst_n     (hps_fpga_reset_n),
  .signal_in (hps_reset_req[1]),
  .pulse_out (hps_warm_reset)
);
  defparam pulse_warm_reset.PULSE_EXT = 2;
  defparam pulse_warm_reset.EDGE_TYPE = 1;
  defparam pulse_warm_reset.IGNORE_RST_WHILE_BUSY = 1;

endmodule
