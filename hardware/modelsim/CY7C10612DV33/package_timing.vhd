--************************************************************************
--**    MODEL   :       	package_timing.vhd                          **
--**    COMPANY :       	Cypress Semiconductor                       **
--**	PACKAGING INFO : 	10ZSXI										**
--**    REVISION:       	1.0 (Created new timing package model)      **
--************************************************************************


library IEEE,std;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_arith.all;
use IEEE.std_logic_unsigned.all;
use ieee.std_logic_textio.all ;
use std.textio.all ;

--****************************************************************

package package_timing is

------------------------------------------------------------------------------------------------
-- Read Cycle timing
------------------------------------------------------------------------------------------------
constant    tRC    :   TIME    :=   10 ns;    --   Read Cycle Time
constant    tAA    :   TIME    :=   10 ns;    --   Address to Data Valid
constant    tOHA   :   TIME    :=   2.5 ns;   --   Data Hold from Address Change
constant    tACE   :   TIME    :=   10 ns;    --   Random access CEb Low to Data Valid
constant    tDOE   :   TIME    :=   6.5 ns;   --   OE Low to Data Valid
constant    tLZOE  :   TIME    :=   0 ns;     --   OE Low to LOW Z
constant    tHZOE  :   TIME    :=   4 ns;     --   OE High to HIGH Z
constant    tLZCE  :   TIME    :=   3 ns;     --   CEb LOW to LOW Z
constant    tHZCE  :   TIME    :=   4 ns;     --   CEb HIGH to HIGH Z

constant    tDBE   :   TIME    :=   5 ns;
constant    tLZBE  :   TIME    :=   0 ns;
constant    tHZBE  :   TIME    :=   3 ns;


------------------------------------------------------------------------------------------------
-- Write Cycle timing
------------------------------------------------------------------------------------------------
constant    tWC    :   TIME    :=   10 ns;    --   Write Cycle Time
constant    tSCE   :   TIME    :=   8 ns;     --   CEb LOW to Write End
constant    tAW    :   TIME    :=   8 ns;     --   Address Setup to Write End
constant    tSA    :   TIME    :=   0 ns;     --   Address Setup to Write Start
constant    tHA    :   TIME    :=   0 ns;     --   Address Hold from Write End
constant    tPWE   :   TIME    :=   10 ns;    --   WEb pulse width
constant    tSD    :   TIME    :=   6 ns;     --   Data Setup to Write End
constant    tHD    :   TIME    :=   0 ns;     --   Data Hold from Write End
constant    tLZWE  :   TIME    :=   2 ns;     --   WEb Low to High Z
constant    tHZWE  :   TIME    :=   5 ns;     --   WEb High to Low Z
constant	tBW	   :   TIME	   :=	7 ns;
end package_timing;


package body package_timing is

end package_timing;















