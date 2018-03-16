-- -------------------------------------------------------------
--
-- Band-pass for reconfigurable region
--
-- -------------------------------------------------------------

LIBRARY IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.ALL;

ENTITY filter_rr IS
   PORT( clk                             :   IN    std_logic; 
         clk_enable                      :   IN    std_logic; 
         reset                           :   IN    std_logic; 
         filter_in                       :   IN    std_logic_vector(31 DOWNTO 0); -- sfix16_En15
         filter_out                      :   OUT   std_logic_vector(31 DOWNTO 0)  -- sfix16_En15
         );

END filter_rr;


----------------------------------------------------------------
--Module Architecture: filter_bp
----------------------------------------------------------------
ARCHITECTURE rtl OF filter_rr IS
 
    component filter_bp IS
   PORT( clk                             :   IN    std_logic; 
         clk_enable                      :   IN    std_logic; 
         reset                           :   IN    std_logic; 
         filter_in                       :   IN    std_logic_vector(15 DOWNTO 0); -- sfix16_En15
         filter_out                      :   OUT   std_logic_vector(15 DOWNTO 0)  -- sfix16_En15
         );
    END component;

BEGIN

    filter_rr_bp_inst_0 : filter_bp
        port map(
               clk => clk,--                             :   IN    std_logic; 
               clk_enable => clk_enable,--                     :   IN    std_logic; 
               reset => reset,--                          :   IN    std_logic; 
               filter_in => filter_in(31 downto 16),--                      :   IN    std_logic_vector(15 DOWNTO 0); -- sfix16_En15
               filter_out => filter_out(31 downto 16)--                     :   OUT   std_logic_vector(15 DOWNTO 0)  -- sfix16_En15
               );
  
      filter_rr_bp_inst_1 : filter_bp
        port map(
               clk => clk,--                             :   IN    std_logic; 
            clk_enable => clk_enable,--                     :   IN    std_logic; 
             reset => reset,--                          :   IN    std_logic; 
               filter_in => filter_in(15 downto 0),--                      :   IN    std_logic_vector(15 DOWNTO 0); -- sfix16_En15
               filter_out => filter_out(15 downto 0)--                     :   OUT   std_logic_vector(15 DOWNTO 0)  -- sfix16_En15
               ); 

END rtl;
