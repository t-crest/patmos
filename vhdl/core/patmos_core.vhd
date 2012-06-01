library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.patmos_type_package.all;

entity patmos_core is 
  port
  (
    clk                   : in std_logic;
    rst                   : in std_logic;
    led         	  : out std_logic
    txd      	          : out std_logic;
    rxd     : in  std_logic
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


signal head_in						   : unsigned(4 downto 0);
signal tail_in						   : unsigned(4 downto 0);
signal spill, fill					   : std_logic; 
------------------------------------------------------- uart signals
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


begin -- architecture begin
------------------------------------------------------- fetch	
		  
  is_beq <= '1' when fetch_dout.instruction(26 downto 22) = "11111" else '0';
  branch <= branch_taken and is_beq;
  beq_imm <= "0000000000000000000000000" & fetch_dout.instruction(6 downto 0);
  
  led <= mem_dout.write_back_reg_out(0);
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
  pc_offset_adder: entity work.patmos_adder(arch) -- for branch instruction
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
  
  ------------------------------------------------------
    special_reg_file: entity work.patmos_special_register_file(arch)
	port map(clk, rst, decode_dout.st_out, fetch_dout.instruction(10 downto 7),
	         mem_dout.write_back_reg_out(3 downto 0), decode_din.rs1_data_in_special, decode_din.rs2_data_in_special,
	          mux_mem_reg, mem_dout.reg_write_out);
  
   
  stack_cache: entity work.patmos_stack_cache(arch)
   port map(clk, head_in, tail_in, decode_din.head_in, decode_din.tail_in, execute_dout.alu_result_out, execute_dout.alu_result_out,
   	decode_din.stack_data_in, decode_dout.stack_data_out, spill, fill, decode_dout.st_out);
 -- entity patmos_stack_cache is
 -- port
 -- (
 --   	clk       	         		: in std_logic;
 --       head_in				 		: in unsigned(5 downto 0); -- from  
 --       tail_in				 		: in unsigned(5 downto 0);	-- 
 --      head_out				 	: out unsigned(5 downto 0); -- from  
 --      tail_out				 	: out unsigned(5 downto 0);	-- 
 --     	number_of_bytes_to_spill 	: in unsigned(21 downto 0);
 --       number_of_bytes_to_fill  	: in unsigned(21 downto 0);
 --       dout_to_mem					: out unsigned(31 downto 0);
 --       din_from_mem				: in unsigned(31 downto 0);
 --       spill		        	    : in std_logic;
 --       fill		        	    : in std_logic;
 --       st							: in unsigned(5 downto 0) -- stack pointer
 -- );  
  

  ------------------------------------------------------ execute
  mux_imm: entity work.patmos_mux_32(arch) -- immediate or rt
  port map(decode_dout.rs2_data_out, decode_dout.ALUi_immediate_out, 
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
  alu_din.ALU_function_type <= decode_dout.ALU_function_type_out;
  
  
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
  execute: entity work.patmos_execute(arch)
  port map(clk, rst, execute_din, execute_dout);
  

  ------------------------------------------------------- memory
  -- mem/io decoder
   io_decode: process(execute_dout.alu_result_out)
	begin
		if(to_integer(execute_dout.alu_result_out) < 126) then
			mem_write <= execute_dout.mem_write_out;
			mem_read <= execute_dout.mem_read_out;
		    io_write <= '0';	
		    io_read <= '0';
		    address <= "00000000000000000000000000000001";
		end if;
		if (to_integer(execute_dout.alu_result_out) >= 126) then
			mem_write <= '0';
			mem_read <= '0';
			io_write <= execute_dout.mem_write_out;
			io_read <= execute_dout.mem_read_out;
			address <= std_logic_vector(execute_dout.alu_result_out);
		end if;
	end process;
  
  sc_uart_inst : sc_uart port map       -- Maps internal signals to ports
    (
      address => address,
      wr_data => std_logic_vector(execute_dout.mem_write_data_out),
      rd      => io_read,
      wr      => io_write,
      unsigned(rd_data) => mem_data_out,
      rdy_cnt => rdy_cnt,
      clk     => clk,
      reset   => rst,
      txd     => txd,
      rxd     => rxd,
     ncts    => '0',
      nrts    => open
     );

  

  -- memory access
  memory: entity work.patmos_data_memory(arch)
  port map(clk, rst, execute_dout.alu_result_out, 
            execute_dout.mem_write_data_out,
            mem_data_out, 
            mem_read, mem_write);
  --clk, rst, add, data_in(store), data_out(load), read_en, write_en

  --------------------------
  mem_din.reg_write_in <= mem_write;
  mem_din.mem_to_reg_in <= execute_dout.mem_to_reg_out;
  mem_din.alu_result_in <= execute_dout.alu_result_out;
  mem_din.write_back_reg_in <= execute_dout.write_back_reg_out;
  mem_din.mem_data_in <= mem_data_out;
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


end architecture arch;



