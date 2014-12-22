-- ============================================================
-- File Name: ddio_in.vhd
-- Megafunction Name(s):
-- 			ALTDDIO_IN
--
-- Simulation Library Files(s):
-- 			altera_mf
-- ============================================================

--Copyright (C) 1991-2014 Altera Corporation. All rights reserved.
--Your use of Altera Corporation's design tools, logic functions 
--and other software and tools, and its AMPP partner logic 
--functions, and any output files from any of the foregoing 
--(including device programming or simulation files), and any 
--associated documentation or information are expressly subject 
--to the terms and conditions of the Altera Program License 
--Subscription Agreement, the Altera Quartus II License Agreement,
--the Altera MegaCore Function License Agreement, or other 
--applicable license agreement, including, without limitation, 
--that your use is for the sole purpose of programming logic 
--devices manufactured by Altera and sold by Altera or its 
--authorized distributors.  Please refer to the applicable 
--agreement for further details.


LIBRARY ieee;
USE ieee.std_logic_1164.all;

LIBRARY altera_mf;
USE altera_mf.altera_mf_components.all;

ENTITY ddio_in IS
	generic (width : natural);
	PORT
	(
		datain		: IN STD_LOGIC_VECTOR (width-1 DOWNTO 0);
		inclock		: IN STD_LOGIC ;
		dataout_h		: OUT STD_LOGIC_VECTOR (width-1 DOWNTO 0);
		dataout_l		: OUT STD_LOGIC_VECTOR (width-1 DOWNTO 0)
	);
END ddio_in;


ARCHITECTURE SYN OF ddio_in IS

	SIGNAL sub_wire0	: STD_LOGIC_VECTOR (width-1 DOWNTO 0);
	SIGNAL sub_wire1	: STD_LOGIC_VECTOR (width-1 DOWNTO 0);

BEGIN
	dataout_h    <= sub_wire0(width-1 DOWNTO 0);
	dataout_l    <= sub_wire1(width-1 DOWNTO 0);

	ALTDDIO_IN_component : ALTDDIO_IN
	GENERIC MAP (
		intended_device_family => "Stratix V",
		invert_input_clocks => "OFF",
		lpm_hint => "UNUSED",
		lpm_type => "altddio_in",
		power_up_high => "OFF",
		width => width
	)
	PORT MAP (
		datain => datain,
		inclock => inclock,
		dataout_h => sub_wire0,
		dataout_l => sub_wire1
	);

END SYN;
