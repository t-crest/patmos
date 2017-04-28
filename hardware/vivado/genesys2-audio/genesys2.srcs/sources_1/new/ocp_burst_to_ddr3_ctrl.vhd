----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 01/11/2017 05:01:18 PM
-- Design Name: 
-- Module Name: ocp_burst_to_ddr3_ctrl - Behavioral
-- Project Name: 
-- Target Devices: 
-- Tool Versions: 
-- Description: 
-- 
-- Dependencies: 
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments: This code is intentionally not parametrized.
-- 
----------------------------------------------------------------------------------


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity ocp_burst_to_ddr3_ctrl is
	port(
		-- Common
		clk               : in  std_logic;
		rst               : in  std_logic;

		-- OCPburst IN (slave)
		MCmd              : in  std_logic_vector(2 downto 0);
		MAddr             : in  std_logic_vector(31 downto 0);
		MData             : in  std_logic_vector(31 downto 0);
		MDataValid        : in  std_logic;
		MDataByteEn       : in  std_logic_vector(3 downto 0);

		SResp             : out std_logic_vector(1 downto 0);
		SData             : out std_logic_vector(31 downto 0);
		SCmdAccept        : out std_logic;
		SDataAccept       : out std_logic;

		-- Xilinx interface
		app_addr          : out std_logic_vector(28 downto 0); --
		app_cmd           : out std_logic_vector(2 downto 0); --
		app_en            : out std_logic;
		app_wdf_data      : out std_logic_vector(255 downto 0);
		app_wdf_end       : out std_logic;
		app_wdf_mask      : out std_logic_vector(31 downto 0);
		app_wdf_wren      : out std_logic;
		app_rd_data       : in  std_logic_vector(255 downto 0); --
		app_rd_data_end   : in  std_logic; --
		app_rd_data_valid : in  std_logic;
		app_rdy           : in  std_logic;
		app_wdf_rdy       : in  std_logic
	);
end ocp_burst_to_ddr3_ctrl;

--    output[2:0] io_memBridgePins_M_Cmd,
--    output[31:0] io_memBridgePins_M_Addr,
--    output[31:0] io_memBridgePins_M_Data,
--    output io_memBridgePins_M_DataValid,
--    output[3:0] io_memBridgePins_M_DataByteEn,
--    input [1:0] io_memBridgePins_S_Resp,
--    input [31:0] io_memBridgePins_S_Data,
--    input  io_memBridgePins_S_CmdAccept,
--    input  io_memBridgePins_S_DataAccept,

architecture Behavioral of ocp_burst_to_ddr3_ctrl is
	type state_type is (IDLE, WRITE_BUFFER, WRITE_COMMAND, WRITE_DATA, READ_COMMAND, READ_WAIT, READ_BUFFER);
	signal state, next_state : state_type;

	-- MDataByteEn related signals
	signal MDataByteEn_buffer, next_MDataByteEn_buffer : std_logic_vector(31 downto 0);
	signal MDataByteEn_load                            : std_logic;
	signal MDataByteEn_sel                             : unsigned(2 downto 0); -- 3 bit array to select 

	-- MData related signals
	signal MData_buffer, next_MData_buffer : std_logic_vector(255 downto 0);
	signal MData_load                      : std_logic;
	signal MData_sel                       : unsigned(2 downto 0); -- 3 bit array to select 

	-- MAddr related signals
	signal MAddr_buffer, next_MAddr_buffer : std_logic_vector(31 downto 0);
	signal MAddr_load                      : std_logic;

	-- MData related signals
	signal SData_buffer, next_SData_buffer : std_logic_vector(255 downto 0);
	signal SData_load                      : std_logic;
	signal SData_sel                       : unsigned(2 downto 0); -- 3 bit array to select 

	-- Word counter
	signal count    : unsigned(2 downto 0); -- 3 bit array that counts
	signal count_en, count_last_item: std_logic;

begin

		--Registers
	process(clk, rst)
	begin
		if rising_edge(clk) then
			if rst = '1' then
				state              <= IDLE;
				MDataByteEn_buffer <= (others => '1');
				MData_buffer       <= (others => '0');
				MAddr_buffer       <= (others => '0');
				SData_buffer       <= (others => '0');
			else
				state              <= next_state;
				MDataByteEn_buffer <= next_MDataByteEn_buffer;
				MData_buffer       <= next_MData_buffer;
				MAddr_buffer       <= next_MAddr_buffer;
				SData_buffer       <= next_SData_buffer;
			end if;
		end if;
	end process;

	--Counter
	process(clk, rst)
	begin
		if rising_edge(clk) then
			if rst = '1' then
			count <= (others => '0');
			elsif (count_en = '1') then
				count <= count + 1;
			end if;
		end if;
	end process;

	-- This is the mux that deflects the (SData)
	process(SData_sel, SData_buffer)
	begin
		for I in 0 to 7 loop
			if (to_integer(SData_sel) = I) then
				SData <= SData_buffer(((I + 1) * 32 - 1) downto I * 32);
			end if;
		end loop;
	end process;

	-- This is the mux that stores the (MDataByteEn)
	process(MDataByteEn_sel, MDataByteEn_load, MDataByteEn_buffer, MDataByteEn)
	begin
		next_MDataByteEn_buffer <= MDataByteEn_buffer;
		for I in 0 to 7 loop
			if (to_integer(MDataByteEn_sel) = I) and (MDataByteEn_load = '1') then
				next_MDataByteEn_buffer((4 * (I + 1)) - 1 downto 4 * I) <= not (MDataByteEn); -- NB: filling directly form the ocp interface 
			end if;
		end loop;
	end process;

	-- This is the mux that stores the (MDataByteEn)
	process(MData_sel, MData_load, MData_buffer, MData)
	begin
		next_MData_buffer <= MData_buffer;
		for I in 0 to 7 loop
			if (to_integer(MData_sel) = I) and (MData_load = '1') then
				next_MData_buffer(((I + 1) * 32 - 1) downto I * 32) <= MData; -- NB: filling directly form the ocp interface 
			end if;
		end loop;
	end process;

	-- This is the mux that stores the (MAddr)
	next_MAddr_buffer <= MAddr when (MAddr_load = '1') else MAddr_buffer;

	-- This is the mux that stores the (SData)
	next_SData_buffer <= app_rd_data when (SData_load = '1') else SData_buffer;

	count_last_item <= '1' when (count = "111") else '0';
    
	-- Output assignments
	app_wdf_mask <= MDataByteEn_buffer;
	app_wdf_data <= MData_buffer;

	-- Address assigment
	app_addr(28 downto 3) <= MAddr_buffer(30 downto 5);
	app_addr(2 downto 0)  <= (others => '0');

	-- Mux selection assigment
	MData_sel       <= count;
	MDataByteEn_sel <= count;
	SData_sel       <= count;

	process(state, MCmd, MDataValid, count_last_item, app_rdy, app_wdf_rdy, app_rd_data_valid)
	begin
		next_state <= state;

		MAddr_load       <= '0';
		MData_load       <= '0';
		MDataByteEn_load <= '0';
		SData_load       <= '0';

		count_en <= '0';

		SCmdAccept  <= '0';
		SDataAccept <= '0';

		app_cmd      <= "001";
		app_en       <= '0';
		app_wdf_end  <= '0';
		app_wdf_wren <= '0';

		SResp <= "00";

		case state is
			when IDLE =>
				MAddr_load <= '1';
				--Next state
				if (MCmd = "001") then  -- write
					--Accepting the command
					SCmdAccept <= '1';
					--Accepting the data if valid
					if (MDataValid = '1') then
						SDataAccept      <= '1';
						MData_load       <= '1';
						MDataByteEn_load <= '1';
						count_en         <= '1';
					end if;
					next_state <= WRITE_BUFFER;
				elsif (MCmd = "010") then
					SCmdAccept <= '1';  -- read
					next_state <= READ_COMMAND;
				end if;

			when WRITE_BUFFER =>
				--Accepting the data if valid
				if (MDataValid = '1') then
					SDataAccept      <= '1';
					MData_load       <= '1';
					MDataByteEn_load <= '1';
					count_en         <= '1';
				end if;
				--Next state
				if (count_last_item = '1') and (MDataValid = '1') then --exit when the counter is full
					next_state <= WRITE_COMMAND;
				end if;

			when WRITE_COMMAND =>
				app_cmd <= "000";
				app_en  <= '1';
				--Next state
				if (app_rdy = '1') then
					next_state <= WRITE_DATA;
				end if;

			when WRITE_DATA =>
				app_wdf_end  <= '1';
				app_wdf_wren <= '1';
				--Next state
				if (app_wdf_rdy = '1') then
					SResp      <= "01"; --Sending the DVA
					next_state <= IDLE;
				end if;

			when READ_COMMAND =>
				app_cmd <= "001";
				app_en  <= '1';

				--Next state
				if (app_rd_data_valid = '1') and (app_rdy = '1') then
					--SResp <= "01";
					SData_load <= '1';
					next_state <= READ_BUFFER;
				elsif (app_rd_data_valid = '0') and (app_rdy = '1') then
					next_state <= READ_WAIT;
				end if;

			when READ_WAIT =>
				--Next state
				if (app_rd_data_valid = '1') then
					--SResp <= "01";
					SData_load <= '1';
					next_state <= READ_BUFFER;
				end if;

			when READ_BUFFER =>
				SResp    <= "01";
				count_en <= '1';
				--Next state
				if (count_last_item = '1') then --exit when the counter is full
					next_state <= IDLE;
				end if;

		end case;
	end process;

end Behavioral;
