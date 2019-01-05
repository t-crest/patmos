----------------------------
--DO NOT MODIFY! This is an auto-generated file by argo.ArgoConfig$$anon$1
----------------------------
library ieee;
use ieee.std_logic_1164.all;
use work.config_types.all;

package config is

	constant SPM_ADDR_WIDTH : integer := 16;
	constant TARGET_ARCHITECTURE : ARCHITECTURES := FPGA;
	constant TARGET_IMPLEMENTATION : IMPLEMENTATIONS := SYNC;
	constant GATING_ENABLED : integer := 1;
	constant N : integer := 2; -- Horizontal width
	constant M : integer := 2; -- Vertical Height
	constant NODES : integer := 4;
	constant PRD_LENGTH : integer := 8; -- The number of timeslots in one TDM period

	constant PDELAY : time := 500 ps;
	constant NA_HPERIOD : time := 5 ns;
	constant P_HPERIOD : time := 5 ns;
	constant SKEW : time := 0 ns;
	constant delay : time := 0.3 ns;

end package ; -- aegean_def