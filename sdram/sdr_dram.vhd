--------------------------------------------------------------------------------
--! SDRAM controller (Single Data Rate)
--!
--! @version 0.1
--! Simple, non-pipelined, non-optimized controller
--!
--! @author Edgar Lakis <edgar.lakis@gmail.com>
--! @section LICENSE
--! Copyright Technical University of Denmark. All rights reserved.
--! This file is part of the time-predictable VLIW Patmos.
--! 
--! Redistribution and use in source and binary forms, with or without
--! modification, are permitted provided that the following conditions are met:
--! 
--!    1. Redistributions of source code must retain the above copyright notice,
--!       this list of conditions and the following disclaimer.
--! 
--!    2. Redistributions in binary form must reproduce the above copyright
--!       notice, this list of conditions and the following disclaimer in the
--!       documentation and/or other materials provided with the distribution.
--! 
--! THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
--! OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
--! OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
--! NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
--! DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
--! (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
--! LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
--! ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
--! (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
--! THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
--! 
--! The views and conclusions contained in the software and documentation are
--! those of the authors and should not be interpreted as representing official
--! policies, either expressed or implied, of the copyright holder.
--------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

--! SDRAM controller (Single Data Rate)

--! Simple controller with predictable timing.
--! Uses closed page policy.
entity sdr_sdram is
    generic(
        --! @name User interface
        --! \{
        ADDR_WIDTH         : natural := 16; --! The size of address: @p ocp_MAddr
        DATA_WIDTH         : natural := 32; --! The size of data word: @p ocp_MData, @p ocp_SData
        BURST_LENGTH       : natural := 2; --! The number of words transfered per request
        --! \}
        --! @name SDRAM interface
        --! \{
        SA_WIDTH           : natural := 13; --! Number of @p sdram_SA bits
        -- Address Mapping
        CS_WIDTH           : natural;   --! Number of Chip Selects bits in user @p ocp_MAddr
        CS_LOW_BIT         : natural;   --! Last position of Chip Selects bits in user @p ocp_MAddr
        BA_WIDTH           : natural;   --! Number of Bank Address bits in user @p ocp_MAddr
        BA_LOW_BIT         : natural;   --! Last position of Bank Address bits in user @p ocp_MAddr
        ROW_WIDTH          : natural;   --! Number of Row bits in user @p ocp_MAddr
        ROW_LOW_BIT        : natural;   --! Last position of Row bits in user @p ocp_MAddr
        COL_WIDTH          : natural;   --! Number of Column bits in user @p ocp_MAddr
        COL_LOW_BIT        : natural;   --! Last position of Column bits in user @p ocp_MAddr
        --! \}
        --! @name SDRAM timing
        --! Use the values from SDRAM chip's Datasheet
        --! \{
        tCLK               : time;      --! Clock period
        tINIT_IDLE         : time;      --! Inactivity perdiod required during initialization 
        INIT_REFRESH_COUNT : natural;   --! Number of Refresh commands required during initialization
        tCAC_CYCLES        : natural;   --! CAS latency
        tRRD               : time;      --! Row to Row Delay (ACT[0]-ACT[1])
        tRCD               : time;      --! Row to Column Delay (ACT-READ/WRITE)
        tRAS               : time;      --! Row Access Strobe (ACT-PRE)
        tRC                : time;      --! Row Cycle (REF-REF,ACT-ACT)
        tRP                : time;      --! Row Precharge (PRE-ACT)
        tCCD               : time;      --! Column Command Delay Time
        tDPL               : time;      --! Input Data to Precharge (DQ_WR-PRE)
        tDAL               : time;      --! Input Data to Activate (DQ_WR-ACT/PRE)
        tRBD               : time;      --! Burst Stop to High Impedance (Read)
        tWBD               : time;      --! Burst Stop to Input in Invalid (Write)
        tPQL               : time;      --! Last Output to Auto-Precharge Start (READ)
        tQMD               : time;      --! DQM to Output (Read)
        tDMD               : time;      --! DQM to Input (Write)
        -- FIXME: Some datasheets provide tMRD as fixed cycle count, not as time period
        --        tMRD               : time;      --! Mode Register Delay (program time)
        tMRD_CYCLES        : natural;   --! Mode Register Delay (program time)
        tREF               : time       --! Refresh Cycle (for each row)
    --! \}
    );
    port(
        rst                : in    std_logic; --! Reset
        clk                : in    std_logic; --! Clock
        pll_locked         : in    std_logic; --! '1' when PLL has locked the clocks 
        --! @name User interface
        --! Simple OCP like protocol
        --! \{
        ocp_MCmd           : in    std_logic_vector(2 downto 0); --! Request (Idle/Read/Write)
        ocp_MCmd_doRefresh : in    std_logic; --! 
        ocp_MAddr          : in    std_logic_vector(ADDR_WIDTH - 1 downto 0); --! Request Address
        --! Acknowledges the validity of the next word. For Read Request this denotes the transmission of
        --! valid word. For Write Request this acknowledges that the current word is accepted and next word
        --! should be provided during next cycle.
        ocp_SCmdAccept     : out   std_logic; --! Acknowledges the request and the Data 
        ocp_MData          : in    std_logic_vector(DATA_WIDTH - 1 downto 0); --! Write Data  
        ocp_MDataByteEn    : in    std_logic_vector(DATA_WIDTH / 8 - 1 downto 0); --! Write Data mask
        ocp_MDataValid     : in    std_logic; --! Write Data valid (handshaking during write)
        ocp_MDataLast      : in    std_logic; --! Write Data Last (handshaking during write)
        ocp_SDataAccept    : out   std_logic; --! Write Data accept (handshaking during write)
        ocp_SData          : out   std_logic_vector(DATA_WIDTH - 1 downto 0); --! Read Data
        ocp_SResp          : out   std_logic; --! The Read Data is Valid
        ocp_SRespLast      : out   std_logic; --! Last data in burst
        --! \}
        --! @name SDRAM interface 
        --! \{
        sdram_CKE          : out   std_logic; --! Clock Enable
        sdram_RAS_n        : out   std_logic; --! Row Address Strobe
        sdram_CAS_n        : out   std_logic; --! Column Address Strobe
        sdram_WE_n         : out   std_logic; --! Write Enable
        sdram_CS_n         : out   std_logic_vector(2 ** CS_WIDTH - 1 downto 0); --! Chip Selects
        sdram_BA           : out   std_logic_vector(BA_WIDTH - 1 downto 0); --! Bank Address
        sdram_SA           : out   std_logic_vector(SA_WIDTH - 1 downto 0); --! SDRAM Address
        sdram_DQ           : inout std_logic_vector(DATA_WIDTH - 1 downto 0); --! Data
        sdram_DQM          : out   std_logic_vector(DATA_WIDTH / 8 - 1 downto 0) --! Data mask
    --! \}
    );
end entity sdr_sdram;

library ieee;
use ieee.std_logic_1164.all;

entity bin_counter is
    generic(
        AUTO_RESET : boolean := false;
        PERIOD     : natural
    );
    port(
        clk        : in  std_logic;
        sync_reset : in  std_logic;
        enable     : in  std_logic;
        done_tick  : out std_logic
    );
end entity bin_counter;

library ieee;
use ieee.math_real.ceil;

architecture RTL of sdr_sdram is
    --! @brief Variable size binary decoder (active low)
    function BinDecode_n(num : std_logic_vector) return std_logic_vector is
        variable result : std_logic_vector(2 ** num'length - 1 downto 0);
    begin
        result                            := (others => '1');
        result(to_integer(unsigned(num))) := '0';
        return result;
    end function BinDecode_n;

    --! @brief Rounds the constant in time units to cycles
    function RoundTimeConstantToCycles(
        constant clkPeriod  : time;     --! Clock Period
        constant timePeriod : time)     --! Time period to be rounded to cycles
        return natural is
        constant resolution : time := 100 ps;
        variable r          : real := real(real(timePeriod / resolution) / real(clkPeriod / resolution));
    begin                               -- RoundTimeConstantToCycles
        return natural(ceil(r));
    end RoundTimeConstantToCycles;

    constant OCP_CMD_READ  : std_logic_vector(2 downto 0) := "001";
    constant OCP_CMD_WRITE : std_logic_vector(2 downto 0) := "010";

    constant BL  : natural := BURST_LENGTH;
    constant CAC : natural := tCAC_CYCLES; --! CAS latency
    --    constant RRD : natural := RoundTimeConstantToCycles(tCLK, tRRD); --! Row to Row Delay (ACT[0]-ACT[1])
    constant RCD : natural := RoundTimeConstantToCycles(tCLK, tRCD); --! Row to Column Delay (ACT-READ/WRITE)
    constant RAS : natural := RoundTimeConstantToCycles(tCLK, tRAS); --! Row Access Strobe (ACT-PRE)
    constant RC  : natural := RoundTimeConstantToCycles(tCLK, tRC); --! Row Cycle (REF-REF,ACT-ACT)
    constant RP  : natural := RoundTimeConstantToCycles(tCLK, tRP); --! Row Precharge (PRE-ACT)
    --    constant CCD : natural := RoundTimeConstantToCycles(tCLK, tCCD); --! Column Command Delay Time
    constant DPL : natural := RoundTimeConstantToCycles(tCLK, tDPL); --! Input Data to Precharge (DQ_WR-PRE)
    constant DAL : natural := RoundTimeConstantToCycles(tCLK, tDAL); --! Input Data to Activate (DQ_WR-ACT/PRE)
    --    constant RBD : natural := RoundTimeConstantToCycles(tCLK, tRBD); --! Burst Stop to High Impedance (Read)
    --    constant WBD : natural := RoundTimeConstantToCycles(tCLK, tWBD); --! Burst Stop to Input in Invalid (Write)
    constant PQL : natural := RoundTimeConstantToCycles(tCLK, tPQL); --! Last Output to Auto-Precharge Start (READ)
    --    constant QMD : natural := RoundTimeConstantToCycles(tCLK, tQMD); --! DQM to Output (Read)
    --    constant DMD : natural := RoundTimeConstantToCycles(tCLK, tDMD); --! DQM to Input (Write)
    --    constant MRD : natural := RoundTimeConstantToCycles(tCLK, tMRD); --! Mode Register Delay (program time)
    constant MRD : natural := tMRD_CYCLES;
    --    constant REF : natural := natural(tREF / (2**ROW_WIDTH)); --! Refresh Cycle (for each row)


    function DefineModeRegister return std_logic_vector is
        variable result : std_logic_vector(BA_WIDTH + SA_WIDTH - 1 downto 0);

    begin
        -- Set reserved bits to '0'
        result             := (others => '0');
        -- Write Burst Mode: '0'- use same burst length as read; '1'- write single word 
        result(9)          := '0';
        -- Operating Mode: "00"- Normal Operation (other reserved for testing etc.)  
        result(8 downto 7) := "00";
        -- CAS Latency: usual binary encoding
        result(6 downto 4) := std_logic_vector(to_unsigned(tCAC_CYCLES, 3));
        -- Burst Type: '0'- Sequential; '1'- interleaved
        result(3)          := '0';
        -- Burst Length: "000".."011": 1,2,4,8; "111": Full Page
        case BURST_LENGTH is
            when 1 => result(2 downto 0) := "000";
            when 2 => result(2 downto 0) := "001";
            when 4 => result(2 downto 0) := "010";
            when 8 => result(2 downto 0) := "011";
            when 0 => result(2 downto 0) := "111";
                assert result(3) = '0' report "Full Page Burst not possible in interleaved mode" severity error;
                report "Full Page Bursts not supported by controller" severity error;
            when others =>
                report "Burst Length not supported by SDRAM" severity error;
        end case;
        return result;
    end function DefineModeRegister;

    -- We use autoprecharge, ensure that it will be invoked after tRAS 
    function CalculateAct2ReadCycles return natural is
        variable result : natural;
    begin
        result := RCD;
        if (RCD + CAC + BL - 1 - PQL < RAS) then
            result := RAS + PQL - (BL - 1) - CAC;
        end if;
        return result;
    end function CalculateAct2ReadCycles;

    -- We use autoprecharge, ensure that it will be invoked after tRAS 
    function CalculateAct2WriteCycles return natural is
        variable result : natural;
    begin
        result := RCD;
        if (RCD + BL - 1 + DPL < RAS) then
            result := RAS - DPL - (BL - 1);
        end if;
        return result;
    end function CalculateAct2WriteCycles;

    constant c_INIT_IDLE_CYCLES        : natural := RoundTimeConstantToCycles(tCLK, tINIT_IDLE);
    constant c_PRECHARGE_CYCLES        : natural := RP;
    constant c_REFRESH_CYCLES          : natural := RC;
    constant c_PROGRAM_REGISTER_CYCLES : natural := MRD;
    constant c_ACT2READ_CYCLES         : natural := CalculateAct2ReadCycles;
    constant c_ACT2WRITE_CYCLES        : natural := CalculateAct2WriteCycles;
    constant c_WRITE2READY_CYCLES      : natural := DAL;

    type t_state is (initWaitLock, initWaitIdle, initPrecharge, initPrechargeComplete, initRefresh, initRefreshComplete, initProgramModeReg, initProgramModeRegComplete, ready, readCmd, readDataWait, readData, writeCmd, writeDataRest, writePrechargeComplete, refreshComplete);

    signal state_r, state_nxt : t_state;
    signal sdram_RAS_n_nxt    : std_logic;
    signal sdram_CAS_n_nxt    : std_logic;
    signal sdram_WE_n_nxt     : std_logic;
    signal sdram_CS_n_nxt     : std_logic_vector(2 ** CS_WIDTH - 1 downto 0);
    signal sdram_BA_nxt       : std_logic_vector(BA_WIDTH - 1 downto 0);
    signal sdram_SA_nxt       : std_logic_vector(SA_WIDTH - 1 downto 0);
    signal sdram_DQM_nxt      : std_logic_vector(DATA_WIDTH / 8 - 1 downto 0);
    -- registered Data input
    signal sdram_DQ_r         : std_logic_vector(DATA_WIDTH - 1 downto 0);
    -- registered Data output, before the tri-state
    signal sdram_DQout_r      : std_logic_vector(DATA_WIDTH - 1 downto 0);
    -- registered Data output tri-state controll
    signal sdram_DQoe_r       : std_logic;
    signal sdram_DQoe_nxt     : std_logic;
    alias a_cs                : std_logic_vector(CS_WIDTH - 1 downto 0) is ocp_MAddr(CS_WIDTH + CS_LOW_BIT - 1 downto CS_LOW_BIT);
    alias a_row               : std_logic_vector(ROW_WIDTH - 1 downto 0) is ocp_MAddr(ROW_WIDTH + ROW_LOW_BIT - 1 downto ROW_LOW_BIT);
    alias a_bank              : std_logic_vector(BA_WIDTH - 1 downto 0) is ocp_MAddr(BA_WIDTH + BA_LOW_BIT - 1 downto BA_LOW_BIT);
    alias a_column            : std_logic_vector(COL_WIDTH - 1 downto 0) is ocp_MAddr(COL_WIDTH + COL_LOW_BIT - 1 downto COL_LOW_BIT);

    -- Counters
    signal delay_cnt_nxt, delay_cnt_r                   : integer; -- range -1 to c_INIT_IDLE_CYCLES+c_REFRESH_CYCLES := 0; -- Don't care about the ranges now, just make the simulator run
    signal refresh_repeat_cnt_nxt, refresh_repeat_cnt_r : integer; -- range -1 to INIT_REFRESH_COUNT - 1 := 0;
    signal burst_cnt_nxt, burst_cnt_r                   : integer; -- range -1 to 7 := 0;
    signal delay_cnt_done                               : std_logic;
    signal refresh_repeat_cnt_done                      : std_logic;
    signal burst_cnt_done                               : std_logic;
    -- The DQ is saved in register during read, so need to delay the acknowledgment
    signal ocp_SResp_nxt                                : std_logic;
    signal ocp_SRespLast_nxt                            : std_logic;

begin
    sdram_CKE <= '1';
    sdram_DQ  <= sdram_DQout_r when sdram_DQoe_r = '1' else (others => 'Z');
    ocp_SData <= sdram_DQ_r;

    --! Registers
    reg : process(clk, rst) is
    begin
        if rst = '1' then
            state_r <= initWaitLock;
        elsif rising_edge(clk) then
            state_r              <= state_nxt;
            -- SDRAM i-face registers
            sdram_RAS_n          <= sdram_RAS_n_nxt;
            sdram_CAS_n          <= sdram_CAS_n_nxt;
            sdram_WE_n           <= sdram_WE_n_nxt;
            sdram_CS_n           <= sdram_CS_n_nxt;
            sdram_BA             <= sdram_BA_nxt;
            sdram_SA             <= sdram_SA_nxt;
            sdram_DQM            <= sdram_DQM_nxt;
            sdram_DQ_r           <= sdram_DQ;
            sdram_DQout_r        <= ocp_MData;
            sdram_DQoe_r         <= sdram_DQoe_nxt;
            ocp_SResp            <= ocp_SResp_nxt;
            ocp_SRespLast        <= ocp_SRespLast_nxt;
            -- Counters
            delay_cnt_r          <= delay_cnt_nxt;
            refresh_repeat_cnt_r <= refresh_repeat_cnt_nxt;
            burst_cnt_r          <= burst_cnt_nxt;
        end if;
    end process reg;

    refresh_repeat_cnt_done <= '1' when refresh_repeat_cnt_r = 0 else '0';
    delay_cnt_done          <= '1' when delay_cnt_r = 0 else '0';
    burst_cnt_done          <= '1' when burst_cnt_r = 0 else '0';

    --! State machine
    controller : process(a_bank, a_column, a_cs, a_row, burst_cnt_done, burst_cnt_r, delay_cnt_done, delay_cnt_r, ocp_MCmd, ocp_MCmd_doRefresh, ocp_MDataByteEn, pll_locked, refresh_repeat_cnt_done, refresh_repeat_cnt_r, state_r)
        function sl2int(bit : std_logic) return natural is
        begin
            if bit = '1' then
                return 1;
            else
                return 0;
            end if;
        end function sl2int;

    begin
        -- NOP
        sdram_RAS_n_nxt   <= '1';
        sdram_CAS_n_nxt   <= '1';
        sdram_WE_n_nxt    <= '1';
        -- all chips
        sdram_CS_n_nxt    <= (others => '0');
        -- row of the bank
        sdram_BA_nxt      <= a_bank;
        sdram_SA_nxt      <= a_row;
        -- Data Disabled/High-Z
        -- sdram_DQM_nxt   <= (others => '1');
        sdram_DQM_nxt     <= not ocp_MDataByteEn; -- TODO: handle masking by using tQMD and tDMD
        sdram_DQoe_nxt    <= '0';
        -- OCP acknowledge
        ocp_SCmdAccept    <= '0';
        ocp_SResp_nxt     <= '0';
        ocp_SRespLast_nxt <= '0';
        ocp_SDataAccept   <= '0';
        state_nxt         <= state_r;

        -- Counters
        delay_cnt_nxt          <= delay_cnt_r - sl2int(not delay_cnt_done);
        burst_cnt_nxt          <= burst_cnt_r - sl2int(not burst_cnt_done);
        -- Count only in special state
        refresh_repeat_cnt_nxt <= refresh_repeat_cnt_r;
        case state_r is
            when initWaitLock =>
                if pll_locked = '1' then
                    delay_cnt_nxt <= c_INIT_IDLE_CYCLES - 1;
                    state_nxt     <= initWaitIdle;
                end if;
            when initWaitIdle =>
                if delay_cnt_done = '1' then
                    state_nxt <= initPrecharge;
                end if;
            when initPrecharge =>
                sdram_RAS_n_nxt  <= '0';
                sdram_CAS_n_nxt  <= '1';
                sdram_WE_n_nxt   <= '0';
                sdram_SA_nxt(10) <= '1'; -- Precharge all banks
                sdram_CS_n_nxt   <= (others => '0'); -- All chips
                -- TODO: Add check for cnt constant beeing 0 and bypass the idle state
                -- TODO: Check if doing waiting in single state would improve the design
                -- TODO: The separate state machines will be used in pipelined version, so it might be better to do it differently
                delay_cnt_nxt    <= c_PRECHARGE_CYCLES - 2; -- (-1) because of counter implementation; extra (-1) because we stay idle during whole counting
                state_nxt        <= initPrechargeComplete;
            when initPrechargeComplete =>
                if delay_cnt_done = '1' then
                    refresh_repeat_cnt_nxt <= INIT_REFRESH_COUNT - 1;
                    state_nxt              <= initRefresh;
                end if;
            when initRefresh =>
                sdram_RAS_n_nxt        <= '0';
                sdram_CAS_n_nxt        <= '0';
                sdram_WE_n_nxt         <= '1';
                sdram_CS_n_nxt         <= (others => '0'); -- All chips
                refresh_repeat_cnt_nxt <= refresh_repeat_cnt_r - 1;
                delay_cnt_nxt          <= c_REFRESH_CYCLES - 2; -- (-1) because of counter implementation; extra (-1) because we stay idle during whole counting
                state_nxt              <= initRefreshComplete;
            when initRefreshComplete =>
                if delay_cnt_done = '1' then
                    if refresh_repeat_cnt_done = '1' then
                        state_nxt <= initProgramModeReg;
                    else
                        state_nxt <= initRefresh;
                    end if;
                end if;
            when initProgramModeReg =>
                sdram_RAS_n_nxt <= '0';
                sdram_CAS_n_nxt <= '0';
                sdram_WE_n_nxt  <= '0';
                sdram_BA_nxt    <= DefineModeRegister(BA_WIDTH + SA_WIDTH - 1 downto SA_WIDTH);
                sdram_SA_nxt    <= DefineModeRegister(SA_WIDTH - 1 downto 0);
                sdram_CS_n_nxt  <= (others => '0'); -- All chips
                delay_cnt_nxt   <= c_PROGRAM_REGISTER_CYCLES - 2; -- (-1) because of counter implementation; extra (-1) because we stay idle during whole counting
                state_nxt       <= initProgramModeRegComplete;
            when initProgramModeRegComplete =>
                if delay_cnt_done = '1' then
                    state_nxt <= ready;
                end if;
            when ready =>
                ocp_SCmdAccept <= '1';
                -- Read/Write/Refresh
                if ocp_MCmd = OCP_CMD_READ or ocp_MCmd = OCP_CMD_WRITE or ocp_MCmd_doRefresh = '1' then
                    -- Activate / Refresh
                    sdram_RAS_n_nxt <= '0';
                    sdram_CAS_n_nxt <= not ocp_MCmd_doRefresh; -- '0': Refresh; '1': Activate
                    sdram_WE_n_nxt  <= '1';
                    sdram_BA_nxt    <= a_bank;
                    sdram_SA_nxt    <= a_row;
                    sdram_CS_n_nxt  <= BinDecode_n(a_cs) and (sdram_CS_n_nxt'range => not ocp_MCmd_doRefresh); -- Refresh => All chips (Active LOW)

                    if ocp_MCmd_doRefresh = '1' then
                        delay_cnt_nxt <= c_REFRESH_CYCLES - 2; -- (-1) because of counter implementation; extra (-1) because we stay idle during whole counting
                        state_nxt     <= refreshComplete;
                    elsif ocp_MCmd = OCP_CMD_READ then
                        delay_cnt_nxt <= c_ACT2READ_CYCLES - 1; -- (-1) because of the counter implementation
                        state_nxt     <= readCmd;
                    else
                        delay_cnt_nxt <= c_ACT2WRITE_CYCLES - 1; -- (-1) because of the counter implementation
                        state_nxt     <= writeCmd;
                    end if;
                end if;
            when readCmd =>
                if delay_cnt_done = '1' then
                    -- Read with AutoPrecharge
                    sdram_RAS_n_nxt              <= '1';
                    sdram_CAS_n_nxt              <= '0';
                    sdram_WE_n_nxt               <= '1';
                    sdram_BA_nxt                 <= a_bank;
                    sdram_CS_n_nxt               <= BinDecode_n(a_cs);
                    sdram_SA_nxt(a_column'range) <= a_column;
                    sdram_SA_nxt(10)             <= '1'; --! auto precharge
                    if a_column'high >= 10 then
                        sdram_SA_nxt(a_column'high + 1 downto 11) <= a_column(a_column'high downto 10);
                    end if;

                    -- Schedule Read Data
                    delay_cnt_nxt <= tCAC_CYCLES - 2 + 1; -- (-1) because of counter implementation; extra (-1) because we stay idle during whole counting; (+1) because the sdram_DQ input is registered in IOB
                    state_nxt     <= readDataWait;
                end if;
            when readDataWait =>
                if delay_cnt_done = '1' then
                    burst_cnt_nxt <= BURST_LENGTH - 1; -- (-1) because of the counter implementation
                    state_nxt     <= readData;
                end if;
            when readData =>
                ocp_SResp_nxt <= '1';
                if burst_cnt_done = '1' then
                    ocp_SRespLast_nxt <= '1';
                    state_nxt         <= ready;
                end if;
            when writeCmd =>
                if delay_cnt_done = '1' then
                    -- Write with AutoPrecharge
                    sdram_RAS_n_nxt              <= '1';
                    sdram_CAS_n_nxt              <= '0';
                    sdram_WE_n_nxt               <= '0';
                    sdram_BA_nxt                 <= a_bank;
                    sdram_CS_n_nxt               <= BinDecode_n(a_cs);
                    sdram_SA_nxt(a_column'range) <= a_column;
                    sdram_SA_nxt(10)             <= '1'; --! auto precharge
                    if a_column'high >= 10 then
                        sdram_SA_nxt(a_column'high + 1 downto 11) <= a_column(a_column'high downto 10);
                    end if;

                    -- First word of data and schedule the rest
                    ocp_SDataAccept <= '1';
                    sdram_DQM_nxt   <= not ocp_MDataByteEn;
                    sdram_DQoe_nxt  <= '1';
                    burst_cnt_nxt   <= BURST_LENGTH - 2; -- (-1) because of counter implementation; extra (-1) because current state sends first word
                    state_nxt       <= writeDataRest;
                end if;
            when writeDataRest =>
                ocp_SDataAccept <= '1';
                sdram_DQM_nxt   <= not ocp_MDataByteEn;
                sdram_DQoe_nxt  <= '1';
                if burst_cnt_done = '1' then
                    delay_cnt_nxt <= c_WRITE2READY_CYCLES - 2; -- (-1) because of counter implementation; extra (-1) because we stay idle during whole counting
                    state_nxt     <= writePrechargeComplete;
                end if;
            when writePrechargeComplete =>
                if delay_cnt_done = '1' then
                    state_nxt <= ready;
                end if;
            when refreshComplete =>
                if delay_cnt_done = '1' then
                    state_nxt <= ready;
                end if;
        end case;
    end process controller;
end architecture RTL;
