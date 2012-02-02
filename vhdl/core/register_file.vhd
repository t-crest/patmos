library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity register_file is
  port
  (
    clk;
    read_address1 : std_logic_vector(4 downto 0);
    read_address2 : std_logic_vector(4 downto 0);
    write_address : std_logic_vector(4 downto 0);
    read_data1    : std_logic_vector(31 downto 0);
    read_data2    : std_logic_vector(31 downto 0);
    write_data    : std_logic_vector(31 downto 0);
  );
end entity register_file;

architecture arch of register_file is

begin
  read:  process (clk)
  begin
    read_data1 <= reg_bank[read_address1];
    read_data2 <= reg_bank[read_address2];
  end process read;
  
  write:  process (clk)
  begin
    
  end process write;
end arch;


    begin
        for(i=0;i<32;i=i+1)
            reg_bank[i]=0;
    end

    assign qa=(r_rdaddress_a==0)?0:
           ((r_wraddress==r_rdaddress_a)&&(1==r_wren))?r_data:
           reg_bank[r_rdaddress_a];

    assign qb=(r_rdaddress_b==0)?0:
           ((r_wraddress==r_rdaddress_b)&&(1==r_wren))?r_data:
           reg_bank[r_rdaddress_b];

    always@(posedge clock)
        if (~rd_clk_cls)
        begin
            r_rdaddress_a <=rdaddress_a;
            r_rdaddress_b<=rdaddress_b;
        end

    always@(posedge clock)
    begin
        r_data <=data;
        r_wraddress<=wraddress;
        r_wren<=wren;
    end
    always@(posedge clock)
        if (r_wren)
            reg_bank[r_wraddress] <= r_data ;
endmodule

