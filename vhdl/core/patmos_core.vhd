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
-- Short descripton.
--
-- Author: Sahar Abbaspour
-- Author: Martin Schoeberl (martin@jopdesign.com)
--------------------------------------------------------------------------------


library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.patmos_type_package.all;

use work.sdram_config.all;
use work.sdram_controller_interface.all;

entity patmos_core is
	port(
		clk                			: in  std_logic;
		rst							:in  std_logic;
		mem_write					: in std_logic;
		mem_data_out_muxed			: in std_logic_vector(31 downto 0);
		data_mem_data_out			: out std_logic_vector(31 downto 0);
		execute_dout_core			: out execution_not_reg;
        -- SDRAM controller interface
        gm_slave           : in  SDRAM_controller_slave_type;
        gm_master          : out SDRAM_controller_master_type
	);
end entity patmos_core;

architecture arch of patmos_core is

	signal fetch_dout             : fetch_out_type;
	signal fetch_reg1, fetch_reg2 : std_logic_vector(4 downto 0);
	signal decode_din             : decode_in_type;
	signal decode_dout            : decode_out_type;
	signal execute_reg            : execution_reg;
	signal execute_not_reg        : execution_not_reg;

	signal mem_dout               : mem_out_type;

	begin               -- architecture begin

	------------------------------------------------------- fetch	

	fet : entity work.patmos_fetch
		port map(clk, rst, decode_dout, execute_not_reg, mem_dout, fetch_reg1, fetch_reg2, fetch_dout);
	-------------------------------------------------------- decode

	reg_file : entity work.patmos_register_file(arch)
		port map(clk,
			     rst,
			     fetch_reg1,
			     fetch_reg2,
			     execute_reg.write_back_reg,
			     decode_din.rs1_data_in,
			     decode_din.rs2_data_in,
			     mem_dout.data,
			     execute_reg.reg_write);

	decode_din.operation <= fetch_dout.instruction;
	decode_din.pc <= fetch_dout.pc;
	decode_din.instr_b   <= fetch_dout.instr_b;
	dec : entity work.patmos_decode(arch)
		port map(clk, rst, decode_din, mem_dout, decode_dout);

	---------------------------------------------------- execute

	alu: entity work.patmos_alu(arch)
	port map(clk, rst, decode_dout, execute_reg, execute_not_reg, mem_dout);
	execute_dout_core <= execute_not_reg;
	------------------------------------------------------- memory


	memory_stage : entity work.patmos_mem_stage(arch)
		port map(clk, rst, mem_write, mem_data_out_muxed, execute_reg, execute_not_reg, mem_dout, decode_dout, gm_slave, gm_master);

	data_mem_data_out <= mem_dout.data_mem_data_out;
end architecture arch;




