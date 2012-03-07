
--------------------------------------
-- determine the forwarding type
--------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.patmos_type_package.all;

entity forward_type_select is
  port
  (
    read_register                 : in unsigned(4 downto 0);
    alu_write_register            : in unsigned(4 downto 0);
    mem_write_register            : in unsigned(4 downto 0);
    alu_write_enable              : in std_logic;
    mem_write_enable              : in std_logic;
    fw_type                       : out forwarding_type
  );
end entity forward_type_select;

architecture arch of forward_type_select is
begin
  fw_type <= FWALU when (alu_write_register = read_register and alu_write_enable = '1' and alu_write_register /= "00000")
        else FWMEM when (mem_write_register = read_register and mem_write_enable = '1' and mem_write_register /= "00000")
        else FWNOP;
end arch;

---------------------------------
-- determine the forwarding value
---------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.patmos_type_package.all;

entity forward_value_select is
  port
  (
    fw_alu              : in unsigned(31 downto 0); --rs forwarded from previous alu
    fw_mem              : in unsigned(31 downto 0); --rs forwarded from data memory
    fw_in               : in unsigned(31 downto 0); --rs from register file
    fw_out              : out unsigned(31 downto 0);
    fw_ctrl             : in forwarding_type
  );
end entity forward_value_select;

architecture arch of forward_value_select is
begin
  fw_pro: process (fw_alu, fw_mem, fw_in, fw_ctrl)
  begin
    if fw_ctrl = FWALU  then
      fw_out <= fw_alu;
    elsif fw_ctrl = FWMEM then
      fw_out <= fw_mem;
    elsif fw_ctrl = FWNOP then
      fw_out <= fw_in;
    end if;
  end process;
end arch;

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.patmos_type_package.all;

entity patmos_forward is
  port
  (
    rs                     : in unsigned(4 downto 0); -- register exec reads
    rt                     : in unsigned(4 downto 0); -- register exec reads
    alu_we                 : in std_logic;
    mem_we                 : in std_logic;
    alu_wr_rn              : in unsigned(4 downto 0);
    mem_wr_rn              : in unsigned(4 downto 0);
    mux_fw_rs              : out forwarding_type;
    mux_fw_rt              : out forwarding_type
  );
end entity patmos_forward;

architecture arch of patmos_forward is
begin
  uut_rs: entity work.forward_type_select(arch)
	port map(rs, alu_wr_rn, mem_wr_rn, alu_we, mem_we, mux_fw_rs);
	
	uut_rt: entity work.forward_type_select(arch)
	port map(rt, alu_wr_rn, mem_wr_rn, alu_we, mem_we, mux_fw_rt);
	  
end arch;
