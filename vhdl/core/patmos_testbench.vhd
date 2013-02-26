-- 
-- Copyright Technical University of Denmark. All rights reserved.
-- This file is part of the time-predictable VLIW Patmos.
-- 
-- Redistribution and use in source and binary forms, with or without
-- modification, are permitted provided that the following conditions are met:
-- 
--    1. Redistributions of source code must retain the above copyright notice,
--       this list of conditions and the following disclaimer.
-- 
--    2. Redistributions in binary form must reproduce the above copyright
--       notice, this list of conditions and the following disclaimer in the
--       documentation and/or other materials provided with the distribution.
-- 
-- THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
-- OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
-- OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
-- NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
-- DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
-- (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
-- LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
-- ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
-- (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
-- THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
-- 
-- The views and conclusions contained in the software and documentation are
-- those of the authors and should not be interpreted as representing official
-- policies, either expressed or implied, of the copyright holder.
-- 


--------------------------------------------------------------------------------
-- Single core test bench.
-- MS: Test bench does not belong into the core folder.
-- MS: there shall be a simple test bench without SDRAM and one with
--
-- Author: Sahar Abbaspour
--------------------------------------------------------------------------------

use std.textio.all;

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.patmos_type_package.all;

entity patmos_testbench is
end entity patmos_testbench;

use work.patmos_config.all;
--use work.sdram_config.all;
--use work.sdram_controller_interface.all;

architecture timed of patmos_testbench is
    signal clk          : std_logic := '1';
    signal led          : std_logic;
    signal txd          : std_logic;
    signal rxd          : std_logic;
    signal oSRAM_A      : std_logic_vector(18 downto 0); -- edit
    signal SRAM_DQ      : std_logic_vector(31 downto 0); -- edit
    signal oSRAM_CE1_N  : std_logic;
    signal oSRAM_OE_N   : std_logic;
    signal oSRAM_BE_N   : std_logic_vector(3 downto 0);
    signal oSRAM_WE_N   : std_logic;
    signal oSRAM_GW_N   : std_logic;
    signal oSRAM_CLK    : std_logic;
    signal oSRAM_ADSC_N : std_logic;
    signal oSRAM_ADSP_N : std_logic;
    signal oSRAM_ADV_N  : std_logic;
    signal oSRAM_CE2    : std_logic;
    signal oSRAM_CE3_N  : std_logic;
    signal internal_rst : std_logic;

--    signal gm_master : SDRAM_controller_master_type;
--    signal gm_slave  : SDRAM_controller_slave_type;

--    function if_then_else(cond : boolean; true_val : natural; false_val : natural) return natural is
--    begin
--        if cond then
--            return true_val;
--        else
--            return false_val;
--        end if;
--    end function if_then_else;
--
--    constant BURST_LENGTH : natural           := if_then_else(USE_GLOBAL_MEMORY_SDRAM, 1, 8);
--    -- 100MHz, 2 cycles read data latency
    constant tCLK_PERIOD  : time              := 20 ns;
--    constant CAS_LATENCY  : natural           := 2;
--    constant SDRAM        : sdram_config_type := GetSDRAMParameters(tCLK_PERIOD, CAS_LATENCY);
--
--    -- Address Mapping => (bank & row & column)
--    constant CS_WIDTH    : integer := 0; -- 1 rank
--    constant COL_LOW_BIT : integer := 0;
--    constant ROW_LOW_BIT : integer := COL_LOW_BIT + SDRAM.COL_WIDTH; -- 9
--    constant BA_LOW_BIT  : integer := ROW_LOW_BIT + SDRAM.ROW_WIDTH; -- 9+13=22
--    constant CS_LOW_BIT  : integer := BA_LOW_BIT + SDRAM.BA_WIDTH; -- 22+2=24
--
--    --===========================================================
--    -- Timing parameters for IS42S16160B: speed grade -7, tCL=3
--    --===========================================================
--    constant tOH         : TIME    := 3.0 ns;
--    constant tMRD_CYCLES : INTEGER := 2; -- 2 Clk Cycles
--    constant tRAS        : TIME    := SDRAM.RAS * tCLK_PERIOD;
--    constant tRC         : TIME    := SDRAM.RC * tCLK_PERIOD;
--    constant tRCD        : TIME    := SDRAM.RCD * tCLK_PERIOD;
--    constant tRP         : TIME    := SDRAM.RP * tCLK_PERIOD;
--    constant tRRD        : TIME    := SDRAM.RRD * tCLK_PERIOD;
--    constant tWRa        : TIME    := 6.0 ns - 6 ns; -- A2 Version - Auto precharge mode only (1 Clk + 6 ns)
--    constant tWRp        : TIME    := 20 ns + 14.0 ns; -- A2 Version - Precharge mode only (12 ns)
--    constant tCH         : TIME    := 2.5 ns;
--    constant tCL         : TIME    := 2.5 ns;
--    constant tCK         : TIME    := 10 ns;
--    constant tAS         : TIME    := 2 ns;
--    constant tDS         : TIME    := 2 ns;
--    constant tCKS        : TIME    := tDS;
--    constant tCMS        : TIME    := tDS;
--    constant tDH         : TIME    := 0 ns; --1 ns; -- We use 0 delay behavioural model, so Hold violation checks are disabled
--    constant tCKH        : TIME    := tDH;
--    constant tCMH        : TIME    := tDH;

    signal rst          : std_logic;
--    signal sdram_sa     : std_logic_vector(SDRAM.SA_WIDTH - 1 downto 0);
--    signal sdram_ba     : std_logic_vector(SDRAM.BA_WIDTH - 1 downto 0);
--    signal sdram_cs_n   : std_logic_vector(2 ** CS_WIDTH - 1 downto 0);
--    signal sdram_cke    : std_logic;
--    signal sdram_ras_n  : std_logic;
--    signal sdram_cas_n  : std_logic;
--    signal sdram_we_n   : std_logic;
--    signal sdram_dq     : std_logic_vector(SDRAM_DATA_WIDTH - 1 downto 0);
--    signal sdram_dqm    : std_logic_vector(SDRAM_DATA_WIDTH / 8 - 1 downto 0);
--    signal sdram_dq_out : STD_LOGIC_VECTOR(SDRAM_DATA_WIDTH - 1 DOWNTO 0);
--    signal sdram_dq_dir : STD_LOGIC_VECTOR(SDRAM_DATA_WIDTH / 8 - 1 downto 0);

begin
    core : entity work.patmos(rtl)
        port map(clk, led, txd, rxd); --, gm_slave, gm_master);

    --  , oSRAM_A, SRAM_DQ, oSRAM_CE1_N
    --  	, oSRAM_OE_N, oSRAM_BE_N, oSRAM_WE_N, oSRAM_GW_N, oSRAM_CLK,
    --  	oSRAM_ADSC_N, oSRAM_ADSP_N, oSRAM_ADV_N, oSRAM_CE2, oSRAM_CE3_N
    --  );  

    -- The SDRAM Controller
--    sdr_sdram_inst : entity work.sdr_sdram
--        generic map(SHORT_INITIALIZATION => true,
--                    USE_AUTOMATIC_REFRESH => true,
--                    BURST_LENGTH          => BURST_LENGTH,
--                    SDRAM                 => SDRAM,
--                    CS_WIDTH              => CS_WIDTH,
--                    CS_LOW_BIT            => CS_LOW_BIT,
--                    BA_LOW_BIT            => BA_LOW_BIT,
--                    ROW_LOW_BIT           => ROW_LOW_BIT,
--                    COL_LOW_BIT           => COL_LOW_BIT)
--        port map(rst         => rst,
--                 clk         => clk,
--                 pll_locked  => '1',
--                 ocpSlave    => gm_slave,
--                 ocpMaster   => gm_master,
--                 sdram_CKE   => sdram_CKE,
--                 sdram_RAS_n => sdram_RAS_n,
--                 sdram_CAS_n => sdram_CAS_n,
--                 sdram_WE_n  => sdram_WE_n,
--                 sdram_CS_n  => sdram_CS_n,
--                 sdram_BA    => sdram_BA,
--                 sdram_SA    => sdram_SA,
--                 sdram_DQ    => sdram_DQ,
--                 sdram_DQM   => sdram_DQM);
--
--    -- SDRAM simulation model
--    -- Edgar: TODO: need to use some open-source alternative
--
--    chips : for i in 0 to 0 generate
--        B0 : entity work.mt48lc2m32b2
--            generic map(
--                tOH       => tOH,
--                tMRD      => tMRD_CYCLES,
--                tRAS      => tRAS,
--                tRC       => tRC,
--                tRCD      => tRCD,
--                tRP       => tRP,
--                tRRD      => tRRD,
--                tWRa      => tWRa,
--                tWRp      => tWRp,
--                tAS       => tAS,
--                tCH       => tCH,
--                tCL       => tCL,
--                tCK       => tCK,
--                tDH       => tDH,
--                tDS       => tDS,
--                tCKH      => tCKH,
--                tCKS      => tCKS,
--                tCMH      => tCMH,
--                tCMS      => tCMS,
--                addr_bits => SDRAM.ROW_WIDTH,
--                data_bits => SDRAM_DATA_WIDTH,
--                col_bits  => SDRAM.COL_WIDTH)
--            port map(
--                Dq_in  => sdram_dq'delayed(tCLK_PERIOD / 10), -- Edgar: signals are delayed, as a workaround for clock gating inside the SDRAM model
--                Dq_out => sdram_dq_out,
--                Dq_dir => sdram_dq_dir,
--                Addr   => sdram_sa'delayed(tCLK_PERIOD / 10),
--                Ba     => sdram_Ba'delayed(tCLK_PERIOD / 10),
--                Clk    => clk,
--                Cke    => sdram_cke'delayed(tCLK_PERIOD / 10),
--                Cs_n   => sdram_cs_n(i)'delayed(tCLK_PERIOD / 10),
--                Ras_n  => sdram_ras_n'delayed(tCLK_PERIOD / 10),
--                Cas_n  => sdram_cas_n'delayed(tCLK_PERIOD / 10),
--                We_n   => sdram_We_n'delayed(tCLK_PERIOD / 10),
--                Dqm    => sdram_Dqm'delayed(tCLK_PERIOD / 10)
--            );
--    end generate chips;
--
--    -- Delayed tri-state:
--    gen_delay_dq_out : for i in sdram_dq_dir'range generate
--        sdram_dq((i + 1) * 8 - 1 downto i * 8) <= sdram_dq_out((i + 1) * 8 - 1 downto i * 8)'delayed(tCLK_PERIOD / 10) when sdram_dq_dir'delayed(tCLK_PERIOD / 10)(i) = '1' else (others => 'Z');
--    end generate gen_delay_dq_out;

    clk <= not clk after tCLK_PERIOD/2;
    rst <= '1', '0' after tCLK_PERIOD*1.1;

    process
        variable l : line;
    begin
        write(l, string'("Patmos start"));
        writeline(output, l);
        wait;
    end process;

end architecture timed;