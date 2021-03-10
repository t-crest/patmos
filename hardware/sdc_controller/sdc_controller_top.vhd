--
-- Authors: Martin Schwendinger, Philipp Birkl ({martin.schwendinger,philipp.birkl}@student.tuwien.ac.at)
-- License: GNU Lesser General Public License
--
-- SD card controller top-level file for the SD card controller by Marek Czerski 
-- translating OCPcore to wishbone, OCPcore/wishbone to bram and bram providing buffer itself.
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity sdc_controller_top is
	generic(
		ADDR_WIDTH 		: 		natural 	:= 14	-- determines the size of rx-tx buffer, should be at least 8; MSB is toggle
	);
	port(
		clk           	: in  	std_logic;
		rst           	: in  	std_logic;
		-- OCP IN (slave) for Patmos
		M_Cmd          	: in  	std_logic_vector(2 downto 0);
		M_Addr         	: in  	std_logic_vector(ADDR_WIDTH-1 downto 0);
		M_Data         	: in  	std_logic_vector(31 downto 0);
		M_ByteEn       	: in  	std_logic_vector(3 downto 0);
		S_Resp         	: out 	std_logic_vector(1 downto 0);
		S_Data         	: out 	std_logic_vector(31 downto 0);
		-- SD port
		sd_dat_dat 		: in 	std_logic_vector(3 downto 0); 	-- data in from sd card
		sd_dat_out 		: out 	std_logic_vector(3 downto 0);	-- data out from sd card
		sd_dat_oe  		: out 	std_logic; 						-- sdcard tristate data output enable
		sd_cmd_dat 		: in 	std_logic;  					-- command in from sdcard
		sd_cmd_out 		: out 	std_logic; 						-- command out to sdcard
		sd_cmd_oe  		: out 	std_logic; 						-- sdcard tristate cmd output enable
		sd_clk_o_pad 	: out 	std_logic; 						-- clk for sdcard
		-- interrupts
		int_cmd 		: out 	std_logic;
		int_data 		: out 	std_logic
	);
end entity;

architecture arch of sdc_controller_top is

	component sdc_controller is
		port(
			-- wishbone common
			wb_clk_i 		: in 	std_logic;
			wb_rst_i 		: in 	std_logic;
			wb_dat_i 		: in 	std_logic_vector(31 downto 0);
			wb_dat_o 		: out 	std_logic_vector(31 downto 0);
			-- wishbone slave
			wb_adr_i    	: in 	std_logic_vector(7 downto 0);
			wb_sel_i		: in 	std_logic_vector(3 downto 0);
			wb_we_i			: in 	std_logic;
			wb_cyc_i		: in 	std_logic;
			wb_stb_i		: in 	std_logic;
			wb_ack_o		: out 	std_logic;
			-- wishbone master
			m_wb_adr_o		: out 	std_logic_vector(31 downto 0);
			m_wb_sel_o		: out 	std_logic_vector(3 downto 0);
			m_wb_we_o		: out 	std_logic;
			m_wb_dat_i		: in 	std_logic_vector(31 downto 0);
			m_wb_dat_o		: out 	std_logic_vector(31 downto 0);
			m_wb_cyc_o		: out 	std_logic;
			m_wb_stb_o		: out 	std_logic;
			m_wb_ack_i		: in 	std_logic;
			m_wb_cti_o		: out 	std_logic_vector(2 downto 0);	-- unused
			m_wb_bte_o		: out 	std_logic_vector(1 downto 0);	-- unused
			-- SD port
			sd_dat_dat_i	: in	std_logic_vector(3 downto 0);
			sd_dat_out_o	: out 	std_logic_vector(3 downto 0);
			sd_dat_oe_o		: out 	std_logic;
			sd_cmd_dat_i	: in 	std_logic;
			sd_cmd_out_o	: out 	std_logic;
			sd_cmd_oe_o		: out 	std_logic;
			sd_clk_o_pad	: out 	std_logic;
			sd_clk_i_pad    : in 	std_logic;
			-- interrupts
			int_cmd 		: out 	std_logic;
			int_data		: out 	std_logic
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
	
	-- wishbone signals for registers
	signal next_wb_r_addr_o, wb_r_addr_o : std_logic_vector(7 downto 0);
	signal next_wb_r_data_o, wb_r_data_o : std_logic_vector(31 downto 0);
	signal wb_r_data_i                   : std_logic_vector(31 downto 0);
	signal wb_r_err_i                    : std_logic;
	signal next_wb_r_we_o, wb_r_we_o     : std_logic;
	signal next_wb_r_stb_o, wb_r_stb_o   : std_logic;
	signal wb_r_ack_i                    : std_logic;
	signal next_wb_r_cyc_o, wb_r_cyc_o   : std_logic;
	signal next_wb_r_sel_o, wb_r_sel_o	 : std_logic_vector(3 downto 0);

	-- OCP signals for buffer
	signal M_Cmd_b                	: std_logic_vector(2 downto 0);
	signal M_Cmd_r                	: std_logic_vector(2 downto 0);
	signal S_Resp_b               	: std_logic_vector(1 downto 0);
	signal S_Data_b               	: std_logic_vector(31 downto 0);
	signal next_S_Resp_r, S_Resp_r 	: std_logic_vector(1 downto 0);
	signal next_S_Data_r, S_Data_r 	: std_logic_vector(31 downto 0);
	signal next_mux_sel, mux_sel 	: std_logic;

	-- wishbone signals for buffer
	signal wb_b_addr_i : std_logic_vector(31 downto 0);
	signal wb_b_sel_i  : std_logic_vector(3 downto 0);
	signal wb_b_we_i   : std_logic;
	signal wb_b_data_o : std_logic_vector(31 downto 0);
	signal wb_b_data_i : std_logic_vector(31 downto 0);
	signal wb_b_cyc_i  : std_logic;
	signal wb_b_stb_i  : std_logic;
	signal wb_b_ack_o  : std_logic;
	signal wb_b_err_o  : std_logic;

	-- ocp to bram
	signal bram_addr_ocp 	: std_logic_vector(ADDR_WIDTH-4 downto 0);
	signal bram_data_ocp_i 	: std_logic_vector(31 downto 0);
	signal bram_data_ocp_o 	: std_logic_vector(31 downto 0);
	signal bram_we_ocp 		: std_logic_vector(3 downto 0);
	signal next_S_Resp_b 	: std_logic_vector(1 downto 0);

	-- wishbone to bram
	signal bram_addr_wb 		: std_logic_vector(ADDR_WIDTH-4 downto 0);
	signal bram_data_wb_i 		: std_logic_vector(31 downto 0);
	signal bram_data_wb_o 		: std_logic_vector(31 downto 0);
	signal bram_we_wb 			: std_logic_vector(3 downto 0);
	type wb_bram_state_type 	is (IDLE, WRITE, READ, ACK);
	signal wb_bram_state 		: wb_bram_state_type;
	signal next_wb_bram_state 	: wb_bram_state_type;

begin

	-- differ between sdc registers and buffer
	M_Cmd_r <= M_Cmd when M_Addr(M_Addr'length-1) = '0' else (others => '0');	--control registers, 	if MSB is 0
	M_Cmd_b <= M_Cmd when M_Addr(M_Addr'length-1) = '1' else (others => '0');	--control buffer, 		if MSB is 1
	S_Resp  <= S_Resp_r when (mux_sel = '1') else S_Resp_b;
	S_Data  <= S_Data_r when (mux_sel = '1') else S_Data_b;
	
	-- control mux
	process(wb_r_ack_i, M_Cmd_r, M_Addr, M_Data, M_ByteEn, wb_r_data_o, wb_r_we_o, wb_r_stb_o, wb_r_cyc_o, wb_r_addr_o, wb_r_data_i, wb_r_sel_o)
	begin
		if (wb_r_ack_i = '0') then
			next_S_Resp_r <= "00";
			next_S_Data_r <= (others  => '0');
			next_mux_sel <= '0';
			case M_Cmd_r is
				when "001" =>           		-- write
					next_wb_r_we_o              <= '1';
					next_wb_r_stb_o             <= '1';
					next_wb_r_cyc_o             <= '1';
					next_wb_r_addr_o	        <= M_Addr(9 downto 2);
					next_wb_r_sel_o		        <= M_ByteEn;
					next_wb_r_data_o            <= M_Data;
				when "010" =>           		-- read
					next_wb_r_we_o              <= '0';
					next_wb_r_stb_o             <= '1';
					next_wb_r_cyc_o             <= '1';
					next_wb_r_addr_o 			<= M_Addr(9 downto 2);
					next_wb_r_sel_o				<= M_ByteEn;
					next_wb_r_data_o            <= wb_r_data_o;
				when others =>          		-- idle
					next_wb_r_we_o              <= wb_r_we_o;
					next_wb_r_stb_o             <= wb_r_stb_o;
					next_wb_r_cyc_o             <= wb_r_cyc_o;
					next_wb_r_addr_o 			<= wb_r_addr_o;
					next_wb_r_sel_o				<= wb_r_sel_o;
					next_wb_r_data_o            <= wb_r_data_o;
			end case;
		else
			next_wb_r_we_o              <= '0';
			next_wb_r_stb_o             <= '0';
			next_wb_r_cyc_o             <= '0';
			next_wb_r_addr_o			<= wb_r_addr_o;
			next_wb_r_sel_o				<= wb_r_sel_o;
			next_wb_r_data_o            <= wb_r_data_o;
			next_S_Resp_r               <= "01";
			next_S_Data_r               <= wb_r_data_i;
			next_mux_sel                <= '1'; 		-- put out the data from the registers
		end if;
	end process;

	-- register
	process(clk, rst)
	begin
		if rst = '1' then
			wb_r_we_o               <= '0';
			wb_r_stb_o              <= '0';
			wb_r_cyc_o              <= '0';
			wb_r_addr_o				<= (others => '0');
			wb_r_sel_o				<= (others => '0');
			wb_r_data_o             <= (others => '0');
			S_Resp_r                <= (others => '0');
			S_Data_r                <= (others => '0');
			mux_sel                 <= '0';
		elsif rising_edge(clk) then
			wb_r_we_o               <= next_wb_r_we_o;
			wb_r_stb_o              <= next_wb_r_stb_o;
			wb_r_cyc_o              <= next_wb_r_cyc_o;
			wb_r_addr_o				<= next_wb_r_addr_o;
			wb_r_sel_o				<= next_wb_r_sel_o;
			wb_r_data_o             <= next_wb_r_data_o;
			S_Resp_r                <= next_S_Resp_r;
			S_Data_r                <= next_S_Data_r;
			mux_sel                 <= next_mux_sel;
		end if;
	end process;

	-- sdc controller instantiation
	sdc_controller_comp_0 : sdc_controller
		port map(
			-- wishbone common
			wb_clk_i 		=> clk,
			wb_rst_i 		=> rst,
			-- wishbone slave
			wb_dat_i 		=> wb_r_data_o,
			wb_dat_o 		=> wb_r_data_i,
			wb_adr_i    	=> wb_r_addr_o,
			wb_sel_i		=> wb_r_sel_o,
			wb_we_i			=> wb_r_we_o,
			wb_cyc_i		=> wb_r_cyc_o,
			wb_stb_i		=> wb_r_stb_o,
			wb_ack_o		=> wb_r_ack_i,
			-- wishbone master
			m_wb_adr_o		=> wb_b_addr_i,
			m_wb_sel_o		=> wb_b_sel_i,
			m_wb_we_o		=> wb_b_we_i,
			m_wb_dat_i		=> wb_b_data_o,
			m_wb_dat_o		=> wb_b_data_i,
			m_wb_cyc_o		=> wb_b_cyc_i,
			m_wb_stb_o		=> wb_b_stb_i,
			m_wb_ack_i		=> wb_b_ack_o,
			m_wb_cti_o		=> open,
			m_wb_bte_o		=> open,
			-- SD port
			sd_dat_dat_i	=> sd_dat_dat,
			sd_dat_out_o	=> sd_dat_out,
			sd_dat_oe_o		=> sd_dat_oe,
			sd_cmd_dat_i	=> sd_cmd_dat,
			sd_cmd_out_o	=> sd_cmd_out,
			sd_cmd_oe_o		=> sd_cmd_oe,
			sd_clk_o_pad	=> sd_clk_o_pad,
			sd_clk_i_pad    => clk,
			-- interrupts
			int_cmd 		=> int_cmd,
			int_data		=> int_data
		);

	-- OCP to bram
	bram_addr_ocp <= std_logic_vector(resize(unsigned(M_Addr(M_Addr'length-1 downto 2)), ADDR_WIDTH-3)); -- tanslation from byte to word based address
	bram_data_ocp_i <= M_Data;
	S_Data_b <= bram_data_ocp_o;
	process(M_Cmd_b, M_ByteEn)
	begin 
		case M_Cmd_b is
			when "001" =>       -- write
				bram_we_ocp     <= M_ByteEn;
				next_S_Resp_b 	<= "01";
			when "010" =>       -- read
				bram_we_ocp     <= (others => '0');
				next_S_Resp_b 	<= "01";
			when others =>      -- idle
				bram_we_ocp     <= (others => '0');
				next_S_Resp_b 	<= "00";
		end case;
	end process;

	-- wishbone to bram
	bram_addr_wb   <= std_logic_vector(resize(unsigned(wb_b_addr_i(wb_b_addr_i'length-1 downto 2)), ADDR_WIDTH-3));	--tanslation from byte to word based address
	bram_data_wb_i <= wb_b_data_i;
	wb_b_data_o  <= bram_data_wb_o;
	process(wb_bram_state, wb_b_stb_i, wb_b_cyc_i, wb_b_we_i)
	begin
		bram_we_wb     <= (others => '0');
		wb_b_ack_o     <= '0';
		next_wb_bram_state <= wb_bram_state;
		case wb_bram_state is
			when IDLE =>
				if (wb_b_stb_i = '1') and (wb_b_cyc_i = '1') then
					if wb_b_we_i = '0' then
						next_wb_bram_state <= READ;
					else
						next_wb_bram_state <= WRITE;
					end if;
				else
					next_wb_bram_state <= IDLE;
				end if;
			when WRITE =>
				bram_we_wb     <= (others => '1');
				next_wb_bram_state <= ACK;
			when READ =>
				next_wb_bram_state <= ACK;
			when ACK =>
				wb_b_ack_o   <= '1';
				next_wb_bram_state <= IDLE;
		end case;
	end process;

	-- register OCP/wishbone to bram
	process(clk, rst)
	begin
		if rst = '1' then
			S_Resp_b <= (others => '0');
			wb_bram_state <= IDLE;
		elsif rising_edge(clk) then
			S_Resp_b <= next_S_Resp_b;
			wb_bram_state <= next_wb_bram_state;
		end if;
	end process;

	-- 4x bram
	gen_tdp_sc_bram :
	for i in 0 to 3 generate tdp_sc_bram_comp_X : tdp_sc_bram 
		generic map (
			DATA_WIDTH => 8,
			ADDR_WIDTH => ADDR_WIDTH-3
		)
		port map (
			clk 		=> clk,
			addr_a 		=> bram_addr_ocp,
			addr_b 		=> bram_addr_wb,
			data_a_i 	=> bram_data_ocp_i(8*(i+1)-1 downto 8*i),
			data_b_i 	=> bram_data_wb_i(8*(i+1)-1 downto 8*i),
			we_a 		=> bram_we_ocp(i),
			we_b 		=> bram_we_wb(i),
			data_a_o 	=> bram_data_ocp_o(8*(i+1)-1 downto 8*i),
			data_b_o 	=> bram_data_wb_o(8*(i+1)-1 downto 8*i)
		);
	end generate;

end architecture;
