--
-- Copyright: 2014, Technical University of Denmark, DTU Compute
-- Author: Luca Pezzarossa (lpez@dtu.dk)
-- License: Simplified BSD License
--
-- Actuators and propdrive bridge
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity Actuators_PropDrive is
	generic(
		OCP_DATA_WIDTH : natural := 32;
		OCP_ADDR_WIDTH : natural := 16;
		ACTUATOR_NUMBER : natural := 4;
	   PROPDRIVE_NUMBER : natural := 4
	);
	port(
		clk        : in  std_logic;
		reset        : in  std_logic;

		-- OCP IN (slave)
		MCmd       : in  std_logic_vector(2 downto 0);
		MAddr      : in  std_logic_vector(OCP_ADDR_WIDTH - 1 downto 0);
		MData      : in  std_logic_vector(OCP_DATA_WIDTH - 1 downto 0);
		MByteEn    : in  std_logic_vector(3 downto 0);
		SResp      : out std_logic_vector(1 downto 0);
		SData      : out std_logic_vector(OCP_DATA_WIDTH - 1 downto 0);

		-- Actuator and propdrive OUT
		actuator_out_port  : out   std_logic_vector(ACTUATOR_NUMBER-1 downto 0);
		propdrive_out_port  : out   std_logic_vector(PROPDRIVE_NUMBER-1 downto 0)
	);
end Actuators_PropDrive;

architecture rtl of Actuators_PropDrive is

	component Actuator is
	port(
	  out_port  : out   std_logic;
	  address : in    std_logic_vector(1 downto 0);
	  chipselect : in    std_logic;
	  clk : in    std_logic;
	  reset_n : in    std_logic;
	  write_n : in    std_logic;
	  writedata : in   std_logic_vector(31 downto 0)
	);
	end component;

	component PropDrive is
	port(
	  out_port  : out   std_logic;
	  address : in    std_logic_vector(1 downto 0);
	  chipselect : in    std_logic;
	  clk : in    std_logic;
	  reset_n : in    std_logic;
	  write_n : in    std_logic;
	  writedata : in   std_logic_vector(31 downto 0);
	  readdata : out    std_logic_vector(31 downto 0)
	);
	end component;

	signal next_SResp : std_logic_vector(1 downto 0);
	signal next_SData, SData_int : std_logic_vector(31 downto 0);
	
	signal reset_n : std_logic;
	
	type actuator_2_bit_array_type is array (ACTUATOR_NUMBER-1 downto 0) of std_logic_vector(1 downto 0);
	type actuator_32_bit_array_type is array (ACTUATOR_NUMBER-1 downto 0) of std_logic_vector(31 downto 0);
	signal actuator_address :    actuator_2_bit_array_type;
	signal actuator_chipselect :    std_logic_vector(ACTUATOR_NUMBER-1 downto 0);
	signal actuator_write_n :    std_logic_vector(ACTUATOR_NUMBER-1 downto 0);
	signal actuator_writedata :   actuator_32_bit_array_type;

   type propdrive_2_bit_array_type is array (PROPDRIVE_NUMBER-1 downto 0) of std_logic_vector(1 downto 0);
	type propdrive_32_bit_array_type is array (PROPDRIVE_NUMBER-1 downto 0) of std_logic_vector(31 downto 0);
	signal propdrive_address :    propdrive_2_bit_array_type;
	signal propdrive_chipselect :    std_logic_vector(PROPDRIVE_NUMBER-1 downto 0);
	signal propdrive_write_n :    std_logic_vector(PROPDRIVE_NUMBER-1 downto 0);
	signal propdrive_writedata :   propdrive_32_bit_array_type;
	signal propdrive_readdata :    propdrive_32_bit_array_type;
	
	constant ACTUATOR_ADDR_BASE  :  integer := 0;
	constant PROPDRIVE_ADDR_BASE  :  integer := 16;
	
begin
   reset_n <= not reset;
	SData <= SData_int;

	propdrive_write_n <= (others => '0');
	--Control mux
	process(MCmd, MByteEn)
	begin
	   actuator_chipselect <= (others => '0');
		propdrive_chipselect <= (others => '0');
		next_SData <= SData_int;
		case MCmd is
			when "001" =>               -- write
			   for I in 0 to ACTUATOR_NUMBER-1 loop
					if (MAddr(6 downto 2) = std_logic_vector(to_unsigned(ACTUATOR_ADDR_BASE+I ,5))) then
						actuator_chipselect(I) <= '1';
					end if;
				end loop;
				
				for I in 0 to PROPDRIVE_NUMBER-1 loop
					if (MAddr(6 downto 2) = std_logic_vector(to_unsigned(PROPDRIVE_ADDR_BASE+I ,5))) then
						propdrive_chipselect(I) <= '1';
					end if;
				end loop;
				
				next_SResp <= "01";
			when "010" =>               -- read
				for I in 0 to PROPDRIVE_NUMBER-1 loop
					if (MAddr(6 downto 2) = std_logic_vector(to_unsigned(PROPDRIVE_ADDR_BASE+I ,5))) then
						next_SData <= propdrive_readdata(I);
					end if;
				end loop;
				next_SResp <= "01";
			when others =>              -- idle
			
				next_SResp <= "00";
		end case;
	end process;

	--Registers
	process(clk, reset)
	begin
		if reset = '1' then
			SResp <= (others => '0');
			SData_int <= (others => '0');
		elsif rising_edge(clk) then
			SResp <= next_SResp;
			SData_int <= next_SData;
		end if;
	end process;
	
	generate_actuator: for I in 0 to ACTUATOR_NUMBER-1 generate
      Actuator_inst : Actuator
			port map(
				  out_port  => actuator_out_port(I),
				  address  => (others => '0'),
				  chipselect  => actuator_chipselect(I),
				  clk => clk,
				  reset_n  => reset_n,
				  write_n  => actuator_write_n(I),
				  writedata  => MData
			);		
   end generate generate_actuator;

	generate_propdrive: for I in 0 to PROPDRIVE_NUMBER-1 generate
      PropDrive_inst : PropDrive
			port map(  
				  out_port   => propdrive_out_port(I),
				  address  => (others => '0'),
				  chipselect  => propdrive_chipselect(I),
				  clk  => clk,
				  reset_n  => reset_n,
				  write_n  => propdrive_write_n(I),
				  writedata  => MData,
				  readdata  => propdrive_readdata(I)
			);	
   end generate generate_propdrive;	
	
end rtl;

