-- ============================================================
-- File Name: ddio_out.vhd
-- Megafunction Name(s):
-- 			ALTDDIO_OUT
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

ENTITY ddio_out IS
	generic (width : natural);
	PORT
	(
		datain_h		: IN STD_LOGIC_VECTOR (width-1 DOWNTO 0);
		datain_l		: IN STD_LOGIC_VECTOR (width-1 DOWNTO 0);
		outclock		: IN STD_LOGIC ;
		dataout		: OUT STD_LOGIC_VECTOR (width-1 DOWNTO 0)
	);
END ddio_out;


ARCHITECTURE SYN OF ddio_out IS

	SIGNAL sub_wire0	: STD_LOGIC_VECTOR (width-1 DOWNTO 0);

BEGIN
	dataout    <= sub_wire0(width-1 DOWNTO 0);

	ALTDDIO_OUT_component : ALTDDIO_OUT
	GENERIC MAP (
		extend_oe_disable => "OFF",
		intended_device_family => "Stratix V",
		invert_output => "OFF",
		lpm_hint => "UNUSED",
		lpm_type => "altddio_out",
		oe_reg => "UNREGISTERED",
		power_up_high => "OFF",
		width => width
	)
	PORT MAP (
		datain_h => datain_h,
		datain_l => datain_l,
		outclock => outclock,
		dataout => sub_wire0
	);

END SYN;
