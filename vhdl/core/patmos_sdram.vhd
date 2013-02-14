-- 
-- Copyright 2010 Martin Schoeberl, martin@jopdesign.com. All rights reserved.
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
--
-- top level of the Leros CPU
-- That should be instanziated in a FPGA specific top level


library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.patmos_type_package.all;

use work.patmos_config_global.all;
use work.patmos_config.all;

use work.sdram_config.all;
use work.sdram_controller_interface.all;

entity patmos_sdram is
	port  (
			clk : in std_logic;
			led : out std_logic;
			txd : out std_logic;
			rxd : in std_logic
		    ;-- sdram I/O device controll interface
	        dma_addr_special_i : out std_logic;
	        dma_addr_i         : out std_logic_vector(3 downto 0);
	        dma_rd_i           : out std_logic;
	        dma_rd_data_i      : in  std_logic_vector(31 downto 0);
	        dma_wr_i           : out std_logic;
	        dma_wr_data_i      : out std_logic_vector(31 downto 0);
            -- Direct SDRAM access interface.
            -- Edgar: have it both here for now for integration testing.
            gm_slave           : in  SDRAM_controller_slave_type;
            gm_master          : out SDRAM_controller_master_type
	       );
end patmos_sdram;

architecture rtl of patmos_sdram is

	signal mem_write	: std_logic;
	signal mem_data_out_muxed : std_logic_vector(31 downto 0);
	signal pat_rst				: std_logic;
	signal data_mem_data_out	: std_logic_vector(31 downto 0);
	signal execute_dout		: execution_not_reg;

begin

    core : entity work.patmos_core
        port map(
            clk                => clk,
            rst                => pat_rst,
            mem_write          => mem_write,
            mem_data_out_muxed => mem_data_out_muxed,
            data_mem_data_out  => data_mem_data_out,
            execute_dout_core  => execute_dout,
            gm_slave           => gm_slave,
            gm_master          => gm_master
            );

    io : entity work.patmos_io_sdram
        generic map(
            USE_SDRAM => true
        )
        port map(
            clk                => clk,
            pat_rst            => pat_rst,
            mem_write          => mem_write,
            data_mem_data_out  => data_mem_data_out,
            mem_data_out_muxed => mem_data_out_muxed,
            execute_dout       => execute_dout,
            led                => led,
            txd                => txd,
            rxd                => rxd,
            dma_addr_special_i => dma_addr_special_i,
            dma_addr_i         => dma_addr_i,
            dma_rd_i           => dma_rd_i,
            dma_rd_data_i      => dma_rd_data_i,
            dma_wr_i           => dma_wr_i,
            dma_wr_data_i      => dma_wr_data_i
            );	
end rtl;
