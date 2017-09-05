library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity nexys4ddr_io is
	Port(
		clk              : in  std_logic;
		clk_pwm              : in  std_logic;
		reset                : in  std_logic;
		
		-- OCP IN (slave)
		MCmd                 : in  std_logic_vector(2 downto 0);
		MAddr                : in  std_logic_vector(15 downto 0);
		MData                : in  std_logic_vector(31 downto 0);
		MByteEn              : in  std_logic_vector(3 downto 0);
		SResp                : out std_logic_vector(1 downto 0);
		SData                : out std_logic_vector(31 downto 0);

		green_leds           : out std_logic_vector(15 downto 0); -- (15) -> LD15 ... LD0 <- (0)
		rgb_leds             : out std_logic_vector(5 downto 0); -- (5) -> LD17_R LD17_G LD17_B | LD16_R LD16_G LD16_B <- (0)
		seven_segments       : out std_logic_vector(7 downto 0); -- (7) -> DP CG CF CE CD CC CB CA <- (0)
		seven_segments_drive : out std_logic_vector(7 downto 0); -- (7) -> AN7 ... AN0 <- (0)
		buttons              : in  std_logic_vector(4 downto 0); -- (4) -> BTNT BTNR BTND BTNL BTNC <- (0)
		switches             : in  std_logic_vector(15 downto 0)); -- (15) -> SW15 ... SW0 <- (0)
end nexys4ddr_io;

architecture Behavioral of nexys4ddr_io is
	signal SResp_next : std_logic_vector(1 downto 0);
	signal SData_next : std_logic_vector(31 downto 0);

	signal PWM_count : unsigned(16 downto 0);
	
	alias PWM_reg : unsigned(7 downto 0) is PWM_count(16 downto 9);	
	alias seven_segments_id : unsigned(2 downto 0) is PWM_reg(7 downto 5);
	type seven_seven_segments_reg_type is array (0 to 7) of std_logic_vector(7 downto 0);
	signal seven_segments_reg : seven_seven_segments_reg_type;
	signal seven_segments_wr  : std_logic_vector(7 downto 0);
	
	type green_leds_reg_type is array (0 to 15) of unsigned(7 downto 0);
	signal green_leds_reg : green_leds_reg_type;
	signal green_leds_wr  : std_logic_vector(15 downto 0);
	signal green_leds_wr_en  : std_logic;

	type rgb_leds_reg_type is array (0 to 1) of unsigned(23 downto 0);
	signal rgb_leds_reg : rgb_leds_reg_type;
	signal rgb_leds_wr  : std_logic_vector(1 downto 0);
	
	signal buttons_debounced, buttons_debounced_prev : std_logic_vector(4 downto 0); -- (4) -> BTNT BTNR BTND BTNL BTNC <- (0)
	type debounce_count_type is array (0 to 4) of unsigned(12 downto 0);
	signal debounce_count :debounce_count_type; 
	constant DEBOUNCE_TIME : integer := 3000;
	signal buttons_prev, buttons_sync, buttons_sync_int : std_logic_vector(4 downto 0);
	type buttons_press_reg_type is array (0 to 4) of unsigned(7 downto 0);
	signal buttons_press_reg : buttons_press_reg_type; 
	signal buttons_press_reg_clear : std_logic_vector(4 downto 0);

	signal switches_sync, switches_sync_int : std_logic_vector(15 downto 0); 

begin

--OCP management--------------------------------------------------------------------------------------
	process(MCmd, MAddr, switches_sync, buttons_press_reg, buttons_debounced)
	begin
		seven_segments_wr  <=  (others  => '0');
		green_leds_wr <=  (others  => '0');
		green_leds_wr_en <= '0';
		rgb_leds_wr <= (others  =>  '0');
		buttons_press_reg_clear <= (others => '0');
		SResp_next <= "00";
		SData_next <= (others => '0');
		
		case MCmd is
			when "001" =>               -- write
				if (unsigned(MAddr(15 downto 2))=0) then --offset hex:0000, addr hex:000
						green_leds_wr_en <= '1';
				end if;
				for I in 0 to 15 loop
					if (unsigned(MAddr(15 downto 2))=(1024+I)) then --offset hex:1000
						green_leds_wr(I) <= '1';
					end if;
				end loop;
				for I in 0 to 7 loop
					if (unsigned(MAddr(15 downto 2))=(2048+I)) then --offset hex:2000
						seven_segments_wr(I) <= '1';
					end if;
				end loop;
				for I in 0 to 1 loop
					if (unsigned(MAddr(15 downto 2))=(3072+I)) then --offset hex:3000
						rgb_leds_wr(I) <= '1';
					end if;
				end loop;
				SResp_next <= "01";
			when "010" =>               -- read
				if (unsigned(MAddr(15 downto 2))=4096) then --offset hex:4000, addr hex:000
					SData_next(15 downto 0) <= switches_sync;	
				end if;
				for I in 0 to 4 loop
					if (unsigned(MAddr(15 downto 2))=(5120+I)) then --offset hex:5000
						SData_next(8) <= buttons_debounced(I);
						SData_next(7 downto 0) <= std_logic_vector(buttons_press_reg(I));
						buttons_press_reg_clear(I) <= '1';
					end if;
				end loop;
				SResp_next <= "01";
			when others =>              -- idle
				SResp_next <= "00";
		end case;
	end process;

--RGB leds managements----------------------------------------------------------------------------------
	rgb_l_mux_PROC : process(PWM_reg, rgb_leds_reg)
	begin
		for I in 0 to 1 loop
			--blue
			if (rgb_leds_reg(I)(7 downto 0) = 0) then
				rgb_leds(3*I) <= '0';
			elsif(rgb_leds_reg(I)(7 downto 0) >= PWM_reg)then
				rgb_leds(3*I) <= '1';
			else
				rgb_leds(3*I) <= '0';
			end if;
			--green
			if (rgb_leds_reg(I)(15 downto 8) = 0) then
				rgb_leds(3*I+1) <= '0';
			elsif(rgb_leds_reg(I)(15 downto 8) >= PWM_reg)then
				rgb_leds(3*I+1) <= '1';
			else
				rgb_leds(3*I+1) <= '0';
			end if;
			--red
			if (rgb_leds_reg(I)(23 downto 16) = 0) then
				rgb_leds(3*I+2) <= '0';
			elsif(rgb_leds_reg(I)(23 downto 16) >= PWM_reg)then
				rgb_leds(3*I+2) <= '1';
			else
				rgb_leds(3*I+2) <= '0';
			end if;
		end loop;
	end process;

--Green leds managements----------------------------------------------------------------------------------
	g_l_mux_PROC : process(PWM_reg, green_leds_reg)
	begin
		for I in 0 to 15 loop
			if (green_leds_reg(I) = 0) then
				green_leds(I) <= '0';
			elsif((green_leds_reg(I)) >= PWM_reg)then
				green_leds(I) <= '1';
			else
				green_leds(I) <= '0';
			end if;
		end loop;
	end process;
	
--Seven segments display----------------------------------------------------------------------------------

	-- Seven segments display output mux
	s_s_mux_PROC : process(seven_segments_id, seven_segments_reg)
	begin
		seven_segments_drive <= (others => '1');
		seven_segments <= seven_segments_reg(0);
		for I in 0 to 7 loop
			if (seven_segments_id = I) then
				seven_segments_drive(I)  <= '0';
				seven_segments <= seven_segments_reg(I);
			end if;
		end loop;
	end process;
	
--Registers and counters----------------------------------------------------------------------------------

	-- OCP clocked register
	OCP_reg_PROC : process(clk)
	begin
		if rising_edge(clk) then
			if reset = '1' then
				SResp <= (others => '0');
				SData <= (others => '0');
				switches_sync <= (others => '0');
				switches_sync_int <= (others => '0');
				buttons_sync <= (others => '0');
				buttons_sync_int <= (others => '0'); 
				buttons_debounced <= (others => '0'); 
				buttons_press_reg <= (others => (others => '0'));
				seven_segments_reg <= (others => (others => '1'));
				green_leds_reg <= (others => (others => '0'));
				rgb_leds_reg <= (others => (others => '0'));
			else
				SResp <= SResp_next;
				SData <= SData_next;
				switches_sync <= switches_sync_int;
				switches_sync_int <= switches;
				buttons_sync <= buttons_sync_int;
				buttons_sync_int <= buttons;
				buttons_debounced <= buttons_debounced_prev;
				for I in 0 to 4 loop
					if(buttons_debounced_prev(I) = '1' and buttons_debounced(I) = '0') then
						if(buttons_press_reg_clear(I) = '1') then
							buttons_press_reg(I) <= X"01";
						else
							if (buttons_press_reg(I) /= X"FF") then
								buttons_press_reg(I) <= buttons_press_reg(I) + 1;
							end if;
						end if;
    				else
    					if(buttons_press_reg_clear(I) = '1') then
							buttons_press_reg(I) <= (others => '0');
						end if;				
    				end if; 
    			end loop;	
				for I in 0 to 7 loop
					if seven_segments_wr(I) = '1' then
						seven_segments_reg(I) <= not(MData(7 downto 0));
					end if;
				end loop;
				for I in 0 to 15 loop
					if green_leds_wr(I) = '1' then
						green_leds_reg(I) <= unsigned(MData(7 downto 0));
					end if;
					if green_leds_wr_en = '1' then
						if (MData(I)='0') then
							green_leds_reg(I) <= (others => '0');
						else
							green_leds_reg(I) <= (others => '1');
						end if;
					end if;
				end loop;
				for I in 0 to 1 loop
					if rgb_leds_wr(I) = '1' then
						rgb_leds_reg(I) <= unsigned(MData(23 downto 0));
					end if;
				end loop;
			end if;
		end if;
	end process;


	-- PWM main counter
	PWM_reg_PROC : process(clk_pwm)
	begin
		if rising_edge(clk_pwm) then
			if reset = '1' then
				PWM_count <= (others => '0');
			else
				PWM_count <= PWM_count + 1;
			end if;
		end if;
	end process;

----Debouncer----------------------------------------------------------------------------------
	
	Debouncer_PROC : process(clk)
	begin
		if rising_edge(clk) then
			if reset = '1' then
				debounce_count <= (others => (others => '0'));
				buttons_debounced_prev <= (others => '0');
				buttons_prev <= (others => '0');
			else
				for I in 0 to 4 loop
					if(buttons_sync(I) = buttons_prev(I)) then 
		        		if (debounce_count(I) = DEBOUNCE_TIME) then
		            		buttons_debounced_prev(I)<=buttons_prev(I);
		       			else
		            		debounce_count(I)<=debounce_count(I)+1;
						end if;
		    		else
		           		debounce_count(I)<=(others => '0');
			       		buttons_prev(I)<=buttons_sync(I);
		       		end if;
		       	end loop;
			end if;
		end if;
	end process;

end Behavioral;

