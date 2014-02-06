
--************************************************************************
--**    MODEL   :       cy7c10612dv33.vhd	     	                    **
--**    COMPANY :       Cypress Semiconductor                           **
--**    REVISION:       1.0 Created new base model		                **
--**	NOTE	:		Testbench for the validation of A[0:19] & DQ[0:15]
--**					memory with single CE and BHE, BLE				**
--************************************************************************

Library IEEE,work;
Use IEEE.Std_Logic_1164.All;
use IEEE.Std_Logic_unsigned.All;

use work.package_timing.all;
use work.package_utility.all;

------------------------
-- Entity Description
------------------------

Entity cy7c10612dv33 is
generic
	(ADDR_BITS			: integer := 20;
	DATA_BITS			: integer := 16;
	depth 				: integer := 1048576;

	TimingInfo			: BOOLEAN := TRUE;
	TimingChecks		: std_logic := '1'
	);
Port (
    CE_b    : IN Std_Logic;	                                                -- Chip Enable CE#
    WE_b  	: IN Std_Logic;	                                                -- Write Enable WE#
    OE_b  	: IN Std_Logic;                                                 -- Output Enable OE#
    BHE_b	: IN std_logic;                                                 -- Byte Enable High BHE#
    BLE_b   : IN std_logic;                                                 -- Byte Enable Low BLE#
    A 		: IN Std_Logic_Vector(addr_bits-1 downto 0);                    -- Address Inputs A
    DQ		: INOUT Std_Logic_Vector(DATA_BITS-1 downto 0):=(others=>'Z')   -- Read/Write Data IO
    );
End cy7c10612dv33;

-----------------------------
-- End Entity Description
-----------------------------
-----------------------------
-- Architecture Description
-----------------------------

Architecture behave_arch Of cy7c10612dv33 Is

Type mem_array_type Is array (depth-1 downto 0) of std_logic_vector(DATA_BITS-1 downto 0);

signal write_enable : std_logic;
signal read_enable : std_logic;
signal byte_enable : std_logic;

signal data_skew : Std_Logic_Vector(DATA_BITS-1 downto 0);

signal address_internal: Std_Logic_Vector(addr_bits-1 downto 0);

constant tSD_dataskew : time := tSD - 1 ns;

begin

byte_enable <= not(BHE_b and BLE_b);
write_enable <= not(CE_b) and not(WE_b) and byte_enable;
read_enable <= not(CE_b) and (WE_b) and not(OE_b) and byte_enable;

data_skew <= DQ after 1 ns; -- changed on feb 15

process (OE_b)
begin
    if (OE_b'event and OE_b = '1' and write_enable /= '1') then
        DQ <=(others=>'Z') after tHZOE;
    end if;
end process;

process (CE_b)
begin
    if (CE_b'event and CE_b = '1') then
        DQ <=(others=>'Z') after tHZCE;
    end if;
end process;

process (write_enable'delayed(tHA))
begin
    if (write_enable'delayed(tHA) = '0' and TimingInfo) then
	assert (A'last_event = 0 ns) or (A'last_event > tHA)
	report "Address hold time tHA violated";
    end if;
end process;

process (write_enable'delayed(tHD))
begin
    if (write_enable'delayed(tHD) = '0' and TimingInfo) then
	assert (DQ'last_event > tHD) or (DQ'last_event = 0 ns)
	report "Data hold time tHD violated";
    end if;
end process;

-- main process
process

VARIABLE mem_array: mem_array_type;

--- Variables for timing checks
VARIABLE tPWE_chk : TIME := -10 ns;
VARIABLE tAW_chk : TIME := -10 ns;
VARIABLE tSD_chk : TIME := -10 ns;
VARIABLE tRC_chk : TIME := 0 ns;
VARIABLE tBAW_chk : TIME := 0 ns;
VARIABLE tBBW_chk : TIME := 0 ns;
VARIABLE tBCW_chk : TIME := 0 ns;
VARIABLE tBDW_chk : TIME := 0 ns;

---- not sure about the following variable value ??????????????????????????????????????------
VARIABLE tBW : TIME := 0 ns;


VARIABLE write_flag : BOOLEAN := TRUE;

VARIABLE accesstime : TIME := 0 ns;

begin

    -- start of write
    if (write_enable = '1' and write_enable'event) then

       DQ(DATA_BITS-1 downto 0)<=(others=>'Z') after tHZWE;

       if (A'last_event >= tSA) then
          address_internal <= A;
          tPWE_chk := NOW;
          tAW_chk := A'last_event;
          write_flag := TRUE;

       else
          if (TimingInfo) then
		       assert FALSE
		       report "Address setup violated";
	       end if;
          write_flag := FALSE;

       end if;

    -- end of write (with CE high or WE high)
    elsif (write_enable = '0' and write_enable'event) then

        --- check for pulse width
        if (NOW - tPWE_chk >= tPWE or NOW - tPWE_chk <= 0.1 ns or NOW = 0 ns) then
            --- pulse width OK, do nothing
        else
      	   if (TimingInfo) then
           	assert FALSE
		      report "Pulse Width violation";
	       end if;

	     write_flag := FALSE;
	     end if;

        --- check for address setup with write end, i.e., tAW
        if (NOW - tAW_chk >= tAW or NOW = 0 ns) then
            --- tAW OK, do nothing
        else
      	   if (TimingInfo) then
	         assert FALSE
		      report "Address setup tAW violation";
	       end if;

          write_flag := FALSE;
        end if;

        --- check for data setup with write end, i.e., tSD
        if (NOW - tSD_chk >= tSD_dataskew or NOW - tSD_chk <= 0.1 ns or NOW = 0 ns) then
            --- tSD OK, do nothing
        else
      	   if (TimingInfo) then
	          assert FALSE
	   	      report "Data setup tSD violation";
	       end if;
          write_flag := FALSE;
        end if;

        -- perform write operation if no violations
        if (write_flag = TRUE) then

            if (BLE_b = '1' and BLE_b'last_event = write_enable'last_event and NOW /= 0 ns) then
            	mem_array(conv_integer1(address_internal))(7 downto 0) := data_skew(7 downto 0);
            end if;

            if (BHE_b = '1' and BHE_b'last_event = write_enable'last_event and NOW /= 0 ns) then
            	mem_array(conv_integer1(address_internal))(15 downto 8) := data_skew(15 downto 8);
            end if;

            if (BLE_b = '0' and NOW - tBAW_chk >= tBW) then
            	mem_array(conv_integer1(address_internal))(7 downto 0) := data_skew(7 downto 0);
            elsif (NOW - tBAW_chk < tBW and NOW - tBAW_chk > 0.1 ns and NOW > 0 ns) then
            	assert FALSE report "Insufficient pulse width for lower byte to be written";
            end if;

            if (BHE_b = '0' and NOW - tBBW_chk >= tBW) then
            	mem_array(conv_integer1(address_internal))(15 downto 8) := data_skew(15 downto 8);
            elsif (NOW - tBBW_chk < tBW and NOW - tBBW_chk > 0.1 ns and NOW > 0 ns) then
            	assert FALSE report "Insufficient pulse width for higher byte to be written";
            end if;

        end if;

    -- end of write (with BLE high)
    elsif (BLE_b'event and not(BHE_b'event) and write_enable = '1') then

       if (BLE_b = '0') then

      	   --- Reset timing variables
          tAW_chk := A'last_event;
          tBAW_chk := NOW;
          write_flag := TRUE;

       elsif (BLE_b = '1') then

          --- check for pulse width
          if (NOW - tPWE_chk >= tPWE) then
            --- tPWE OK, do nothing
          else
      	      if (TimingInfo) then
            	   assert FALSE
		          report "Pulse Width violation";
	          end if;

	          write_flag := FALSE;
	       end if;

           --- check for address setup with write end, i.e., tAW
           if (NOW - tAW_chk >= tAW) then
            --- tAW OK, do nothing
           else
      	       if (TimingInfo) then
	              assert FALSE
		           report "Address setup tAW violation for Lower Byte Write";
	           end if;

              write_flag := FALSE;
           end if;

           --- check for byte write setup with write end, i.e., tBW
           if (NOW - tBAW_chk >= tBW) then
            --- tBW OK, do nothing
           else
      	       if (TimingInfo) then
                 assert FALSE
		           report "Lower Byte setup tBW violation";
	           end if;

              write_flag := FALSE;
           end if;

	        --- check for data setup with write end, i.e., tSD
           if (NOW - tSD_chk >= tSD_dataskew or NOW - tSD_chk <= 0.1 ns or NOW = 0 ns) then
            --- tSD OK, do nothing
           else
      	       if (TimingInfo) then
	              assert FALSE
	   	          report "Data setup tSD violation for Lower Byte Write";
	           end if;

              write_flag := FALSE;
           end if;

	        --- perform WRITE operation if no violations
	        if (write_flag = TRUE) then
	           mem_array(conv_integer1(address_internal))(7 downto 0) := data_skew(7 downto 0);
              if (BHE_b = '0') then
            	    mem_array(conv_integer1(address_internal))(15 downto 8) := data_skew(15 downto 8);
              end if;
	        end if;

   	       --- Reset timing variables
           tAW_chk := A'last_event;
           tBAW_chk := NOW;
           write_flag := TRUE;

      end if;

    -- end of write (with BHE high)
    elsif (BHE_b'event and not(BLE_b'event) and write_enable = '1') then

      if (BHE_b = '0') then

    	   --- Reset timing variables
        tAW_chk := A'last_event;
        tBBW_chk := NOW;
        write_flag := TRUE;

      elsif (BHE_b = '1') then

        --- check for pulse width
        if (NOW - tPWE_chk >= tPWE) then
            --- tPWE OK, do nothing
        else
      	   if (TimingInfo) then
           	assert FALSE
		      report "Pulse Width violation";
	       end if;

	     write_flag := FALSE;
	     end if;

        --- check for address setup with write end, i.e., tAW
        if (NOW - tAW_chk >= tAW) then
            --- tAW OK, do nothing
        else
      	   if (TimingInfo) then
	         assert FALSE
		      report "Address setup tAW violation for Upper Byte Write";
	       end if;
          write_flag := FALSE;
        end if;

        --- check for byte setup with write end, i.e., tBW
        if (NOW - tBBW_chk >= tBW) then
            --- tBW OK, do nothing
        else
      	   if (TimingInfo) then
	         assert FALSE
		      report "Upper Byte setup tBW violation";
	       end if;

        write_flag := FALSE;
        end if;

        --- check for data setup with write end, i.e., tSD
        if (NOW - tSD_chk >= tSD_dataskew or NOW - tSD_chk <= 0.1 ns or NOW = 0 ns) then
            --- tSD OK, do nothing
        else
      	   if (TimingInfo) then
	          assert FALSE
	   	      report "Data setup tSD violation for Upper Byte Write";
	       end if;

          write_flag := FALSE;
        end if;

	     --- perform WRITE operation if no violations

	     if (write_flag = TRUE) then
	       mem_array(conv_integer1(address_internal))(15 downto 8) := data_skew(15 downto 8);
          if (BLE_b = '0') then
            	mem_array(conv_integer1(address_internal))(7 downto 0) := data_skew(7 downto 0);
          end if;

      	 end if;

	     --- Reset timing variables
	     tAW_chk := A'last_event;
        tBBW_chk := NOW;
        write_flag := TRUE;

     end if;

  end if;
  --- END OF WRITE

  if (data_skew'event and read_enable /= '1') then
    	tSD_chk := NOW;
  end if;

  --- START of READ

  --- Tri-state the data bus if CE or OE disabled
  if (read_enable = '0' and read_enable'event) then
    if (OE_b'last_event >= CE_b'last_event) then
   		DQ <=(others=>'Z') after tHZCE;
   	elsif (CE_b'last_event > OE_b'last_event) then
   		DQ <=(others=>'Z') after tHZOE;
   	end if;
  end if;

  --- Address-controlled READ operation
  if (A'event) then
    if (A'last_event = CE_b'last_event and CE_b = '1') then
       DQ <=(others=>'Z') after tHZCE;
    end if;

    if (NOW - tRC_chk >= tRC or NOW - tRC_chk <= 0.1 ns or tRC_chk = 0 ns) then
      --- tRC OK, do nothing
    else

       if (TimingInfo) then
	       assert FALSE
	   	   report "Read Cycle time tRC violation";
	    end if;

    end if;

    if (read_enable = '1') then

	   if (BLE_b = '0') then
		   DQ (7 downto 0) <= mem_array (conv_integer1(A))(7 downto 0) after tAA;
	   end if;

	   if (BHE_b = '0') then
		   DQ (15 downto 8) <= mem_array (conv_integer1(A))(15 downto 8) after tAA;
	   end if;

      tRC_chk := NOW;

	end if;

	if (write_enable = '1') then
	   --- do nothing
	end if;

  end if;

  if (read_enable = '0' and read_enable'event) then
     DQ <=(others=>'Z') after tHZCE;
     if (NOW - tRC_chk >= tRC or tRC_chk = 0 ns or A'last_event = read_enable'last_event) then
     --- tRC_chk needs to be reset when read ends
        tRC_CHK := 0 ns;
     else
         if (TimingInfo) then
	        assert FALSE
		     report "Read Cycle time tRC violation";
 	      end if;
	      tRC_CHK := 0 ns;
     end if;

   end if;

   --- READ operation triggered by CE/OE/BHE/BLE
   if (read_enable = '1' and read_enable'event) then

      tRC_chk := NOW;

       --- CE triggered READ
       if (CE_b'last_event = read_enable'last_event ) then --  changed rev2

           if (BLE_b = '0') then
		         DQ (7 downto 0)<= mem_array (conv_integer1(A)) (7 downto 0) after tACE;
           end if;

           if (BHE_b = '0') then
		         DQ (15 downto 8)<= mem_array (conv_integer1(A)) (15 downto 8) after tACE;
           end if;

       end if;


 	    --- OE triggered READ
       if (OE_b'last_event = read_enable'last_event) then

           -- if address or CE changes before OE such that tAA/tACE > tDOE
           if (CE_b'last_event < tACE - tDOE and A'last_event < tAA - tDOE) then

               if (A'last_event < CE_b'last_event) then

                  accesstime:=tAA-A'last_event;
                  if (BLE_b = '0') then
		               DQ (7 downto 0)<= mem_array (conv_integer1(A)) (7 downto 0) after accesstime;
                  end if;

                  if (BHE_b = '0') then
   		               DQ (15 downto 8)<= mem_array (conv_integer1(A)) (15 downto 8) after accesstime;
                  end if;

               else
                  accesstime:=tACE-CE_b'last_event;
                  if (BLE_b = '0') then
		               DQ (7 downto 0)<= mem_array (conv_integer1(A)) (7 downto 0) after accesstime;
                  end if;

                  if (BHE_b = '0') then
   		               DQ (15 downto 8)<= mem_array (conv_integer1(A)) (15 downto 8) after accesstime;
                  end if;
              end if;

           -- if address changes before OE such that tAA > tDOE
           elsif (A'last_event < tAA - tDOE) then

                  accesstime:=tAA-A'last_event;
                  if (BLE_b = '0') then
		               DQ (7 downto 0)<= mem_array (conv_integer1(A)) (7 downto 0) after accesstime;
                  end if;

                  if (BHE_b = '0') then
   		               DQ (15 downto 8)<= mem_array (conv_integer1(A)) (15 downto 8) after accesstime;
                  end if;

           -- if CE changes before OE such that tACE > tDOE
           elsif (CE_b'last_event < tACE - tDOE) then

                  accesstime:=tACE-CE_b'last_event;
                  if (BLE_b = '0') then
		               DQ (7 downto 0)<= mem_array (conv_integer1(A)) (7 downto 0) after accesstime;
                  end if;

                  if (BHE_b = '0') then
   		               DQ (15 downto 8)<= mem_array (conv_integer1(A)) (15 downto 8) after accesstime;
                  end if;

           -- if OE changes such that tDOE > tAA/tACE
           else
                   if (BLE_b = '0') then
		               DQ (7 downto 0)<= mem_array (conv_integer1(A)) (7 downto 0) after tDOE;
                   end if;

                   if (BHE_b = '0') then
   		               DQ (15 downto 8)<= mem_array (conv_integer1(A)) (15 downto 8) after tDOE;
                   end if;

           end if;

       end if;
       --- END of OE triggered READ

 	    --- BLE/BHE triggered READ
       if (BLE_b'last_event = read_enable'last_event or BHE_b'last_event = read_enable'last_event) then

           -- if address or CE changes before BHE/BLE such that tAA/tACE > tDBE
           if (CE_b'last_event < tACE - tDBE and A'last_event < tAA - tDBE) then

               if (A'last_event < BLE_b'last_event) then
                  accesstime:=tAA-A'last_event;

                  if (BLE_b = '0') then
                     DQ (7 downto 0)<= mem_array (conv_integer1(A)) (7 downto 0) after accesstime;
                  end if;

                  if (BHE_b = '0') then
   		               DQ (15 downto 8)<= mem_array (conv_integer1(A)) (15 downto 8) after accesstime;
                  end if;

               else
                  accesstime:=tACE-CE_b'last_event;

                  if (BLE_b = '0') then
                     DQ (7 downto 0)<= mem_array (conv_integer1(A)) (7 downto 0) after accesstime;
                  end if;

                  if (BHE_b = '0') then
   		               DQ (15 downto 8)<= mem_array (conv_integer1(A)) (15 downto 8) after accesstime;
                  end if;
              end if;

           -- if address changes before BHE/BLE such that tAA > tDBE
           elsif (A'last_event < tAA - tDBE) then
                  accesstime:=tAA-A'last_event;

                  if (BLE_b = '0') then
                     DQ (7 downto 0)<= mem_array (conv_integer1(A)) (7 downto 0) after accesstime;
                  end if;

                  if (BHE_b = '0') then
   		               DQ (15 downto 8)<= mem_array (conv_integer1(A)) (15 downto 8) after accesstime;
                  end if;

           -- if CE changes before BHE/BLE such that tACE > tDBE
           elsif (CE_b'last_event < tACE - tDBE) then
                  accesstime:=tACE-CE_b'last_event;

                  if (BLE_b = '0') then
                     DQ (7 downto 0)<= mem_array (conv_integer1(A)) (7 downto 0) after accesstime;
                  end if;

                  if (BHE_b = '0') then
   		               DQ (15 downto 8)<= mem_array (conv_integer1(A)) (15 downto 8) after accesstime;
                  end if;

           -- if BHE/BLE changes such that tDBE > tAA/tACE
           else
                   if (BLE_b = '0') then
		               DQ (7 downto 0)<= mem_array (conv_integer1(A)) (7 downto 0) after tDBE;
                   end if;

                   if (BHE_b = '0') then
   		               DQ (15 downto 8)<= mem_array (conv_integer1(A)) (15 downto 8) after tDBE;
                   end if;

           end if;

       end if;
       -- END of BHE/BLE controlled READ

       if (WE_b'last_event = read_enable'last_event) then

           if (BLE_b = '0') then
		      DQ (7 downto 0)<= mem_array (conv_integer1(A)) (7 downto 0) after tACE;
           end if;

           if (BHE_b = '0') then
		      DQ (15 downto 8)<= mem_array (conv_integer1(A)) (15 downto 8) after tACE;
           end if;

       end if;

     end if;
     --- END OF CE/OE/BHE/BLE controlled READ

    --- If either BHE or BLE toggle during read mode
    if (BLE_b'event and BLE_b = '0' and read_enable = '1' and not(read_enable'event)) then
	   DQ (7 downto 0) <= mem_array (conv_integer1(A)) (7 downto 0) after tDBE;
    end if;

    if (BHE_b'event and BHE_b = '0' and read_enable = '1' and not(read_enable'event)) then
	   DQ (15 downto 8) <= mem_array (conv_integer1(A)) (15 downto 8) after tDBE;
    end if;

    --- tri-state bus depending on BHE/BLE
    if (BLE_b'event and BLE_b = '1') then
        DQ (7 downto 0) <= (others=>'Z') after tHZBE;
    end if;

    if (BHE_b'event and BHE_b = '1') then
        DQ (15 downto 8) <=(others=>'Z') after tHZBE;
    end if;

    wait on write_enable, A, read_enable, DQ, BLE_b, BHE_b, data_skew;

end process;
end behave_arch;
