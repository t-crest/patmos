library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


entity patmos_stack_cache is
  port
  (
    	clk       	         		: in std_logic;
    	rst							: in std_logic;
        head_in				 		: in unsigned(4 downto 0); -- from  
        tail_in				 		: in unsigned(4 downto 0);	-- 
        head_out				 	: out unsigned(4 downto 0); -- from  
        tail_out				 	: out unsigned(4 downto 0);	-- 
      	number_of_bytes_to_spill 	: in unsigned(31 downto 0);
        number_of_bytes_to_fill  	: in unsigned(31 downto 0);
        dout_to_mem					: out unsigned(31 downto 0); -- mem interface
        din_from_mem				: in unsigned(31 downto 0); -- mem interface
        din_from_cpu				: in unsigned(31 downto 0);
        dout_to_cpu					: out unsigned(31 downto 0);
        spill		        	    : in std_logic;
        fill		        	    : in std_logic;
        read_enable          	    : in std_logic;
        write_enable          	    : in std_logic;
        address						: in unsigned(4 downto 0);
        st							: in unsigned(3 downto 0) -- stack pointer
  );    
end entity patmos_stack_cache;
architecture arch of patmos_stack_cache is

type stack_cache_type is array (0 to 31) of unsigned(31 downto 0);
signal stack_cache					 : stack_cache_type;
--signal head_pt, tail_pt        		 : unsigned(5 downto 0);
type state_type is (s0,s1); 
signal stack_state: state_type := s0;
signal number_of_bytes_to_spill_reg :unsigned(31 downto 0);
signal number_of_bytes_to_fill_reg :unsigned(31 downto 0);
signal tail_reg : unsigned(4 downto 0);
signal head_reg : unsigned(4 downto 0);
signal tail_new, head_new : unsigned(4 downto 0);

begin

  dout_to_cpu <= stack_cache(to_integer(unsigned(address)));
  st_cache : process(clk, rst)
  begin
    if(rst = '1') then
        for i in 0 to 31 loop -- initialize register file
          stack_cache(i)<= (others => '0');
        end loop;
       head_out <= (others => '0');
       tail_out <= (others => '0');
   elsif (rising_edge(clk)) then     
   		if(write_enable = '1') then
     		stack_cache(to_integer(unsigned(address))) <= din_from_cpu;
  		 end if;
   
  		number_of_bytes_to_spill_reg <= number_of_bytes_to_spill;
  		tail_reg <= tail_new;
  		head_reg <= head_new;
  ------------------------------------- spill
      if(spill = '1') then  
      	
      	case stack_state is
   		  when s0 =>  
   		  	dout_to_mem <= stack_cache(to_integer(unsigned(tail_reg)));
   		  	number_of_bytes_to_spill_reg <= number_of_bytes_to_spill_reg - 1;
   		  	stack_state <= s1;
   		  	tail_reg <= tail_reg + 1; --move the stack pointer
   		  when s1 =>
      		if (number_of_bytes_to_spill_reg > 0) then
      			stack_state <= s0;
      		else
      			tail_new <= tail_in;
      			tail_out <= tail_in;
      			head_new <= head_in;
      			head_out <= head_in;
      		end if;	
      	 when others => NULL;
        end case;	         
      end if;
  ------------------------------------- fill      
 --      if(fill = '1') then  
 --     	case stack_state is
  -- 		  when s0 =>  
 --  		  	stack_cache(to_integer(unsigned(st))) <= din_from_mem;
 --  		  	number_of_bytes_to_fill_reg = number_of_bytes_to_fill_reg - 1;
 --  		  	stack_state <= s1;
 --  		  when s1 =>
 --     		if (number_of_bytes_to_spill > 0) then
 --     			stack_state <= s0;
 --     		end if;	
 --     	 when others => NULL;
 --       end case;	         
 --     end if;
      
    end if;
  end process st_cache;
  
  --tail_out <= tail_reg;
  
     
end arch;

