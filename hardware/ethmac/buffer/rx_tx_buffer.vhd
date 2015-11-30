--
-- Copyright: 2014, Technical University of Denmark, DTU Compute
-- Author: Luca Pezzarossa (lpez@dtu.dk)
-- License: Simplified BSD License
--
-- RX/TX buffer
--

library ieee;
use ieee.std_logic_1164.all;

entity rx_tx_buffer is
	generic(
		ADDR_WIDTH : natural
	);
	port(
		clk       : in  std_logic;
		rst       : in  std_logic;

		-- OCP IN (slave) for Patmos
		MCmd      : in  std_logic_vector(2 downto 0);
		MAddr     : in  std_logic_vector((ADDR_WIDTH - 1) downto 0);
		MData     : in  std_logic_vector(31 downto 0);
		MByteEn   : in  std_logic_vector(3 downto 0);
		SResp     : out std_logic_vector(1 downto 0);
		SData     : out std_logic_vector(31 downto 0);

		-- Wishbone Slave (for EthMac controller)
		wb_addr_i : in  std_logic_vector((ADDR_WIDTH - 1) downto 0);
		wb_sel_i  : in  std_logic_vector(3 downto 0);
		wb_we_i   : in  std_logic;
		wb_data_o : out std_logic_vector(31 downto 0);
		wb_data_i : in  std_logic_vector(31 downto 0);
		wb_cyc_i  : in  std_logic;
		wb_stb_i  : in  std_logic;
		wb_ack_o  : out std_logic;
		wb_err_o  : out std_logic
	);
end rx_tx_buffer;

architecture rtl of rx_tx_buffer is
	component ocp_to_bram is
		generic(
			DATA_WIDTH : natural;
			ADDR_WIDTH : natural
		);
		port(
			clk        : in  std_logic;
			rst        : in  std_logic;

			-- OCP IN (slave)
			MCmd       : in  std_logic_vector(2 downto 0);
			MAddr      : in  std_logic_vector((ADDR_WIDTH - 1) downto 0);
			MData      : in  std_logic_vector((DATA_WIDTH - 1) downto 0);
			MByteEn    : in  std_logic_vector(3 downto 0);
			SResp      : out std_logic_vector(1 downto 0);
			SData      : out std_logic_vector((DATA_WIDTH - 1) downto 0);

			-- Ram OUT (byte based)
			ram_addr   : out std_logic_vector((ADDR_WIDTH - 1) downto 0);
			ram_data_o : out std_logic_vector((DATA_WIDTH - 1) downto 0);
			ram_we     : out std_logic_vector(3 downto 0);
			ram_data_i : in  std_logic_vector((DATA_WIDTH - 1) downto 0)
		);
	end component;

	component wb_to_bram is
		generic(
			DATA_WIDTH : natural;
			ADDR_WIDTH : natural
		);
		port(
			clk        : in  std_logic;
			rst        : in  std_logic;

			-- Wishbone IN
			wb_addr_i  : in  std_logic_vector((ADDR_WIDTH - 1) downto 0);
			wb_sel_i   : in  std_logic_vector(3 downto 0);
			wb_we_i    : in  std_logic;
			wb_data_o  : out std_logic_vector((DATA_WIDTH - 1) downto 0);
			wb_data_i  : in  std_logic_vector((DATA_WIDTH - 1) downto 0);
			wb_cyc_i   : in  std_logic;
			wb_stb_i   : in  std_logic;
			wb_ack_o   : out std_logic;
			wb_err_o   : out std_logic;

			-- Ram OUT
			ram_addr   : out std_logic_vector((ADDR_WIDTH - 1) downto 0);
			ram_data_o : out std_logic_vector((DATA_WIDTH - 1) downto 0);
			ram_we     : out std_logic;
			ram_data_i : in  std_logic_vector((DATA_WIDTH - 1) downto 0)
		);
	end component;

	component tdp_sc_bram is
		generic(
			DATA_WIDTH : natural;
			ADDR_WIDTH : natural
		);
		port(
			clk      : in  std_logic;
			addr_a   : in  std_logic_vector((ADDR_WIDTH - 1) downto 0);
			addr_b   : in  std_logic_vector((ADDR_WIDTH - 1) downto 0);
			data_a_i : in  std_logic_vector((DATA_WIDTH - 1) downto 0);
			data_b_i : in  std_logic_vector((DATA_WIDTH - 1) downto 0);
			we_a     : in  std_logic;
			we_b     : in  std_logic;
			data_a_o : out std_logic_vector((DATA_WIDTH - 1) downto 0);
			data_b_o : out std_logic_vector((DATA_WIDTH - 1) downto 0)
		);
	end component;

	signal addr_a_int   : std_logic_vector((ADDR_WIDTH - 1) downto 0);
	signal addr_b_int   : std_logic_vector((ADDR_WIDTH - 1) downto 0);
	signal data_a_i_int : std_logic_vector(31 downto 0);
	signal data_b_i_int : std_logic_vector(31 downto 0);
	signal we_a_int     : std_logic_vector(3 downto 0);
	signal we_b_int     : std_logic;
	signal data_a_o_int : std_logic_vector(31 downto 0);
	signal data_b_o_int : std_logic_vector(31 downto 0);

begin 

ocp_to_bram_comp_0 : ocp_to_bram 
    generic map
	(
		DATA_WIDTH => 32,
		ADDR_WIDTH => ADDR_WIDTH
	) port map (
		clk        => clk,
		rst        => rst,

		-- OCP IN (slave)
		MCmd       => MCmd,
		MAddr      => MAddr,
		MData      => MData,
		MByteEn    => MByteEn,
		SResp      => SResp,
		SData      => SData,

		-- Ram OUT (byte based)
		ram_addr   => addr_a_int,
		ram_data_o => data_a_i_int,
		ram_we     => we_a_int,
		ram_data_i => data_a_o_int
	);

wb_to_bram_comp_0 : wb_to_bram
    generic map (
        DATA_WIDTH => 32,
        ADDR_WIDTH => ADDR_WIDTH
    )
    port map (
        clk => clk,
        rst => rst,

        wb_addr_i => wb_addr_i,
	wb_sel_i => wb_sel_i,
	wb_we_i => wb_we_i,
        wb_data_o => wb_data_o,
        wb_data_i => wb_data_i,
        wb_cyc_i => wb_cyc_i,
        wb_stb_i => wb_stb_i,
        wb_ack_o => wb_ack_o,
        wb_err_o => wb_err_o,

        ram_addr => addr_b_int,
        ram_data_o => data_b_i_int, 
        ram_we => we_b_int,
        ram_data_i => data_b_o_int
        ); 

tdp_sc_bram_comp_0 : tdp_sc_bram -- Byte 0 LSB 
    generic map (
        DATA_WIDTH => 8,
        ADDR_WIDTH => ADDR_WIDTH
    )
    port map (
        clk => clk,
        addr_a => addr_a_int,
        addr_b => addr_b_int,
        data_a_i => data_a_i_int(7 downto 0),
        data_b_i => data_b_i_int(7 downto 0),
        we_a => we_a_int(0),
        we_b => we_b_int,
        data_a_o => data_a_o_int(7 downto 0),
        data_b_o => data_b_o_int(7 downto 0)
    );

tdp_sc_bram_comp_1 : tdp_sc_bram -- Byte 1 LSB 
    generic map (
        DATA_WIDTH => 8,
        ADDR_WIDTH => ADDR_WIDTH
    )
    port map (
        clk => clk,
        addr_a => addr_a_int,
        addr_b => addr_b_int,
        data_a_i => data_a_i_int(15 downto 8),
        data_b_i => data_b_i_int(15 downto 8),
        we_a => we_a_int(1),
        we_b => we_b_int,
        data_a_o => data_a_o_int(15 downto 8),
        data_b_o => data_b_o_int(15 downto 8)
    );

tdp_sc_bram_comp_2 : tdp_sc_bram -- Byte 2 LSB 
    generic map (
        DATA_WIDTH => 8,
        ADDR_WIDTH => ADDR_WIDTH
    )
    port map (
        clk => clk,
        addr_a => addr_a_int,
        addr_b => addr_b_int,
        data_a_i => data_a_i_int(23 downto 16),
        data_b_i => data_b_i_int(23 downto 16),
        we_a => we_a_int(2),
        we_b => we_b_int,
        data_a_o => data_a_o_int(23 downto 16),
        data_b_o => data_b_o_int(23 downto 16)
    );

tdp_sc_bram_comp_3 : tdp_sc_bram -- Byte 3 LSB 
    generic map (
        DATA_WIDTH => 8,
        ADDR_WIDTH => ADDR_WIDTH
    )
    port map (
        clk => clk,
        addr_a => addr_a_int,
        addr_b => addr_b_int,
        data_a_i => data_a_i_int(31 downto 24),
        data_b_i => data_b_i_int(31 downto 24),
        we_a => we_a_int(3),
        we_b => we_b_int,
        data_a_o => data_a_o_int(31 downto 24),
        data_b_o => data_b_o_int(31 downto 24)
    );

end rtl;
