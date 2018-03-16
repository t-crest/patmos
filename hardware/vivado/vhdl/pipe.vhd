-- -------------------------------------------------------------
-- A pipe of length PIPE_LENGTH
-- Luca Pezzarossa
-- -------------------------------------------------------------
LIBRARY IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.ALL;

ENTITY pipe IS
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
END pipe;

ARCHITECTURE rtl OF pipe IS
  TYPE pipeline_type IS ARRAY (NATURAL range <>) OF std_logic_vector(DATA_SIZE-1 DOWNTO 0);
  SIGNAL pipeline                   : pipeline_type(0 TO PIPE_LENGTH-1);
  
BEGIN

  pipeline_process : PROCESS (clk)
  BEGIN
    IF clk'event AND clk = '1' THEN
      IF reset = '1' THEN
        pipeline(0 TO PIPE_LENGTH-1) <= (OTHERS => (OTHERS => '0'));
      ELSIF clk_enable = '1' THEN
        pipeline(0) <= pipe_in;
        pipeline(1 TO PIPE_LENGTH-1) <= pipeline(0 TO PIPE_LENGTH-2);
      END IF;
    END IF; 
  END PROCESS pipeline_process;

  pipe_out <= pipeline(PIPE_LENGTH-1);
END rtl;
