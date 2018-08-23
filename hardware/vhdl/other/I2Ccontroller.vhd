-----------------------------------------------------------------
-- I2C controller - can only read 1 byte at a time (no buffers)
-----------------------------------------------------------------

--------------------------------------------------------------------------------
--
--   Original source:  i2c_master.vhd
--   Dependencies:     none
--   Design Software:  Quartus II 64-bit Version 13.1 Build 162 SJ Full Version
--
--   HDL CODE IS PROVIDED "AS IS."  DIGI-KEY EXPRESSLY DISCLAIMS ANY
--   WARRANTY OF ANY KIND, WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT
--   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
--   PARTICULAR PURPOSE, OR NON-INFRINGEMENT. IN NO EVENT SHALL DIGI-KEY
--   BE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR CONSEQUENTIAL
--   DAMAGES, LOST PROFITS OR LOST DATA, HARM TO YOUR EQUIPMENT, COST OF
--   PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, ANY CLAIMS
--   BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF),
--   ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION, OR OTHER SIMILAR COSTS.
--
--   Version History
--   Version 1.0 11/01/2012 Scott Larson
--     Initial Public Release
--   Version 2.0 06/20/2014 Scott Larson
--     Added ability to interface with different slaves in the same transaction
--     Corrected ack_error bug where ack_error went 'Z' instead of '1' on error
--     Corrected timing of when ack_error signal clears
--   Version 2.1 10/21/2014 Scott Larson
--     Replaced gated clock with clock enable
--     Adjusted timing of SCL during start and stop conditions
--   Version 2.2 02/05/2015 Scott Larson
--     Corrected small SDA glitch introduced in version 2.1
-- 
--------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;

entity I2Ccontroller is
	GENERIC(
		OCP_DATA_WIDTH : integer := 32;
		OCP_ADDR_WIDTH : integer := 16;
		input_clk      : INTEGER := 50_000_000; --input clock speed from user logic in Hz
		bus_clk        : INTEGER := 100_000); --speed the i2c bus (scl) will run at in Hz
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

		sda     : INOUT STD_LOGIC;      --serial data output of i2c bus
		scl     : INOUT STD_LOGIC       --serial clock output of i2c bus

	);
end I2Ccontroller;

architecture rtl of I2Ccontroller is
    CONSTANT divider  :  INTEGER := (input_clk/bus_clk)/4; --number of clocks in 1/4 cycle of scl
    TYPE machine IS(ready, start, command, slv_ack1, wr, rd, slv_ack2, mstr_ack, stop); --needed states
    SIGNAL state         : machine;                        --state machine
    SIGNAL data_clk      : STD_LOGIC;                      --data clock for sda
    SIGNAL data_clk_prev : STD_LOGIC;                      --data clock during previous system clock
    SIGNAL scl_clk       : STD_LOGIC;                      --constantly running internal scl
    SIGNAL scl_ena       : STD_LOGIC := '0';               --enables internal scl to output
    SIGNAL sda_int       : STD_LOGIC := '1';               --internal sda
    SIGNAL sda_ena_n     : STD_LOGIC;                      --enables internal sda to output
    SIGNAL addr_rw       : STD_LOGIC_VECTOR(7 DOWNTO 0);   --latched in address and read/write
    SIGNAL data_tx       : STD_LOGIC_VECTOR(7 DOWNTO 0);   --latched in data to write to slave
    SIGNAL data_rx       : STD_LOGIC_VECTOR(7 DOWNTO 0);   --data received from slave
    SIGNAL bit_cnt       : INTEGER RANGE 0 TO 7 := 7;      --tracks bit number in transaction
    SIGNAL stretch       : STD_LOGIC := '0';               --identifies if slave is stretching scl

	--attribute mark_debug : string;
	--attribute mark_debug of state : signal is "true"; --   std_logic_vector(28 downto 0); --
	--attribute mark_debug of ocp_state : signal is "true"; --   std_logic_vector(28 downto 0); --

	--  these signals are driven by the ocp logic
	SIGNAL reset_n       : STD_LOGIC;
	SIGNAL ena       : STD_LOGIC;       --IN    latch in command
	SIGNAL addr      : STD_LOGIC_VECTOR(6 DOWNTO 0); --IN address of target slave
	SIGNAL rw        : STD_LOGIC;       --IN '0' is write, '1' is read
	SIGNAL data_wr   : STD_LOGIC_VECTOR(7 DOWNTO 0); --IN data to write to slave
	SIGNAL busy      : STD_LOGIC;       --OUT indicates transaction in progress
	SIGNAL data_rd   : STD_LOGIC_VECTOR(7 DOWNTO 0); --OUT data read from slave
	SIGNAL ack_error : STD_LOGIC;       --BUFFER flag if improper acknowledge from slave

	--attribute mark_debug of ena : signal is "true"; --       : STD_LOGIC;       --IN    latch in command
	--attribute mark_debug of addr : signal is "true"; --      : STD_LOGIC_VECTOR(6 DOWNTO 0); --IN address of target slave
	--attribute mark_debug of rw : signal is "true"; --       : STD_LOGIC;       --IN '0' is write, '1' is read
	--attribute mark_debug of data_wr : signal is "true"; --  : STD_LOGIC_VECTOR(7 DOWNTO 0); --IN data to write to slave
	--attribute mark_debug of busy : signal is "true"; --     : STD_LOGIC;       --OUT indicates transaction in progress
	--attribute mark_debug of data_rd : signal is "true"; --  : STD_LOGIC_VECTOR(7 DOWNTO 0); --OUT data read from slave
	--attribute mark_debug of ack_error : signal is "true"; --: STD_LOGIC;       --BUFFER flag if improper acknowledge from slave
	
    TYPE ocp_state_type IS (ready, i2c_1, i2c_4, i2c_5, wait_i2c, send_dva); 
    SIGNAL ocp_state : ocp_state_type;  --state machine

	signal MData_buff : std_logic_vector(31 downto 0);

--	-- Build a 2-D array type for the RAM
--	type memory_t is array ((2 ** DATA_BUFF_ADDR - 1) downto 0) of std_logic_vector(7 downto 0);
--	-- Declare the RAM variable.
--	shared variable ram : memory_t;
	

begin

    -- The negated reset for the i2c controller
    reset_n <= not reset;
    
	-- manage the ocp transaction
	PROCESS(clk, reset)
	BEGIN
		IF (reset = '1') THEN           --reset asserted
			addr      <= (others => '0');
			rw        <= '0';
			data_wr   <= (others => '0');
			ocp_state <= ready;
			ena       <= '0';
			SResp     <= "00";
			SData     <= (others => '0');
			MData_buff <= (others => '0');
		ELSIF (rising_edge(clk)) THEN
			CASE ocp_state IS
				WHEN ready =>
					SResp <= "00";
					case MCmd is
						when "001" =>   -- write (start i2c transaction)
							if (to_integer(unsigned(MAddr)) = 0) then
								MData_buff <= MData;
								addr      <= MData(7 downto 1);
								rw        <= '0'; --always stating with a write (for the register address)
								data_wr   <= MData(15 downto 8); --write the regaddress MData(31 downto 24);
								ocp_state <= i2c_1;
							else
								ocp_state <= send_dva;
							end if;

						when "010" =>   -- read
							ocp_state <= send_dva;

						when others =>  -- idle
							ocp_state <= ready;
					end case;

				WHEN i2c_1 =>
					ena <= '1';
					if (busy = '1') then
						ocp_state <= i2c_4; --the i2c controller has started
					end if;

--				WHEN i2c_2 =>				
--					data_wr <= MData_buff(15 downto 8); --write the data --MData(23 downto 16);
--					if (busy = '0') then --the i2c controller has acknowledge the new request
--						ocp_state <= i2c_3;
--					end if;

--				WHEN i2c_3 =>
--					if (busy = '1') then
--						ocp_state <= i2c_4; --it has started with the new value
--					end if;

				WHEN i2c_4 =>
					if MData_buff(0) = '0' then
						--wr
						rw      <= '0';
						data_wr <= MData_buff(23 downto 16); --wite the data
					else
						--rd
						rw <= '1';
					end if;
					if (busy = '0') then
						ocp_state <= i2c_5;
					end if;

				WHEN i2c_5 =>
					if (busy = '1') then
						ocp_state <= wait_i2c; --it has started with the new value
					end if;

				WHEN wait_i2c =>
					ena <= '0';
					if (busy = '1') then
						ocp_state <= wait_i2c;
					else
						SData(31 downto 9) <= (others => '0');
						SData(8)           <= ack_error;
						SData(7 downto 0)  <= data_rd;
						ocp_state          <= send_dva;
					end if;

				WHEN send_dva =>
					SResp     <= "01";
					ocp_state <= ready;

			END CASE;
		END IF;
	END PROCESS;

	--------------------------------------------------
	-- I2C related stuff
	--------------------------------------------------	
	  --generate the timing for the bus clock (scl_clk) and the data clock (data_clk)
    PROCESS(clk, reset_n)
      VARIABLE count  :  INTEGER RANGE 0 TO divider*4;  --timing for clock generation
    BEGIN
      IF(reset_n = '0') THEN                --reset asserted
        stretch <= '0';
        count := 0;
      ELSIF(clk'EVENT AND clk = '1') THEN
        data_clk_prev <= data_clk;          --store previous value of data clock
        IF(count = divider*4-1) THEN        --end of timing cycle
          count := 0;                       --reset timer
        ELSIF(stretch = '0') THEN           --clock stretching from slave not detected
          count := count + 1;               --continue clock generation timing
        END IF;
        CASE count IS
          WHEN 0 TO divider-1 =>            --first 1/4 cycle of clocking
            scl_clk <= '0';
            data_clk <= '0';
          WHEN divider TO divider*2-1 =>    --second 1/4 cycle of clocking
            scl_clk <= '0';
            data_clk <= '1';
          WHEN divider*2 TO divider*3-1 =>  --third 1/4 cycle of clocking
            scl_clk <= '1';                 --release scl
            IF(scl = '0') THEN              --detect if slave is stretching clock
              stretch <= '1';
            ELSE
              stretch <= '0';
            END IF;
            data_clk <= '1';
          WHEN OTHERS =>                    --last 1/4 cycle of clocking
            scl_clk <= '1';
            data_clk <= '0';
        END CASE;
      END IF;
    END PROCESS;
  
    --state machine and writing to sda during scl low (data_clk rising edge)
    PROCESS(clk, reset_n)
    BEGIN
      IF(reset_n = '0') THEN                 --reset asserted
        state <= ready;                      --return to initial state
        busy <= '1';                         --indicate not available
        scl_ena <= '0';                      --sets scl high impedance
        sda_int <= '1';                      --sets sda high impedance
        ack_error <= '0';                    --clear acknowledge error flag
        bit_cnt <= 7;                        --restarts data bit counter
        data_rd <= "00000000";               --clear data read port
      ELSIF(clk'EVENT AND clk = '1') THEN
        IF(data_clk = '1' AND data_clk_prev = '0') THEN  --data clock rising edge
          CASE state IS
            WHEN ready =>                      --idle state
              IF(ena = '1') THEN               --transaction requested
                busy <= '1';                   --flag busy
                addr_rw <= addr & rw;          --collect requested slave address and command
                data_tx <= data_wr;            --collect requested data to write
                state <= start;                --go to start bit
              ELSE                             --remain idle
                busy <= '0';                   --unflag busy
                state <= ready;                --remain idle
              END IF;
            WHEN start =>                      --start bit of transaction
              busy <= '1';                     --resume busy if continuous mode
              sda_int <= addr_rw(bit_cnt);     --set first address bit to bus
              state <= command;                --go to command
            WHEN command =>                    --address and command byte of transaction
              IF(bit_cnt = 0) THEN             --command transmit finished
                sda_int <= '1';                --release sda for slave acknowledge
                bit_cnt <= 7;                  --reset bit counter for "byte" states
                state <= slv_ack1;             --go to slave acknowledge (command)
              ELSE                             --next clock cycle of command state
                bit_cnt <= bit_cnt - 1;        --keep track of transaction bits
                sda_int <= addr_rw(bit_cnt-1); --write address/command bit to bus
                state <= command;              --continue with command
              END IF;
            WHEN slv_ack1 =>                   --slave acknowledge bit (command)
              IF(addr_rw(0) = '0') THEN        --write command
                sda_int <= data_tx(bit_cnt);   --write first bit of data
                state <= wr;                   --go to write byte
              ELSE                             --read command
                sda_int <= '1';                --release sda from incoming data
                state <= rd;                   --go to read byte
              END IF;
            WHEN wr =>                         --write byte of transaction
              busy <= '1';                     --resume busy if continuous mode
              IF(bit_cnt = 0) THEN             --write byte transmit finished
                sda_int <= '1';                --release sda for slave acknowledge
                bit_cnt <= 7;                  --reset bit counter for "byte" states
                state <= slv_ack2;             --go to slave acknowledge (write)
              ELSE                             --next clock cycle of write state
                bit_cnt <= bit_cnt - 1;        --keep track of transaction bits
                sda_int <= data_tx(bit_cnt-1); --write next bit to bus
                state <= wr;                   --continue writing
              END IF;
            WHEN rd =>                         --read byte of transaction
              busy <= '1';                     --resume busy if continuous mode
              IF(bit_cnt = 0) THEN             --read byte receive finished
                IF(ena = '1' AND addr_rw = addr & rw) THEN  --continuing with another read at same address
                  sda_int <= '0';              --acknowledge the byte has been received
                ELSE                           --stopping or continuing with a write
                  sda_int <= '1';              --send a no-acknowledge (before stop or repeated start)
                END IF;
                bit_cnt <= 7;                  --reset bit counter for "byte" states
                data_rd <= data_rx;            --output received data
                state <= mstr_ack;             --go to master acknowledge
              ELSE                             --next clock cycle of read state
                bit_cnt <= bit_cnt - 1;        --keep track of transaction bits
                state <= rd;                   --continue reading
              END IF;
            WHEN slv_ack2 =>                   --slave acknowledge bit (write)
              IF(ena = '1') THEN               --continue transaction
                busy <= '0';                   --continue is accepted
                addr_rw <= addr & rw;          --collect requested slave address and command
                data_tx <= data_wr;            --collect requested data to write
                IF(addr_rw = addr & rw) THEN   --continue transaction with another write
                  sda_int <= data_wr(bit_cnt); --write first bit of data
                  state <= wr;                 --go to write byte
                ELSE                           --continue transaction with a read or new slave
                  state <= start;              --go to repeated start
                END IF;
              ELSE                             --complete transaction
                state <= stop;                 --go to stop bit
              END IF;
            WHEN mstr_ack =>                   --master acknowledge bit after a read
              IF(ena = '1') THEN               --continue transaction
                busy <= '0';                   --continue is accepted and data received is available on bus
                addr_rw <= addr & rw;          --collect requested slave address and command
                data_tx <= data_wr;            --collect requested data to write
                IF(addr_rw = addr & rw) THEN   --continue transaction with another read
                  sda_int <= '1';              --release sda from incoming data
                  state <= rd;                 --go to read byte
                ELSE                           --continue transaction with a write or new slave
                  state <= start;              --repeated start
                END IF;    
              ELSE                             --complete transaction
                state <= stop;                 --go to stop bit
              END IF;
            WHEN stop =>                       --stop bit of transaction
              busy <= '0';                     --unflag busy
              state <= ready;                  --go to idle state
          END CASE;    
        ELSIF(data_clk = '0' AND data_clk_prev = '1') THEN  --data clock falling edge
          CASE state IS
            WHEN start =>                  
              IF(scl_ena = '0') THEN                  --starting new transaction
                scl_ena <= '1';                       --enable scl output
                ack_error <= '0';                     --reset acknowledge error output
              END IF;
            WHEN slv_ack1 =>                          --receiving slave acknowledge (command)
              IF(sda /= '0' OR ack_error = '1') THEN  --no-acknowledge or previous no-acknowledge
                ack_error <= '1';                     --set error output if no-acknowledge
              END IF;
            WHEN rd =>                                --receiving slave data
              data_rx(bit_cnt) <= sda;                --receive current slave data bit
            WHEN slv_ack2 =>                          --receiving slave acknowledge (write)
              IF(sda /= '0' OR ack_error = '1') THEN  --no-acknowledge or previous no-acknowledge
                ack_error <= '1';                     --set error output if no-acknowledge
              END IF;
            WHEN stop =>
              scl_ena <= '0';                         --disable scl
            WHEN OTHERS =>
              NULL;
          END CASE;
        END IF;
      END IF;
    END PROCESS;  
  
    --set sda output
    WITH state SELECT
      sda_ena_n <= data_clk_prev WHEN start,     --generate start condition
                   NOT data_clk_prev WHEN stop,  --generate stop condition
                   sda_int WHEN OTHERS;          --set to internal sda signal    
        
    --set scl and sda outputs
    scl <= '0' WHEN (scl_ena = '1' AND scl_clk = '0') ELSE 'Z';
    sda <= '0' WHEN sda_ena_n = '0' ELSE 'Z';
end rtl;
