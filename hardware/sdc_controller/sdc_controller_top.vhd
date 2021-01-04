library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity sdc_controller_top is
	generic(
		BUFF_ADDR_WIDTH : natural -- 
	);
	port(
		clk           : in  	std_logic;
		rst           : in  	std_logic;

		-- OCP IN (slave) for Patmos
		M_Cmd          	: in  	std_logic_vector(2 downto 0);
		M_Addr         	: in  	std_logic_vector(BUFF_ADDR_WIDTH-1 downto 0);
		M_Data         	: in  	std_logic_vector(31 downto 0);
		M_ByteEn       	: in  	std_logic_vector(3 downto 0);
		S_Resp         	: out 	std_logic_vector(1 downto 0);
		S_Data         	: out 	std_logic_vector(31 downto 0);

		-- sdcard port
		sd_dat_dat 		: in 	std_logic_vector(3 downto 0); 	-- data in from sd card
		sd_dat_out 		: out 	std_logic_vector(3 downto 0);	-- data out from sd card
		sd_dat_oe  		: out 	std_logic; -- SD Card tristate Data Output enable
		sd_cmd_dat 		: in 	std_logic;  -- command in from sdcard
		sd_cmd_out 		: out 	std_logic; -- command out to sdcard
		sd_cmd_oe  		: out 	std_logic; -- SD Card tristate CMD Output enable
		sd_clk_o_pad 	: out 	std_logic; -- clk for sdcard
		
		-- interrupts
		int_cmd : out std_logic;
		int_data : out std_logic
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
			--card_detect 	: in 	std_logic;
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
	
	component rx_tx_buffer is
		generic(
			ADDR_WIDTH : natural
		);
		port(
			clk       : in  	std_logic;
			rst       : in  	std_logic;
	
			-- OCP IN (slave) for Patmos
			MCmd      : in  	std_logic_vector(2 downto 0);
			MAddr     : in  	std_logic_vector(ADDR_WIDTH-1 downto 0);
			MData     : in  	std_logic_vector(31 downto 0);
			MByteEn   : in  	std_logic_vector(3 downto 0);
			SResp     : out 	std_logic_vector(1 downto 0);
			SData     : out 	std_logic_vector(31 downto 0);
	
			-- wishbone slave
			wb_addr_i : in  	std_logic_vector(ADDR_WIDTH-1 downto 0);
			wb_sel_i  : in  	std_logic_vector(3 downto 0);
			wb_we_i   : in  	std_logic;
			wb_data_o : out 	std_logic_vector(31 downto 0);
			wb_data_i : in  	std_logic_vector(31 downto 0);
			wb_cyc_i  : in  	std_logic;
			wb_stb_i  : in  	std_logic;
			wb_ack_o  : out 	std_logic;
			wb_err_o  : out 	std_logic
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

	-- OCP signals for buffer
	signal M_Cmd_b                : std_logic_vector(2 downto 0);
	signal M_Cmd_r                : std_logic_vector(2 downto 0);
	signal S_Resp_b               : std_logic_vector(1 downto 0);
	signal S_Data_b               : std_logic_vector(31 downto 0);
	signal next_S_Resp_r, S_Resp_r : std_logic_vector(1 downto 0);
	signal next_S_Data_r, S_Data_r : std_logic_vector(31 downto 0);
	signal next_mux_sel, mux_sel : std_logic;

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

begin

	M_Cmd_b <= "000" when M_Addr(M_Addr'length-1) = '1' else M_Cmd;	--control buffer, if MSB is 1
	M_Cmd_r <= "000" when M_Addr(M_Addr'length-1) = '0' else M_Cmd;	--control registers, if MSB is 0
	S_Resp  <= S_Resp_r when (mux_sel = '1') else S_Resp_b;
	S_Data  <= S_Data_r when (mux_sel = '1') else S_Data_b;
	
	--Control mux
	process(wb_r_ack_i, M_Cmd_r, M_Addr, M_Data, wb_r_data_o, wb_r_we_o, wb_r_stb_o, wb_r_cyc_o, wb_r_addr_o, wb_r_data_i)
	begin
		if (wb_r_ack_i = '0') then
			next_S_Resp_r <= "00";
			next_S_Data_r <= (others  => '0');
			next_mux_sel <= '0';
			case M_Cmd_r is
				when "001" =>           -- write
					next_wb_r_we_o              <= '1';
					next_wb_r_stb_o             <= '1';
					next_wb_r_cyc_o             <= '1';
					next_wb_r_addr_o			<= M_Addr(7 downto 0);
					next_wb_r_data_o           	<= M_Data;
				when "010" =>           -- read
					next_wb_r_we_o              <= '0';
					next_wb_r_stb_o             <= '1';
					next_wb_r_cyc_o             <= '1';
					next_wb_r_addr_o 			<= M_Addr(7 downto 0);
					next_wb_r_data_o            <= wb_r_data_o;
				when others =>          -- idle
					next_wb_r_we_o              <= wb_r_we_o;
					next_wb_r_stb_o             <= wb_r_stb_o;
					next_wb_r_cyc_o             <= wb_r_cyc_o;
					next_wb_r_addr_o 			<= wb_r_addr_o;
					next_wb_r_data_o            <= wb_r_data_o;
			end case;
		else
			next_wb_r_we_o               <= '0';
			next_wb_r_stb_o              <= '0';
			next_wb_r_cyc_o              <= '0';
			next_wb_r_addr_o			 <= wb_r_addr_o;
			next_wb_r_data_o             <= wb_r_data_o;
			next_S_Resp_r                <= "01";
			next_S_Data_r                <= wb_r_data_i;
			next_mux_sel                 <= '1'; --put out the data from the registers
		end if;
	end process;

	--Register
	process(clk, rst)
	begin
		if rst = '1' then
			wb_r_we_o                <= '0';
			wb_r_stb_o               <= '0';
			wb_r_cyc_o               <= '0';
			wb_r_addr_o				 <= (others => '0');
			wb_r_data_o              <= (others => '0');
			S_Resp_r                 <= (others => '0');
			S_Data_r                 <= (others => '0');
			mux_sel                  <= '0';
		elsif rising_edge(clk) then
			wb_r_we_o                <= next_wb_r_we_o;
			wb_r_stb_o               <= next_wb_r_stb_o;
			wb_r_cyc_o               <= next_wb_r_cyc_o;
			wb_r_addr_o				 <= next_wb_r_addr_o;
			wb_r_data_o              <= next_wb_r_data_o;
			S_Resp_r                 <= next_S_Resp_r;
			S_Data_r                 <= next_S_Data_r;
			mux_sel                  <= next_mux_sel;
		end if;
	end process;


	sdc_controller_comp_0 : sdc_controller
		port map(
			-- wishbone common
			wb_clk_i 		=> clk,
			wb_rst_i 		=> rst,
			-- wishbone slave
			wb_dat_i 		=> wb_r_data_o,
			wb_dat_o 		=> wb_r_data_i,
			wb_adr_i    	=> wb_r_addr_o,
			wb_sel_i		=> "1111", -- no byte enable
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
	
	rx_tx_buffer_comp_0 : rx_tx_buffer
		generic map(
			ADDR_WIDTH => BUFF_ADDR_WIDTH
		)
		port map(
			clk       => clk,
			rst       => rst,

			-- OCP IN (slave) for Patmos
			MCmd      => M_Cmd_b,
			MAddr     => M_Addr(BUFF_ADDR_WIDTH-1 downto 0),
			MData     => M_Data,
			MByteEn   => M_ByteEn,
			SResp     => S_Resp_b,
			SData     => S_Data_b,

			-- wishbone slave
			wb_addr_i => wb_b_addr_i(BUFF_ADDR_WIDTH-1 downto 0),
			wb_sel_i  => wb_b_sel_i,
			wb_we_i   => wb_b_we_i,
			wb_data_o => wb_b_data_o,
			wb_data_i => wb_b_data_i,
			wb_cyc_i  => wb_b_cyc_i,
			wb_stb_i  => wb_b_stb_i,
			wb_ack_o  => wb_b_ack_o,
			wb_err_o  => open
		);

end architecture;