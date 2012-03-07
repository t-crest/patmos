--TO DO: 
-- add forwading values to input multiplexers of ALU
-- replcae pd with predicate_reg(pd) (number of predicate register that should be written with 0 or 1)

library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use work.patmos_type_package.all;


entity patmos_execute is
  port
  (
    clk                             : in std_logic;
    inst_type                       : in instruction_type;
    ALU_function_type               : in unsigned(3 downto 0);
    ALU_instruction_type            : in ALU_inst_type;
    ALUi_immediate                  : in unsigned(11 downto 0);
    rs                              : in unsigned(31 downto 0);
    rt                              : in unsigned(31 downto 0);
    pd                              : out unsigned(2 downto 0);  -- this is the index of predicate bit, ALUp instructions write in predicate bits and others use them!
    rd                              : out unsigned(31 downto 0);
    wb_we                           : in std_logic;
    wb_we_out_exec                  : out std_logic
   -- func                            : in unsigned(3 downto 0)-- which function of ALUr?
 --   write_enable      : out std_logic
     );
  
end entity patmos_execute;

architecture arch of patmos_execute is


--shift left logical
function shift_left_logical (rs, rt : unsigned(31 downto 0))
                 return unsigned is
  variable shift_out  : unsigned(31 downto 0):= (others => '0');
  variable shift_value  : unsigned(4 downto 0):= (others => '0');
begin
  shift_value(4 downto 0 ) := rt(4 downto 0);
  case (shift_value) is
      when "00000" => shift_out := rs;
      when "00001" => shift_out := rs(31 downto 1) & '0'; 
      when "00010" => shift_out := rs(31 downto 2) & "00";   
      when "00011" => shift_out := rs(31 downto 3) & "000"; 
      when "00100" => shift_out := rs(31 downto 4) & "0000"; 
      when "00101" => shift_out := rs(31 downto 5) & "00000"; 
      when "00110" => shift_out := rs(31 downto 6) & "000000"; 
      when "00111" => shift_out := rs(31 downto 7) & "0000000"; 
      when "01000" => shift_out := rs(31 downto 8) & "00000000"; 
      when "01001" => shift_out := rs(31 downto 9) & "000000000"; 
      when "01010" => shift_out := rs(31 downto 10) & "0000000000"; 
      when "01011" => shift_out := rs(31 downto 11) & "00000000000"; 
      when "01100" => shift_out := rs(31 downto 12) & "000000000000"; 
      when "01101" => shift_out := rs(31 downto 13) & "0000000000000"; 
      when "01110" => shift_out := rs(31 downto 14) & "00000000000000"; 
      when "01111" => shift_out := rs(31 downto 15) & "000000000000000"; 
      when "10000" => shift_out := rs(31 downto 16) & "0000000000000000"; 
      when "10001" => shift_out := rs(31 downto 17) & "00000000000000000"; 
      when "10010" => shift_out := rs(31 downto 18) & "000000000000000000";   
      when "10011" => shift_out := rs(31 downto 19) & "0000000000000000000"; 
      when "10100" => shift_out := rs(31 downto 20) & "00000000000000000000"; 
      when "10101" => shift_out := rs(31 downto 21) & "000000000000000000000"; 
      when "10110" => shift_out := rs(31 downto 22) & "0000000000000000000000"; 
      when "10111" => shift_out := rs(31 downto 23) & "00000000000000000000000"; 
      when "11000" => shift_out := rs(31 downto 24) & "000000000000000000000000"; 
      when "11001" => shift_out := rs(31 downto 25) & "0000000000000000000000000"; 
      when "11010" => shift_out := rs(31 downto 26) & "00000000000000000000000000"; 
      when "11011" => shift_out := rs(31 downto 27) & "000000000000000000000000000"; 
      when "11100" => shift_out := rs(31 downto 28) & "0000000000000000000000000000"; 
      when "11101" => shift_out := rs(31 downto 29) & "00000000000000000000000000000"; 
      when "11110" => shift_out := rs(31 downto 30) & "000000000000000000000000000000"; 
      when "11111" => shift_out := rs(31 downto 31) & "0000000000000000000000000000000"; 
      when  others => shift_out := rs;
   end case;
  return shift_out ;
end shift_left_logical;

function shift_right_logical (rs, rt : unsigned(31 downto 0))
                 return unsigned is
  variable shift_out  : unsigned(31 downto 0):= (others => '0');
  variable shift_value  : unsigned(4 downto 0):= (others => '0');
begin
  shift_value(4 downto 0 ) := rt(4 downto 0);
  case (shift_value) is
      when "00000" => shift_out := rs;
      when "00001" => shift_out :=  '0' & rs(31 downto 1); 
      when "00010" => shift_out :=  "00" & rs(31 downto 2);   
      when "00011" => shift_out :=  "000" & rs(31 downto 3); 
      when "00100" => shift_out :=  "0000" & rs(31 downto 4); 
      when "00101" => shift_out :=  "00000" & rs(31 downto 5); 
      when "00110" => shift_out :=  "000000" & rs(31 downto 6); 
      when "00111" => shift_out :=  "0000000" & rs(31 downto 7); 
      when "01000" => shift_out :=  "00000000" & rs(31 downto 8); 
      when "01001" => shift_out :=  "000000000" & rs(31 downto 9) ; 
      when "01010" => shift_out :=  "0000000000" & rs(31 downto 10); 
      when "01011" => shift_out :=  "00000000000" & rs(31 downto 11); 
      when "01100" => shift_out :=  "000000000000" & rs(31 downto 12); 
      when "01101" => shift_out :=  "0000000000000" & rs(31 downto 13); 
      when "01110" => shift_out :=  "00000000000000" & rs(31 downto 14); 
      when "01111" => shift_out :=  "000000000000000" & rs(31 downto 15); 
      when "10000" => shift_out :=  "0000000000000000" & rs(31 downto 16); 
      when "10001" => shift_out :=  "00000000000000000" & rs(31 downto 17); 
      when "10010" => shift_out :=  "000000000000000000" & rs(31 downto 18);   
      when "10011" => shift_out :=  "0000000000000000000" & rs(31 downto 19); 
      when "10100" => shift_out :=  "00000000000000000000" & rs(31 downto 20); 
      when "10101" => shift_out :=  "000000000000000000000" & rs(31 downto 21); 
      when "10110" => shift_out :=  "0000000000000000000000" & rs(31 downto 22); 
      when "10111" => shift_out :=  "00000000000000000000000" & rs(31 downto 23); 
      when "11000" => shift_out :=  "000000000000000000000000" & rs(31 downto 24); 
      when "11001" => shift_out :=  "0000000000000000000000000" & rs(31 downto 25); 
      when "11010" => shift_out :=  "00000000000000000000000000" & rs(31 downto 26); 
      when "11011" => shift_out :=  "000000000000000000000000000" & rs(31 downto 27); 
      when "11100" => shift_out :=  "0000000000000000000000000000" & rs(31 downto 28); 
      when "11101" => shift_out :=  "00000000000000000000000000000" & rs(31 downto 29); 
      when "11110" => shift_out :=  "000000000000000000000000000000" & rs(31 downto 30); 
      when "11111" => shift_out :=  "0000000000000000000000000000000" & rs(31 downto 31); 
      when  others => shift_out := rs;
   end case;
  return shift_out ;
end shift_right_logical;

function shift_right_arith (rs, rt : unsigned(31 downto 0))
                 return unsigned is
  variable shift_out  : unsigned(31 downto 0):= (others => '0');
  variable shift_value  : unsigned(4 downto 0):= (others => '0');
begin
  shift_value(4 downto 0 ) := rt(4 downto 0);
  case (shift_value) is
      when "00000" => shift_out := rs;
      when "00001" => shift_out :=  rs(31) & rs(31 downto 1); 
      when "00010" => shift_out :=  rs(31 downto 30) & rs(31 downto 2);   
      when "00011" => shift_out :=  rs(31 downto 29) & rs(31 downto 3); 
      when "00100" => shift_out :=  rs(31 downto 28) & rs(31 downto 4); 
      when "00101" => shift_out :=  rs(31 downto 27) & rs(31 downto 5); 
      when "00110" => shift_out :=  rs(31 downto 26) & rs(31 downto 6); 
      when "00111" => shift_out :=  rs(31 downto 25) & rs(31 downto 7); 
      when "01000" => shift_out :=  rs(31 downto 24) & rs(31 downto 8); 
      when "01001" => shift_out :=  rs(31 downto 23) & rs(31 downto 9) ; 
      when "01010" => shift_out :=  rs(31 downto 22) & rs(31 downto 10); 
      when "01011" => shift_out :=  rs(31 downto 21) & rs(31 downto 11); 
      when "01100" => shift_out :=  rs(31 downto 20) & rs(31 downto 12); 
      when "01101" => shift_out :=  rs(31 downto 19) & rs(31 downto 13); 
      when "01110" => shift_out :=  rs(31 downto 18) & rs(31 downto 14); 
      when "01111" => shift_out :=  rs(31 downto 17) & rs(31 downto 15); 
      when "10000" => shift_out :=  rs(31 downto 16) & rs(31 downto 16); 
      when "10001" => shift_out :=  rs(31 downto 15) & rs(31 downto 17); 
      when "10010" => shift_out :=  rs(31 downto 14) & rs(31 downto 18);   
      when "10011" => shift_out :=  rs(31 downto 13) & rs(31 downto 19); 
      when "10100" => shift_out :=  rs(31 downto 12) & rs(31 downto 20); 
      when "10101" => shift_out :=  rs(31 downto 11) & rs(31 downto 21); 
      when "10110" => shift_out :=  rs(31 downto 10) & rs(31 downto 22); 
      when "10111" => shift_out :=  rs(31 downto 9) & rs(31 downto 23); 
      when "11000" => shift_out :=  rs(31 downto 8) & rs(31 downto 24); 
      when "11001" => shift_out :=  rs(31 downto 7) & rs(31 downto 25); 
      when "11010" => shift_out :=  rs(31 downto 6) & rs(31 downto 26); 
      when "11011" => shift_out :=  rs(31 downto 5) & rs(31 downto 27); 
      when "11100" => shift_out :=  rs(31 downto 4) & rs(31 downto 28); 
      when "11101" => shift_out :=  rs(31 downto 3) & rs(31 downto 29); 
      when "11110" => shift_out :=  rs(31 downto 2) & rs(31 downto 30); 
      when "11111" => shift_out :=  rs(31 downto 1) & rs(31 downto 31); 
      when  others => shift_out := rs;
   end case;
  return shift_out ;
end shift_right_arith;

begin
  alu_op: process(clk)
  begin
    if rising_edge(clk) then

  --if (inst_type = ALU)--if ALU
  case inst_type is
    when ALU => 
      wb_we_out_exec <= wb_we;
      case ALU_instruction_type is 
        when ALUr => 
         case ALU_function_type is
          when "0000" => rd <= rs + rt;
          when "0001" => rd <= rs - rt;
          when "0010" => rd <= rt - rs;
          when "0011" => rd <= shift_left_logical(rs, rt);
          when "0100" => rd <= shift_right_logical(rs, rt);
          when "0101" => rd <= shift_right_arith(rs, rt);
          when "0110" => rd <= rs or rt ;
          when "0111" => rd <= rs and rt ;
        --??  when "1000" => rd <= shift_left_logical(rs, rt) or ; --??
          when "1001" => rd <= rs - rt; --??
          when "1010" => rd <= rt xor rs; 
          when "1011" => rd <= rs nor rt; 
       --   when "1100" => rd <= shift_right_logical(rs, rt); --??
       --   when "1101" => rd <= shift_right_arith(rs, rt); --??
          when "1110" => rd <= shift_left_logical(rs, "00000000000000000000000000000001" ) + rt;
          when "1111" => rd <= shift_left_logical(rs, "00000000000000000000000000000010" ) + rt ;  
          when others => NULL;
       end case;
       
      when ALUu =>    
        wb_we_out_exec <= wb_we;
        case ALU_function_type is
          when "0000" => rd <= rs(7)& rs(7) &rs(7)& rs(7)& rs(7)& rs(7)& rs(7)& rs(7)& 
                               rs(7)& rs(7) &rs(7)& rs(7)& rs(7)& rs(7)& rs(7)& rs(7)&
                               rs(7)& rs(7) &rs(7)& rs(7)& rs(7)& rs(7)& rs(7)& rs(7)& 
                               rs(7 downto 0);
          when "0001" => rd <= rs(7)& rs(7) &rs(7)& rs(7)& rs(7)& rs(7)& rs(7)& rs(7)&
                               rs(7)& rs(7) &rs(7)& rs(7)& rs(7)& rs(7)& rs(7)& rs(7)& 
                               rs(15 downto 0);
          when "0010" => rd <= "0000000000000000" & rs(15 downto 0);
          when "0101" => rd <= "0" & rs(30 downto 0);
          when others => NULL;
        end case;   
      
      --
      --when ALUm =>
    --  case ALU_function_type is
      --    when "0000" => rd <= mul rs rt;
        --  when "0001" => rd <= ;
        --  when others => NULL;
      --end case;
      when ALUc =>    
        wb_we_out_exec <= wb_we;
        case ALU_function_type is
          when "0000" => 
            if (rs = "00000000000000000000000000000000") then
              pd <= "001"; -- predicate_reg(pd)
            else
              pd <= "000";
            end if;
          when "0001" => 
            if (rs /=  "00000000000000000000000000000000")then
              pd <= "001";
            else
              pd <= "000";
            end if;
          when "0010" => 
            if (rs = rt)then
              pd <= "001";
            else
              pd <= "000";
            end if;
          when "0011" =>
            if (rs /= rt)then
              pd <= "001";
            else
              pd <= "000";
            end if;
          when "0100" =>
            if (rs < rt)then
              pd <= "001";
            else
              pd <= "000";
            end if;
          when "0101" => 
            if (rs <= rt)then
              pd <= "001";
            else
              pd <= "000";
            end if;
          when "0110" => 
            if (unsigned(rs) < unsigned(rt))then
              pd <= "001";
            else
              pd <= "000";
            end if;
          when "0111" =>
            if (unsigned(rs) <= unsigned(rt))then
              pd <= "001";
            else
              pd <= "000";
            end if;
          when "1000" =>
            if ((rs and shift_left_logical("00000000000000000000000000000001", rt)) = "11111111111111111111111111111111") then
              pd <= "001";
            else
              pd <= "000";
            end if;
          when others => NULL;
        end case; 
      --  
    when others => NULL; -- case ALU_inst_type (ALUr, ALUc, ...)
    end case; 
    when ALUi =>
      wb_we_out_exec <= wb_we;
        case ALU_function_type is
          when "0000" => rd <= rs + ("00000000000000000000" & ALUi_immediate);
          when "0001" => rd <= rs - ("00000000000000000000" & ALUi_immediate);
          when "0010" => rd <= ("00000000000000000000" & ALUi_immediate) - rs;
          when "0011" => rd <= shift_left_logical(rs, ("00000000000000000000" & ALUi_immediate));
          when "0100" => rd <= shift_right_logical(rs, ("00000000000000000000" & ALUi_immediate));
          when "0101" => rd <= shift_right_arith(rs, ("00000000000000000000" & ALUi_immediate));
          when "0110" => rd <= rs or ("00000000000000000000" & ALUi_immediate);
          when "0111" => rd <= rs and ("00000000000000000000" & ALUi_immediate);
          when others => rd <= rs + ("00000000000000000000" & ALUi_immediate);
        end case;
   
  when others => NULL; -- inst_type
  end case;
end if;
  end process alu_op;
end arch;

 -------------------------------
 -- ALU multiplexer: rt
 -------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;
use work.patmos_type_package.all;
 
 entity multiplexer_b is -- ctrl: 1 for register and 0 for extension
 
  port
  (
    rt                         : in std_logic_vector(31 downto 0);
    fw_alu                     : in std_logic_vector(31 downto 0);
    fw_mem                     : in std_logic_vector(31 downto 0);
    sign_extended_immediate    : in std_logic_vector(31 downto 0);
    fw_ctrl                    : in forwarding_type; 
    mux_b_ctrl                 : in std_logic;
    mux_b_out                  : out std_logic_vector(31 downto 0)
  );
 end entity multiplexer_b;
 
 architecture arch of multiplexer_b is
 begin
   process (rt, fw_alu, fw_mem, sign_extended_immediate, fw_ctrl, mux_b_ctrl)
     begin
       case mux_b_ctrl is
         when '1' => 
           if fw_ctrl = FWALU then 
             mux_b_out <= fw_alu;
           elsif fw_ctrl = FWMEM then 
             mux_b_out <= fw_alu;
           else  
             mux_b_out <= rt;
           end if;
         when '0' =>
           mux_b_out <= sign_extended_immediate;
         when others => mux_b_out <= rt;
       end case;
     end process;
 end arch;
 
  -------------------------------
 -- ALU multiplexer: rs
 -------------------------------
 library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;
use work.patmos_type_package.all;
 
 entity multiplexer_a is -- ctrl: 1 for register and 0 for extension
 
  port
  (
    rs                         : in std_logic_vector(31 downto 0);
    fw_alu                     : in std_logic_vector(31 downto 0);
    fw_mem                     : in std_logic_vector(31 downto 0);
--    sign_extended_immediate    : in std_logic_vector(31 downto 0);
    fw_ctrl                    : in forwarding_type; 
    mux_a_ctrl                 : in std_logic;
    mux_a_out                  : out std_logic_vector(31 downto 0)
  );
 end entity multiplexer_a;
 
 architecture arch of multiplexer_a is
 begin
   process (rs, fw_alu, fw_mem, fw_ctrl, mux_a_ctrl)
     begin
       case mux_a_ctrl is
         when '1' => 
           if fw_ctrl = FWALU then 
             mux_a_out <= fw_alu;
           elsif fw_ctrl = FWMEM then 
             mux_a_out <= fw_alu;
           else  
             mux_a_out <= rs;
           end if;
        -- when '0' =>
          -- mux_a_out <= sign_extended_immediate;
         when others => mux_a_out <= rs;
       end case;
     end process;
 end arch;
 
 -------------------------------------
 -- multiply
 -------------------------------------

