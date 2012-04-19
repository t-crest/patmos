--instructions should be divided to four memory banks as in mips... the memory implementation should
--be changed...
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity instruction_mem is
port
( 
   instruction_address                          : in std_logic_vector;
   instruction_in                               : in  std_logic_vector;
   instruction_out                              : out  std_logic_vector;
   rw                                           : in  std_logic
);
end entity instruction_mem;

architecture arch of instruction_mem is

   type memory is array (natural range <>, natural range <>) of std_logic;
   begin
      process
         constant instruction_mem_size : integer := 2 ** instruction_address'length;
         variable instruction_mem : memory(0 to instruction_mem_size-1, 7 downto 0);
      begin
         instruction_mem(0,0) := '0';instruction_mem(1,0) := '1';instruction_mem(2,0) := '0';instruction_mem(3,0) := '1';
         instruction_mem(0,1) := '0';instruction_mem(1,1) := '1';instruction_mem(2,1) := '0';instruction_mem(3,1) := '1';
         instruction_mem(0,2) := '0';instruction_mem(1,2) := '1';instruction_mem(2,2) := '0';instruction_mem(3,2) := '1';
         instruction_mem(0,3) := '0';instruction_mem(1,3) := '1';instruction_mem(2,3) := '0';instruction_mem(3,3) := '1';   
         instruction_mem(0,4) := '0';instruction_mem(1,4) := '1';instruction_mem(2,4) := '0';instruction_mem(3,4) := '1';  
         instruction_mem(0,5) := '0';instruction_mem(1,5) := '1';instruction_mem(2,5) := '0';instruction_mem(3,5) := '1';   
         instruction_mem(0,6) := '0';instruction_mem(1,6) := '1';instruction_mem(2,6) := '0';instruction_mem(3,6) := '1';
         instruction_mem(0,7) := '0';instruction_mem(1,7) := '1';instruction_mem(2,7) := '0';instruction_mem(3,7) := '1';
         instruction_mem(4,0) := '0';instruction_mem(5,0) := '1';instruction_mem(6,0) := '0';instruction_mem(7,0) := '1'; 
         instruction_mem(4,1) := '0';instruction_mem(5,1) := '1';instruction_mem(6,1) := '0';instruction_mem(7,1) := '1';
         instruction_mem(4,2) := '0';instruction_mem(5,2) := '1';instruction_mem(6,2) := '0';instruction_mem(7,2) := '1'; 
         instruction_mem(4,3) := '0';instruction_mem(5,3) := '1';instruction_mem(6,3) := '0';instruction_mem(7,3) := '1';
         instruction_mem(4,4) := '0';instruction_mem(5,4) := '1';instruction_mem(6,4) := '0';instruction_mem(7,4) := '1'; 
         instruction_mem(4,5) := '0';instruction_mem(5,5) := '1';instruction_mem(6,5) := '0';instruction_mem(7,5) := '1';  
         instruction_mem(4,5) := '0';instruction_mem(5,5) := '1';instruction_mem(6,5) := '0';instruction_mem(7,5) := '1'; 
         instruction_mem(4,6) := '0';instruction_mem(5,6) := '1';instruction_mem(6,6) := '0';instruction_mem(7,6) := '1'; 
         instruction_mem(4,7) := '0';instruction_mem(5,7) := '1';instruction_mem(6,7) := '0';instruction_mem(7,7) := '1'; 
         --
         instruction_mem(8,0) := '0';instruction_mem(9,0) := '1';instruction_mem(10,0) := '0';instruction_mem(11,0) := '1';
         instruction_mem(8,1) := '0';instruction_mem(9,1) := '1';instruction_mem(10,1) := '0';instruction_mem(11,1) := '1';
         instruction_mem(8,2) := '0';instruction_mem(9,2) := '1';instruction_mem(10,2) := '0';instruction_mem(11,2) := '1';
         instruction_mem(8,3) := '0';instruction_mem(9,3) := '1';instruction_mem(10,3) := '0';instruction_mem(11,3) := '1';   
         instruction_mem(8,4) := '0';instruction_mem(9,4) := '1';instruction_mem(10,4) := '0';instruction_mem(11,4) := '1';  
         instruction_mem(8,5) := '0';instruction_mem(9,5) := '1';instruction_mem(10,5) := '0';instruction_mem(11,5) := '1';   
         instruction_mem(8,6) := '0';instruction_mem(9,6) := '1';instruction_mem(10,6) := '0';instruction_mem(11,6) := '1';
         instruction_mem(8,7) := '0';instruction_mem(9,7) := '1';instruction_mem(10,7) := '0';instruction_mem(11,7) := '1';
         instruction_mem(12,0) := '0';instruction_mem(13,0) := '1';instruction_mem(14,0) := '0';instruction_mem(15,0) := '1'; 
         instruction_mem(12,1) := '0';instruction_mem(13,1) := '1';instruction_mem(14,1) := '0';instruction_mem(15,1) := '1';
         instruction_mem(12,2) := '0';instruction_mem(13,2) := '1';instruction_mem(14,2) := '0';instruction_mem(15,2) := '1'; 
         instruction_mem(12,3) := '0';instruction_mem(13,3) := '1';instruction_mem(14,3) := '0';instruction_mem(15,3) := '1';
         instruction_mem(12,4) := '0';instruction_mem(13,4) := '1';instruction_mem(14,4) := '0';instruction_mem(15,4) := '1'; 
         instruction_mem(12,5) := '0';instruction_mem(13,5) := '1';instruction_mem(14,5) := '0';instruction_mem(15,5) := '1';  
         instruction_mem(12,5) := '0';instruction_mem(13,5) := '1';instruction_mem(14,5) := '0';instruction_mem(15,5) := '1'; 
         instruction_mem(12,6) := '0';instruction_mem(13,6) := '1';instruction_mem(14,6) := '0';instruction_mem(15,6) := '1'; 
         instruction_mem(12,7) := '0';instruction_mem(13,7) := '1';instruction_mem(14,7) := '0';instruction_mem(15,7) := '1'; 
 
         if rw = '1' then
            for i in 0 to 7 loop
               instruction_out(i) <= instruction_mem(to_integer(unsigned(instruction_address)), i);
            end loop;
        else
            for i in 0 to 7 loop
               instruction_mem (to_integer(unsigned(instruction_address)), i) := instruction_in(i);     
            end loop;    
        end if;
         wait on rw, instruction_address, instruction_in; 
    end process;    
end architecture arch;   

