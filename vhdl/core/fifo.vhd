-- 
--  Copyright (C) 2001-2008, Martin Schoeberl (martin@jopdesign.com)
-- This file is part of JOP and the time-predictable VLIW Patmos.
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
-- Author: Martin Schoeberl (martin@jopdesign.com)
--------------------------------------------------------------------------------


--
--	fifo.vhd
--
--	simple fifo
--
--	uses FF and every rd or wr has to 'bubble' through the hole fifo.
--	
--	Author: Martin Schoeberl	martin.schoeberl@chello.at
--
--
--	resources on ACEX1K
--
--		(width+2)*depth-1 LCs
--
--
--	2002-01-06	first working version
--	2002-11-03	a signal for reaching threshold
--	2005-02-20	change entity order for modelsim vcom
--

library ieee;
use ieee.std_logic_1164.all;

entity fifo_elem is

generic (width : integer);
port (
	clk		: in std_logic;
	reset	: in std_logic;

	din		: in std_logic_vector(width-1 downto 0);
	dout	: out std_logic_vector(width-1 downto 0);

	rd		: in std_logic;
	wr		: in std_logic;

	rd_prev	: out std_logic;
	full	: out std_logic
);
end fifo_elem;

architecture rtl of fifo_elem is

	signal buf		: std_logic_vector(width-1 downto 0);
	signal f		: std_logic;

begin

	dout <= buf;

process(clk, reset, f)

begin

	full <= f;

	if (reset='1') then

		buf <= (others => '0');
		f <= '0';
		rd_prev <= '0';

	elsif rising_edge(clk) then

		rd_prev <= '0';
		if f='0' then
			if wr='1' then
				rd_prev <= '1';
				buf <= din;
				f <= '1';
			end if;
		else
			if rd='1' then
				f <= '0';
			end if;
		end if;

	end if;

end process;

end rtl;

library ieee;
use ieee.std_logic_1164.all;

entity fifo is

generic (width : integer := 8; depth : integer := 4; thres : integer := 2);
port (
	clk		: in std_logic;
	reset	: in std_logic;

	din		: in std_logic_vector(width-1 downto 0);
	dout	: out std_logic_vector(width-1 downto 0);

	rd		: in std_logic;
	wr		: in std_logic;

	empty	: out std_logic;
	full	: out std_logic;
	half	: out std_logic
);
end fifo ;

architecture rtl of fifo is

component fifo_elem is

generic (width : integer);
port (
	clk		: in std_logic;
	reset	: in std_logic;

	din		: in std_logic_vector(width-1 downto 0);
	dout	: out std_logic_vector(width-1 downto 0);

	rd		: in std_logic;
	wr		: in std_logic;

	rd_prev	: out std_logic;
	full	: out std_logic
);
end component;

	signal r, w, rp, f	: std_logic_vector(depth-1 downto 0);
	type d_array is array (0 to depth-1) of std_logic_vector(width-1 downto 0);
	signal di, do		: d_array;
	
	

begin


	g1: for i in 0 to depth-1 generate

		f1: fifo_elem generic map (width)
			port map (clk, reset, di(i), do(i), r(i), w(i), rp(i), f(i));

		x: if i<depth-1 generate
			r(i) <= rp(i+1);
			w(i+1) <= f(i);
			di(i+1) <= do(i);
		end generate;

	end generate;

	di(0) <= din;
	dout <= do(depth-1);
	w(0) <= wr;
	r(depth-1) <= rd;

	full <= f(0);
	half <= f(depth-thres);
	empty <= not f(depth-1);
	
end rtl;

