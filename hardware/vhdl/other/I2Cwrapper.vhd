-- I2C-wrapper for use with I2Controller.
-- This ensures that the SCL/SDA gates are correctly tri-stated
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity i2cwrapper is
  Port (
   sda, scl: inout std_logic;
   sda_topatmos: out std_logic;
   sda_frompatmos: in std_logic;
   scl_frompatmos: in std_logic;
   scl_topatmos: out std_logic;
   en: in std_logic
);
end i2cwrapper;

architecture Behavioral of i2cwrapper is

begin
    scl <= '0' WHEN (en='1' AND scl_frompatmos='0') ELSE 'Z';
    scl_topatmos <= scl;
    
    sda <= '0' when (en='1' AND sda_frompatmos='0') ELSE 'Z';
    sda_topatmos <= sda;
end Behavioral;
