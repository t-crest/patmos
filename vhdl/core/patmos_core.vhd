library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.patmos_type_package.all;
use work.sc_pack.all;


entity patmos_core is 
  port
  (
    clk                   : in std_logic;
    out_rst                   : in std_logic;
    led         	  : out std_logic;
    txd      	          : out std_logic;
    rxd     		: in  std_logic;
    oSRAM_A		 : out std_logic_vector(18 downto 0);		-- edit
	SRAM_DQ		 : inout std_logic_vector(31 downto 0);		-- edit
	oSRAM_CE1_N	 : out std_logic;
	oSRAM_OE_N	 : out std_logic;
	oSRAM_BE_N	 : out std_logic_vector(3 downto 0);
	oSRAM_WE_N	 : out std_logic;
	oSRAM_GW_N   : out std_logic;
	oSRAM_CLK	 : out std_logic;
	oSRAM_ADSC_N : out std_logic;
	oSRAM_ADSP_N : out std_logic;
	oSRAM_ADV_N	 : out std_logic;
	oSRAM_CE2	 : out std_logic;
	oSRAM_CE3_N  : out std_logic
  );
end entity patmos_core;

architecture arch of patmos_core is 

signal pc                              : unsigned(32 - 1 downto 0);
signal pc_next                         : unsigned(32 - 1 downto 0);
signal pc_offset                       : unsigned(32 - 1 downto 0);
signal mux_branch                      : unsigned(32 - 1 downto 0);
signal branch                          : std_logic := '0';
signal fetch_din                       : fetch_in_type;
signal fetch_dout                      : fetch_out_type;
signal decode_din                      : decode_in_type;
signal decode_dout                     : decode_out_type;
signal alu_din                         : alu_in_type;
signal alu_dout                        : alu_out_type;
signal execute_din                     : execution_in_type;
signal execute_dout                    : execution_out_type;
signal stack_cache_din				   : patmos_stack_cache_in;
signal stack_cache_dout		 		   : patmos_stack_cache_out;
signal stack_cache_ctrl_din	  		   : patmos_stack_cache_ctrl_in;
signal stack_cache_ctrl_dout		   : patmos_stack_cache_ctrl_out;
signal mem_din                         : mem_in_type;
signal mem_dout                        : mem_out_type;
signal mux_mem_reg                  	  : unsigned(31 downto 0); 
signal mux_alu_src                     : unsigned(31 downto 0); 
signal alu_src1                        : unsigned(31 downto 0);
signal alu_src2                        : unsigned(31 downto 0);
signal fw_ctrl_rs1                     : forwarding_type;
signal fw_ctrl_rs2                     : forwarding_type;
signal br_src1                         : unsigned(31 downto 0);
signal br_src2                         : unsigned(31 downto 0);
signal fw_ctrl_br1                     : forwarding_type;
signal fw_ctrl_br2                     : forwarding_type;
signal mem_data_out           	        : unsigned(31 downto 0); 
signal branch_taken                    : std_logic; 
signal is_beq                          : std_logic; 
signal beq_imm                         : unsigned(31 downto 0);  

signal address_uart : std_logic_vector(31 downto 0)  := (others => '0');
signal mem_data_out_uart				: std_logic_vector(31 downto 0); 
signal mem_data_out_muxed				: unsigned(31 downto 0); 
signal mem_data_out3					: unsigned(31 downto 0);

signal head_in						   : unsigned(4 downto 0);
signal tail_in						   : unsigned(4 downto 0);
signal spill, fill					   : std_logic; 

signal clk2 		: std_logic;
signal sc_mem_out_wr_data	: unsigned(31 downto 0);
signal ram_cnt			: integer := 3;
signal clk_int			: std_logic;
-- MS: maybe some signal sorting would be nice
	-- for generation of internal reset
	signal int_res			: std_logic;
	signal res_cnt			: unsigned(2 downto 0) := "000";	-- for the simulation

	attribute altera_attribute : string;
	attribute altera_attribute of res_cnt : signal is "POWER_UP_LEVEL=LOW";

signal ram_addr			: std_logic_vector(18 downto 0);	-- edit
signal ram_dout			: std_logic_vector(31 downto 0);	-- edit
signal ram_din			: std_logic_vector(31 downto 0);	-- edit
signal ram_dout_en		: std_logic;
signal ram_clk			: std_logic;
signal ram_nsc			: std_logic;
signal ram_ncs			: std_logic;
signal ram_noe			: std_logic;
signal ram_nwe			: std_logic;
signal sc_mem_out		: sc_out_type;
signal sc_mem_in		: sc_in_type;
------------------------------------------------------- uart signals
signal rst				:std_logic;--out_rst
signal mem_write					   : std_logic;
signal io_write						   : std_logic;	
signal mem_read						   : std_logic;
signal io_read						   : std_logic;	
signal rdy_cnt : unsigned(1 downto 0);
signal address : std_logic_vector(31 downto 0)  := (others => '0');
constant BLINK_FREQ : integer := 1;

   component sc_uart                     --  Declaration of uart driver
    generic (addr_bits : integer := 32;
             clk_freq  : integer := 50000000;
             baud_rate : integer := 115200;
             txf_depth : integer := 16; txf_thres : integer := 8;
             rxf_depth : integer := 16; rxf_thres : integer := 8);
    port(
      clk   : in std_logic;
      reset : in std_logic;

      address : in  std_logic_vector(31 downto 0);
      wr_data : in  std_logic_vector(31 downto 0);
      rd, wr  : in  std_logic;
      rd_data : out std_logic_vector(31 downto 0);
      rdy_cnt : out unsigned(1 downto 0);
      txd     : out std_logic;
      rxd     : in  std_logic;
      ncts    : in  std_logic;
      nrts    : out std_logic
      );
  end component;
	
	component sc_mem_if
		generic (ram_ws : integer; addr_bits : integer);

port (

	clk, reset	: in std_logic;
	clk2		: in std_logic;	-- an inverted clock

--
--	SimpCon memory interface
--
	sc_mem_out		: in sc_out_type;
	sc_mem_in		: out sc_in_type;

-- memory interface

	ram_addr	: out std_logic_vector(addr_bits-1 downto 0);
	ram_dout	: out std_logic_vector(31 downto 0);
	ram_din		: in std_logic_vector(31 downto 0);
	ram_dout_en	: out std_logic;
	ram_clk		: out std_logic;
	ram_nsc		: out std_logic;
	ram_ncs		: out std_logic;
	ram_noe		: out std_logic;
	ram_nwe		: out std_logic

);
	end component;

begin -- architecture begin


--
--	internal reset generation
--	should include the PLL lock signal
--
process(clk_int)
begin
	if rising_edge(clk_int) then
		if (res_cnt/="111") then
			res_cnt <= res_cnt+1;
		end if;
		int_res <= not res_cnt(0) or not res_cnt(1) or not res_cnt(2);
	end if;
end process;

-- rst <= not out_rst;
rst <= int_res;
 led <= '1';
------------------------------------------------------- fetch	
		  
  is_beq <= '1' when fetch_dout.instruction(26 downto 22) = "11111" else '0';
  branch <= branch_taken and is_beq;
  beq_imm <= "0000000000000000000000000" & fetch_dout.instruction(6 downto 0);
  
 
  fetch_din.pc <= pc_next;
  
  fet: entity work.patmos_fetch(arch)
	port map(clk, rst, fetch_din, fetch_dout);

  pc_adder: entity work.patmos_adder(arch)
  port map(pc, "00000000000000000000000000000100", pc_next);
  
  mux_pc: entity work.patmos_mux_32(arch)
  port map(pc_next, pc_offset, branch, mux_branch);
  
  pc_gen: entity work.patmos_pc_generator(arch)
  port map(clk, rst, mux_branch, pc);

  inst_mem: entity work.patmos_rom(arch)
  port map(pc, fetch_din.instruction);
-------------------------------------------------------- decode
  pc_offset_adder: entity work.patmos_adder2(arch) -- for branch instruction
  port map(fetch_dout.pc, beq_imm, pc_offset);
  
  reg_file: entity work.patmos_register_file(arch)
	port map(clk, rst, fetch_dout.instruction(16 downto 12), fetch_dout.instruction(11 downto 7),
	         mem_dout.write_back_reg_out, decode_din.rs1_data_in, decode_din.rs2_data_in,
	          mux_mem_reg, mem_dout.reg_write_out);
	 
  decode_din.operation <= fetch_dout.instruction;
	dec: entity work.patmos_decode(arch)
	port map(clk, rst, decode_din, decode_dout);

  mux_br1: entity work.patmos_forward_value(arch)
  port map(execute_dout.alu_result_out, mux_mem_reg, decode_din.rs1_data_in, br_src1, fw_ctrl_br1);
                                                         
  mux_br2: entity work.patmos_forward_value(arch)
  port map(execute_dout.alu_result_out, mux_mem_reg, decode_din.rs2_data_in, br_src2, fw_ctrl_br2);
  
  forward_br: entity work.patmos_forward(arch)
  port map(fetch_dout.instruction(16 downto 12), fetch_dout.instruction(11 downto 7), execute_dout.reg_write_out, mem_dout.reg_write_out, 
           execute_dout.write_back_reg_out, mem_dout.write_back_reg_out, fw_ctrl_br1, fw_ctrl_br2);
  
  equal_check: entity work.patmos_equal_check(arch)
  port map(br_src1, br_src2, branch_taken);
  
  --------------- special register file
  -- there is a problem here, st_out should be dynamic, it is not dedicated to stack cache
    special_reg_file: entity work.patmos_special_register_file(arch) -- the first operand may not be st_out, this should change, 
	port map(clk, rst, decode_dout.st_out, fetch_dout.instruction(10 downto 7),
	         decode_dout.st_out, decode_din.rs1_data_in_special, decode_din.rs2_data_in_special,
	          stack_cache_ctrl_dout.st_out, stack_cache_ctrl_dout.reg_write_out);
  

  ---------------------------------------------------- execute
	
  mux_imm: entity work.patmos_mux_32(arch) -- immediate or rt
  port map(alu_src2, decode_dout.ALUi_immediate_out, 
           decode_dout.alu_src_out, mux_alu_src);

  mux_rs1: entity work.patmos_forward_value(arch)
  port map(execute_dout.alu_result_out, mux_mem_reg, decode_dout.rs1_data_out, alu_src1, fw_ctrl_rs1);
                                                         
  mux_rs2: entity work.patmos_forward_value(arch)
  port map(execute_dout.alu_result_out, mux_mem_reg, decode_dout.rs2_data_out, alu_src2, fw_ctrl_rs2);
  
  forward: entity work.patmos_forward(arch)
  port map(decode_dout.rs1_out, decode_dout.rs2_out, execute_dout.reg_write_out, mem_dout.reg_write_out, 
           execute_dout.write_back_reg_out, mem_dout.write_back_reg_out, fw_ctrl_rs1, fw_ctrl_rs2);
  
  
  
  
  alu_din.rs1 <= alu_src1;
  alu_din.rs2 <= mux_alu_src;
  alu_din.inst_type <= decode_dout.inst_type_out;
  alu_din.ALU_instruction_type <= decode_dout.ALU_instruction_type_out;
  alu_din.ALU_function_type <= decode_dout.ALU_function_type_out;
  alu_din.STT_instruction_type <= decode_dout.STT_instruction_type_out;
  alu_din.LDT_instruction_type <= decode_dout.LDT_instruction_type_out;
  
  alu: entity work.patmos_alu(arch)
  port map(clk, rst, alu_din, alu_dout);
  
  -----------------------
  execute_din.reg_write_in <= decode_dout.reg_write_out;
  execute_din.mem_read_in <= decode_dout.mem_read_out;
  execute_din.mem_write_in <= decode_dout.mem_write_out;
  execute_din.mem_to_reg_in <= decode_dout.mem_to_reg_out;
  execute_din.alu_result_in <= alu_dout.rd;
  execute_din.write_back_reg_in <= decode_dout.rd_out;
  execute_din.mem_write_data_in <= alu_src2;
  execute_din.STT_instruction_type_in <= decode_dout.STT_instruction_type_out;
  execute_din.LDT_instruction_type_in <= decode_dout.LDT_instruction_type_out;
 -- execute_din.tail_in <= alu_dout.tail_out;
 -- execute_din.head_in <= alu_dout.head_out;
  execute_din.st_in <= alu_dout.st_out;
  execute: entity work.patmos_execute(arch)
  port map(clk, rst, execute_din, execute_dout);
  
   -----------------------------------------------cache - memory------------------------------------------------------------
   ---------------------------------------------------- stack cache controller
   
 	stack_cache_ctrl_din.stc_immediate_in <= decode_dout.ALUi_immediate_out(4 downto 0);
 	stack_cache_ctrl_din.instruction <= decode_dout.STC_instruction_type_out;
 	stack_cache_ctrl_din.st_in <= decode_dout.rs1_data_out_special;
 	stack_cache_ctrl: entity work.patmos_stack_cache_ctrl(arch)
 	port map(clk, rst, stack_cache_ctrl_din, stack_cache_ctrl_dout);
 
 
   ---------------------------------------------------- stack cache
   
 	stack_cache_in: process(execute_dout.mem_write_data_out) -- which type of transfer to stack cache?
 	begin
 		if (execute_dout.STT_instruction_type_out = SWS) then
 			stack_cache_din.din_from_cpu <= execute_dout.mem_write_data_out;
 		elsif (execute_dout.STT_instruction_type_out = SBS) then
 			stack_cache_din.din_from_cpu(16 downto 0) <= execute_dout.mem_write_data_out(16 downto 0);
 		elsif (execute_dout.STT_instruction_type_out = SHS) then
 			stack_cache_din.din_from_cpu(8 downto 0) <= execute_dout.mem_write_data_out(8 downto 0);
 		end if;
 	end process;
 	
 	stack_cache_out:process(stack_cache_dout.dout_to_cpu) -- which type of transfer from stack cache?
 	begin
 		if (execute_dout.LDT_instruction_type_out = LWS) then
 			mem_data_out <= stack_cache_dout.dout_to_cpu;
 		elsif (execute_dout.LDT_instruction_type_out = LBS) then
 			mem_data_out(16 downto 0) <= stack_cache_dout.dout_to_cpu(16 downto 0);
 		elsif (execute_dout.LDT_instruction_type_out = LHS) then
 			mem_data_out(8 downto 0) <= stack_cache_dout.dout_to_cpu(8 downto 0);
 		end if;
 	end process;
 	
 --	stack_cache_din.din_from_cpu <= execute_dout.mem_write_data_out; -- transfer to stack cache no matter what, should change based on controlling signals
 	
 --	mem_data_out <= stack_cache_dout.dout_to_cpu;
 	stack_cache_din.spill_fill <= stack_cache_ctrl_dout.spill_fill;
 	stack_cache_din.write_enable <= mem_write;
 	stack_cache_din.address <= execute_dout.alu_result_out(4 downto 0);
 	stack_cache_din.head_tail <= stack_cache_ctrl_dout.head_tail;
 	
 	
 	stack_cache: entity work.patmos_stack_cache(arch)
 	port map(clk, rst, stack_cache_din, stack_cache_dout);

  ------------------------------------------------------- memory
  -- mem/io decoder
   io_decode: process(execute_dout.alu_result_out)
	begin
		if(to_integer(execute_dout.alu_result_out) < 127) then
			mem_write <= execute_dout.mem_write_out;
			mem_read <= execute_dout.mem_read_out;
		    io_write <= '0';	
		    io_read <= '0';
		   -- address <= "00000000000000000000000000000001";
		end if;
		if (to_integer(execute_dout.alu_result_out) <= 127) then
			mem_write <= '0';
			mem_read <= '0';
			io_write <= execute_dout.mem_write_out;
			io_read <= execute_dout.mem_read_out;
			address_uart <= std_logic_vector(execute_dout.alu_result_out);
		end if;
	end process;
	
	   io_mem_read_mux: process(mem_data_out_uart, mem_data_out, execute_dout.alu_result_out)
	begin
		if(to_integer(execute_dout.alu_result_out) <= 127) then
			mem_data_out_muxed <= unsigned(mem_data_out_uart);
		elsif (to_integer(execute_dout.alu_result_out) > 127) then
			mem_data_out_muxed <= mem_data_out;
		else 
			mem_data_out_muxed <= mem_data_out;
		end if;
	end process;
  
  sc_uart_inst : sc_uart port map       -- Maps internal signals to ports
    (
      address => address_uart,
      wr_data => std_logic_vector(execute_dout.mem_write_data_out),
      rd      => io_read,
      wr      => io_write,
      rd_data => mem_data_out_uart,
      rdy_cnt => rdy_cnt,
      clk     => clk,
      reset   => rst,
      txd     => txd,
      rxd     => rxd,
     ncts    => '0',
      nrts    => open
     );

  

  -- memory access
-- memory: entity work.patmos_data_memory(arch)
--  port map(clk, rst, execute_dout.alu_result_out, 
--            execute_dout.mem_write_data_out,
--            mem_data_out, 
--            mem_read, mem_write);
  --clk, rst, add, data_in(store), data_out(load), read_en, write_en

  --------------------------
  mem_din.reg_write_in <= execute_dout.reg_write_out;
  mem_din.mem_to_reg_in <= execute_dout.mem_to_reg_out;
  mem_din.alu_result_in <= execute_dout.alu_result_out;
  mem_din.write_back_reg_in <= execute_dout.write_back_reg_out;
  mem_din.mem_data_in <= mem_data_out_muxed;
  mem_din.mem_write_data_in <= execute_dout.mem_write_data_out;
  memory_stage: entity work.patmos_mem_stage(arch)
  port map(clk, rst, mem_din, mem_dout);

  
  ------------------------------------------------------- write back
  
--  write_back: entity work.patmos_mux_32(arch)
--  port map(mem_dout.alu_result_out, mem_dout.mem_data_out, mem_dout.mem_to_reg_out, mux_mem_reg);
  
  

  write_back: process(mem_dout.alu_result_out, mem_dout.mem_data_out, mem_dout.mem_to_reg_out)
	begin
		if(mem_dout.mem_to_reg_out = '0') then
			mux_mem_reg <= mem_dout.alu_result_out;
		elsif(mem_dout.mem_to_reg_out = '1') then
			mux_mem_reg <= mem_dout.mem_data_out;
		else mux_mem_reg <= mem_dout.alu_result_out;
			end if;
	end process;
	
	------------------------------------------------------ SRAM Interface
	sc_mem_out.wr_data <= std_logic_vector(stack_cache_dout.dout_to_mem);
	sc_mem_out.address <= std_logic_vector(stack_cache_ctrl_dout.st_out(22 downto 0));
	
	scm: sc_mem_if
	generic map (
			ram_ws => ram_cnt-1,
			addr_bits => 19			-- edit
		)
	port map (clk, int_res, clk2,
			sc_mem_out, sc_mem_in,

			ram_addr => ram_addr,
			ram_dout => ram_dout,
			ram_din => ram_din,
			ram_dout_en	=> ram_dout_en,
			ram_clk => ram_clk,
			ram_nsc => ram_nsc,
			ram_ncs => ram_ncs,
			ram_noe => ram_noe,
			ram_nwe => ram_nwe
		);
	oSRAM_A <= ram_addr;
	oSRAM_CE1_N <= ram_ncs;
	oSRAM_OE_N <= ram_noe;
	oSRAM_WE_N <= ram_nwe;
	oSRAM_BE_N <= (others => '0');
	oSRAM_GW_N <= '1';
	oSRAM_CLK <= ram_clk;
	
	oSRAM_ADSC_N <= ram_ncs;
	oSRAM_ADSP_N <= '1';
	oSRAM_ADV_N	<= '1';
	
	oSRAM_CE2 <= not ram_ncs;
    oSRAM_CE3_N <= ram_ncs;





end architecture arch;





