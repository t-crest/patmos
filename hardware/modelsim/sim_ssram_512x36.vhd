--------------------------------------------------------------------------------
--  File Name: sim_ssram_512x36.vhd
--------------------------------------------------------------------------------
--  Copyright (C) 2003-2008 Free Model Foundry; http://www.FreeModelFoundry.com
--
--  This program is free software; you can redistribute it and/or modify
--  it under the terms of the GNU General Public License version 2 as
--  published by the Free Software Foundation.
--
--  MODIFICATION HISTORY:
--
--  version: |  author:  | mod date: | changes made:
--    V1.0    M.Radmanovic 03 Aug 13   Inital Release
--    V1.1    R. Munden    08 May 28   Updated FILE declaration syntax
--
--------------------------------------------------------------------------------
--  PART DESCRIPTION:
--
--  Library:    SRAM
--  Technology:
--  Part:        CY7C1380
--
--   Description: 512K 36 Pipelined SRAM
--------------------------------------------------------------------------------

LIBRARY IEEE;   USE IEEE.vital_timing.ALL;
                USE IEEE.vital_primitives.ALL;

PACKAGE default_timings is

    CONSTANT Default_tipd : VitalDelayType01 := (3.0 ns, 3.0 ns);
    CONSTANT Default_tpd : VitalDelayType01Z := (others => 3.1 ns);
    CONSTANT Default_tpw : VitalDelayType := 2.0 ns;
    CONSTANT Default_period : VitalDelayType := 5.0 ns;
    CONSTANT Default_tsetup : VitalDelayType := 1.4 ns;
    CONSTANT Default_thold : VitalDelayType := 0.4 ns;

END default_timings;


LIBRARY IEEE;   USE IEEE.std_logic_1164.ALL;
                USE IEEE.numeric_std.ALL;
                USE STD.textio.ALL;
                USE IEEE.vital_timing.ALL;
                USE IEEE.vital_primitives.ALL;

USE work.gen_utils.ALL;
USE work.conversions.ALL;
use work.default_timings.all;

--------------------------------------------------------------------------------
-- ENTITY DECLARATION
--------------------------------------------------------------------------------
ENTITY memory IS
    GENERIC (
        -- tipd delays: interconnect path delays
        tipd_A0                  : VitalDelayType01 := Default_tipd;
        tipd_A1                  : VitalDelayType01 := Default_tipd;
        tipd_A2                  : VitalDelayType01 := Default_tipd;
        tipd_A3                  : VitalDelayType01 := Default_tipd;
        tipd_A4                  : VitalDelayType01 := Default_tipd;
        tipd_A5                  : VitalDelayType01 := Default_tipd;
        tipd_A6                  : VitalDelayType01 := Default_tipd;
        tipd_A7                  : VitalDelayType01 := Default_tipd;
        tipd_A8                  : VitalDelayType01 := Default_tipd;
        tipd_A9                  : VitalDelayType01 := Default_tipd;
        tipd_A10                 : VitalDelayType01 := Default_tipd;
        tipd_A11                 : VitalDelayType01 := Default_tipd;
        tipd_A12                 : VitalDelayType01 := Default_tipd;
        tipd_A13                 : VitalDelayType01 := Default_tipd;
        tipd_A14                 : VitalDelayType01 := Default_tipd;
        tipd_A15                 : VitalDelayType01 := Default_tipd;
        tipd_A16                 : VitalDelayType01 := Default_tipd;
        tipd_A17                 : VitalDelayType01 := Default_tipd;
        tipd_A18                 : VitalDelayType01 := Default_tipd;
        tipd_DQA0                : VitalDelayType01 := Default_tipd;
        tipd_DQA1                : VitalDelayType01 := Default_tipd;
        tipd_DQA2                : VitalDelayType01 := Default_tipd;
        tipd_DQA3                : VitalDelayType01 := Default_tipd;
        tipd_DQA4                : VitalDelayType01 := Default_tipd;
        tipd_DQA5                : VitalDelayType01 := Default_tipd;
        tipd_DQA6                : VitalDelayType01 := Default_tipd;
        tipd_DQA7                : VitalDelayType01 := Default_tipd;
        tipd_DPA                 : VitalDelayType01 := Default_tipd;
        tipd_DQB0                : VitalDelayType01 := Default_tipd;
        tipd_DQB1                : VitalDelayType01 := Default_tipd;
        tipd_DQB2                : VitalDelayType01 := Default_tipd;
        tipd_DQB3                : VitalDelayType01 := Default_tipd;
        tipd_DQB4                : VitalDelayType01 := Default_tipd;
        tipd_DQB5                : VitalDelayType01 := Default_tipd;
        tipd_DQB6                : VitalDelayType01 := Default_tipd;
        tipd_DQB7                : VitalDelayType01 := Default_tipd;
        tipd_DPB                 : VitalDelayType01 := Default_tipd;
        tipd_DQC0                : VitalDelayType01 := Default_tipd;
        tipd_DQC1                : VitalDelayType01 := Default_tipd;
        tipd_DQC2                : VitalDelayType01 := Default_tipd;
        tipd_DQC3                : VitalDelayType01 := Default_tipd;
        tipd_DQC4                : VitalDelayType01 := Default_tipd;
        tipd_DQC5                : VitalDelayType01 := Default_tipd;
        tipd_DQC6                : VitalDelayType01 := Default_tipd;
        tipd_DQC7                : VitalDelayType01 := Default_tipd;
        tipd_DPC                 : VitalDelayType01 := Default_tipd;
        tipd_DQD0                : VitalDelayType01 := Default_tipd;
        tipd_DQD1                : VitalDelayType01 := Default_tipd;
        tipd_DQD2                : VitalDelayType01 := Default_tipd;
        tipd_DQD3                : VitalDelayType01 := Default_tipd;
        tipd_DQD4                : VitalDelayType01 := Default_tipd;
        tipd_DQD5                : VitalDelayType01 := Default_tipd;
        tipd_DQD6                : VitalDelayType01 := Default_tipd;
        tipd_DQD7                : VitalDelayType01 := Default_tipd;
        tipd_DPD                 : VitalDelayType01 := Default_tipd;
        tipd_BWANeg              : VitalDelayType01 := Default_tipd;
        tipd_BWBNeg              : VitalDelayType01 := Default_tipd;
        tipd_BWCNeg              : VitalDelayType01 := Default_tipd;
        tipd_BWDNeg              : VitalDelayType01 := Default_tipd;
        tipd_GWNeg               : VitalDelayType01 := Default_tipd;
        tipd_BWENeg              : VitalDelayType01 := Default_tipd;
        tipd_CLK                 : VitalDelayType01 := Default_tipd;
        tipd_CE1Neg              : VitalDelayType01 := Default_tipd;
        tipd_CE2                 : VitalDelayType01 := Default_tipd;
        tipd_CE3Neg              : VitalDelayType01 := Default_tipd;
        tipd_OENeg               : VitalDelayType01 := Default_tipd;
        tipd_ADVNeg              : VitalDelayType01 := Default_tipd;
        tipd_ADSPNeg             : VitalDelayType01 := Default_tipd;
        tipd_ADSCNeg             : VitalDelayType01 := Default_tipd;
        tipd_MODE                : VitalDelayType01 := VitalZeroDelay01;
        tipd_ZZ                  : VitalDelayType01 := VitalZeroDelay01;
        -- tpd delays
        tpd_CLK_DQA0             : VitalDelayType01Z := Default_tpd;
        tpd_OENeg_DQA0           : VitalDelayType01Z := Default_tpd;
        -- tpw values: pulse widths
        tpw_CLK_posedge        : VitalDelayType := Default_tpw;
        tpw_CLK_negedge        : VitalDelayType := Default_tpw;
        -- tperiod min (calculated as 1/max freq)
        tperiod_CLK_posedge    : VitalDelayType := Default_period;
        -- tsetup values: setup times
        tsetup_A0_CLK           : VitalDelayType := Default_tsetup;
        tsetup_DQA0_CLK         : VitalDelayType := Default_tsetup;
        tsetup_ADVNeg_CLK       : VitalDelayType := Default_tsetup;
        tsetup_ADSCNeg_CLK      : VitalDelayType := Default_tsetup;
        tsetup_CE2_CLK          : VitalDelayType := Default_tsetup;
        tsetup_BWANeg_CLK       : VitalDelayType := Default_tsetup;
        -- thold values: hold times
        thold_A0_CLK            : VitalDelayType := Default_thold;
        thold_DQA0_CLK          : VitalDelayType := Default_thold;
        thold_ADVNeg_CLK        : VitalDelayType := Default_thold;
        thold_ADSCNeg_CLK       : VitalDelayType := Default_thold;
        thold_CE2_CLK           : VitalDelayType := Default_thold;
        thold_BWANeg_CLK        : VitalDelayType := Default_thold;
        thold_ADSCNeg_ZZ        : VitalDelayType := UnitDelay;
        -- generic control parameters
        InstancePath        : STRING    := DefaultInstancePath;
        TimingChecksOn      : BOOLEAN   := DefaultTimingChecks;
        MsgOn               : BOOLEAN   := DefaultMsgOn;
        XOn                 : BOOLEAN   := DefaultXon;
        SeverityMode        : SEVERITY_LEVEL := WARNING;
        -- memory file to be loaded
        mem_file_name       : STRING    := "mem_main.dat";
        -- For FMF SDF technology file usage
        TimingModel         : STRING    := DefaultTimingModel
    );
    PORT (
        A0              : IN    std_logic := 'U';
        A1              : IN    std_logic := 'U';
        A2              : IN    std_logic := 'U';
        A3              : IN    std_logic := 'U';
        A4              : IN    std_logic := 'U';
        A5              : IN    std_logic := 'U';
        A6              : IN    std_logic := 'U';
        A7              : IN    std_logic := 'U';
        A8              : IN    std_logic := 'U';
        A9              : IN    std_logic := 'U';
        A10             : IN    std_logic := 'U';
        A11             : IN    std_logic := 'U';
        A12             : IN    std_logic := 'U';
        A13             : IN    std_logic := 'U';
        A14             : IN    std_logic := 'U';
        A15             : IN    std_logic := 'U';
        A16             : IN    std_logic := 'U';
        A17             : IN    std_logic := 'U';
        A18             : IN    std_logic := 'U';
        DQA0            : INOUT std_logic := 'U';
        DQA1            : INOUT std_logic := 'U';
        DQA2            : INOUT std_logic := 'U';
        DQA3            : INOUT std_logic := 'U';
        DQA4            : INOUT std_logic := 'U';
        DQA5            : INOUT std_logic := 'U';
        DQA6            : INOUT std_logic := 'U';
        DQA7            : INOUT std_logic := 'U';
        DPA             : INOUT std_logic := 'U';
        DQB0            : INOUT std_logic := 'U';
        DQB1            : INOUT std_logic := 'U';
        DQB2            : INOUT std_logic := 'U';
        DQB3            : INOUT std_logic := 'U';
        DQB4            : INOUT std_logic := 'U';
        DQB5            : INOUT std_logic := 'U';
        DQB6            : INOUT std_logic := 'U';
        DQB7            : INOUT std_logic := 'U';
        DPB             : INOUT std_logic := 'U';
        DQC0            : INOUT std_logic := 'U';
        DQC1            : INOUT std_logic := 'U';
        DQC2            : INOUT std_logic := 'U';
        DQC3            : INOUT std_logic := 'U';
        DQC4            : INOUT std_logic := 'U';
        DQC5            : INOUT std_logic := 'U';
        DQC6            : INOUT std_logic := 'U';
        DQC7            : INOUT std_logic := 'U';
        DPC             : INOUT std_logic := 'U';
        DQD0            : INOUT std_logic := 'U';
        DQD1            : INOUT std_logic := 'U';
        DQD2            : INOUT std_logic := 'U';
        DQD3            : INOUT std_logic := 'U';
        DQD4            : INOUT std_logic := 'U';
        DQD5            : INOUT std_logic := 'U';
        DQD6            : INOUT std_logic := 'U';
        DQD7            : INOUT std_logic := 'U';
        DPD             : INOUT std_logic := 'U';
        BWANeg          : IN    std_logic := 'U';
        BWBNeg          : IN    std_logic := 'U';
        BWCNeg          : IN    std_logic := 'U';
        BWDNeg          : IN    std_logic := 'U';
        GWNeg           : IN    std_logic := 'U';
        BWENeg          : IN    std_logic := 'U';
        CLK             : IN    std_logic := 'U';
        CE1Neg          : IN    std_logic := 'U';
        CE2             : IN    std_logic := 'U';
        CE3Neg          : IN    std_logic := 'U';
        OENeg           : IN    std_logic := 'U';
        ADVNeg          : IN    std_logic := 'U';
        ADSPNeg         : IN    std_logic := 'U';
        ADSCNeg         : IN    std_logic := 'U';
        MODE            : IN    std_logic := 'U';
        ZZ              : IN    std_logic := 'U'
    );
    ATTRIBUTE VITAL_LEVEL0 of memory : ENTITY IS TRUE;
END memory;

--------------------------------------------------------------------------------
-- ARCHITECTURE DECLARATION
--------------------------------------------------------------------------------
ARCHITECTURE vhdl_behavioral of memory IS
    ATTRIBUTE VITAL_LEVEL0 of vhdl_behavioral : ARCHITECTURE IS TRUE;

    CONSTANT partID           : STRING := "cy7c1380";

    SIGNAL A0_ipd              : std_ulogic := 'U';
    SIGNAL A1_ipd              : std_ulogic := 'U';
    SIGNAL A2_ipd              : std_ulogic := 'U';
    SIGNAL A3_ipd              : std_ulogic := 'U';
    SIGNAL A4_ipd              : std_ulogic := 'U';
    SIGNAL A5_ipd              : std_ulogic := 'U';
    SIGNAL A6_ipd              : std_ulogic := 'U';
    SIGNAL A7_ipd              : std_ulogic := 'U';
    SIGNAL A8_ipd              : std_ulogic := 'U';
    SIGNAL A9_ipd              : std_ulogic := 'U';
    SIGNAL A10_ipd             : std_ulogic := 'U';
    SIGNAL A11_ipd             : std_ulogic := 'U';
    SIGNAL A12_ipd             : std_ulogic := 'U';
    SIGNAL A13_ipd             : std_ulogic := 'U';
    SIGNAL A14_ipd             : std_ulogic := 'U';
    SIGNAL A15_ipd             : std_ulogic := 'U';
    SIGNAL A16_ipd             : std_ulogic := 'U';
    SIGNAL A17_ipd             : std_ulogic := 'U';
    SIGNAL A18_ipd             : std_ulogic := 'U';
    SIGNAL DQA0_ipd            : std_ulogic := 'U';
    SIGNAL DQA1_ipd            : std_ulogic := 'U';
    SIGNAL DQA2_ipd            : std_ulogic := 'U';
    SIGNAL DQA3_ipd            : std_ulogic := 'U';
    SIGNAL DQA4_ipd            : std_ulogic := 'U';
    SIGNAL DQA5_ipd            : std_ulogic := 'U';
    SIGNAL DQA6_ipd            : std_ulogic := 'U';
    SIGNAL DQA7_ipd            : std_ulogic := 'U';
    SIGNAL DPA_ipd             : std_ulogic := 'U';
    SIGNAL DQB0_ipd            : std_ulogic := 'U';
    SIGNAL DQB1_ipd            : std_ulogic := 'U';
    SIGNAL DQB2_ipd            : std_ulogic := 'U';
    SIGNAL DQB3_ipd            : std_ulogic := 'U';
    SIGNAL DQB4_ipd            : std_ulogic := 'U';
    SIGNAL DQB5_ipd            : std_ulogic := 'U';
    SIGNAL DQB6_ipd            : std_ulogic := 'U';
    SIGNAL DQB7_ipd            : std_ulogic := 'U';
    SIGNAL DPB_ipd             : std_ulogic := 'U';
    SIGNAL DQC0_ipd            : std_ulogic := 'U';
    SIGNAL DQC1_ipd            : std_ulogic := 'U';
    SIGNAL DQC2_ipd            : std_ulogic := 'U';
    SIGNAL DQC3_ipd            : std_ulogic := 'U';
    SIGNAL DQC4_ipd            : std_ulogic := 'U';
    SIGNAL DQC5_ipd            : std_ulogic := 'U';
    SIGNAL DQC6_ipd            : std_ulogic := 'U';
    SIGNAL DQC7_ipd            : std_ulogic := 'U';
    SIGNAL DPC_ipd             : std_ulogic := 'U';
    SIGNAL DQD0_ipd            : std_ulogic := 'U';
    SIGNAL DQD1_ipd            : std_ulogic := 'U';
    SIGNAL DQD2_ipd            : std_ulogic := 'U';
    SIGNAL DQD3_ipd            : std_ulogic := 'U';
    SIGNAL DQD4_ipd            : std_ulogic := 'U';
    SIGNAL DQD5_ipd            : std_ulogic := 'U';
    SIGNAL DQD6_ipd            : std_ulogic := 'U';
    SIGNAL DQD7_ipd            : std_ulogic := 'U';
    SIGNAL DPD_ipd             : std_ulogic := 'U';
    SIGNAL BWANeg_ipd          : std_ulogic := 'U';
    SIGNAL BWBNeg_ipd          : std_ulogic := 'U';
    SIGNAL BWCNeg_ipd          : std_ulogic := 'U';
    SIGNAL BWDNeg_ipd          : std_ulogic := 'U';
    SIGNAL GWNeg_ipd           : std_ulogic := 'U';
    SIGNAL BWENeg_ipd          : std_ulogic := 'U';
    SIGNAL CLK_ipd             : std_ulogic := 'U';
    SIGNAL CE1Neg_ipd          : std_ulogic := 'U';
    SIGNAL CE2_ipd             : std_ulogic := 'U';
    SIGNAL OENeg_ipd           : std_ulogic := 'U';
    SIGNAL CE3Neg_ipd          : std_ulogic := 'U';
    SIGNAL ADVNeg_ipd          : std_ulogic := 'U';
    SIGNAL ADSPNeg_ipd         : std_ulogic := 'U';
    SIGNAL ADSCNeg_ipd         : std_ulogic := 'U';
    SIGNAL MODE_ipd            : std_ulogic := 'U';
    SIGNAL ZZ_ipd              : std_ulogic := 'U';

BEGIN

    ----------------------------------------------------------------------------
    -- Wire Delays
    ----------------------------------------------------------------------------
    WireDelay : BLOCK
    BEGIN

        w_1 : VitalWireDelay (A0_ipd, A0, tipd_A0);
        w_2 : VitalWireDelay (A1_ipd, A1, tipd_A1);
        w_3 : VitalWireDelay (A2_ipd, A2, tipd_A2);
        w_4 : VitalWireDelay (A3_ipd, A3, tipd_A3);
        w_5 : VitalWireDelay (A4_ipd, A4, tipd_A4);
        w_6 : VitalWireDelay (A5_ipd, A5, tipd_A5);
        w_7 : VitalWireDelay (A6_ipd, A6, tipd_A6);
        w_8 : VitalWireDelay (A7_ipd, A7, tipd_A7);
        w_9 : VitalWireDelay (A8_ipd, A8, tipd_A8);
        w_10 : VitalWireDelay (A9_ipd, A9, tipd_A9);
        w_11 : VitalWireDelay (A10_ipd, A10, tipd_A10);
        w_12 : VitalWireDelay (A11_ipd, A11, tipd_A11);
        w_13 : VitalWireDelay (A12_ipd, A12, tipd_A12);
        w_14 : VitalWireDelay (A13_ipd, A13, tipd_A13);
        w_15 : VitalWireDelay (A14_ipd, A14, tipd_A14);
        w_16 : VitalWireDelay (A15_ipd, A15, tipd_A15);
        w_17 : VitalWireDelay (A16_ipd, A16, tipd_A16);
        w_18 : VitalWireDelay (A17_ipd, A17, tipd_A17);
        w_19 : VitalWireDelay (A18_ipd, A18, tipd_A18);
        w_20 : VitalWireDelay (DQA0_ipd, DQA0, tipd_DQA0);
        w_21 : VitalWireDelay (DQA1_ipd, DQA1, tipd_DQA1);
        w_22 : VitalWireDelay (DQA2_ipd, DQA2, tipd_DQA2);
        w_24 : VitalWireDelay (DQA3_ipd, DQA3, tipd_DQA3);
        w_25 : VitalWireDelay (DQA4_ipd, DQA4, tipd_DQA4);
        w_26 : VitalWireDelay (DQA5_ipd, DQA5, tipd_DQA5);
        w_27 : VitalWireDelay (DQA6_ipd, DQA6, tipd_DQA6);
        w_28 : VitalWireDelay (DQA7_ipd, DQA7, tipd_DQA7);
        w_29 : VitalWireDelay (DPA_ipd, DPA, tipd_DPA);
        w_30 : VitalWireDelay (DQB0_ipd, DQB0, tipd_DQB0);
        w_31 : VitalWireDelay (DQB1_ipd, DQB1, tipd_DQB1);
        w_32 : VitalWireDelay (DQB2_ipd, DQB2, tipd_DQB2);
        w_33 : VitalWireDelay (DQB3_ipd, DQB3, tipd_DQB3);
        w_34 : VitalWireDelay (DQB4_ipd, DQB4, tipd_DQB4);
        w_35 : VitalWireDelay (DQB5_ipd, DQB5, tipd_DQB5);
        w_36 : VitalWireDelay (DQB6_ipd, DQB6, tipd_DQB6);
        w_37 : VitalWireDelay (DQB7_ipd, DQB7, tipd_DQB7);
        w_38 : VitalWireDelay (DPB_ipd, DPB, tipd_DPB);
        w_39 : VitalWireDelay (DQC0_ipd, DQC0, tipd_DQC0);
        w_40 : VitalWireDelay (DQC1_ipd, DQC1, tipd_DQC1);
        w_41 : VitalWireDelay (DQC2_ipd, DQC2, tipd_DQC2);
        w_42 : VitalWireDelay (DQC3_ipd, DQC3, tipd_DQC3);
        w_43 : VitalWireDelay (DQC4_ipd, DQC4, tipd_DQC4);
        w_44 : VitalWireDelay (DQC5_ipd, DQC5, tipd_DQC5);
        w_45 : VitalWireDelay (DQC6_ipd, DQC6, tipd_DQC6);
        w_46 : VitalWireDelay (DQC7_ipd, DQC7, tipd_DQC7);
        w_47 : VitalWireDelay (DPC_ipd, DPC, tipd_DPC);
        w_48 : VitalWireDelay (DQD0_ipd, DQD0, tipd_DQD0);
        w_49 : VitalWireDelay (DQD1_ipd, DQD1, tipd_DQD1);
        w_50 : VitalWireDelay (DQD2_ipd, DQD2, tipd_DQD2);
        w_51 : VitalWireDelay (DQD3_ipd, DQD3, tipd_DQD3);
        w_52 : VitalWireDelay (DQD4_ipd, DQD4, tipd_DQD4);
        w_53 : VitalWireDelay (DQD5_ipd, DQD5, tipd_DQD5);
        w_54 : VitalWireDelay (DQD6_ipd, DQD6, tipd_DQD6);
        w_55 : VitalWireDelay (DQD7_ipd, DQD7, tipd_DQD7);
        w_56 : VitalWireDelay (DPD_ipd, DPD, tipd_DPD);
        w_57 : VitalWireDelay (BWANeg_ipd, BWANeg, tipd_BWANeg);
        w_58 : VitalWireDelay (BWBNeg_ipd, BWBNeg, tipd_BWBNeg);
        w_59 : VitalWireDelay (BWCNeg_ipd, BWCNeg, tipd_BWCNeg);
        w_60 : VitalWireDelay (BWDNeg_ipd, BWDNeg, tipd_BWDNeg);
        w_61 : VitalWireDelay (GWNeg_ipd, GWNeg, tipd_GWNeg);
        w_62 : VitalWireDelay (BWENeg_ipd, BWENeg, tipd_BWENeg);
        w_63 : VitalWireDelay (CLK_ipd, CLK, tipd_CLK);
        w_64 : VitalWireDelay (CE1Neg_ipd, CE1Neg, tipd_CE1Neg);
        w_65 : VitalWireDelay (CE2_ipd, CE2, tipd_CE2);
        w_66 : VitalWireDelay (CE3Neg_ipd, CE3Neg, tipd_CE3Neg);
        w_67 : VitalWireDelay (OENeg_ipd, OENeg, tipd_OENeg);
        w_68 : VitalWireDelay (ADVNeg_ipd, ADVNeg, tipd_ADVNeg);
        w_69 : VitalWireDelay (ADSPNeg_ipd, ADSPNeg, tipd_ADSPNeg);
        w_70 : VitalWireDelay (ADSCNeg_ipd, ADSCNeg, tipd_ADSCNeg);
        w_71 : VitalWireDelay (MODE_ipd, MODE, tipd_MODE);
        w_72 : VitalWireDelay (ZZ_ipd, ZZ, tipd_ZZ);

    END BLOCK;


    ----------------------------------------------------------------------------
    -- Main Behavior Block
    ----------------------------------------------------------------------------
    Behavior: BLOCK

        PORT (
            BWANIn          : IN    std_ulogic := 'U';
            BWBNIn          : IN    std_ulogic := 'U';
            BWCNIn          : IN    std_ulogic := 'U';
            BWDNIn          : IN    std_ulogic := 'U';
            GWNIn           : IN    std_ulogic := 'U';
            BWENIn          : IN    std_ulogic := 'U';
            DatAIn          : IN    std_logic_vector(8 downto 0);
            DatBIn          : IN    std_logic_vector(8 downto 0);
            DatCIn          : IN    std_logic_vector(8 downto 0);
            DatDIn          : IN    std_logic_vector(8 downto 0);
            DataOut         : OUT   std_logic_vector(35 downto 0)
                                                     := (others => 'Z');
            CLKIn           : IN    std_ulogic := 'U';
            AddressIn       : IN    std_logic_vector(18 downto 0);
            OENegIn         : IN    std_ulogic := 'U';
            ADVNIn          : IN    std_ulogic := 'U';
            ADSPNIn         : IN    std_ulogic := 'U';
            ADSCNIn         : IN    std_ulogic := 'U';
            MODEIn          : IN    std_ulogic := 'U';
            ZZIn            : IN    std_ulogic := 'U';
            CE2In           : IN    std_ulogic := 'U';
            CE1NegIn        : IN    std_ulogic := 'U';
            CE3NegIn        : IN    std_ulogic := 'U'
        );
        PORT MAP (
            BWANIn => BWANeg_ipd,
            BWBNIn => BWBNeg_ipd,
            BWCNIn => BWCNeg_ipd,
            BWDNIn => BWDNeg_ipd,
            GWNIn => GWNeg_ipd,
            BWENIn => BWENeg_ipd,
            CLKIn => CLK_ipd,
            OENegIn => OENeg_ipd,
            ADVNIn => ADVNeg_ipd,
            ADSPNIn => ADSPNeg_ipd,
            ADSCNIn => ADSCNeg_ipd,
            MODEIn => MODE_ipd,
            ZZIn => ZZ_ipd,
            CE2In => CE2_ipd,
            CE1NegIn => CE1Neg_ipd,
            CE3NegIn => CE3Neg_ipd,
            DataOut(0) =>  DQA0,
            DataOut(1) =>  DQA1,
            DataOut(2) =>  DQA2,
            DataOut(3) =>  DQA3,
            DataOut(4) =>  DQA4,
            DataOut(5) =>  DQA5,
            DataOut(6) =>  DQA6,
            DataOut(7) =>  DQA7,
            DataOut(8) =>  DPA,
            DataOut(9) =>  DQB0,
            DataOut(10) =>  DQB1,
            DataOut(11) =>  DQB2,
            DataOut(12) =>  DQB3,
            DataOut(13) =>  DQB4,
            DataOut(14) =>  DQB5,
            DataOut(15) =>  DQB6,
            DataOut(16) =>  DQB7,
            DataOut(17) =>  DPB,
            DataOut(18) =>  DQC0,
            DataOut(19) =>  DQC1,
            DataOut(20) =>  DQC2,
            DataOut(21) =>  DQC3,
            DataOut(22) =>  DQC4,
            DataOut(23) =>  DQC5,
            DataOut(24) =>  DQC6,
            DataOut(25) =>  DQC7,
            DataOut(26) =>  DPC,
            DataOut(27) =>  DQD0,
            DataOut(28) =>  DQD1,
            DataOut(29) =>  DQD2,
            DataOut(30) =>  DQD3,
            DataOut(31) =>  DQD4,
            DataOut(32) =>  DQD5,
            DataOut(33) =>  DQD6,
            DataOut(34) =>  DQD7,
            DataOut(35) =>  DPD,
            DatAIn(0) =>  DQA0_ipd,
            DatAIn(1) =>  DQA1_ipd,
            DatAIn(2) =>  DQA2_ipd,
            DatAIn(3) =>  DQA3_ipd,
            DatAIn(4) =>  DQA4_ipd,
            DatAIn(5) =>  DQA5_ipd,
            DatAIn(6) =>  DQA6_ipd,
            DatAIn(7) =>  DQA7_ipd,
            DatAIn(8) =>  DPA_ipd,
            DatBIn(0) =>  DQB0_ipd,
            DatBIn(1) =>  DQB1_ipd,
            DatBIn(2) =>  DQB2_ipd,
            DatBIn(3) =>  DQB3_ipd,
            DatBIn(4) =>  DQB4_ipd,
            DatBIn(5) =>  DQB5_ipd,
            DatBIn(6) =>  DQB6_ipd,
            DatBIn(7) =>  DQB7_ipd,
            DatBIn(8) =>  DPB_ipd,
            DatCIn(0) =>  DQC0_ipd,
            DatCIn(1) =>  DQC1_ipd,
            DatCIn(2) =>  DQC2_ipd,
            DatCIn(3) =>  DQC3_ipd,
            DatCIn(4) =>  DQC4_ipd,
            DatCIn(5) =>  DQC5_ipd,
            DatCIn(6) =>  DQC6_ipd,
            DatCIn(7) =>  DQC7_ipd,
            DatCIn(8) =>  DPC_ipd,
            DatDIn(0) =>  DQD0_ipd,
            DatDIn(1) =>  DQD1_ipd,
            DatDIn(2) =>  DQD2_ipd,
            DatDIn(3) =>  DQD3_ipd,
            DatDIn(4) =>  DQD4_ipd,
            DatDIn(5) =>  DQD5_ipd,
            DatDIn(6) =>  DQD6_ipd,
            DatDIn(7) =>  DQD7_ipd,
            DatDIn(8) =>  DPD_ipd,
            AddressIn(0) => A0_ipd,
            AddressIn(1) => A1_ipd,
            AddressIn(2) => A2_ipd,
            AddressIn(3) => A3_ipd,
            AddressIn(4) => A4_ipd,
            AddressIn(5) => A5_ipd,
            AddressIn(6) => A6_ipd,
            AddressIn(7) => A7_ipd,
            AddressIn(8) => A8_ipd,
            AddressIn(9) => A9_ipd,
            AddressIn(10) => A10_ipd,
            AddressIn(11) => A11_ipd,
            AddressIn(12) => A12_ipd,
            AddressIn(13) => A13_ipd,
            AddressIn(14) => A14_ipd,
            AddressIn(15) => A15_ipd,
            AddressIn(16) => A16_ipd,
            AddressIn(17) => A17_ipd,
            AddressIn(18) => A18_ipd
        );
        -- Type definition for state machine
        TYPE mem_state IS (desel,
                           begin_rd,
                           begin_SPwrite,
                           SPwrite,
                           SCwrite,
                           read
                          );

        SIGNAL state     : mem_state;

        TYPE sequence IS ARRAY (0 to 3) OF INTEGER RANGE -3 to 3;
        TYPE seqtab IS ARRAY (0 to 3) OF sequence;

        FILE mem_file    : text IS mem_file_name;

        CONSTANT il0 : sequence := (0, 1, 2, 3);
        CONSTANT il1 : sequence := (0, -1, 2, 1);
        CONSTANT il2 : sequence := (0, 1, -2, -1);
        CONSTANT il3 : sequence := (0, -1, -2, -3);
        CONSTANT il  : seqtab := (il0, il1, il2, il3);

        CONSTANT ln0 : sequence := (0, 1, 2, 3);
        CONSTANT ln1 : sequence := (0, 1, 2, -1);
        CONSTANT ln2 : sequence := (0, 1, -2, -1);
        CONSTANT ln3 : sequence := (0, -3, -2, -1);
        CONSTANT ln  : seqtab := (ln0, ln1, ln2, ln3);

        SIGNAL Burst_Seq : seqtab;

        SIGNAL D_zd      : std_logic_vector(35 DOWNTO 0);

    BEGIN

        Burst_Setup : PROCESS

        BEGIN

           IF (MODEIn = '1') THEN
               Burst_Seq <= il;
           ELSE
               Burst_Seq <= ln;
           END IF;
           WAIT;  -- static pin during device operation

        END PROCESS Burst_Setup;

    ----------------------------------------------------------------------------
    -- Main Behavior Process
    ----------------------------------------------------------------------------
        Behavior : PROCESS (BWANIn, BWBNIn, BWCNIn, BWDNIn, DatAIn, DatBIn,
                            DatCIn, DatDIN, CLKIn, AddressIn, GWNIn, BWENIn,
                            OENegIn, ADVNIn, ADSPNIn, ADSCNIn, CE2In, CE1NegIn,
                            CE3NegIn, ZZIn)

            -- Type definition for commands
            TYPE command_type is (ds,
                                  begin_SPwr,
                                  SPwr_burst,
                                  SPwr_susp,
                                  SCwr,
                                  begin_read,
                                  read_burst,
                                  read_susp
                                 );

            -- Timing Check Variables

            VARIABLE Tviol_AddressIn_CLK      : X01 := '0';
            VARIABLE TD_AddressIn_CLK         : VitalTimingDataType;

            VARIABLE Tviol_DatDIn_CLK   : X01 := '0';
            VARIABLE TD_DatDIn_CLK      : VitalTimingDataType;

            VARIABLE Tviol_DatCIn_CLK   : X01 := '0';
            VARIABLE TD_DatCIn_CLK      : VitalTimingDataType;

            VARIABLE Tviol_DatBIn_CLK   : X01 := '0';
            VARIABLE TD_DatBIn_CLK      : VitalTimingDataType;

            VARIABLE Tviol_DatAIn_CLK   : X01 := '0';
            VARIABLE TD_DatAIn_CLK      : VitalTimingDataType;

            VARIABLE Tviol_BWDN_CLK     : X01 := '0';
            VARIABLE TD_BWDN_CLK        : VitalTimingDataType;

            VARIABLE Tviol_BWCN_CLK     : X01 := '0';
            VARIABLE TD_BWCN_CLK        : VitalTimingDataType;

            VARIABLE Tviol_BWBN_CLK     : X01 := '0';
            VARIABLE TD_BWBN_CLK        : VitalTimingDataType;

            VARIABLE Tviol_BWAN_CLK     : X01 := '0';
            VARIABLE TD_BWAN_CLK        : VitalTimingDataType;

            VARIABLE Tviol_BWEN_CLK     : X01 := '0';
            VARIABLE TD_BWEN_CLK        : VitalTimingDataType;

            VARIABLE Tviol_GWN_CLK     : X01 := '0';
            VARIABLE TD_GWN_CLK        : VitalTimingDataType;

            VARIABLE Tviol_ADVNIn_CLK     : X01 := '0';
            VARIABLE TD_ADVNIn_CLK        : VitalTimingDataType;

            VARIABLE Tviol_ADSCNIn_CLK     : X01 := '0';
            VARIABLE TD_ADSCNIn_CLK        : VitalTimingDataType;

            VARIABLE Tviol_ADSPNIn_CLK     : X01 := '0';
            VARIABLE TD_ADSPNIn_CLK        : VitalTimingDataType;

            VARIABLE Tviol_CE1NegIn_CLK : X01 := '0';
            VARIABLE TD_CE1NegIn_CLK    : VitalTimingDataType;

            VARIABLE Tviol_CE3NegIn_CLK : X01 := '0';
            VARIABLE TD_CE3NegIn_CLK    : VitalTimingDataType;

            VARIABLE Tviol_CE2In_CLK    : X01 := '0';
            VARIABLE TD_CE2In_CLK       : VitalTimingDataType;

            VARIABLE Tviol_ADSCNIn_ZZ     : X01 := '0';
            VARIABLE TD_ADSCNIn_ZZ        : VitalTimingDataType;

            VARIABLE Tviol_ADSPNIn_ZZ     : X01 := '0';
            VARIABLE TD_ADSPNIn_ZZ        : VitalTimingDataType;

            VARIABLE Tviol_CE1NegIn_ZZ : X01 := '0';
            VARIABLE TD_CE1NegIn_ZZ    : VitalTimingDataType;

            VARIABLE Tviol_CE3NegIn_ZZ : X01 := '0';
            VARIABLE TD_CE3NegIn_ZZ    : VitalTimingDataType;

            VARIABLE Tviol_CE2In_ZZ    : X01 := '0';
            VARIABLE TD_CE2In_ZZ       : VitalTimingDataType;

            VARIABLE Pviol_CLK    : X01 := '0';
            VARIABLE PD_CLK       : VitalPeriodDataType := VitalPeriodDataInit;

            -- Memory array declaration
            TYPE MemStore IS ARRAY (0 to 524288) OF INTEGER
                             RANGE  -2 TO 511;

            VARIABLE MemDataA   : MemStore;
            VARIABLE MemDataB   : MemStore;
            VARIABLE MemDataC   : MemStore;
            VARIABLE MemDataD   : MemStore;

            VARIABLE MemAddr    : NATURAL RANGE 0 TO 524288;
            VARIABLE startaddr  : NATURAL RANGE 0 TO 524288;

            VARIABLE Burst_Cnt  : NATURAL RANGE 0 TO 4 := 0;
            VARIABLE memstart   : NATURAL RANGE 0 TO 3 := 0;
            VARIABLE offset     : INTEGER RANGE -3 TO 3 := 0;

            VARIABLE ind     : NATURAL := 0;
            VARIABLE buf     : line;
			VARIABLE val	 : integer;


            VARIABLE command    : command_type;
            VARIABLE R          : std_logic;

            VARIABLE cycle1     : BOOLEAN := false;
            VARIABLE cycle2     : BOOLEAN := false;

            -- Functionality Results Variables
            VARIABLE Violation  : X01 := '0';

            VARIABLE OBuf1      : std_logic_vector(35 DOWNTO 0)
                                   := (OTHERS => 'Z');
            VARIABLE OBuf2      : std_logic_vector(35 DOWNTO 0)
                                   := (OTHERS => 'Z');

        BEGIN

            --------------------------------------------------------------------
            -- Timing Check Section
            --------------------------------------------------------------------
            IF (TimingChecksOn) THEN

                VitalSetupHoldCheck (
                    TestSignal      => AddressIn,
                    TestSignalName  => "Address",
                    RefSignal       => CLKIn,
                    RefSignalName   => "CLK",
                    SetupHigh       => tsetup_A0_CLK,
                    SetupLow        => tsetup_A0_CLK,
                    HoldHigh        => thold_A0_CLK,
                    HoldLow         => thold_A0_CLK,
                    CheckEnabled    => (ZZIn = '0'),
                    RefTransition   => '/',
                    HeaderMsg       => InstancePath & PartID,
                    TimingData      => TD_AddressIn_CLK,
                    XOn             => XOn,
                    MsgOn           => MsgOn,
                    Violation       => Tviol_AddressIn_CLK );

                VitalSetupHoldCheck (
                    TestSignal      => DatDIn,
                    TestSignalName  => "DatD",
                    RefSignal       => CLKIn,
                    RefSignalName   => "CLK",
                    SetupHigh       => tsetup_DQA0_CLK,
                    SetupLow        => tsetup_DQA0_CLK,
                    HoldHigh        => thold_DQA0_CLK,
                    HoldLow         => thold_DQA0_CLK,
                    CheckEnabled    => (ZZIn ='0'),
                    RefTransition   => '/',
                    HeaderMsg       => InstancePath & PartID,
                    TimingData      => TD_DatDIn_CLK,
                    XOn             => XOn,
                    MsgOn           => MsgOn,
                    Violation       => Tviol_DatDIn_CLK );

                VitalSetupHoldCheck (
                    TestSignal      => DatCIn,
                    TestSignalName  => "DatC",
                    RefSignal       => CLKIn,
                    RefSignalName   => "CLK",
                    SetupHigh       => tsetup_DQA0_CLK,
                    SetupLow        => tsetup_DQA0_CLK,
                    HoldHigh        => thold_DQA0_CLK,
                    HoldLow         => thold_DQA0_CLK,
                    CheckEnabled    => (ZZIn ='0'),
                    RefTransition   => '/',
                    HeaderMsg       => InstancePath & PartID,
                    TimingData      => TD_DatCIn_CLK,
                    XOn             => XOn,
                    MsgOn           => MsgOn,
                    Violation       => Tviol_DatCIn_CLK );

                VitalSetupHoldCheck (
                    TestSignal      => DatBIn,
                    TestSignalName  => "DatB",
                    RefSignal       => CLKIn,
                    RefSignalName   => "CLK",
                    SetupHigh       => tsetup_DQA0_CLK,
                    SetupLow        => tsetup_DQA0_CLK,
                    HoldHigh        => thold_DQA0_CLK,
                    HoldLow         => thold_DQA0_CLK,
                    CheckEnabled    => (ZZIn ='0'),
                    RefTransition   => '/',
                    HeaderMsg       => InstancePath & PartID,
                    TimingData      => TD_DatBIn_CLK,
                    XOn             => XOn,
                    MsgOn           => MsgOn,
                    Violation       => Tviol_DatBIn_CLK );

                VitalSetupHoldCheck (
                    TestSignal      => DatAIn,
                    TestSignalName  => "DatA",
                    RefSignal       => CLKIn,
                    RefSignalName   => "CLK",
                    SetupHigh       => tsetup_DQA0_CLK,
                    SetupLow        => tsetup_DQA0_CLK,
                    HoldHigh        => thold_DQA0_CLK,
                    HoldLow         => thold_DQA0_CLK,
                    CheckEnabled    => (ZZIn ='0'),
                    RefTransition   => '/',
                    HeaderMsg       => InstancePath & PartID,
                    TimingData      => TD_DatAIn_CLK,
                    XOn             => XOn,
                    MsgOn           => MsgOn,
                    Violation       => Tviol_DatAIn_CLK );

                VitalSetupHoldCheck (
                    TestSignal      => GWNIn,
                    TestSignalName  => "GW",
                    RefSignal       => CLKIn,
                    RefSignalName   => "CLK",
                    SetupHigh       => tsetup_BWANeg_CLK,
                    SetupLow        => tsetup_BWANeg_CLK,
                    HoldHigh        => thold_BWANeg_CLK,
                    HoldLow         => thold_BWANeg_CLK,
                    CheckEnabled    => (ZZIn ='0'),
                    RefTransition   => '/',
                    HeaderMsg       => InstancePath & PartID,
                    TimingData      => TD_GWN_CLK,
                    XOn             => XOn,
                    MsgOn           => MsgOn,
                    Violation       => Tviol_GWN_CLK );

                VitalSetupHoldCheck (
                    TestSignal      => BWENIn,
                    TestSignalName  => "BWE",
                    RefSignal       => CLKIn,
                    RefSignalName   => "CLK",
                    SetupHigh       => tsetup_BWANeg_CLK,
                    SetupLow        => tsetup_BWANeg_CLK,
                    HoldHigh        => thold_BWANeg_CLK,
                    HoldLow         => thold_BWANeg_CLK,
                    CheckEnabled    => (ZZIn ='0'),
                    RefTransition   => '/',
                    HeaderMsg       => InstancePath & PartID,
                    TimingData      => TD_BWEN_CLK,
                    XOn             => XOn,
                    MsgOn           => MsgOn,
                    Violation       => Tviol_BWEN_CLK );

                VitalSetupHoldCheck (
                    TestSignal      => BWDNIn,
                    TestSignalName  => "BWD",
                    RefSignal       => CLKIn,
                    RefSignalName   => "CLK",
                    SetupHigh       => tsetup_BWANeg_CLK,
                    SetupLow        => tsetup_BWANeg_CLK,
                    HoldHigh        => thold_BWANeg_CLK,
                    HoldLow         => thold_BWANeg_CLK,
                    CheckEnabled    => (ZZIn ='0'),
                    RefTransition   => '/',
                    HeaderMsg       => InstancePath & PartID,
                    TimingData      => TD_BWDN_CLK,
                    XOn             => XOn,
                    MsgOn           => MsgOn,
                    Violation       => Tviol_BWDN_CLK );

                VitalSetupHoldCheck (
                    TestSignal      => BWCNIn,
                    TestSignalName  => "BWC",
                    RefSignal       => CLKIn,
                    RefSignalName   => "CLK",
                    SetupHigh       => tsetup_BWANeg_CLK,
                    SetupLow        => tsetup_BWANeg_CLK,
                    HoldHigh        => thold_BWANeg_CLK,
                    HoldLow         => thold_BWANeg_CLK,
                    CheckEnabled    => (ZZIn ='0'),
                    RefTransition   => '/',
                    HeaderMsg       => InstancePath & PartID,
                    TimingData      => TD_BWCN_CLK,
                    XOn             => XOn,
                    MsgOn           => MsgOn,
                    Violation       => Tviol_BWCN_CLK );

                VitalSetupHoldCheck (
                    TestSignal      => BWBNIn,
                    TestSignalName  => "BWB",
                    RefSignal       => CLKIn,
                    RefSignalName   => "CLK",
                    SetupHigh       => tsetup_BWANeg_CLK,
                    SetupLow        => tsetup_BWANeg_CLK,
                    HoldHigh        => thold_BWANeg_CLK,
                    HoldLow         => thold_BWANeg_CLK,
                    CheckEnabled    => (ZZIn ='0'),
                    RefTransition   => '/',
                    HeaderMsg       => InstancePath & PartID,
                    TimingData      => TD_BWBN_CLK,
                    XOn             => XOn,
                    MsgOn           => MsgOn,
                    Violation       => Tviol_BWBN_CLK );

                VitalSetupHoldCheck (
                    TestSignal      => BWANIn,
                    TestSignalName  => "BWA",
                    RefSignal       => CLKIn,
                    RefSignalName   => "CLK",
                    SetupHigh       => tsetup_BWANeg_CLK,
                    SetupLow        => tsetup_BWANeg_CLK,
                    HoldHigh        => thold_BWANeg_CLK,
                    HoldLow         => thold_BWANeg_CLK,
                    CheckEnabled    => (ZZIn ='0'),
                    RefTransition   => '/',
                    HeaderMsg       => InstancePath & PartID,
                    TimingData      => TD_BWAN_CLK,
                    XOn             => XOn,
                    MsgOn           => MsgOn,
                    Violation       => Tviol_BWAN_CLK );

                VitalSetupHoldCheck (
                    TestSignal      => ADVNIn,
                    TestSignalName  => "ADV",
                    RefSignal       => CLKIn,
                    RefSignalName   => "CLK",
                    SetupHigh       => tsetup_ADVNeg_CLK,
                    SetupLow        => tsetup_ADVNeg_CLK,
                    HoldHigh        => thold_ADVNeg_CLK,
                    HoldLow         => thold_ADVNeg_CLK,
                    CheckEnabled    => (ZZIn ='0'),
                    RefTransition   => '/',
                    HeaderMsg       => InstancePath & PartID,
                    TimingData      => TD_ADVNIn_CLK,
                    XOn             => XOn,
                    MsgOn           => MsgOn,
                    Violation       => Tviol_ADVNIn_CLK );

                VitalSetupHoldCheck (
                    TestSignal      => ADSPNIn,
                    TestSignalName  => "ADSP",
                    RefSignal       => CLKIn,
                    RefSignalName   => "CLK",
                    SetupHigh       => tsetup_ADSCNeg_CLK,
                    SetupLow        => tsetup_ADSCNeg_CLK,
                    HoldHigh        => thold_ADSCNeg_CLK,
                    HoldLow         => thold_ADSCNeg_CLK,
                    CheckEnabled    => (ZZIn ='0'),
                    RefTransition   => '/',
                    HeaderMsg       => InstancePath & PartID,
                    TimingData      => TD_ADSPNIn_CLK,
                    XOn             => XOn,
                    MsgOn           => MsgOn,
                    Violation       => Tviol_ADSPNIn_CLK );

                VitalSetupHoldCheck (
                    TestSignal      => ADSPNIn,
                    TestSignalName  => "ADSP",
                    RefSignal       => ZZIn,
                    RefSignalName   => "ZZ",
                    HoldHigh        => thold_ADSCNeg_ZZ,
                    CheckEnabled    => true,
                    RefTransition   => '\',
                    HeaderMsg       => InstancePath & PartID,
                    TimingData      => TD_ADSPNIn_ZZ,
                    XOn             => XOn,
                    MsgOn           => MsgOn,
                    Violation       => Tviol_ADSPNIn_ZZ );

                VitalSetupHoldCheck (
                    TestSignal      => ADSCNIn,
                    TestSignalName  => "ADSC",
                    RefSignal       => ZZIn,
                    RefSignalName   => "ZZ",
                    HoldHigh        => thold_ADSCNeg_ZZ,
                    CheckEnabled    => true,
                    RefTransition   => '\',
                    HeaderMsg       => InstancePath & PartID,
                    TimingData      => TD_ADSCNIn_ZZ,
                    XOn             => XOn,
                    MsgOn           => MsgOn,
                    Violation       => Tviol_ADSCNIn_ZZ );

                VitalSetupHoldCheck (
                    TestSignal      => ADSCNIn,
                    TestSignalName  => "ADSC",
                    RefSignal       => CLKIn,
                    RefSignalName   => "CLK",
                    SetupHigh       => tsetup_ADSCNeg_CLK,
                    SetupLow        => tsetup_ADSCNeg_CLK,
                    HoldHigh        => thold_ADSCNeg_CLK,
                    HoldLow         => thold_ADSCNeg_CLK,
                    CheckEnabled    => (ZZIn ='0'),
                    RefTransition   => '/',
                    HeaderMsg       => InstancePath & PartID,
                    TimingData      => TD_ADSCNIn_CLK,
                    XOn             => XOn,
                    MsgOn           => MsgOn,
                    Violation       => Tviol_ADSCNIn_CLK );

                VitalSetupHoldCheck (
                    TestSignal      => CE1NegIn,
                    TestSignalName  => "CE1Neg",
                    RefSignal       => CLKIn,
                    RefSignalName   => "CLK",
                    SetupHigh       => tsetup_CE2_CLK,
                    SetupLow        => tsetup_CE2_CLK,
                    HoldHigh        => thold_CE2_CLK,
                    HoldLow         => thold_CE2_CLK,
                    CheckEnabled    => (zzIn ='0'),
                    RefTransition   => '/',
                    HeaderMsg       => InstancePath & PartID,
                    TimingData      => TD_CE1NegIn_CLK,
                    XOn             => XOn,
                    MsgOn           => MsgOn,
                    Violation       => Tviol_CE1NegIn_CLK );

                VitalSetupHoldCheck (
                    TestSignal      => CE1NegIn,
                    TestSignalName  => "CE1Neg",
                    RefSignal       => ZZIn,
                    RefSignalName   => "ZZ",
                    HoldHigh        => thold_ADSCNeg_ZZ,
                    CheckEnabled    => true,
                    RefTransition   => '\',
                    HeaderMsg       => InstancePath & PartID,
                    TimingData      => TD_CE1NegIn_ZZ,
                    XOn             => XOn,
                    MsgOn           => MsgOn,
                    Violation       => Tviol_CE1NegIn_ZZ );

                VitalSetupHoldCheck (
                    TestSignal      => CE3NegIn,
                    TestSignalName  => "CE3Neg",
                    RefSignal       => CLKIn,
                    RefSignalName   => "CLK",
                    SetupHigh       => tsetup_CE2_CLK,
                    SetupLow        => tsetup_CE2_CLK,
                    HoldHigh        => thold_CE2_CLK,
                    HoldLow         => thold_CE2_CLK,
                    CheckEnabled    => (ZZIn ='0'),
                    RefTransition   => '/',
                    HeaderMsg       => InstancePath & PartID,
                    TimingData      => TD_CE3NegIn_CLK,
                    XOn             => XOn,
                    MsgOn           => MsgOn,
                    Violation       => Tviol_CE3NegIn_CLK );

                VitalSetupHoldCheck (
                    TestSignal      => CE3NegIn,
                    TestSignalName  => "CE3Neg",
                    RefSignal       => ZZIn,
                    RefSignalName   => "ZZ",
                    HoldHigh        => thold_ADSCNeg_ZZ,
                    CheckEnabled    => true,
                    RefTransition   => '\',
                    HeaderMsg       => InstancePath & PartID,
                    TimingData      => TD_CE3NegIn_ZZ,
                    XOn             => XOn,
                    MsgOn           => MsgOn,
                    Violation       => Tviol_CE3NegIn_ZZ );

                VitalSetupHoldCheck (
                    TestSignal      => CE2In,
                    TestSignalName  => "CE2",
                    RefSignal       => CLKIn,
                    RefSignalName   => "CLK",
                    SetupHigh       => tsetup_CE2_CLK,
                    SetupLow        => tsetup_CE2_CLK,
                    HoldHigh        => thold_CE2_CLK,
                    HoldLow         => thold_CE2_CLK,
                    CheckEnabled    => (ZZIn ='0'),
                    RefTransition   => '/',
                    HeaderMsg       => InstancePath & PartID,
                    TimingData      => TD_CE2In_CLK,
                    XOn             => XOn,
                    MsgOn           => MsgOn,
                    Violation       => Tviol_CE2In_CLK );

                VitalSetupHoldCheck (
                    TestSignal      => CE2In,
                    TestSignalName  => "CE2",
                    RefSignal       => ZZIn,
                    RefSignalName   => "ZZ",
                    HoldLow         => thold_ADSCNeg_ZZ,
                    CheckEnabled    => true,
                    RefTransition   => '\',
                    HeaderMsg       => InstancePath & PartID,
                    TimingData      => TD_CE2In_ZZ,
                    XOn             => XOn,
                    MsgOn           => MsgOn,
                    Violation       => Tviol_CE2In_ZZ );

                VitalPeriodPulseCheck (
                    TestSignal      =>  CLKIn,
                    TestSignalName  =>  "CLK",
                    Period          =>  tperiod_CLK_posedge,
                    PulseWidthLow   =>  tpw_CLK_negedge,
                    PulseWidthHigh  =>  tpw_CLK_posedge,
                    PeriodData      =>  PD_CLK,
                    XOn             =>  XOn,
                    MsgOn           =>  MsgOn,
                    Violation       =>  Pviol_CLK,
                    HeaderMsg       =>  InstancePath & PartID,
                    CheckEnabled    =>  (ZZIn ='0') );

                Violation := Pviol_CLK OR Tviol_DatAIn_CLK OR Tviol_DatBIn_CLK
                             OR Tviol_DatCIn_CLK OR Tviol_DatDIn_CLK
                             OR Tviol_AddressIn_CLK OR Tviol_ADSCNIn_CLK OR
                             Tviol_CE2In_CLK OR Tviol_CE3NegIn_CLK OR
                             Tviol_CE1NegIn_CLK OR Tviol_ADVNIn_CLK OR
                             Tviol_ADSPNIn_CLK OR Tviol_BWAN_CLK OR
                             Tviol_BWBN_CLK OR Tviol_BWCN_CLK OR
                             Tviol_BWDN_CLK OR Tviol_BWEN_CLK OR
                             Tviol_GWN_CLK OR Tviol_ADSCNIn_ZZ OR
                             Tviol_ADSPNIn_ZZ OR Tviol_CE1NegIn_ZZ
                             OR Tviol_CE2In_ZZ OR Tviol_CE3NegIn_ZZ;


                ASSERT Violation = '0'
                    REPORT InstancePath & partID & ": simulation may be" &
                           " inaccurate due to timing violations"
                    SEVERITY SeverityMode;

            END IF;

    ----------------------------------------------------------------------------
    -- Functional Section
    ----------------------------------------------------------------------------

    IF rising_edge(CLKIn) AND ZZIn = '0' THEN
        ASSERT (not(Is_X(BWANIn)))
            REPORT InstancePath & partID & ": Unusable value for BWAN"
            SEVERITY SeverityMode;
        ASSERT (not(Is_X(BWBNIn)))
            REPORT InstancePath & partID & ": Unusable value for BWBN"
            SEVERITY SeverityMode;
        ASSERT (not(Is_X(BWCNIn)))
            REPORT InstancePath & partID & ": Unusable value for BWCN"
            SEVERITY SeverityMode;
        ASSERT (not(Is_X(BWDNIn)))
            REPORT InstancePath & partID & ": Unusable value for BWDN"
            SEVERITY SeverityMode;
        ASSERT (not(Is_X(GWNIn)))
            REPORT InstancePath & partID & ": Unusable value for GWN"
            SEVERITY SeverityMode;
        ASSERT (not(Is_X(BWENIn)))
            REPORT InstancePath & partID & ": Unusable value for BWEN"
            SEVERITY SeverityMode;
        ASSERT (not(Is_X(ADVNIn)))
            REPORT InstancePath & partID & ": Unusable value for ADVN"
            SEVERITY SeverityMode;
        ASSERT (not(Is_X(ADSPNIn)))
            REPORT InstancePath & partID & ": Unusable value for ADSPN"
            SEVERITY SeverityMode;
        ASSERT (not(Is_X(ADSCNIn)))
            REPORT InstancePath & partID & ": Unusable value for ADSCN"
            SEVERITY SeverityMode;
        ASSERT (not(Is_X(CE2In)))
            REPORT InstancePath & partID & ": Unusable value for CE2"
            SEVERITY SeverityMode;
        ASSERT (not(Is_X(CE1NegIn)))
            REPORT InstancePath & partID & ": Unusable value for CE1Neg"
            SEVERITY SeverityMode;
        ASSERT (not(Is_X(CE3NegIn)))
            REPORT InstancePath & partID & ": Unusable value for CE3Neg"
            SEVERITY SeverityMode;

        -- Command Decode
        IF (GWNIn = '1' AND (BWENIn = '1' OR (BWENIn = '0' AND BWANIn = '1'
           AND BWBNIn = '1' AND BWCNIn = '1' AND BWDNIn = '1'))) THEN
            R := '1';
        ELSE
            R := '0';
        END IF;

        IF ((CE3NegIn = '1' AND CE1NegIn = '0') AND (ADSPNIn = '0' OR
              (ADSPNIn = '1' AND ADSCNIn = '0'))) OR
              (CE1NegIn = '1' AND ADSCNIn = '0') OR
              ((CE2In = '0' AND CE1NegIn = '0') AND (ADSPNIn = '0' OR
              (ADSPNIn = '1' AND ADSCNIn = '0'))) THEN
            command := ds;
        ELSIF ((CE3NegIn = '0' AND CE2In = '1' AND CE1NegIn = '0')
              AND (ADSPNIn = '0' OR (ADSPNIn = '1' AND ADSCNIn = '0'))
              AND R = '1') THEN
              command := begin_read;
        ELSIF (ADSCNIn = '1' AND (ADSPNIn = '1' OR CE1NegIn = '1')
              AND R = '1') THEN
            IF ADVNIn = '0' THEN
                command := read_burst;
            ELSE
                command := read_susp;
            END IF;
        ELSIF (CE3NegIn = '0' AND CE2In = '1' AND CE1NegIn = '0'
              AND ADSPNIn = '0' AND R = '0') THEN
            command := begin_SPwr;
        ELSIF (ADSCNIn = '1' AND (ADSPNIn = '1' OR CE1NegIn = '1')
              AND R = '0') THEN
            IF ADVNIn = '0' THEN
                command := SPwr_burst;
            ELSE
                command := SPwr_susp;
            END IF;
        ELSIF (CE3NegIn = '0' AND CE2In = '1' AND CE1NegIn = '0' AND
              ADSPNIn = '1' AND ADSCNIn = '0' AND R = '0') THEN
            command := SCwr;
        ELSE
            ASSERT false
                REPORT InstancePath & partID & ": Could not decode "
                       & "command."
                SEVERITY SeverityMode;
        END IF;

        OBuf2 := OBuf1;
        cycle2 := cycle1;

        -- The State Machine
        CASE state IS
            WHEN desel =>
                CASE command IS
                    WHEN ds =>
                        OBuf1 := (others => 'Z');
                        cycle1 := true;
                    WHEN begin_SPwr =>
                        cycle1 := false;
                        cycle2 := false;
                        state <= begin_SPwrite;
                        MemAddr := to_nat(AddressIn);
                        startaddr := MemAddr;
                        memstart := to_nat(AddressIn(1 downto 0));
                    WHEN begin_read =>
                        cycle1 := false;
                        cycle2 := false;
                        state <= begin_rd;
                        MemAddr := to_nat(AddressIn);
                        startaddr := MemAddr;
                        memstart := to_nat(AddressIn(1 downto 0));
                        IF MemDataA(MemAddr) = -2 THEN
                            OBuf1(8 downto 0) := (others => 'U');
                        ELSIF MemDataA(MemAddr) = -1 THEN
                            OBuf1(8 downto 0) := (others => 'X');
                        ELSE
                            OBuf1(8 downto 0) := to_slv(MemDataA(MemAddr),9);
                        END IF;
                        IF MemDataB(MemAddr) = -2 THEN
                            OBuf1(17 downto 9) := (others => 'U');
                        ELSIF MemDataB(MemAddr) = -1 THEN
                            OBuf1(17 downto 9) := (others => 'X');
                        ELSE
                            OBuf1(17 downto 9) := to_slv(MemDataB(MemAddr),9);
                        END IF;
                        IF MemDataC(MemAddr) = -2 THEN
                            OBuf1(26 downto 18) := (others => 'U');
                        ELSIF MemDataC(MemAddr) = -1 THEN
                            OBuf1(26 downto 18) := (others => 'X');
                        ELSE
                            OBuf1(26 downto 18) := to_slv(MemDataC(MemAddr),9);
                        END IF;
                        IF MemDataD(MemAddr) = -2 THEN
                            OBuf1(35 downto 27) := (others => 'U');
                        ELSIF MemDataD(MemAddr) = -1 THEN
                            OBuf1(35 downto 27) := (others => 'X');
                        ELSE
                            OBuf1(35 downto 27) := to_slv(MemDataD(MemAddr),9);
                        END IF;
                    WHEN SCwr =>
                        cycle1 := false;
                        cycle2 := false;
                        state <= SCwrite;
                        MemAddr := to_nat(AddressIn);
                        OBuf1 := (others => 'Z');
                        IF GWNIn = '0' THEN
                            MemDataA(MemAddr) := -1;
                            MemDataB(MemAddr) := -1;
                            MemDataC(MemAddr) := -1;
                            MemDataD(MemAddr) := -1;
                            IF Violation /= 'X' THEN
                                MemDataA(MemAddr) := to_nat(DatAIn);
                                MemDataB(MemAddr) := to_nat(DatBIn);
                                MemDataC(MemAddr) := to_nat(DatCIn);
                                MemDataD(MemAddr) := to_nat(DatDIn);
                            END IF;
                        ELSE
                            IF (BWANIn = '0') THEN
                                MemDataA(MemAddr) := -1;
                                IF Violation /= 'X' THEN
                                    MemDataA(MemAddr) := to_nat(DatAIn);
                                END IF;
                            END IF;
                            IF (BWBNIn = '0') THEN
                                MemDataB(MemAddr) := -1;
                                IF Violation /= 'X' THEN
                                    MemDataB(MemAddr) := to_nat(DatBIn);
                                END IF;
                            END IF;
                            IF (BWCNIn = '0') THEN
                                MemDataC(MemAddr) := -1;
                                IF Violation /= 'X' THEN
                                    MemDataC(MemAddr) := to_nat(DatCIn);
                                END IF;
                            END IF;
                            IF (BWDNIn = '0') THEN
                                MemDataD(MemAddr) := -1;
                                IF Violation /= 'X' THEN
                                    MemDataD(MemAddr) := to_nat(DatDIn);
                                END IF;
                            END IF;
                        END IF;
                    WHEN SPwr_burst =>
                        cycle1 := false;
                        cycle2 := false;
                        OBuf1 := (others => 'Z');
                    WHEN SPwr_susp =>
                        cycle1 := false;
                        cycle2 := false;
                        OBuf1 := (others => 'Z');
                    WHEN read_burst =>
                        cycle1 := false;
                        cycle2 := false;
                        OBuf1 := (others => 'Z');
                    WHEN read_susp =>
                        cycle1 := false;
                        cycle2 := false;
                        OBuf1 := (others => 'Z');
                END CASE;

            WHEN begin_rd =>
                Burst_Cnt := 0;
                CASE command IS
                    WHEN ds =>
                        state <= desel;
                        OBuf1 := (others => 'Z');
                    WHEN begin_SPwr =>
                        OBuf1 := (others => 'Z');
                        state <= begin_SPwrite;
                        MemAddr := to_nat(AddressIn);
                        startaddr := MemAddr;
                        memstart := to_nat(AddressIn(1 downto 0));
                    WHEN begin_read =>
                        MemAddr := to_nat(AddressIn);
                        startaddr := MemAddr;
                        memstart := to_nat(AddressIn(1 downto 0));
                        IF MemDataA(MemAddr) = -2 THEN
                            OBuf1(8 downto 0) := (others => 'U');
                        ELSIF MemDataA(MemAddr) = -1 THEN
                            OBuf1(8 downto 0) := (others => 'X');
                        ELSE
                            OBuf1(8 downto 0) := to_slv(MemDataA(MemAddr),9);
                        END IF;
                        IF MemDataB(MemAddr) = -2 THEN
                            OBuf1(17 downto 9) := (others => 'U');
                        ELSIF MemDataB(MemAddr) = -1 THEN
                            OBuf1(17 downto 9) := (others => 'X');
                        ELSE
                            OBuf1(17 downto 9) := to_slv(MemDataB(MemAddr),9);
                        END IF;
                        IF MemDataC(MemAddr) = -2 THEN
                            OBuf1(26 downto 18) := (others => 'U');
                        ELSIF MemDataC(MemAddr) = -1 THEN
                            OBuf1(26 downto 18) := (others => 'X');
                        ELSE
                            OBuf1(26 downto 18) := to_slv(MemDataC(MemAddr),9);
                        END IF;
                        IF MemDataD(MemAddr) = -2 THEN
                            OBuf1(35 downto 27) := (others => 'U');
                        ELSIF MemDataD(MemAddr) = -1 THEN
                            OBuf1(35 downto 27) := (others => 'X');
                        ELSE
                            OBuf1(35 downto 27) := to_slv(MemDataD(MemAddr),9);
                        END IF;
                    WHEN SCwr =>
                        state <= SCwrite;
                        MemAddr := to_nat(AddressIn);
                        OBuf1 := (others => 'Z');
                         IF GWNIn = '0' THEN
                            MemDataA(MemAddr) := -1;
                            MemDataB(MemAddr) := -1;
                            MemDataC(MemAddr) := -1;
                            MemDataD(MemAddr) := -1;
                            IF Violation /= 'X' THEN
                                MemDataA(MemAddr) := to_nat(DatAIn);
                                MemDataB(MemAddr) := to_nat(DatBIn);
                                MemDataC(MemAddr) := to_nat(DatCIn);
                                MemDataD(MemAddr) := to_nat(DatDIn);
                            END IF;
                        ELSE
                            IF (BWANIn = '0') THEN
                                MemDataA(MemAddr) := -1;
                                IF Violation /= 'X' THEN
                                    MemDataA(MemAddr) := to_nat(DatAIn);
                                END IF;
                            END IF;
                            IF (BWBNIn = '0') THEN
                                MemDataB(MemAddr) := -1;
                                IF Violation /= 'X' THEN
                                    MemDataB(MemAddr) := to_nat(DatBIn);
                                END IF;
                            END IF;
                            IF (BWCNIn = '0') THEN
                                MemDataC(MemAddr) := -1;
                                IF Violation /= 'X' THEN
                                    MemDataC(MemAddr) := to_nat(DatCIn);
                                END IF;
                            END IF;
                            IF (BWDNIn = '0') THEN
                                MemDataD(MemAddr) := -1;
                                IF Violation /= 'X' THEN
                                    MemDataD(MemAddr) := to_nat(DatDIn);
                                END IF;
                            END IF;
                        END IF;
                    WHEN SPwr_burst =>
                        OBuf1 := (others => 'Z');
                    WHEN SPwr_susp =>
                        state <= SPwrite;
                        OBuf1 := (others => 'Z');
                        IF GWNIn = '0' THEN
                            MemDataA(MemAddr) := -1;
                            MemDataB(MemAddr) := -1;
                            MemDataC(MemAddr) := -1;
                            MemDataD(MemAddr) := -1;
                            IF Violation /= 'X' THEN
                                MemDataA(MemAddr) := to_nat(DatAIn);
                                MemDataB(MemAddr) := to_nat(DatBIn);
                                MemDataC(MemAddr) := to_nat(DatCIn);
                                MemDataD(MemAddr) := to_nat(DatDIn);
                            END IF;
                        ELSE
                            IF (BWANIn = '0') THEN
                                MemDataA(MemAddr) := -1;
                                IF Violation /= 'X' THEN
                                    MemDataA(MemAddr) := to_nat(DatAIn);
                                END IF;
                            END IF;
                            IF (BWBNIn = '0') THEN
                                MemDataB(MemAddr) := -1;
                                IF Violation /= 'X' THEN
                                    MemDataB(MemAddr) := to_nat(DatBIn);
                                END IF;
                            END IF;
                            IF (BWCNIn = '0') THEN
                                MemDataC(MemAddr) := -1;
                                IF Violation /= 'X' THEN
                                    MemDataC(MemAddr) := to_nat(DatCIn);
                                END IF;
                            END IF;
                            IF (BWDNIn = '0') THEN
                                MemDataD(MemAddr) := -1;
                                IF Violation /= 'X' THEN
                                    MemDataD(MemAddr) := to_nat(DatDIn);
                                END IF;
                            END IF;
                        END IF;
                    WHEN read_burst =>
                        state <= read;
                        Burst_Cnt := Burst_Cnt + 1;
                        IF (Burst_Cnt = 4) THEN
                            Burst_Cnt := 0;
                        END IF;
                        offset := Burst_Seq(memstart)(Burst_Cnt);
                        MemAddr := startaddr + offset;
                        IF MemDataA(MemAddr) = -2 THEN
                            OBuf1(8 downto 0) := (others => 'U');
                        ELSIF MemDataA(MemAddr) = -1 THEN
                            OBuf1(8 downto 0) := (others => 'X');
                        ELSE
                            OBuf1(8 downto 0) := to_slv(MemDataA(MemAddr),9);
                        END IF;
                        IF MemDataB(MemAddr) = -2 THEN
                            OBuf1(17 downto 9) := (others => 'U');
                        ELSIF MemDataB(MemAddr) = -1 THEN
                            OBuf1(17 downto 9) := (others => 'X');
                        ELSE
                            OBuf1(17 downto 9) := to_slv(MemDataB(MemAddr),9);
                        END IF;
                        IF MemDataC(MemAddr) = -2 THEN
                            OBuf1(26 downto 18) := (others => 'U');
                        ELSIF MemDataC(MemAddr) = -1 THEN
                            OBuf1(26 downto 18) := (others => 'X');
                        ELSE
                            OBuf1(26 downto 18) := to_slv(MemDataC(MemAddr),9);
                        END IF;
                        IF MemDataD(MemAddr) = -2 THEN
                            OBuf1(35 downto 27) := (others => 'U');
                        ELSIF MemDataD(MemAddr) = -1 THEN
                            OBuf1(35 downto 27) := (others => 'X');
                        ELSE
                            OBuf1(35 downto 27) := to_slv(MemDataD(MemAddr),9);
                        END IF;
                    WHEN read_susp =>
                        OBuf1 := (others => 'Z');
                END CASE;

            WHEN begin_SPwrite =>
                Burst_Cnt := 0;
                CASE command IS
                    WHEN ds =>
                        state <= desel;
                        OBuf1 := (others => 'Z');
                    WHEN begin_SPwr =>
                        state <= begin_SPwrite;
                        MemAddr := to_nat(AddressIn);
                        startaddr := MemAddr;
                        memstart := to_nat(AddressIn(1 downto 0));
                    WHEN begin_read =>
                        state <= begin_rd;
                        MemAddr := to_nat(AddressIn);
                        startaddr := MemAddr;
                        memstart := to_nat(AddressIn(1 downto 0));
                        IF MemDataA(MemAddr) = -2 THEN
                            OBuf1(8 downto 0) := (others => 'U');
                        ELSIF MemDataA(MemAddr) = -1 THEN
                            OBuf1(8 downto 0) := (others => 'X');
                        ELSE
                            OBuf1(8 downto 0) := to_slv(MemDataA(MemAddr),9);
                        END IF;
                        IF MemDataB(MemAddr) = -2 THEN
                            OBuf1(17 downto 9) := (others => 'U');
                        ELSIF MemDataB(MemAddr) = -1 THEN
                            OBuf1(17 downto 9) := (others => 'X');
                        ELSE
                            OBuf1(17 downto 9) := to_slv(MemDataB(MemAddr),9);
                        END IF;
                        IF MemDataC(MemAddr) = -2 THEN
                            OBuf1(26 downto 18) := (others => 'U');
                        ELSIF MemDataC(MemAddr) = -1 THEN
                            OBuf1(26 downto 18) := (others => 'X');
                        ELSE
                            OBuf1(26 downto 18) := to_slv(MemDataC(MemAddr),9);
                        END IF;
                        IF MemDataD(MemAddr) = -2 THEN
                            OBuf1(35 downto 27) := (others => 'U');
                        ELSIF MemDataD(MemAddr) = -1 THEN
                            OBuf1(35 downto 27) := (others => 'X');
                        ELSE
                            OBuf1(35 downto 27) := to_slv(MemDataD(MemAddr),9);
                        END IF;
                    WHEN SCwr =>
                        state <= SCwrite;
                        MemAddr := to_nat(AddressIn);
                        OBuf1 := (others => 'Z');
                        IF GWNIn = '0' THEN
                            MemDataA(MemAddr) := -1;
                            MemDataB(MemAddr) := -1;
                            MemDataC(MemAddr) := -1;
                            MemDataD(MemAddr) := -1;
                            IF Violation /= 'X' THEN
                                MemDataA(MemAddr) := to_nat(DatAIn);
                                MemDataB(MemAddr) := to_nat(DatBIn);
                                MemDataC(MemAddr) := to_nat(DatCIn);
                                MemDataD(MemAddr) := to_nat(DatDIn);
                            END IF;
                        ELSE
                            IF (BWANIn = '0') THEN
                                MemDataA(MemAddr) := -1;
                                IF Violation /= 'X' THEN
                                    MemDataA(MemAddr) := to_nat(DatAIn);
                                END IF;
                            END IF;
                            IF (BWBNIn = '0') THEN
                                MemDataB(MemAddr) := -1;
                                IF Violation /= 'X' THEN
                                    MemDataB(MemAddr) := to_nat(DatBIn);
                                END IF;
                            END IF;
                            IF (BWCNIn = '0') THEN
                                MemDataC(MemAddr) := -1;
                                IF Violation /= 'X' THEN
                                    MemDataC(MemAddr) := to_nat(DatCIn);
                                END IF;
                            END IF;
                            IF (BWDNIn = '0') THEN
                                MemDataD(MemAddr) := -1;
                                IF Violation /= 'X' THEN
                                    MemDataD(MemAddr) := to_nat(DatDIn);
                                END IF;
                            END IF;
                        END IF;
                    WHEN SPwr_burst =>
                        OBuf1 := (others => 'Z');
                    WHEN SPwr_susp =>
                        state <= SPwrite;
                        OBuf1 := (others => 'Z');
                        IF GWNIn = '0' THEN
                            MemDataA(MemAddr) := -1;
                            MemDataB(MemAddr) := -1;
                            MemDataC(MemAddr) := -1;
                            MemDataD(MemAddr) := -1;
                            IF Violation /= 'X' THEN
                                MemDataA(MemAddr) := to_nat(DatAIn);
                                MemDataB(MemAddr) := to_nat(DatBIn);
                                MemDataC(MemAddr) := to_nat(DatCIn);
                                MemDataD(MemAddr) := to_nat(DatDIn);
                            END IF;
                        ELSE
                            IF (BWANIn = '0') THEN
                                MemDataA(MemAddr) := -1;
                                IF Violation /= 'X' THEN
                                    MemDataA(MemAddr) := to_nat(DatAIn);
                                END IF;
                            END IF;
                            IF (BWBNIn = '0') THEN
                                MemDataB(MemAddr) := -1;
                                IF Violation /= 'X' THEN
                                    MemDataB(MemAddr) := to_nat(DatBIn);
                                END IF;
                            END IF;
                            IF (BWCNIn = '0') THEN
                                MemDataC(MemAddr) := -1;
                                IF Violation /= 'X' THEN
                                    MemDataC(MemAddr) := to_nat(DatCIn);
                                END IF;
                            END IF;
                            IF (BWDNIn = '0') THEN
                                MemDataD(MemAddr) := -1;
                                IF Violation /= 'X' THEN
                                    MemDataD(MemAddr) := to_nat(DatDIn);
                                END IF;
                            END IF;
                        END IF;
                    WHEN read_burst =>
                        OBuf1 := (others => 'Z');
                    WHEN read_susp =>
                        OBuf1 := (others => 'Z');
                END CASE;

            WHEN SPwrite =>
                CASE command IS
                    WHEN ds =>
                        state <= desel;
                        OBuf1 := (others => 'Z');
                    WHEN begin_SPwr =>
                        state <= begin_SPwrite;
                        MemAddr := to_nat(AddressIn);
                        startaddr := MemAddr;
                        memstart := to_nat(AddressIn(1 downto 0));
                    WHEN begin_read =>
                        state <= begin_rd;
                        MemAddr := to_nat(AddressIn);
                        startaddr := MemAddr;
                        memstart := to_nat(AddressIn(1 downto 0));
                        IF MemDataA(MemAddr) = -2 THEN
                            OBuf1(8 downto 0) := (others => 'U');
                        ELSIF MemDataA(MemAddr) = -1 THEN
                            OBuf1(8 downto 0) := (others => 'X');
                        ELSE
                            OBuf1(8 downto 0) := to_slv(MemDataA(MemAddr),9);
                        END IF;
                        IF MemDataB(MemAddr) = -2 THEN
                            OBuf1(17 downto 9) := (others => 'U');
                        ELSIF MemDataB(MemAddr) = -1 THEN
                            OBuf1(17 downto 9) := (others => 'X');
                        ELSE
                            OBuf1(17 downto 9) := to_slv(MemDataB(MemAddr),9);
                        END IF;
                        IF MemDataC(MemAddr) = -2 THEN
                            OBuf1(26 downto 18) := (others => 'U');
                        ELSIF MemDataC(MemAddr) = -1 THEN
                            OBuf1(26 downto 18) := (others => 'X');
                        ELSE
                            OBuf1(26 downto 18) := to_slv(MemDataC(MemAddr),9);
                        END IF;
                        IF MemDataD(MemAddr) = -2 THEN
                            OBuf1(35 downto 27) := (others => 'U');
                        ELSIF MemDataD(MemAddr) = -1 THEN
                            OBuf1(35 downto 27) := (others => 'X');
                        ELSE
                            OBuf1(35 downto 27) := to_slv(MemDataD(MemAddr),9);
                        END IF;
                    WHEN SCwr =>
                        state <= SCwrite;
                        MemAddr := to_nat(AddressIn);
                        OBuf1 := (others => 'Z');
                        IF GWNIn = '0' THEN
                            MemDataA(MemAddr) := -1;
                            MemDataB(MemAddr) := -1;
                            MemDataC(MemAddr) := -1;
                            MemDataD(MemAddr) := -1;
                            IF Violation /= 'X' THEN
                                MemDataA(MemAddr) := to_nat(DatAIn);
                                MemDataB(MemAddr) := to_nat(DatBIn);
                                MemDataC(MemAddr) := to_nat(DatCIn);
                                MemDataD(MemAddr) := to_nat(DatDIn);
                            END IF;
                        ELSE
                            IF (BWANIn = '0') THEN
                                MemDataA(MemAddr) := -1;
                                IF Violation /= 'X' THEN
                                    MemDataA(MemAddr) := to_nat(DatAIn);
                                END IF;
                            END IF;
                            IF (BWBNIn = '0') THEN
                                MemDataB(MemAddr) := -1;
                                IF Violation /= 'X' THEN
                                    MemDataB(MemAddr) := to_nat(DatBIn);
                                END IF;
                            END IF;
                            IF (BWCNIn = '0') THEN
                                MemDataC(MemAddr) := -1;
                                IF Violation /= 'X' THEN
                                    MemDataC(MemAddr) := to_nat(DatCIn);
                                END IF;
                            END IF;
                            IF (BWDNIn = '0') THEN
                                MemDataD(MemAddr) := -1;
                                IF Violation /= 'X' THEN
                                    MemDataD(MemAddr) := to_nat(DatDIn);
                                END IF;
                            END IF;
                        END IF;
                    WHEN SPwr_burst =>
                        state <= SPwrite;
                        Burst_Cnt := Burst_Cnt + 1;
                        IF (Burst_Cnt = 4) THEN
                            Burst_Cnt := 0;
                        END IF;
                        offset := Burst_Seq(memstart)(Burst_Cnt);
                        MemAddr := startaddr + offset;
                        OBuf1 := (others => 'Z');
                        IF GWNIn = '0' THEN
                            MemDataA(MemAddr) := -1;
                            MemDataB(MemAddr) := -1;
                            MemDataC(MemAddr) := -1;
                            MemDataD(MemAddr) := -1;
                            IF Violation /= 'X' THEN
                                MemDataA(MemAddr) := to_nat(DatAIn);
                                MemDataB(MemAddr) := to_nat(DatBIn);
                                MemDataC(MemAddr) := to_nat(DatCIn);
                                MemDataD(MemAddr) := to_nat(DatDIn);
                            END IF;
                        ELSE
                            IF (BWANIn = '0') THEN
                                MemDataA(MemAddr) := -1;
                                IF Violation /= 'X' THEN
                                    MemDataA(MemAddr) := to_nat(DatAIn);
                                END IF;
                            END IF;
                            IF (BWBNIn = '0') THEN
                                MemDataB(MemAddr) := -1;
                                IF Violation /= 'X' THEN
                                    MemDataB(MemAddr) := to_nat(DatBIn);
                                END IF;
                            END IF;
                            IF (BWCNIn = '0') THEN
                                MemDataC(MemAddr) := -1;
                                IF Violation /= 'X' THEN
                                    MemDataC(MemAddr) := to_nat(DatCIn);
                                END IF;
                            END IF;
                            IF (BWDNIn = '0') THEN
                                MemDataD(MemAddr) := -1;
                                IF Violation /= 'X' THEN
                                    MemDataD(MemAddr) := to_nat(DatDIn);
                                END IF;
                            END IF;
                        END IF;
                    WHEN SPwr_susp =>
                        state <= SPwrite;
                        OBuf1 := (others => 'Z');
                    WHEN read_burst =>
                        OBuf1 := (others => 'Z');
                    WHEN read_susp =>
                        OBuf1 := (others => 'Z');
                END CASE;

            WHEN SCwrite =>
                Burst_Cnt := 0;
                CASE command IS
                    WHEN ds =>
                        state <= desel;
                        OBuf1 := (others => 'Z');
                    WHEN begin_SPwr =>
                        state <= begin_SPwrite;
                        MemAddr := to_nat(AddressIn);
                        startaddr := MemAddr;
                        memstart := to_nat(AddressIn(1 downto 0));
                    WHEN begin_read =>
                        state <= begin_rd;
                        MemAddr := to_nat(AddressIn);
                        startaddr := MemAddr;
                        memstart := to_nat(AddressIn(1 downto 0));
                        IF MemDataA(MemAddr) = -2 THEN
                            OBuf1(8 downto 0) := (others => 'U');
                        ELSIF MemDataA(MemAddr) = -1 THEN
                            OBuf1(8 downto 0) := (others => 'X');
                        ELSE
                            OBuf1(8 downto 0) := to_slv(MemDataA(MemAddr),9);
                        END IF;
                        IF MemDataB(MemAddr) = -2 THEN
                            OBuf1(17 downto 9) := (others => 'U');
                        ELSIF MemDataB(MemAddr) = -1 THEN
                            OBuf1(17 downto 9) := (others => 'X');
                        ELSE
                            OBuf1(17 downto 9) := to_slv(MemDataB(MemAddr),9);
                        END IF;
                        IF MemDataC(MemAddr) = -2 THEN
                            OBuf1(26 downto 18) := (others => 'U');
                        ELSIF MemDataC(MemAddr) = -1 THEN
                            OBuf1(26 downto 18) := (others => 'X');
                        ELSE
                            OBuf1(26 downto 18) := to_slv(MemDataC(MemAddr),9);
                        END IF;
                        IF MemDataD(MemAddr) = -2 THEN
                            OBuf1(35 downto 27) := (others => 'U');
                        ELSIF MemDataD(MemAddr) = -1 THEN
                            OBuf1(35 downto 27) := (others => 'X');
                        ELSE
                            OBuf1(35 downto 27) := to_slv(MemDataD(MemAddr),9);
                        END IF;
                    WHEN SCwr =>
                        state <= SCwrite;
                        MemAddr := to_nat(AddressIn);
                        OBuf1 := (others => 'Z');
                        IF GWNIn = '0' THEN
                            MemDataA(MemAddr) := -1;
                            MemDataB(MemAddr) := -1;
                            MemDataC(MemAddr) := -1;
                            MemDataD(MemAddr) := -1;
                            IF Violation /= 'X' THEN
                                MemDataA(MemAddr) := to_nat(DatAIn);
                                MemDataB(MemAddr) := to_nat(DatBIn);
                                MemDataC(MemAddr) := to_nat(DatCIn);
                                MemDataD(MemAddr) := to_nat(DatDIn);
                            END IF;
                        ELSE
                            IF (BWANIn = '0') THEN
                                MemDataA(MemAddr) := -1;
                                IF Violation /= 'X' THEN
                                    MemDataA(MemAddr) := to_nat(DatAIn);
                                END IF;
                            END IF;
                            IF (BWBNIn = '0') THEN
                                MemDataB(MemAddr) := -1;
                                IF Violation /= 'X' THEN
                                    MemDataB(MemAddr) := to_nat(DatBIn);
                                END IF;
                            END IF;
                            IF (BWCNIn = '0') THEN
                                MemDataC(MemAddr) := -1;
                                IF Violation /= 'X' THEN
                                    MemDataC(MemAddr) := to_nat(DatCIn);
                                END IF;
                            END IF;
                            IF (BWDNIn = '0') THEN
                                MemDataD(MemAddr) := -1;
                                IF Violation /= 'X' THEN
                                    MemDataD(MemAddr) := to_nat(DatDIn);
                                END IF;
                            END IF;
                        END IF;
                    WHEN SPwr_burst =>
                        OBuf1 := (others => 'Z');
                    WHEN SPwr_susp =>
                        OBuf1 := (others => 'Z');
                    WHEN read_burst =>
                        OBuf1 := (others => 'Z');
                    WHEN read_susp =>
                        OBuf1 := (others => 'Z');
                END CASE;

            WHEN read =>
                CASE command IS
                    WHEN ds =>
                        state <= desel;
                        OBuf1 := (others => 'Z');
                    WHEN begin_SPwr =>
                        OBuf1 := (others => 'Z');
                        state <= begin_SPwrite;
                        MemAddr := to_nat(AddressIn);
                        startaddr := MemAddr;
                        memstart := to_nat(AddressIn(1 downto 0));
                    WHEN begin_read =>
                        state <= begin_rd;
                        MemAddr := to_nat(AddressIn);
                        startaddr := MemAddr;
                        memstart := to_nat(AddressIn(1 downto 0));
                        IF MemDataA(MemAddr) = -2 THEN
                            OBuf1(8 downto 0) := (others => 'U');
                        ELSIF MemDataA(MemAddr) = -1 THEN
                            OBuf1(8 downto 0) := (others => 'X');
                        ELSE
                            OBuf1(8 downto 0) := to_slv(MemDataA(MemAddr),9);
                        END IF;
                        IF MemDataB(MemAddr) = -2 THEN
                            OBuf1(17 downto 9) := (others => 'U');
                        ELSIF MemDataB(MemAddr) = -1 THEN
                            OBuf1(17 downto 9) := (others => 'X');
                        ELSE
                            OBuf1(17 downto 9) := to_slv(MemDataB(MemAddr),9);
                        END IF;
                        IF MemDataC(MemAddr) = -2 THEN
                            OBuf1(26 downto 18) := (others => 'U');
                        ELSIF MemDataC(MemAddr) = -1 THEN
                            OBuf1(26 downto 18) := (others => 'X');
                        ELSE
                            OBuf1(26 downto 18) := to_slv(MemDataC(MemAddr),9);
                        END IF;
                        IF MemDataD(MemAddr) = -2 THEN
                            OBuf1(35 downto 27) := (others => 'U');
                        ELSIF MemDataD(MemAddr) = -1 THEN
                            OBuf1(35 downto 27) := (others => 'X');
                        ELSE
                            OBuf1(35 downto 27) := to_slv(MemDataD(MemAddr),9);
                        END IF;
                    WHEN SCwr =>
                        state <= SCwrite;
                        MemAddr := to_nat(AddressIn);
                        OBuf1 := (others => 'Z');
                        IF GWNIn = '0' THEN
                            MemDataA(MemAddr) := -1;
                            MemDataB(MemAddr) := -1;
                            MemDataC(MemAddr) := -1;
                            MemDataD(MemAddr) := -1;
                            IF Violation /= 'X' THEN
                                MemDataA(MemAddr) := to_nat(DatAIn);
                                MemDataB(MemAddr) := to_nat(DatBIn);
                                MemDataC(MemAddr) := to_nat(DatCIn);
                                MemDataD(MemAddr) := to_nat(DatDIn);
                            END IF;
                        ELSE
                            IF (BWANIn = '0') THEN
                                MemDataA(MemAddr) := -1;
                                IF Violation /= 'X' THEN
                                    MemDataA(MemAddr) := to_nat(DatAIn);
                                END IF;
                            END IF;
                            IF (BWBNIn = '0') THEN
                                MemDataB(MemAddr) := -1;
                                IF Violation /= 'X' THEN
                                    MemDataB(MemAddr) := to_nat(DatBIn);
                                END IF;
                            END IF;
                            IF (BWCNIn = '0') THEN
                                MemDataC(MemAddr) := -1;
                                IF Violation /= 'X' THEN
                                    MemDataC(MemAddr) := to_nat(DatCIn);
                                END IF;
                            END IF;
                            IF (BWDNIn = '0') THEN
                                MemDataD(MemAddr) := -1;
                                IF Violation /= 'X' THEN
                                    MemDataD(MemAddr) := to_nat(DatDIn);
                                END IF;
                            END IF;
                        END IF;
                    WHEN SPwr_burst =>
                        OBuf1 := (others => 'Z');
                    WHEN SPwr_susp =>
                        OBuf1 := (others => 'Z');
                    WHEN read_burst =>
                        state <= read;
                        Burst_Cnt := Burst_Cnt + 1;
                        IF (Burst_Cnt = 4) THEN
                            Burst_Cnt := 0;
                        END IF;
                        offset := Burst_Seq(memstart)(Burst_Cnt);
                        MemAddr := startaddr + offset;
                        IF MemDataA(MemAddr) = -2 THEN
                            OBuf1(8 downto 0) := (others => 'U');
                        ELSIF MemDataA(MemAddr) = -1 THEN
                            OBuf1(8 downto 0) := (others => 'X');
                        ELSE
                            OBuf1(8 downto 0) := to_slv(MemDataA(MemAddr),9);
                        END IF;
                        IF MemDataB(MemAddr) = -2 THEN
                            OBuf1(17 downto 9) := (others => 'U');
                        ELSIF MemDataB(MemAddr) = -1 THEN
                            OBuf1(17 downto 9) := (others => 'X');
                        ELSE
                            OBuf1(17 downto 9) := to_slv(MemDataB(MemAddr),9);
                        END IF;
                        IF MemDataC(MemAddr) = -2 THEN
                            OBuf1(26 downto 18) := (others => 'U');
                        ELSIF MemDataC(MemAddr) = -1 THEN
                            OBuf1(26 downto 18) := (others => 'X');
                        ELSE
                            OBuf1(26 downto 18) := to_slv(MemDataC(MemAddr),9);
                        END IF;
                        IF MemDataD(MemAddr) = -2 THEN
                            OBuf1(35 downto 27) := (others => 'U');
                        ELSIF MemDataD(MemAddr) = -1 THEN
                            OBuf1(35 downto 27) := (others => 'X');
                        ELSE
                            OBuf1(35 downto 27) := to_slv(MemDataD(MemAddr),9);
                        END IF;
                    WHEN read_susp =>
                        state <= read;
                END CASE;
            END CASE;

         IF (OENegIn = '0') THEN
            D_zd <= (others => 'Z'), OBuf2 AFTER 1 ns;
        END IF;

    END IF;

    IF (OENegIn = '1') THEN
        D_zd <= (others => 'Z');
    ELSE
        D_zd <= OBuf2;
    END IF;

    IF ZZIn = '1' THEN
        IF cycle2 = true THEN
            D_zd <= (others => 'Z');
            cycle1 := false;
        ELSE
            ASSERT false
                REPORT InstancePath & partID & ": 2tCYC are "
                       & "required to enter into sleep mode."
                SEVERITY SeverityMode;
        END IF;
    END IF;

    --------------------------------------------------------------------
    -- File Read Section
    --------------------------------------------------------------------
    IF (mem_file_name /= "none") AND (NOW < 1 ns) THEN
        ind := 0;
        WHILE (not ENDFILE (mem_file)) LOOP
            READLINE (mem_file, buf);
			READ (buf, val);
			MemDataD(ind) := to_integer(unsigned(to_signed(val,32)(7 downto 0)));
			MemDataC(ind) := to_integer(unsigned(to_signed(val,32)(15 downto 8)));
			MemDataB(ind) := to_integer(unsigned(to_signed(val,32)(23 downto 16)));
			MemDataA(ind) := to_integer(unsigned(to_signed(val,32)(31 downto 24)));
			ind := ind + 1;
        END LOOP;
    END IF;

    END PROCESS;

        ------------------------------------------------------------------------
        -- Path Delay Process
        ------------------------------------------------------------------------
        DataOutPath : FOR I IN 35 DOWNTO 0 GENERATE
            DataOut_Delay : PROCESS (D_zd(i))
                VARIABLE D_GlitchData:VitalGlitchDataArrayType(35 Downto 0);
            BEGIN
                VitalPathDelay01Z (
                    OutSignal       => DataOut(i),
                    OutSignalName   => "Data",
                    OutTemp         => D_zd(i),
                    Mode            => VitalTransport,
                    GlitchData      => D_GlitchData(i),
                    Paths           => (
                        1 => (InputChangeTime => CLKIn'LAST_EVENT,
                              PathDelay => tpd_CLK_DQA0,
                              PathCondition   => OENegIn = '0'),
                        2 => (InputChangeTime => OENegIn'LAST_EVENT,
                              PathDelay => tpd_OENeg_DQA0,
                              PathCondition   => true)
                   )
                );

            END PROCESS;
        END GENERATE;

    END BLOCK;

END vhdl_behavioral;
