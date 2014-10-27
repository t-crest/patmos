`timescale 1 ps/1 ps

module v6_emac_v2_3_wrapper
   (
      // asynchronous reset
      input         glbl_rst,
	  
      // clock inputs
      input         gtx_clk_bufg,
	  input         cpu_clk,
      input         refclk_bufg,
	  input			dcm_lck,

      output        phy_resetn,
	  
      // GMII Interface
      //---------------

      output [7:0]  gmii_txd,
      output        gmii_tx_en,
      output        gmii_tx_er,
      output        gmii_tx_clk,
      input  [7:0]  gmii_rxd,
      input         gmii_rx_dv,
      input         gmii_rx_er,
      input         gmii_rx_clk,
      input         gmii_col,
      input         gmii_crs,
      input         mii_tx_clk,


      // Receiver (AXI-S) Interface
      //----------------------------------------
	  output [7:0]	rx_axis_fifo_tdata,
	  output  		rx_axis_fifo_tvalid,
	  input 		rx_axis_fifo_tready,
	  output 		rx_axis_fifo_tlast,

      // Transmitter (AXI-S) Interface
      //-------------------------------------------
	  input [7:0]	tx_axis_fifo_tdata,
	  input  		tx_axis_fifo_tvalid,
	  output 		tx_axis_fifo_tready,
	  input 		tx_axis_fifo_tlast
    );

   // control parameters
   parameter            BOARD_PHY_ADDR = 5'h7;

   //----------------------------------------------------------------------------
   // internal signals used in this top level wrapper.
   //----------------------------------------------------------------------------

   reg                  phy_resetn_int;
   // resets (and reset generation)
   wire                 cpu_clk_rst_int;
   reg                  cpu_pre_rst_n = 0;
   reg                  cpu_rst_n = 0;

   wire                 glbl_rst_int;
   reg   [5:0]          phy_reset_count;
   wire                 glbl_rst_intn;
   // USER side RX AXI-S interface
   wire                 rx_fifo_clock;
   wire                 rx_fifo_resetn;

   // USER side TX AXI-S interface
   wire                 tx_fifo_clock;
   wire                 tx_fifo_resetn;

   // signal tie offs
   wire  [7:0]         tx_ifg_delay = 0;    // not used in this example

  //---------------
  // global reset
   reset_sync glbl_reset_gen (
      .clk              (gtx_clk_bufg),
      .enable           (dcm_lck),
      .reset_in         (glbl_rst),
      .reset_out        (glbl_rst_int)
   );

   assign glbl_rst_intn = !glbl_rst_int;

  //----------------------------------------------------------------------------
  // Generate the user side clocks for the axi fifos
  //----------------------------------------------------------------------------
  assign tx_fifo_clock = cpu_clk;
  assign rx_fifo_clock = cpu_clk;

  //----------------------------------------------------------------------------
  // Generate resets required for the fifo side signals etc
  //----------------------------------------------------------------------------
  // in each case the async reset is first captured and then synchronised


  //---------------
  // cpu_clk reset
   reset_sync gtx_reset_gen (
	  .clk              (cpu_clk),
      .enable           (dcm_lck),
      .reset_in         (glbl_rst),
      .reset_out        (cpu_clk_rst_int)
   );

   // Create fully synchronous reset in the cpu_clk domain.
   always @(posedge cpu_clk)
   begin
     if (cpu_clk_rst_int) begin
       cpu_pre_rst_n  <= 0;
       cpu_rst_n      <= 0;
     end
     else begin
       cpu_pre_rst_n  <= 1;
       cpu_rst_n      <= cpu_pre_rst_n;
     end
   end

   //---------------
   // PHY reset
   // the phy reset output (active low) needs to be held for at least 10x25MHZ cycles
   // this is derived using the 125MHz available and a 6 bit counter
   always @(posedge gtx_clk_bufg)
   begin
      if (!glbl_rst_intn) begin
         phy_resetn_int <= 1'b0;
         phy_reset_count <= 6'd0;
      end
      else begin
         if (!(&phy_reset_count)) begin
            phy_reset_count <= phy_reset_count + 6'd1;
         end
         else begin
            phy_resetn_int <= 1'b1;
         end
      end
   end

   assign phy_resetn = phy_resetn_int;

   // generate the user side resets for the axi fifos
   assign tx_fifo_resetn = cpu_rst_n;
   assign rx_fifo_resetn = cpu_rst_n;
   

  //----------------------------------------------------------------------------
  // Instantiate the V6 Hard EMAC core fifo block wrapper
  //----------------------------------------------------------------------------
  v6_emac_v2_3_fifo_block v6emac_fifo_block (
      .gtx_clk                      (gtx_clk_bufg),

      // Reference clock for IDELAYCTRL's
      .refclk                       (refclk_bufg),

      // Receiver Statistics Interface
      //---------------------------------------
      .rx_mac_aclk                  (),
      .rx_reset                     (),
      .rx_statistics_vector         (),
      .rx_statistics_valid          (),

      // Receiver (AXI-S) Interface
      //----------------------------------------
      .rx_fifo_clock                (rx_fifo_clock),
      .rx_fifo_resetn               (rx_fifo_resetn),
      .rx_axis_fifo_tdata           (rx_axis_fifo_tdata),
      .rx_axis_fifo_tvalid          (rx_axis_fifo_tvalid),
      .rx_axis_fifo_tready          (rx_axis_fifo_tready),
      .rx_axis_fifo_tlast           (rx_axis_fifo_tlast),

      // Transmitter Statistics Interface
      //------------------------------------------
      .tx_mac_aclk                  (),
      .tx_reset                     (),
      .tx_ifg_delay                 (tx_ifg_delay),
      .tx_statistics_vector         (),
      .tx_statistics_valid          (),

      // Transmitter (AXI-S) Interface
      //-------------------------------------------
      .tx_fifo_clock                (tx_fifo_clock),
      .tx_fifo_resetn               (tx_fifo_resetn),
      .tx_axis_fifo_tdata           (tx_axis_fifo_tdata),
      .tx_axis_fifo_tvalid          (tx_axis_fifo_tvalid),
      .tx_axis_fifo_tready          (tx_axis_fifo_tready),
      .tx_axis_fifo_tlast           (tx_axis_fifo_tlast),

      // MAC Control Interface
      //------------------------
      .pause_req                    (1'b0),
      .pause_val                    (),

      // GMII Interface
      //-----------------
      .gmii_txd                     (gmii_txd),
      .gmii_tx_en                   (gmii_tx_en),
      .gmii_tx_er                   (gmii_tx_er),
      .gmii_tx_clk                  (gmii_tx_clk),
      .gmii_rxd                     (gmii_rxd),
      .gmii_rx_dv                   (gmii_rx_dv),
      .gmii_rx_er                   (gmii_rx_er),
      .gmii_rx_clk                  (gmii_rx_clk),
      .gmii_col                     (gmii_col),
      .gmii_crs                     (gmii_crs),
      .mii_tx_clk                   (mii_tx_clk),




      // asynchronous reset
      .glbl_rstn                    (glbl_rst_intn),
      .rx_axi_rstn                  (1'b1),
      .tx_axi_rstn                  (1'b1)
   );


endmodule
