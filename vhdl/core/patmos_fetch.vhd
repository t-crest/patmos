library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.patmos_type_package.all;

entity patmos_fetch is
	port(
		clk   : in  std_logic;
		rst   : in  std_logic;
		din   : in  fetch_in_type;
		decin : in  decode_out_type;    -- decin shall be renamed
		dout  : out fetch_out_type
	);
end entity patmos_fetch;

architecture arch of patmos_fetch is
	signal pc, pc_next : unsigned(pc_length - 1 downto 0);
	signal addr        : std_logic_vector(pc_length - 1 downto 0);
	signal feout       : fetch_out_type;
	signal tmp         : std_logic_vector(31 downto 0);

begin
	process(pc, decin)
	begin
		if decin.inst_type_out = BC then
			-- no addition? no relative branch???
			pc_next <= unsigned(decin.imm);
		else
			pc_next <= pc + 1;
		end if;
	end process;

	rom : entity work.patmos_rom
		port map(
			address => addr(7 downto 0),
			-- instruction shall not be unsigned
			q       => tmp
		);
	feout.instruction <= unsigned(tmp);
	process(clk, rst)
	begin
		if (rst = '1') then
			pc      <= (others => '0');
			dout.pc <= (others => '0');
		elsif (rising_edge(clk) and rst = '0') then
			pc               <= pc_next;
			addr             <= std_logic_vector(pc_next);
			dout.instruction <= feout.instruction;
			-- MS: the next pc? PC calculation is REALLY an independent pipe stage!
			dout.pc <= pc;
		end if;
	end process;

end arch;
