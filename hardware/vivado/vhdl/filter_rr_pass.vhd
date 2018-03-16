-- -------------------------------------------------------------
--
-- Pass (no filtering) for reconfigurable region
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
--Module Architecture: filter_rr
----------------------------------------------------------------
ARCHITECTURE rtl OF filter_rr IS
 
    component pipe IS
   GENERIC(
        DATA_SIZE : natural;
        PIPE_LENGTH : natural
        );
   PORT( clk                             :   IN    std_logic; 
         clk_enable                      :   IN    std_logic; 
         reset                           :   IN    std_logic; 
         pipe_in                       :   IN    std_logic_vector(DATA_SIZE-1 DOWNTO 0);
         pipe_out                      :   OUT   std_logic_vector(DATA_SIZE-1 DOWNTO 0)
         );
    END component;

BEGIN

    filter_rr_pipe_inst_0 : pipe
	generic map(
		DATA_SIZE => 32,
	        PIPE_LENGTH => 51)
	port map(
               clk => clk,--                             :   IN    std_logic; 
               clk_enable => clk_enable,--                     :   IN    std_logic; 
               reset => reset,--                          :   IN    std_logic; 
               pipe_in => filter_in,--                      :   IN    std_logic_vector(15 DOWNTO 0); -- sfix16_En15
               pipe_out => filter_out--                     :   OUT   std_logic_vector(15 DOWNTO 0)  -- sfix16_En15
               );
  
END rtl;
