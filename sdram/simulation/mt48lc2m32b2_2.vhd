-----------------------------------------------------------------------------------------
--
--     File Name: MT48LC2M32B2.VHD
--       Version: 1.0
--          Date: Sept 10th, 2001
--         Model: Behavioral
--     Simulator: Model Technology
--
--  Dependencies: None
--
--        Author: Yong Phan
--         Email: yjphan@micron.com
--         Phone: (208) 363-2184
--       Company: Micron Technology, Inc.
--   Part Number: MT48LC2M32B2 (512K x 32 x 4 Banks)
--
--   Description: Micron 64Mb SDRAM
--
--    Limitation: - Doesn't check for 4096-cycle refresh
--
--          Note: - Set simulator resolution to "ps" accuracy
--
--    Disclaimer: THESE DESIGNS ARE PROVIDED "AS IS" WITH NO WARRANTY 
--                WHATSOEVER AND MICRON SPECIFICALLY DISCLAIMS ANY 
--                IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
--                A PARTICULAR PURPOSE, OR AGAINST INFRINGEMENT.
--
--                Copyright (c) 2001 Micron Semiconductor Products, Inc.
--                All rights researved
--
-- Rev  Author          Phone/Mail           Date        Changes
-- ---  --------------  --------------       ----------  -------------------------------------
-- 3.0  Rafal Janta		rafal@alatek.com.pl	 11/12/2002	 - Disjoin data bus 
--      Alatek				
--
--  ----  ----------------------------       ----------  -------------------------------------
--  0.0g  Son Huynh       208-368-3825  05/19/2000  Fix tWR + tRP timing
--        Micron Technology Inc.
--  1.0   Yong Phan       208-363-2184  09/10/2001  Add Load/Unload Function
--        Micron Technology Inc.
-- 
-----------------------------------------------------------------------------------------

USE std.textio.ALL;
LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.std_logic_unsigned.conv_integer;
USE ieee.std_logic_arith.conv_std_logic_vector;

ENTITY mt48lc2m32b2 IS
    GENERIC(
        -- Timing Parameters for -6 and CAS Latency = 2
        tOH       : TIME    := 2.0 ns;
        tMRD      : INTEGER := 2;       -- 2 Clk Cycles
        tRAS      : TIME    := 42.0 ns;
        tRC       : TIME    := 60.0 ns;
        tRCD      : TIME    := 18.0 ns;
        tRP       : TIME    := 18.0 ns;
        tRRD      : TIME    := 12.0 ns;
        tWRa      : TIME    := 6.0 ns;  -- A2 Version - Auto precharge mode only (1 Clk + 6 ns)
        tWRp      : TIME    := 12.0 ns; -- A2 Version - Precharge mode only (12 ns)

        tAS       : TIME    := 1.5 ns;
        tCH       : TIME    := 2.5 ns;
        tCL       : TIME    := 2.5 ns;
        tCK       : TIME    := 10.5 ns;
        tDH       : TIME    := 1.0 ns;
        tDS       : TIME    := 1.5 ns;
        tCKH      : TIME    := 1.0 ns;
        tCKS      : TIME    := 1.5 ns;
        tCMH      : TIME    := 0 ns;
        tCMS      : TIME    := 0 ns;

        addr_bits : INTEGER := 11;
        data_bits : INTEGER := 32;
        col_bits  : INTEGER := 8
    );
    PORT(
        Dq_in  : IN  STD_LOGIC_VECTOR(data_bits - 1 DOWNTO 0) := (OTHERS => 'Z');
        Dq_out : OUT STD_LOGIC_VECTOR(data_bits - 1 DOWNTO 0) := (OTHERS => 'Z');
        Dq_dir : OUT STD_LOGIC_VECTOR(3 downto 0)             := (OTHERS => '0');
        Addr   : IN  STD_LOGIC_VECTOR(addr_bits - 1 DOWNTO 0) := (OTHERS => '0');
        Ba     : IN  STD_LOGIC_VECTOR                         := "00";
        Clk    : IN  STD_LOGIC                                := '0';
        Cke    : IN  STD_LOGIC                                := '0';
        Cs_n   : IN  STD_LOGIC                                := '1';
        Ras_n  : IN  STD_LOGIC                                := '0';
        Cas_n  : IN  STD_LOGIC                                := '0';
        We_n   : IN  STD_LOGIC                                := '0';
        Dqm    : IN  STD_LOGIC_VECTOR(3 DOWNTO 0)             := (OTHERS => '0')
    );
END mt48lc2m32b2;

ARCHITECTURE behave OF mt48lc2m32b2 IS
    TYPE State IS (ACT, A_REF, BST, LMR, NOP, PRECH, READ, READ_A, WRITE, WRITE_A, FILE_LOAD, FILE_UNLOAD);
    TYPE Array4xI IS ARRAY (3 DOWNTO 0) OF INTEGER;
    TYPE Array4xT IS ARRAY (3 DOWNTO 0) OF TIME;
    TYPE Array4xSL IS ARRAY (3 DOWNTO 0) OF STD_LOGIC;
    TYPE Array4x2SLV IS ARRAY (3 DOWNTO 0) OF STD_LOGIC_VECTOR(1 DOWNTO 0); -- For Bank Pipeline
    --    TYPE   Array2x4SLV IS ARRAY (1 DOWNTO 0) OF STD_LOGIC_VECTOR (3 DOWNTO 0);                -- For Dqm Pipeline
    TYPE Array4xCSLV IS ARRAY (4 DOWNTO 0) OF STD_LOGIC_VECTOR(Col_bits - 1 DOWNTO 0); -- For Column Pipeline
    TYPE Array_state IS ARRAY (4 DOWNTO 0) OF State;
    SIGNAL Operation                                                      : State                                    := NOP;
    SIGNAL Mode_reg                                                       : STD_LOGIC_VECTOR(addr_bits - 1 DOWNTO 0) := (OTHERS => '0');
    SIGNAL Active_enable, Aref_enable, Burst_term                         : STD_LOGIC                                := '0';
    SIGNAL Mode_reg_enable, Prech_enable, Read_enable, Write_enable       : STD_LOGIC                                := '0';
    SIGNAL Burst_length_1, Burst_length_2, Burst_length_4, Burst_length_8 : STD_LOGIC                                := '0';
    SIGNAL Cas_latency_1, Cas_latency_2, Cas_latency_3                    : STD_LOGIC                                := '0';
    SIGNAL Cs_in, Ras_in, Cas_in, We_in                                   : STD_LOGIC                                := '0';
    SIGNAL Write_burst_mode                                               : STD_LOGIC                                := '0';
    SIGNAL RAS_clk, Sys_clk, CkeZ                                         : STD_LOGIC                                := '0';

BEGIN
    -- Strip the strength
    Cs_in  <= To_X01(Cs_n);
    Ras_in <= To_X01(Ras_n);
    Cas_in <= To_X01(Cas_n);
    We_in  <= To_X01(We_n);

    -- Commands Decode
    Active_enable   <= NOT (Cs_in) AND NOT (Ras_in) AND Cas_in AND We_in;
    Aref_enable     <= NOT (Cs_in) AND NOT (Ras_in) AND NOT (Cas_in) AND We_in;
    Burst_term      <= NOT (Cs_in) AND Ras_in AND Cas_in AND NOT (We_in);
    Mode_reg_enable <= NOT (Cs_in) AND NOT (Ras_in) AND NOT (Cas_in) AND NOT (We_in);
    Prech_enable    <= NOT (Cs_in) AND NOT (Ras_in) AND Cas_in AND NOT (We_in);
    Read_enable     <= NOT (Cs_in) AND Ras_in AND NOT (Cas_in) AND We_in;
    Write_enable    <= NOT (Cs_in) AND Ras_in AND NOT (Cas_in) AND NOT (We_in);

    -- Burst Length Decode
    Burst_length_1 <= NOT (Mode_reg(2)) AND NOT (Mode_reg(1)) AND NOT (Mode_reg(0));
    Burst_length_2 <= NOT (Mode_reg(2)) AND NOT (Mode_reg(1)) AND Mode_reg(0);
    Burst_length_4 <= NOT (Mode_reg(2)) AND Mode_reg(1) AND NOT (Mode_reg(0));
    Burst_length_8 <= NOT (Mode_reg(2)) AND Mode_reg(1) AND Mode_reg(0);

    -- CAS Latency Decode
    Cas_latency_1 <= NOT (Mode_reg(6)) AND NOT (Mode_reg(5)) AND Mode_reg(4);
    Cas_latency_2 <= NOT (Mode_reg(6)) AND Mode_reg(5) AND NOT (Mode_reg(4));
    Cas_latency_3 <= NOT (Mode_reg(6)) AND Mode_reg(5) AND Mode_reg(4);

    -- Write Burst Mode
    Write_burst_mode <= Mode_reg(9);

    -- RAS Clock for checking tWR and tRP
    PROCESS
    begin
        RAS_clk <= '1';
        wait for 0.5 ns;
        RAS_clk <= '0';
        wait for 0.5 ns;
    END PROCESS;

    -- System Clock
    int_clk : PROCESS(Clk)
    begin
        IF Clk'LAST_VALUE = '0' AND Clk = '1' THEN
            CkeZ <= Cke;
        END IF;
        Sys_clk <= CkeZ AND Clk;
    END PROCESS;

    state_register : PROCESS
        TYPE ram_type IS ARRAY (2 ** col_bits - 1 DOWNTO 0) OF STD_LOGIC_VECTOR(data_bits - 1 DOWNTO 0);
        TYPE ram_pntr IS ACCESS ram_type;
        TYPE ram_stor IS ARRAY (2 ** addr_bits - 1 DOWNTO 0) OF ram_pntr;
        VARIABLE Bank0                : ram_stor;
        VARIABLE Bank1                : ram_stor;
        VARIABLE Bank2                : ram_stor;
        VARIABLE Bank3                : ram_stor;
        VARIABLE Row_index, Col_index : INTEGER                                  := 0;
        VARIABLE Dq_temp              : STD_LOGIC_VECTOR(data_bits - 1 DOWNTO 0) := (OTHERS => '0');

        VARIABLE Col_addr           : Array4xCSLV;
        VARIABLE Bank_addr          : Array4x2SLV;
        VARIABLE Dqm_reg0, Dqm_reg1 : STD_LOGIC_VECTOR(3 DOWNTO 0) := "0000";

        VARIABLE Bank, Previous_bank                                : STD_LOGIC_VECTOR(1 DOWNTO 0)             := "00";
        VARIABLE B0_row_addr, B1_row_addr, B2_row_addr, B3_row_addr : STD_LOGIC_VECTOR(addr_bits - 1 DOWNTO 0) := (OTHERS => '0');
        VARIABLE Col_brst                                           : STD_LOGIC_VECTOR(col_bits - 1 DOWNTO 0)  := (OTHERS => '0');
        VARIABLE Row                                                : STD_LOGIC_VECTOR(addr_bits - 1 DOWNTO 0) := (OTHERS => '0');
        VARIABLE Col                                                : STD_LOGIC_VECTOR(col_bits - 1 DOWNTO 0)  := (OTHERS => '0');
        VARIABLE Burst_counter                                      : INTEGER                                  := 0;

        VARIABLE Command            : Array_state;
        VARIABLE Bank_precharge     : Array4x2SLV;
        VARIABLE A10_precharge      : Array4xSL                    := ('0' & '0' & '0' & '0');
        VARIABLE Auto_precharge     : Array4xSL                    := ('0' & '0' & '0' & '0');
        VARIABLE Read_precharge     : Array4xSL                    := ('0' & '0' & '0' & '0');
        VARIABLE Write_precharge    : Array4xSL                    := ('0' & '0' & '0' & '0');
        VARIABLE RW_interrupt_write : Array4xSL                    := ('0' & '0' & '0' & '0');
        VARIABLE RW_interrupt_read  : Array4xSL                    := ('0' & '0' & '0' & '0');
        VARIABLE RW_interrupt_bank  : STD_LOGIC_VECTOR(1 DOWNTO 0) := "00";
        VARIABLE Count_time         : Array4xT                     := (0 ns & 0 ns & 0 ns & 0 ns);
        VARIABLE Count_precharge    : Array4xI                     := (0 & 0 & 0 & 0);

        VARIABLE Data_in_enable, Data_out_enable : STD_LOGIC := '0';
        VARIABLE Pc_b0, Pc_b1, Pc_b2, Pc_b3      : STD_LOGIC := '0';
        VARIABLE Act_b0, Act_b1, Act_b2, Act_b3  : STD_LOGIC := '0';

        -- Timing Check
        VARIABLE MRD_chk                                : INTEGER  := 0;
        VARIABLE WR_counter                             : Array4xI := (0 & 0 & 0 & 0);
        VARIABLE WR_time                                : Array4xT := (0 ns & 0 ns & 0 ns & 0 ns);
        VARIABLE WR_chkp                                : Array4xT := (0 ns & 0 ns & 0 ns & 0 ns);
        VARIABLE RC_chk, RRD_chk                        : TIME     := 0 ns;
        VARIABLE RAS_chk0, RAS_chk1, RAS_chk2, RAS_chk3 : TIME     := 0 ns;
        VARIABLE RCD_chk0, RCD_chk1, RCD_chk2, RCD_chk3 : TIME     := 0 ns;
        VARIABLE RP_chk0, RP_chk1, RP_chk2, RP_chk3     : TIME     := 0 ns;

        -- Load and Unload variables

        -- Initialize empty rows
        PROCEDURE Init_mem(Bank : STD_LOGIC_VECTOR(1 DOWNTO 0); Row_index : INTEGER) IS
        BEGIN
            IF Bank = "00" THEN
                IF Bank0(Row_index) = NULL THEN -- Check to see if row empty
                    Bank0(Row_index) := NEW ram_type; -- Open new row for access
                    FOR i IN (2 ** col_bits - 1) DOWNTO 0 LOOP -- Filled row with zeros
                        FOR j IN (data_bits - 1) DOWNTO 0 LOOP
                            Bank0(Row_index)(i)(j) := '0';
                        END LOOP;
                    END LOOP;
                END IF;
            ELSIF Bank = "01" THEN
                IF Bank1(Row_index) = NULL THEN
                    Bank1(Row_index) := NEW ram_type;
                    FOR i IN (2 ** col_bits - 1) DOWNTO 0 LOOP
                        FOR j IN (data_bits - 1) DOWNTO 0 LOOP
                            Bank1(Row_index)(i)(j) := '0';
                        END LOOP;
                    END LOOP;
                END IF;
            ELSIF Bank = "10" THEN
                IF Bank2(Row_index) = NULL THEN
                    Bank2(Row_index) := NEW ram_type;
                    FOR i IN (2 ** col_bits - 1) DOWNTO 0 LOOP
                        FOR j IN (data_bits - 1) DOWNTO 0 LOOP
                            Bank2(Row_index)(i)(j) := '0';
                        END LOOP;
                    END LOOP;
                END IF;
            ELSIF Bank = "11" THEN
                IF Bank3(Row_index) = NULL THEN
                    Bank3(Row_index) := NEW ram_type;
                    FOR i IN (2 ** col_bits - 1) DOWNTO 0 LOOP
                        FOR j IN (data_bits - 1) DOWNTO 0 LOOP
                            Bank3(Row_index)(i)(j) := '0';
                        END LOOP;
                    END LOOP;
                END IF;
            END IF;
        END;

        -- Burst Counter
        PROCEDURE Burst_decode IS
            VARIABLE Col_int           : INTEGER                                 := 0;
            VARIABLE Col_vec, Col_temp : STD_LOGIC_VECTOR(col_bits - 1 DOWNTO 0) := (OTHERS => '0');
        BEGIN
            -- Advance Burst Counter
            Burst_counter := Burst_counter + 1;

            -- Burst Type
            IF Mode_reg(3) = '0' THEN
                Col_int  := conv_integer(Col) + 1;
                Col_temp := CONV_STD_LOGIC_VECTOR(Col_int, col_bits);
            ELSIF Mode_reg(3) = '1' THEN
                Col_vec     := CONV_STD_LOGIC_VECTOR(Burst_counter, col_bits);
                Col_temp(2) := Col_vec(2) XOR Col_brst(2);
                Col_temp(1) := Col_vec(1) XOR Col_brst(1);
                Col_temp(0) := Col_vec(0) XOR Col_brst(0);
            END IF;

            -- Burst Length
            IF Burst_length_2 = '1' THEN
                Col(0) := Col_temp(0);
            ELSIF Burst_length_4 = '1' THEN
                Col(1 DOWNTO 0) := Col_temp(1 DOWNTO 0);
            ELSIF Burst_length_8 = '1' THEN
                Col(2 DOWNTO 0) := Col_temp(2 DOWNTO 0);
            ELSE
                Col := Col_temp;
            END IF;

            -- Burst Read Single Write
            IF Write_burst_mode = '1' AND Data_in_enable = '1' THEN
                Data_in_enable := '0';
            END IF;

            -- Data counter
            IF Burst_length_1 = '1' THEN
                IF Burst_counter >= 1 THEN
                    IF Data_in_enable = '1' THEN
                        Data_in_enable := '0';
                    ELSIF Data_out_enable = '1' THEN
                        Data_out_enable := '0';
                    END IF;
                END IF;
            ELSIF Burst_length_2 = '1' THEN
                IF Burst_counter >= 2 THEN
                    IF Data_in_enable = '1' THEN
                        Data_in_enable := '0';
                    ELSIF Data_out_enable = '1' THEN
                        Data_out_enable := '0';
                    END IF;
                END IF;
            ELSIF Burst_length_4 = '1' THEN
                IF Burst_counter >= 4 THEN
                    IF Data_in_enable = '1' THEN
                        Data_in_enable := '0';
                    ELSIF Data_out_enable = '1' THEN
                        Data_out_enable := '0';
                    END IF;
                END IF;
            ELSIF Burst_length_8 = '1' THEN
                IF Burst_counter >= 8 THEN
                    IF Data_in_enable = '1' THEN
                        Data_in_enable := '0';
                    ELSIF Data_out_enable = '1' THEN
                        Data_out_enable := '0';
                    END IF;
                END IF;
            END IF;
        END;

    BEGIN
        WAIT ON Sys_clk, RAS_clk;
        IF Sys_clk'event AND Sys_clk = '1' THEN
            -- Internal Command Pipeline
            Command(0) := Command(1);
            Command(1) := Command(2);
            Command(2) := Command(3);
            Command(3) := NOP;

            Col_addr(0) := Col_addr(1);
            Col_addr(1) := Col_addr(2);
            Col_addr(2) := Col_addr(3);
            Col_addr(3) := (OTHERS => '0');

            Bank_addr(0) := Bank_addr(1);
            Bank_addr(1) := Bank_addr(2);
            Bank_addr(2) := Bank_addr(3);
            Bank_addr(3) := "00";

            Bank_precharge(0) := Bank_precharge(1);
            Bank_precharge(1) := Bank_precharge(2);
            Bank_precharge(2) := Bank_precharge(3);
            Bank_precharge(3) := "00";

            A10_precharge(0) := A10_precharge(1);
            A10_precharge(1) := A10_precharge(2);
            A10_precharge(2) := A10_precharge(3);
            A10_precharge(3) := '0';

            -- Operation Decode (Optional for showing current command on posedge clock / debug feature)
            IF Active_enable = '1' THEN
                Operation <= ACT;
            ELSIF Aref_enable = '1' THEN
                Operation <= A_REF;
            ELSIF Burst_term = '1' THEN
                Operation <= BST;
            ELSIF Mode_reg_enable = '1' THEN
                Operation <= LMR;
            ELSIF Prech_enable = '1' THEN
                Operation <= PRECH;
            ELSIF Read_enable = '1' THEN
                IF Addr(10) = '0' THEN
                    Operation <= READ;
                ELSE
                    Operation <= READ_A;
                END IF;
            ELSIF Write_enable = '1' THEN
                IF Addr(10) = '0' THEN
                    Operation <= WRITE;
                ELSE
                    Operation <= WRITE_A;
                END IF;
            ELSE
                Operation <= NOP;
            END IF;

            -- Dqm pipeline for Read
            Dqm_reg0 := Dqm_reg1;
            Dqm_reg1 := Dqm;
            -- Read or Write with Auto Precharge Counter
            IF Auto_precharge(0) = '1' THEN
                Count_precharge(0) := Count_precharge(0) + 1;
            END IF;
            IF Auto_precharge(1) = '1' THEN
                Count_precharge(1) := Count_precharge(1) + 1;
            END IF;
            IF Auto_precharge(2) = '1' THEN
                Count_precharge(2) := Count_precharge(2) + 1;
            END IF;
            IF Auto_precharge(3) = '1' THEN
                Count_precharge(3) := Count_precharge(3) + 1;
            END IF;

            -- Auto Precharge Timer for tWR
            if (Burst_length_1 = '1' OR Write_burst_mode = '1') then
                if (Count_precharge(0) = 1) then
                    Count_time(0) := NOW;
                end if;
                if (Count_precharge(1) = 1) then
                    Count_time(1) := NOW;
                end if;
                if (Count_precharge(2) = 1) then
                    Count_time(2) := NOW;
                end if;
                if (Count_precharge(3) = 1) then
                    Count_time(3) := NOW;
                end if;
            elsif (Burst_length_2 = '1') then
                if (Count_precharge(0) = 2) then
                    Count_time(0) := NOW;
                end if;
                if (Count_precharge(1) = 2) then
                    Count_time(1) := NOW;
                end if;
                if (Count_precharge(2) = 2) then
                    Count_time(2) := NOW;
                end if;
                if (Count_precharge(3) = 2) then
                    Count_time(3) := NOW;
                end if;
            elsif (Burst_length_4 = '1') then
                if (Count_precharge(0) = 4) then
                    Count_time(0) := NOW;
                end if;
                if (Count_precharge(1) = 4) then
                    Count_time(1) := NOW;
                end if;
                if (Count_precharge(2) = 4) then
                    Count_time(2) := NOW;
                end if;
                if (Count_precharge(3) = 4) then
                    Count_time(3) := NOW;
                end if;
            elsif (Burst_length_8 = '1') then
                if (Count_precharge(0) = 8) then
                    Count_time(0) := NOW;
                end if;
                if (Count_precharge(1) = 8) then
                    Count_time(1) := NOW;
                end if;
                if (Count_precharge(2) = 8) then
                    Count_time(2) := NOW;
                end if;
                if (Count_precharge(3) = 8) then
                    Count_time(3) := NOW;
                end if;
            end if;

            -- tMRD Counter
            MRD_chk := MRD_chk + 1;

            -- tWR Counter
            WR_counter(0) := WR_counter(0) + 1;
            WR_counter(1) := WR_counter(1) + 1;
            WR_counter(2) := WR_counter(2) + 1;
            WR_counter(3) := WR_counter(3) + 1;

            -- Auto Refresh
            IF Aref_enable = '1' THEN
                -- Auto Refresh to Auto Refresh
                ASSERT (NOW - RC_chk >= tRC) REPORT "tRC violation during Auto Refresh" SEVERITY WARNING;
                -- Precharge to Auto Refresh
                ASSERT (NOW - RP_chk0 >= tRP OR NOW - RP_chk1 >= tRP OR NOW - RP_chk2 >= tRP OR NOW - RP_chk3 >= tRP) REPORT "tRP violation during Auto Refresh" SEVERITY WARNING;
                -- All banks must be idle before refresh
                IF (Pc_b3 = '0' OR Pc_b2 = '0' OR Pc_b1 = '0' OR Pc_b0 = '0') THEN
                    ASSERT (FALSE) REPORT "All banks must be Precharge before Auto Refresh" SEVERITY WARNING;
                END IF;
                -- Record current tRC time
                RC_chk := NOW;
            END IF;

            -- Load Mode Register
            IF Mode_reg_enable = '1' THEN
                Mode_reg <= Addr;
                IF (Pc_b3 = '0' OR Pc_b2 = '0' OR Pc_b1 = '0' OR Pc_b0 = '0') THEN
                    ASSERT (FALSE) REPORT "All bank must be Precharge before Load Mode Register" SEVERITY WARNING;
                END IF;
                -- REF to LMR
                ASSERT (NOW - RC_chk >= tRC) REPORT "tRC violation during Load Mode Register" SEVERITY WARNING;
                -- LMR to LMR
                ASSERT (MRD_chk >= tMRD) REPORT "tMRD violation during Load Mode Register" SEVERITY WARNING;
                -- Record current tMRD time
                MRD_chk := 0;
            END IF;

            -- Active Block (latch Bank and Row Address)
            IF Active_enable = '1' THEN
                IF Ba = "00" AND Pc_b0 = '1' THEN
                    Act_b0      := '1';
                    Pc_b0       := '0';
                    B0_row_addr := Addr;
                    RCD_chk0    := NOW;
                    RAS_chk0    := NOW;
                    -- Precharge to Active Bank 0
                    ASSERT (NOW - RP_chk0 >= tRP) REPORT "tRP violation during Activate Bank 0" SEVERITY WARNING;
                ELSIF Ba = "01" AND Pc_b1 = '1' THEN
                    Act_b1      := '1';
                    Pc_b1       := '0';
                    B1_row_addr := Addr;
                    RCD_chk1    := NOW;
                    RAS_chk1    := NOW;
                    -- Precharge to Active Bank 1
                    ASSERT (NOW - RP_chk1 >= tRP) REPORT "tRP violation during Activate Bank 1" SEVERITY WARNING;
                ELSIF Ba = "10" AND Pc_b2 = '1' THEN
                    Act_b2      := '1';
                    Pc_b2       := '0';
                    B2_row_addr := Addr;
                    RCD_chk2    := NOW;
                    RAS_chk2    := NOW;
                    -- Precharge to Active Bank 2
                    ASSERT (NOW - RP_chk2 >= tRP) REPORT "tRP violation during Activate Bank 2" SEVERITY WARNING;
                ELSIF Ba = "11" AND Pc_b3 = '1' THEN
                    Act_b3      := '1';
                    Pc_b3       := '0';
                    B3_row_addr := Addr;
                    RCD_chk3    := NOW;
                    RAS_chk3    := NOW;
                    -- Precharge to Active Bank 3
                    ASSERT (NOW - RP_chk3 >= tRP) REPORT "tRP violation during Activate Bank 3" SEVERITY WARNING;
                ELSIF Ba = "00" AND Pc_b0 = '0' THEN
                    ASSERT (FALSE) REPORT "Bank 0 is not Precharged" SEVERITY WARNING;
                ELSIF Ba = "01" AND Pc_b1 = '0' THEN
                    ASSERT (FALSE) REPORT "Bank 1 is not Precharged" SEVERITY WARNING;
                ELSIF Ba = "10" AND Pc_b2 = '0' THEN
                    ASSERT (FALSE) REPORT "Bank 2 is not Precharged" SEVERITY WARNING;
                ELSIF Ba = "11" AND Pc_b3 = '0' THEN
                    ASSERT (FALSE) REPORT "Bank 3 is not Precharged" SEVERITY WARNING;
                END IF;
                -- Active Bank A to Active Bank B
                IF ((Previous_bank /= Ba) AND (NOW - RRD_chk < tRRD)) THEN
                    ASSERT (FALSE) REPORT "tRRD violation during Activate" SEVERITY WARNING;
                END IF;
                -- LMR to ACT
                ASSERT (MRD_chk >= tMRD) REPORT "tMRD violation during Activate" SEVERITY WARNING;
                -- AutoRefresh to Activate
                ASSERT (NOW - RC_chk >= tRC) REPORT "tRC violation during Activate" SEVERITY WARNING;
                -- Record variable for checking violation
                RRD_chk       := NOW;
                Previous_bank := Ba;
            END IF;

            -- Precharge Block
            IF Prech_enable = '1' THEN
                IF Addr(10) = '1' THEN
                    Pc_b0   := '1';
                    Pc_b1   := '1';
                    Pc_b2   := '1';
                    Pc_b3   := '1';
                    Act_b0  := '0';
                    Act_b1  := '0';
                    Act_b2  := '0';
                    Act_b3  := '0';
                    RP_chk0 := NOW;
                    RP_chk1 := NOW;
                    RP_chk2 := NOW;
                    RP_chk3 := NOW;
                    -- Activate to Precharge all banks
                    ASSERT ((NOW - RAS_chk0 >= tRAS) OR (NOW - RAS_chk1 >= tRAS)) REPORT "tRAS violation during Precharge all banks" SEVERITY WARNING;
                    -- tWR violation check for Write
                    IF ((NOW - WR_chkp(0) < tWRp) OR (NOW - WR_chkp(1) < tWRp) OR (NOW - WR_chkp(2) < tWRp) OR (NOW - WR_chkp(3) < tWRp)) THEN
                        ASSERT (FALSE) REPORT "tWR violation during Precharge ALL banks" SEVERITY WARNING;
                    END IF;
                ELSIF Addr(10) = '0' THEN
                    IF Ba = "00" THEN
                        Pc_b0   := '1';
                        Act_b0  := '0';
                        RP_chk0 := NOW;
                        -- Activate to Precharge bank 0
                        ASSERT (NOW - RAS_chk0 >= tRAS) REPORT "tRAS violation during Precharge bank 0" SEVERITY WARNING;
                    ELSIF Ba = "01" THEN
                        Pc_b1   := '1';
                        Act_b1  := '0';
                        RP_chk1 := NOW;
                        -- Activate to Precharge bank 1
                        ASSERT (NOW - RAS_chk1 >= tRAS) REPORT "tRAS violation during Precharge bank 1" SEVERITY WARNING;
                    ELSIF Ba = "10" THEN
                        Pc_b2   := '1';
                        Act_b2  := '0';
                        RP_chk2 := NOW;
                        -- Activate to Precharge bank 2
                        ASSERT (NOW - RAS_chk2 >= tRAS) REPORT "tRAS violation during Precharge bank 2" SEVERITY WARNING;
                    ELSIF Ba = "11" THEN
                        Pc_b3   := '1';
                        Act_b3  := '0';
                        RP_chk3 := NOW;
                        -- Activate to Precharge bank 3
                        ASSERT (NOW - RAS_chk3 >= tRAS) REPORT "tRAS violation during Precharge bank 3" SEVERITY WARNING;
                    END IF;
                    -- tWR violation check for Write
                    ASSERT (NOW - WR_chkp(CONV_INTEGER(Ba)) >= tWRp) REPORT "tWR violation during Precharge" SEVERITY WARNING;
                END IF;
                -- Terminate a Write Immediately (if same bank or all banks)
                IF (Data_in_enable = '1' AND (Bank = Ba OR Addr(10) = '1')) THEN
                    Data_in_enable := '0';
                END IF;
                -- Precharge Command Pipeline for READ
                IF CAS_latency_3 = '1' THEN
                    Command(2)        := PRECH;
                    Bank_precharge(2) := Ba;
                    A10_precharge(2)  := Addr(10);
                ELSIF CAS_latency_2 = '1' THEN
                    Command(1)        := PRECH;
                    Bank_precharge(1) := Ba;
                    A10_precharge(1)  := Addr(10);
                END IF;
            END IF;

            -- Burst Terminate
            IF Burst_term = '1' THEN
                -- Terminate a Write immediately
                IF Data_in_enable = '1' THEN
                    Data_in_enable := '0';
                END IF;
                -- Terminate a Read depend on CAS Latency
                IF CAS_latency_3 = '1' THEN
                    Command(2) := BST;
                ELSIF CAS_latency_2 = '1' THEN
                    Command(1) := BST;
                END IF;
            END IF;

            -- Read, Write, Column Latch
            IF Read_enable = '1' OR Write_enable = '1' THEN
                -- Check to see if bank is open (ACT) for Read or Write
                IF ((Ba = "00" AND Pc_b0 = '1') OR (Ba = "01" AND Pc_b1 = '1') OR (Ba = "10" AND Pc_b2 = '1') OR (Ba = "11" AND Pc_b3 = '1')) THEN
                    ASSERT (FALSE) REPORT "Cannot Read or Write - Bank is not Activated" SEVERITY WARNING;
                END IF;
                -- Activate to Read or Write
                IF Ba = "00" THEN
                    ASSERT (NOW - RCD_chk0 >= tRCD) REPORT "tRCD violation during Read or Write to Bank 0" SEVERITY WARNING;
                ELSIF Ba = "01" THEN
                    ASSERT (NOW - RCD_chk1 >= tRCD) REPORT "tRCD violation during Read or Write to Bank 1" SEVERITY WARNING;
                ELSIF Ba = "10" THEN
                    ASSERT (NOW - RCD_chk2 >= tRCD) REPORT "tRCD violation during Read or Write to Bank 2" SEVERITY WARNING;
                ELSIF Ba = "11" THEN
                    ASSERT (NOW - RCD_chk3 >= tRCD) REPORT "tRCD violation during Read or Write to Bank 3" SEVERITY WARNING;
                END IF;

                -- Read Command	  
                IF Read_enable = '1' THEN
                    -- CAS Latency Pipeline
                    IF Cas_latency_1 = '1' THEN
                        IF Addr(10) = '1' THEN
                            Command(0) := READ_A;
                        ELSE
                            Command(0) := READ;
                        END IF;
                        Col_addr(0)  := Addr(col_bits - 1 DOWNTO 0);
                        Bank_addr(0) := Ba;
                    ELSIF Cas_latency_2 = '1' THEN
                        IF Addr(10) = '1' THEN
                            Command(1) := READ_A;
                        ELSE
                            Command(1) := READ;
                        END IF;
                        Col_addr(1)  := Addr(col_bits - 1 DOWNTO 0);
                        Bank_addr(1) := Ba;
                    ELSIF Cas_latency_3 = '1' THEN
                        IF Addr(10) = '1' THEN
                            Command(2) := READ_A;
                        ELSE
                            Command(2) := READ;
                        END IF;
                        Col_addr(2)  := Addr(col_bits - 1 DOWNTO 0);
                        Bank_addr(2) := Ba;
                    END IF;

                    -- Read intterupt a Write (terminate Write immediately)
                    IF Data_in_enable = '1' THEN
                        Data_in_enable := '0';
                        -- Interrupt a write with autoprecharge
                        IF (Auto_precharge(CONV_INTEGER(RW_interrupt_bank)) = '1' AND Write_precharge(CONV_INTEGER(RW_interrupt_bank)) = '1') THEN
                            RW_interrupt_write(CONV_INTEGER(RW_interrupt_bank)) := '1';
                            WR_time(CONV_INTEGER(RW_interrupt_bank))            := NOW;
                        END IF;
                    END IF;

                -- Write Command
                ELSIF Write_enable = '1' THEN
                    IF Addr(10) = '1' THEN
                        Command(0) := WRITE_A;
                    ELSE
                        Command(0) := WRITE;
                    END IF;
                    Col_addr(0)  := Addr(col_bits - 1 DOWNTO 0);
                    Bank_addr(0) := Ba;

                    -- Write intterupt a Write (terminate Write immediately)
                    IF Data_in_enable = '1' THEN
                        Data_in_enable := '0';
                        -- Interrupt a Write with Auto Precharge
                        IF (Auto_precharge(CONV_INTEGER(RW_interrupt_bank)) = '1' AND Write_precharge(CONV_INTEGER(RW_interrupt_bank)) = '1') THEN
                            RW_interrupt_write(CONV_INTEGER(RW_interrupt_bank)) := '1';
                            WR_time(CONV_INTEGER(RW_interrupt_bank))            := NOW;
                        END IF;
                    END IF;

                    -- Write interrupt a Read (terminate Read immediately)
                    IF Data_out_enable = '1' THEN
                        Data_out_enable := '0';
                        -- Interrupt a Read with Auto Precharge
                        IF Auto_precharge(CONV_INTEGER(RW_interrupt_bank)) = '1' AND Read_precharge(CONV_INTEGER(RW_interrupt_bank)) = '1' THEN
                            RW_interrupt_read(CONV_INTEGER(RW_interrupt_bank)) := '1';
                        END IF;
                    END IF;
                END IF;

                -- Read or Write with Auto Precharge
                IF Addr(10) = '1' THEN
                    Auto_precharge(CONV_INTEGER(Ba))  := '1';
                    Count_precharge(CONV_INTEGER(Ba)) := 0;
                    RW_interrupt_bank                 := Ba;
                    IF Read_enable = '1' THEN
                        Read_precharge(CONV_INTEGER(Ba)) := '1';
                    ELSIF Write_enable = '1' THEN
                        Write_precharge(CONV_INTEGER(Ba)) := '1';
                    END IF;
                END IF;
            END IF;

            -- Read with AutoPrecharge Calculation
            --      The device start internal precharge when:
            --          1.  BL/2 cycles after command
            --      and 2.  Meet tRAS requirement
            --       or 3.  Interrupt by a Read or Write (with or without Auto Precharge)
            IF ((Auto_precharge(0) = '1') AND (Read_precharge(0) = '1')) THEN
                IF (((NOW - RAS_chk0 >= tRAS) AND ((Burst_length_1 = '1' AND Count_precharge(0) >= 1) OR (Burst_length_2 = '1' AND Count_precharge(0) >= 2) OR (Burst_length_4 = '1' AND Count_precharge(0) >= 4) OR (Burst_length_8 = '1' AND Count_precharge(0) >= 8))) OR (
                        RW_interrupt_read(0) = '1')) THEN
                    Pc_b0                := '1';
                    Act_b0               := '0';
                    RP_chk0              := NOW;
                    Auto_precharge(0)    := '0';
                    Read_precharge(0)    := '0';
                    RW_interrupt_read(0) := '0';
                END IF;
            END IF;
            IF ((Auto_precharge(1) = '1') AND (Read_precharge(1) = '1')) THEN
                IF (((NOW - RAS_chk1 >= tRAS) AND ((Burst_length_1 = '1' AND Count_precharge(1) >= 1) OR (Burst_length_2 = '1' AND Count_precharge(1) >= 2) OR (Burst_length_4 = '1' AND Count_precharge(1) >= 4) OR (Burst_length_8 = '1' AND Count_precharge(1) >= 8))) OR (
                        RW_interrupt_read(1) = '1')) THEN
                    Pc_b1                := '1';
                    Act_b1               := '0';
                    RP_chk1              := NOW;
                    Auto_precharge(1)    := '0';
                    Read_precharge(1)    := '0';
                    RW_interrupt_read(1) := '0';
                END IF;
            END IF;
            IF ((Auto_precharge(2) = '1') AND (Read_precharge(2) = '1')) THEN
                IF (((NOW - RAS_chk2 >= tRAS) AND ((Burst_length_1 = '1' AND Count_precharge(2) >= 1) OR (Burst_length_2 = '1' AND Count_precharge(2) >= 2) OR (Burst_length_4 = '1' AND Count_precharge(2) >= 4) OR (Burst_length_8 = '1' AND Count_precharge(2) >= 8))) OR (
                        RW_interrupt_read(2) = '1')) THEN
                    Pc_b2                := '1';
                    Act_b2               := '0';
                    RP_chk2              := NOW;
                    Auto_precharge(2)    := '0';
                    Read_precharge(2)    := '0';
                    RW_interrupt_read(2) := '0';
                END IF;
            END IF;
            IF ((Auto_precharge(3) = '1') AND (Read_precharge(3) = '1')) THEN
                IF (((NOW - RAS_chk3 >= tRAS) AND ((Burst_length_1 = '1' AND Count_precharge(3) >= 1) OR (Burst_length_2 = '1' AND Count_precharge(3) >= 2) OR (Burst_length_4 = '1' AND Count_precharge(3) >= 4) OR (Burst_length_8 = '1' AND Count_precharge(3) >= 8))) OR (
                        RW_interrupt_read(3) = '1')) THEN
                    Pc_b3                := '1';
                    Act_b3               := '0';
                    RP_chk3              := NOW;
                    Auto_precharge(3)    := '0';
                    Read_precharge(3)    := '0';
                    RW_interrupt_read(3) := '0';
                END IF;
            END IF;

            -- Internal Precharge or Bst
            IF Command(0) = PRECH THEN  -- PRECH terminate a read if same bank or all banks
                IF Bank_precharge(0) = Bank OR A10_precharge(0) = '1' THEN
                    IF Data_out_enable = '1' THEN
                        Data_out_enable := '0';
                    END IF;
                END IF;
            ELSIF Command(0) = BST THEN -- BST terminate a read regardless of bank
                IF Data_out_enable = '1' THEN
                    Data_out_enable := '0';
                END IF;
            END IF;

            -- Turn off databus


            -- Detect Read or Write Command
            IF Command(0) = READ OR Command(0) = READ_A THEN
                Bank     := Bank_addr(0);
                Col      := Col_addr(0);
                Col_brst := Col_addr(0);
                IF Bank_addr(0) = "00" THEN
                    Row := B0_row_addr;
                ELSIF Bank_addr(0) = "01" THEN
                    Row := B1_row_addr;
                ELSIF Bank_addr(0) = "10" THEN
                    Row := B2_row_addr;
                ELSE
                    Row := B3_row_addr;
                END IF;
                Burst_counter   := 0;
                Data_in_enable  := '0';
                Data_out_enable := '1';
            ELSIF Command(0) = WRITE OR Command(0) = WRITE_A THEN
                Bank     := Bank_addr(0);
                Col      := Col_addr(0);
                Col_brst := Col_addr(0);
                IF Bank_addr(0) = "00" THEN
                    Row := B0_row_addr;
                ELSIF Bank_addr(0) = "01" THEN
                    Row := B1_row_addr;
                ELSIF Bank_addr(0) = "10" THEN
                    Row := B2_row_addr;
                ELSE
                    Row := B3_row_addr;
                END IF;
                Burst_counter   := 0;
                Data_in_enable  := '1';
                Data_out_enable := '0';
            END IF;
            
            IF (Data_out_enable = '0') THEN
                Dq_dir <= "0000";
                Dq_out <= (OTHERS => 'Z');
            END IF;


            -- DQ (Driver / Receiver)
            Row_index := CONV_INTEGER(Row);
            Col_index := CONV_INTEGER(Col);
            IF Data_in_enable = '1' THEN
                IF Dqm /= "1111" THEN
                    -- Initialize memory
                    Init_mem(Bank, Row_index);
                    -- Load memory into buffer
                    IF Bank = "00" THEN
                        Dq_temp := Bank0(Row_index)(Col_index);
                    ELSIF Bank = "01" THEN
                        Dq_temp := Bank1(Row_index)(Col_index);
                    ELSIF Bank = "10" THEN
                        Dq_temp := Bank2(Row_index)(Col_index);
                    ELSIF Bank = "11" THEN
                        Dq_temp := Bank3(Row_index)(Col_index);
                    END IF;
                    -- Dqm operation
                    IF Dqm(0) = '0' THEN
                        Dq_temp(7 DOWNTO 0) := Dq_in(7 DOWNTO 0);
                    END IF;
                    IF Dqm(1) = '0' THEN
                        Dq_temp(15 DOWNTO 8) := Dq_in(15 DOWNTO 8);
                    END IF;
                    IF Dqm(2) = '0' THEN
                        Dq_temp(23 DOWNTO 16) := Dq_in(23 DOWNTO 16);
                    END IF;
                    IF Dqm(3) = '0' THEN
                        Dq_temp(31 DOWNTO 24) := Dq_in(31 DOWNTO 24);
                    END IF;
                    -- Write back to memory
                    IF Bank = "00" THEN
                        Bank0(Row_index)(Col_index) := Dq_temp;
                    ELSIF Bank = "01" THEN
                        Bank1(Row_index)(Col_index) := Dq_temp;
                    ELSIF Bank = "10" THEN
                        Bank2(Row_index)(Col_index) := Dq_temp;
                    ELSIF Bank = "11" THEN
                        Bank3(Row_index)(Col_index) := Dq_temp;
                    END IF;
                    -- Reset tWR counter
                    WR_chkp(CONV_INTEGER(Bank))    := NOW;
                    WR_counter(CONV_INTEGER(Bank)) := 0;
                END IF;
                -- Decode next burst address
                Burst_decode;
            ELSIF Data_out_enable = '1' THEN
                IF Dqm_reg0 /= "1111" THEN
                    -- Initialize memory
                    Init_mem(Bank, Row_index);
                    -- Load memory into buffer
                    IF Bank = "00" THEN
                        Dq_temp := Bank0(Row_index)(Col_index);
                    ELSIF Bank = "01" THEN
                        Dq_temp := Bank1(Row_index)(Col_index);
                    ELSIF Bank = "10" THEN
                        Dq_temp := Bank2(Row_index)(Col_index);
                    ELSIF Bank = "11" THEN
                        Dq_temp := Bank3(Row_index)(Col_index);
                    END IF;
                    -- Dqm operation
                    IF Dqm_reg0(0) = '0' THEN
                        Dq_out(7 DOWNTO 0) <= Dq_temp(7 DOWNTO 0);
                        Dq_dir(0)          <= '1';
                    ELSE
                        Dq_dir(0) <= '0';
                    END IF;
                    IF Dqm_reg0(1) = '0' THEN
                        Dq_out(15 DOWNTO 8) <= Dq_temp(15 DOWNTO 8);
                        Dq_dir(1)           <= '1';
                    ELSE
                        Dq_dir(1) <= '0';
                    END IF;
                    IF Dqm_reg0(2) = '0' THEN
                        Dq_out(23 DOWNTO 16) <= Dq_temp(23 DOWNTO 16);
                        Dq_dir(2)            <= '1';
                    ELSE
                        Dq_dir(2) <= '0';
                    END IF;
                    IF Dqm_reg0(3) = '0' THEN
                        Dq_out(31 DOWNTO 24) <= Dq_temp(31 DOWNTO 24);
                        Dq_dir(3)            <= '1';
                    ELSE
                        Dq_dir(3) <= '0';
                    END IF;
                ELSE
                    Dq_out <= (OTHERS => 'Z');
                    Dq_dir <= "0000";
                END IF;
                Burst_decode;
            END IF;

            -- Write with AutoPrecharge Calculation
            --      The device start internal precharge when:
            --          1.  tWR cycles after command
            --      and 2.  Meet tRAS requirement
            --       or 3.  Interrupt by a Read or Write (with or without Auto Precharge)
            IF ((Auto_precharge(0) = '1') AND (Write_precharge(0) = '1')) THEN
                IF (((NOW - RAS_chk0 >= tRAS) AND (((Burst_length_1 = '1' OR Write_burst_mode = '1') AND Count_precharge(0) >= 1 AND NOW - Count_time(0) >= tWRa) OR (Burst_length_2 = '1' AND Count_precharge(0) >= 2 AND NOW - Count_time(0) >= tWRa) OR (Burst_length_4 = '1' AND
                                Count_precharge(0) >= 4 AND NOW - Count_time(0) >= tWRa) OR (Burst_length_8 = '1' AND Count_precharge(0) >= 8 AND NOW - Count_time(0) >= tWRa))) OR (RW_interrupt_write(0) = '1' AND WR_counter(0) >= 1 AND NOW - WR_time(0) >= tWRa)) THEN
                    Auto_precharge(0)     := '0';
                    Write_precharge(0)    := '0';
                    RW_interrupt_write(0) := '0';
                    Pc_b0                 := '1';
                    Act_b0                := '0';
                    RP_chk0               := NOW;
                    --ASSERT FALSE REPORT "Start Internal Precharge Bank 0" SEVERITY NOTE;
                END IF;
            END IF;
            IF ((Auto_precharge(1) = '1') AND (Write_precharge(1) = '1')) THEN
                IF (((NOW - RAS_chk1 >= tRAS) AND (((Burst_length_1 = '1' OR Write_burst_mode = '1') AND Count_precharge(1) >= 1 AND NOW - Count_time(1) >= tWRa) OR (Burst_length_2 = '1' AND Count_precharge(1) >= 2 AND NOW - Count_time(1) >= tWRa) OR (Burst_length_4 = '1' AND
                                Count_precharge(1) >= 4 AND NOW - Count_time(1) >= tWRa) OR (Burst_length_8 = '1' AND Count_precharge(1) >= 8 AND NOW - Count_time(1) >= tWRa))) OR (RW_interrupt_write(1) = '1' AND WR_counter(1) >= 1 AND NOW - WR_time(1) >= tWRa)) THEN
                    Auto_precharge(1)     := '0';
                    Write_precharge(1)    := '0';
                    RW_interrupt_write(1) := '0';
                    Pc_b1                 := '1';
                    Act_b1                := '0';
                    RP_chk1               := NOW;
                END IF;
            END IF;
            IF ((Auto_precharge(2) = '1') AND (Write_precharge(2) = '1')) THEN
                IF (((NOW - RAS_chk2 >= tRAS) AND (((Burst_length_1 = '1' OR Write_burst_mode = '1') AND Count_precharge(2) >= 1 AND NOW - Count_time(2) >= tWRa) OR (Burst_length_2 = '1' AND Count_precharge(2) >= 2 AND NOW - Count_time(2) >= tWRa) OR (Burst_length_4 = '1' AND
                                Count_precharge(2) >= 4 AND NOW - Count_time(2) >= tWRa) OR (Burst_length_8 = '1' AND Count_precharge(2) >= 8 AND NOW - Count_time(2) >= tWRa))) OR (RW_interrupt_write(2) = '1' AND WR_counter(2) >= 1 AND NOW - WR_time(2) >= tWRa)) THEN
                    Auto_precharge(2)     := '0';
                    Write_precharge(2)    := '0';
                    RW_interrupt_write(2) := '0';
                    Pc_b2                 := '1';
                    Act_b2                := '0';
                    RP_chk2               := NOW;
                END IF;
            END IF;
            IF ((Auto_precharge(3) = '1') AND (Write_precharge(3) = '1')) THEN
                IF (((NOW - RAS_chk3 >= tRAS) AND (((Burst_length_1 = '1' OR Write_burst_mode = '1') AND Count_precharge(3) >= 1 AND NOW - Count_time(3) >= tWRa) OR (Burst_length_2 = '1' AND Count_precharge(3) >= 2 AND NOW - Count_time(3) >= tWRa) OR (Burst_length_4 = '1' AND
                                Count_precharge(3) >= 4 AND NOW - Count_time(3) >= tWRa) OR (Burst_length_8 = '1' AND Count_precharge(3) >= 8 AND NOW - Count_time(3) >= tWRa))) OR (RW_interrupt_write(0) = '1' AND WR_counter(0) >= 1 AND NOW - WR_time(3) >= tWRa)) THEN
                    Auto_precharge(3)     := '0';
                    Write_precharge(3)    := '0';
                    RW_interrupt_write(3) := '0';
                    Pc_b3                 := '1';
                    Act_b3                := '0';
                    RP_chk3               := NOW;
                END IF;
            END IF;
        END IF;
    END PROCESS;

    -- Clock timing checks
    Clock_check : PROCESS
        VARIABLE Clk_low, Clk_high : TIME := 0 ns;
    BEGIN
        WAIT ON Clk;
        IF (Clk = '1' AND NOW >= 10 ns) THEN
            ASSERT (NOW - Clk_low >= tCL) REPORT "tCL violation" SEVERITY WARNING;
            ASSERT (NOW - Clk_high >= tCK) REPORT "tCK violation" SEVERITY WARNING;
            Clk_high := NOW;
        ELSIF (Clk = '0' AND NOW /= 0 ns) THEN
            ASSERT (NOW - Clk_high >= tCH) REPORT "tCH violation" SEVERITY WARNING;
            Clk_low := NOW;
        END IF;
    END PROCESS;

    -- Setup timing checks
    Setup_check : PROCESS
    BEGIN
        WAIT ON Clk;
        IF Clk = '1' THEN
            ASSERT (Cke'LAST_EVENT >= tCKS) REPORT "CKE Setup time violation -- tCKS" SEVERITY WARNING;
            ASSERT (Cs_n'LAST_EVENT >= tCMS) REPORT "CS# Setup time violation -- tCMS" SEVERITY WARNING;
            ASSERT (Cas_n'LAST_EVENT >= tCMS) REPORT "CAS# Setup time violation -- tCMS" SEVERITY WARNING;
            ASSERT (Ras_n'LAST_EVENT >= tCMS) REPORT "RAS# Setup time violation -- tCMS" SEVERITY WARNING;
            ASSERT (We_n'LAST_EVENT >= tCMS) REPORT "WE# Setup time violation -- tCMS" SEVERITY WARNING;
            ASSERT (Dqm'LAST_EVENT >= tCMS) REPORT "Dqm Setup time violation -- tCMS" SEVERITY WARNING;
            ASSERT (Addr'LAST_EVENT >= tAS) REPORT "ADDR Setup time violation -- tAS" SEVERITY WARNING;
            ASSERT (Ba'LAST_EVENT >= tAS) REPORT "BA Setup time violation -- tAS" SEVERITY WARNING;
            ASSERT (Dq_in'LAST_EVENT >= tDS) REPORT "Dq Setup time violation -- tDS" SEVERITY WARNING;
        END IF;
    END PROCESS;

    -- Hold timing checks
    Hold_check : PROCESS
    BEGIN
        WAIT ON Clk'DELAYED(tCKH), Clk'DELAYED(tCMH), Clk'DELAYED(tDH);
        IF Clk'DELAYED(tCKH) = '1' THEN
            ASSERT (Cke'LAST_EVENT > tCKH) REPORT "CKE Hold time violation -- tCKH" SEVERITY WARNING;
        END IF;
        IF Clk'DELAYED(tCMH) = '1' THEN
            ASSERT (Cs_n'LAST_EVENT > tCMH) REPORT "CS# Hold time violation -- tCMH" SEVERITY WARNING;
            ASSERT (Cas_n'LAST_EVENT > tCMH) REPORT "CAS# Hold time violation -- tCMH" SEVERITY WARNING;
            ASSERT (Ras_n'LAST_EVENT > tCMH) REPORT "RAS# Hold time violation -- tCMH" SEVERITY WARNING;
            ASSERT (We_n'LAST_EVENT > tCMH) REPORT "WE# Hold time violation -- tCMH" SEVERITY WARNING;
            ASSERT (Dqm'LAST_EVENT > tCMH) REPORT "Dqm Hold time violation -- tCMH" SEVERITY WARNING;
        END IF;
        IF Clk'DELAYED(tDH) = '1' THEN
            ASSERT (Dq_in'LAST_EVENT > tDH) REPORT "DQ Hold time violation -- tDH" SEVERITY WARNING;
        END IF;
    END PROCESS;

END behave;
