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
--------------------------------------------------------------------------------
-- Entity: patmos_types
--------------------------------------------------------------------------------
-- Copyright ... 2010
-- Filename          : patmos_types.vhd
-- Creation date     : 2010-08-10
-- Author(s)         : martin
-- Version           : 1.00
-- Description       : <short description>
--------------------------------------------------------------------------------
-- File History:
-- Date         Version  Author   Comment
-- 2010-08-10   1.00     martin     Creation of File
--------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

package patmos_types is

	constant ALU_WIDTH : integer := 32;

	type alu_in_type is record
		opa			: std_logic_vector(ALU_WIDTH-1 downto 0);
		opb			: std_logic_vector(ALU_WIDTH-1 downto 0);
		add			: std_logic;
	end record;

	type alu_out_type is record
		result		: std_logic_vector(ALU_WIDTH-1 downto 0);
	end record;

	type io_out_type is record
		addr : std_logic_vector(7 downto 0);
		rd : std_logic;
		wr : std_logic;
		wrdata : std_logic_vector(15 downto 0);
	end record;

	type io_in_type is record
		rddata : std_logic_vector(15 downto 0);
	end record;
end package;

