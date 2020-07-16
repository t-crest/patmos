--
-- Copyright: 2013, Technical University of Denmark, DTU Compute
-- Author: Martin Schoeberl (martin@jopdesign.com)
-- License: Simplified BSD License
--

-- VHDL top level for Patmos in Chisel with on-chip memory.
--
-- Includes some 'magic' VHDL code to generate a reset after FPGA configuration.
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity patmos_top is
    port(
			-- Clock and reset
			clk           			  	: in  std_logic;
			--cpu_reset_btn         : in std_logic;
			-- LEDs
			oLedsPins_led 				: out std_logic_vector(7 downto 0);
			--Expansion LEDs
			oLed_expansion				: out std_logic_vector(1 downto 0);
			--Expansion Buttons
			iBtn_expansion 			: in std_logic_vector(1 downto 0);
			--UART
			oUartPins_txd 				: out   std_logic;
			iUartPins_rxd 				: in    std_logic;
			--I2C
			oI2c_scl		  				: inout std_logic;
			ioI2c_sda	  				: inout std_logic;
			--SRAM
			oSram_A  	  				: out std_logic_vector(19 downto 0);
			oSram_E1		  				: out std_logic;
			oSram_E2		  				: out std_logic;
			oSram_BWout	  				: out std_logic_vector(3 downto 0);
			oSram_E3		  				: out std_logic;
			oSram_ADSP	  				: out std_logic;
			oSram_G		  				: out std_logic;
			oSram_BW		  				: out std_logic;
			oSram_ADV	  				: out std_logic;
			oSram_CLK	  				: out std_logic;
			oSram_GW		  				: out std_logic;
			oSram_FT		  				: out std_logic;
			oSram_LBO	  				: out std_logic;
			oSram_ZZ		  				: out std_logic;
			oSram_ADSC	  				: out std_logic;
			ioSram_DQ					: inout std_logic_vector(31 downto 0);
			
			ioSram_DQPb					: inout std_logic;
			ioSram_DQPa					: inout std_logic;
			ioSram_DQPc					: inout std_logic;
			ioSram_DQPd					: inout std_logic;


			--GPIO
			gpio					 		: out std_logic_vector(6 downto 0)

		  


    );
end entity patmos_top;

architecture rtl of patmos_top is
    component Patmos is
        port(
			clk                         : in  std_logic;
			reset                       : in  std_logic;
			io_Leds_led             : out std_logic_vector(7 downto 0);
			io_UartCmp_tx              : out std_logic;
			io_UartCmp_rx              : in  std_logic;
			io_I2CInterface_MCmd    : out std_logic_vector(2 downto 0);
			io_I2CInterface_MAddr   : out std_logic_vector(15 downto 0);
			io_I2CInterface_MData   : out std_logic_vector(31 downto 0);
			io_I2CInterface_MByteEn : out std_logic_vector(3 downto 0);
			io_I2CInterface_SResp   			: in  std_logic_vector(1 downto 0);
			io_I2CInterface_SData   			: in  std_logic_vector(31 downto 0) := (others => '0')
--			io_sSramGSICtrl_ramOut_addr: out std_logic_vector(19 downto 0);
--			io_sSramGSICtrl_ramOut_doutEna: out std_logic;
--			io_sSramGSICtrl_ramOut_zz: out std_logic;
--			io_sSramGSICtrl_ramOut_ft: out std_logic;
--			io_sSramGSICtrl_ramOut_lbo: out std_logic;
--			io_sSramGSICtrl_ramOut_adsp: out std_logic;
--			io_sSramGSICtrl_ramOut_adsc: out std_logic;
--			io_sSramGSICtrl_ramOut_adv: out std_logic;
--			io_sSramGSICtrl_ramOut_bw: out std_logic;
--			io_sSramGSICtrl_ramOut_bwe: out std_logic_vector(3 downto 0);
--			io_sSramGSICtrl_ramOut_e1: out std_logic;
--			io_sSramGSICtrl_ramOut_e2: out std_logic;
--			io_sSramGSICtrl_ramOut_e3: out std_logic;
--			io_sSramGSICtrl_ramOut_gw: out std_logic;
--			io_sSramGSICtrl_ramOut_g: out std_logic;
--			io_sSramGSICtrl_ramOut_dout: out std_logic_vector(31 downto 0);
--			io_sSramGSICtrl_ramIn_din : in std_logic_vector(31 downto 0);
--			io_SSramGSICtrl_ramOut_stateLeds : out std_logic_vector(2 downto 0)
	  );
    end component;

     component I2Ccontroller is
         generic(
             OCP_DATA_WIDTH : integer := 32;
             OCP_ADDR_WIDTH : integer := 16;
             input_clk      : INTEGER := 50_000_000; --input clock speed from user logic in Hz
             bus_clk        : INTEGER := 400_000); --speed the i2c bus (scl) will run at in Hz
         port(
             clk     : in    std_logic;
             reset   : in    std_logic;

             -- OCP IN (slave)
             MCmd    : in    std_logic_vector(2 downto 0);
             MAddr   : in    std_logic_vector((OCP_ADDR_WIDTH - 1) downto 0);
             MData   : in    std_logic_vector((OCP_DATA_WIDTH - 1) downto 0);
             MByteEn : in    std_logic_vector(3 downto 0);
             SResp   : out   std_logic_vector(1 downto 0);
             SData   : out   std_logic_vector((OCP_DATA_WIDTH - 1) downto 0);

             sda     : INOUT STD_LOGIC;  --serial data output of i2c bus
             scl     : INOUT STD_LOGIC   --serial clock output of i2c bus

         );
	 	  end component;
		  

        -- DE2-70: 50 MHz clock => 80 MHz
        -- BeMicro: 16 MHz clock => 25.6 MHz
        -- de10-nano: start with the 50 MHz clock input and no PLL
        constant pll_infreq : real    := 50.0;
        constant pll_mult   : natural := 1;
        constant pll_div    : natural := 1; 

        signal clk_int : std_logic;
		  signal clk_sram : std_logic;

        -- for generation of internal reset
        signal reset_int            : std_logic;
        signal res_reg1, res_reg2 : std_logic;
        signal res_cnt            : unsigned(2 downto 0) := "000"; -- for the simulation

        attribute altera_attribute : string;
        attribute altera_attribute of res_cnt : signal is "POWER_UP_LEVEL=LOW";

        signal address : std_logic_vector(1 downto 0);
        signal reset_n : std_logic;
			
		  signal i2CInterfacePins_MCmd    : std_logic_vector(2 downto 0);
        signal i2CInterfacePins_MAddr   : std_logic_vector(15 downto 0);
        signal i2CInterfacePins_MData   : std_logic_vector(31 downto 0);
        signal i2CInterfacePins_MByteEn : std_logic_vector(3 downto 0);
        signal i2CInterfacePins_SResp   : std_logic_vector(1 downto 0) := (others => '0');
        signal i2CInterfacePins_SData   : std_logic_vector(31 downto 0):= (others => '0');
			
			signal dummy_leds : std_logic_vector(7 downto 0);
		  
		          -- sram signals for tristate inout
        signal sram_out_dout_ena : std_logic;
        signal sram_out_dout : std_logic_vector(31 downto 0);
        signal sram_in_din : std_logic_vector(31 downto 0);
        signal sram_in_din_reg : std_logic_vector(31 downto 0);
		  
		  type t_state is (INIT,PRE_W,WRITE, WR_D, READ,RD_D);
		  signal state : t_state; 
		  
		  signal counter : integer RANGE 0 TO 50000001 := 0; -- Once we hit 101 it will go back to 0
		  
		   

    begin

        pll_inst : entity work.pll generic map(
        			input_freq  => pll_infreq,
        			multiply_by => pll_mult,
        			divide_by   => pll_div
        		)
        		port map(
        			inclk0 => clk,
        			c0     => clk_int,
					c2		 => clk_sram
        		);
        -- we use a PLL
        --clk_int <= clk;
  
        --
        --	internal reset generation
        --	should include the PLL lock signal
        --
		  oSram_CLK <= clk_int;
		  
        process(clk_int)
        begin
            if rising_edge(clk_int) then
                if (res_cnt /= "111") then
                    res_cnt <= res_cnt + 1;
                end if;
                res_reg1 <= not res_cnt(0) or not res_cnt(1) or not res_cnt(2);
                res_reg2 <= res_reg1;
                reset_int  <= res_reg2;
            end if;
        end process;
			
        
       reset_n <= not reset_int;
		
		 

		 
       -- capture input from ssram on falling clk edge
			process(clk_int, reset_int)
			begin
			 if reset_int='1' then
				sram_in_din_reg <= (others => '0');
			 elsif falling_edge(clk_int) then
				sram_in_din_reg <= ioSram_DQ;
			 end if;
			end process;

			-- tristate output to ssram
			process(sram_out_dout_ena, sram_out_dout)
			begin
			 if sram_out_dout_ena='1' then
				ioSram_DQ <= sram_out_dout;
			 else
				ioSram_DQ <= (others => 'Z');
			 end if;
			end process;

			-- input of tristate on positive clk edge
			process(clk_int)
			begin
			if rising_edge(clk_int) then
				  sram_in_din <= sram_in_din_reg;
				end if;
			end process;
		  

        Patmos_inst_0 : Patmos port map(
                clk                         => clk_int,
                reset                       => reset_int,
                io_Leds_led             => oLedsPins_led,
                io_UartCmp_tx              => oUartPins_txd,
                io_UartCmp_rx              => iUartPins_rxd,
                io_I2CInterface_MCmd    => i2CInterfacePins_MCmd,
                io_I2CInterface_MAddr	  => i2CInterfacePins_MAddr,
                io_I2CInterface_MData	  => i2CInterfacePins_MData,
                io_I2CInterface_MByteEn => i2CInterfacePins_MByteEn,
                io_I2CInterface_SResp   => i2CInterfacePins_SResp,
                io_I2CInterface_SData   => i2CInterfacePins_SData
--					 io_sSramGSICtrl_ramOut_addr=> oSram_A,
--					 io_SSramGSICtrl_ramOut_doutEna=> sram_out_dout_ena,
--					 io_sSramGSICtrl_ramOut_zz=> oSram_ZZ,
--					 io_sSramGSICtrl_ramOut_ft=> oSram_FT,
--					 io_sSramGSICtrl_ramOut_lbo=> oSram_LBO,
--					 io_sSramGSICtrl_ramOut_adsp=> oSram_ADSP,
--					 io_sSramGSICtrl_ramOut_adsc=> oSram_ADSC,
--					 io_sSramGSICtrl_ramOut_adv=> oSram_ADV,
--					 io_sSramGSICtrl_ramOut_bw=> oSram_BW,
--					 io_sSramGSICtrl_ramOut_bwe=> oSram_BWout,
--					 io_sSramGSICtrl_ramOut_e1=> oSram_E1,
--					 io_sSramGSICtrl_ramOut_e2=> oSram_E2,
--					 io_sSramGSICtrl_ramOut_e3=> oSram_E3,
--					 io_sSramGSICtrl_ramOut_gw=> oSram_GW,
--					 io_sSramGSICtrl_ramOut_g=> oSram_G,
--					 io_sSramGSICtrl_ramOut_dout=> sram_out_dout,
--					 io_sSramGSICtrl_ramIn_din=> sram_in_din_reg,
--					 io_SSramGSICtrl_ramOut_stateLeds => open
					 
            );
				
				
				
		 I2Ccontroller_inst_0 : I2Ccontroller
            generic map(
                OCP_DATA_WIDTH => 32,
                OCP_ADDR_WIDTH => 16,
                input_clk      => 50_000_000, --input clock speed from user logic in Hz
                bus_clk        => 100_000) --speed the i2c bus (scl) will run at in Hz
            port map(
                clk     => clk_int,
                reset   => reset_int,

                -- OCP IN (slave)
                MCmd    => i2CInterfacePins_MCmd,
                MAddr   => i2CInterfacePins_MAddr,
                MData   => i2CInterfacePins_MData,
                MByteEn => i2CInterfacePins_MByteEn,
                SResp   => i2CInterfacePins_SResp,
                SData   => i2CInterfacePins_SData,
                sda     => ioI2c_sda,
                scl     => oI2c_scl
            );

				
				
--			oSram_ZZ <= '0'; -- High -> Sleep Mode
--			oSram_FT <= '0'; -- Output Data Register Disable
--			oSram_LBO <= '0'; -- 0: Linear Burst Mode, 1: Interleaved Burst Mode
--			oSram_ADSP <= '1'; --Keep burst writes off
--			oSram_ADSC <= '1'; --Keep burst writes off
--			oSram_ADV <= '1'; --Burst counter advance
--			oSram_A <= "01011110101010110010";
--			oSram_BW <= '1'; ---Disable byte write
--			oSram_BWout <= "1111"; ---Disable byte write
--			oSram_E1 <= '0'; --Chip enable active low
--			oSram_E2 <= '1'; --Chip enable active high
--			oSram_E3 <= '0'; --Chip enable active low
		
			
--			process(clk_int)
--			begin
--				if rising_edge(clk_int) then
--					if reset_int='1' then
--						state <= INIT;
--						oLedsPins_led(0) <= '0';
--					end if;
--					
--				case state is
--					when INIT =>
--						oSram_GW <= '1';
--						oSram_G <= '1';
--						sram_out_dout_ena <= '0';					
--						state <= WRITE;
--					
--					when PRE_W =>
--						oLedsPins_led(0) <= '0';
--						counter <= counter + 1;
--						if counter = 50000000 then
--							state <= WRITE;
--						else
--						   state <= PRE_W;
--						end if;
--						
--					when WRITE =>
--						
--						oSram_GW <= '0';
--						oSram_G <= '1';
--						sram_out_dout_ena <= '1';
--						sram_out_dout <= "10101010101010101010101010101010";
--						state <= WR_D;	
--						
--					when WR_D =>
--						oSram_GW <= '1';
--						oSram_G <= '1';
--						sram_out_dout_ena <= '0';
--						state <= READ;
--						
--					when READ =>
--						oSram_GW <= '1';
--						oSram_G <= '0';
--						sram_out_dout_ena <= '0';
--						state <= RD_D;
--						
--					when RD_D =>
--						oSram_GW <= '1';
--						oSram_G <= '1';
--						sram_out_dout_ena <= '0';
--						
--						if sram_in_din_reg = "10101010101010101010101010101010"then
--							oLedsPins_led(0) <= '1';
--						end if;
--						
--						counter <= counter + 1;
--						
--						if counter = 50000000 then
--							state <= PRE_W;
--						else
--						   state <= RD_D;
--						end if;
--						
--					when others => 
--						state <=INIT;
--				end case;
--
--				end if;
--			end process;
				
    end architecture rtl;
