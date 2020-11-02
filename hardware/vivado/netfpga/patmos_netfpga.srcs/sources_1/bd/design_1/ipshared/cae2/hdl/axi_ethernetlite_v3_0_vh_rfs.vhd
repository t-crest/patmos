-- Loadable arithmetic register.
-------------------------------------------------------------------------------
--
-- *************************************************************************
-- **                                                                     **
-- ** DISCLAIMER OF LIABILITY                                             **
-- **                                                                     **
-- ** This text/file contains proprietary, confidential                   **
-- ** information of Xilinx, Inc., is distributed under                   **
-- ** license from Xilinx, Inc., and may be used, copied                  **
-- ** and/or disclosed only pursuant to the terms of a valid              **
-- ** license agreement with Xilinx, Inc. Xilinx hereby                   **
-- ** grants you a license to use this text/file solely for               **
-- ** design, simulation, implementation and creation of                  **
-- ** design files limited to Xilinx devices or technologies.             **
-- ** Use with non-Xilinx devices or technologies is expressly            **
-- ** prohibited and immediately terminates your license unless           **
-- ** covered by a separate agreement.                                    **
-- **                                                                     **
-- ** Xilinx is providing this design, code, or information               **
-- ** "as-is" solely for use in developing programs and                   **
-- ** solutions for Xilinx devices, with no obligation on the             **
-- ** part of Xilinx to provide support. By providing this design,        **
-- ** code, or information as one possible implementation of              **
-- ** this feature, application or standard, Xilinx is making no          **
-- ** representation that this implementation is free from any            **
-- ** claims of infringement. You are responsible for obtaining           **
-- ** any rights you may require for your implementation.                 **
-- ** Xilinx expressly disclaims any warranty whatsoever with             **
-- ** respect to the adequacy of the implementation, including            **
-- ** but not limited to any warranties or representations that this      **
-- ** implementation is free from claims of infringement, implied         **
-- ** warranties of merchantability or fitness for a particular           **
-- ** purpose.                                                            **
-- **                                                                     **
-- ** Xilinx products are not intended for use in life support            **
-- ** appliances, devices, or systems. Use in such applications is        **
-- ** expressly prohibited.                                               **
-- **                                                                     **
-- ** Any modifications that are made to the Source Code are              **
-- ** done at the user’s sole risk and will be unsupported.               **
-- ** The Xilinx Support Hotline does not have access to source           **
-- ** code and therefore cannot answer specific questions related         **
-- ** to source HDL. The Xilinx Hotline support of original source        **
-- ** code IP shall only address issues and questions related             **
-- ** to the standard Netlist version of the core (and thus               **
-- ** indirectly, the original core source).                              **
-- **                                                                     **
-- ** Copyright (c) 2001-2010 Xilinx, Inc. All rights reserved.           **
-- **                                                                     **
-- ** This copyright and support notice must be retained as part          **
-- ** of this text at all times.                                          **
-- **                                                                     **
-- *************************************************************************
--
-------------------------------------------------------------------------------
-- Filename:        ld_arith_reg.vhd
-- Version:         
--------------------------------------------------------------------------------
-- Description:   A register that can be loaded and added to or subtracted from
--                (but not both). The width of the register is specified
--                with a generic. The load value and the arith
--                value, i.e. the value to be added (subtracted), may be of
--                lesser width than the register and may be
--                offset from the LSB position. (Uncovered positions
--                load or add (subtract) zero.) The register can be
--                reset, via the RST signal, to a freely selectable value.
--                The register is defined in terms of big-endian bit ordering.
--
-------------------------------------------------------------------------------
-- Structure: 
--
--              ld_arith_reg.vhd
-------------------------------------------------------------------------------
-- Author:      FO
--
-- History:
--
--      FO      08/01        -- First version
--
--      FO      11/14/01     -- Cosmetic improvements
--
--      FO      02/22/02     -- Switched from MUXCY_L primitive to MUXCY.
--
--     DET     1/17/2008     v4_0
-- ~~~~~~
--     - Incorporated new disclaimer header
-- ^^^^^^
--
-------------------------------------------------------------------------------
-- Naming Conventions:
--      active low signals:                     "*_n"
--      clock signals:                          "clk", "clk_div#", "clk_#x" 
--      reset signals:                          "rst", "rst_n" 
--      generics:                               "C_*" 
--      user defined types:                     "*_TYPE" 
--      state machine next state:               "*_ns" 
--      state machine current state:            "*_cs" 
--      combinatorial signals:                  "*_com" 
--      pipelined or register delay signals:    "*_d#" 
--      counter signals:                        "*cnt*"
--      clock enable signals:                   "*_ce" 
--      internal version of output port         "*_i"
--      device pins:                            "*_pin" 
--      ports:                                  - Names begin with Uppercase 
--      processes:                              "*_PROCESS" 
--      component instantiations:               "<ENTITY_>I_<#|FUNC>
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;

entity ld_arith_reg is
    generic (
        ------------------------------------------------------------------------
        -- True if the arithmetic operation is add, false if subtract.
        C_ADD_SUB_NOT : boolean := false;
        ------------------------------------------------------------------------
        -- Width of the register.
        C_REG_WIDTH   : natural := 8;
        ------------------------------------------------------------------------
        -- Reset value. (No default, must be specified in the instantiation.)
        C_RESET_VALUE : std_logic_vector;
        ------------------------------------------------------------------------
        -- Width of the load data.
        C_LD_WIDTH    : natural :=  8;
        ------------------------------------------------------------------------
        -- Offset from the LSB (toward more significant) of the load data.
        C_LD_OFFSET   : natural :=  0;
        ------------------------------------------------------------------------
        -- Width of the arithmetic data.
        C_AD_WIDTH    : natural :=  8;
        ------------------------------------------------------------------------
        -- Offset from the LSB of the arithmetic data.
        C_AD_OFFSET   : natural :=  0
        ------------------------------------------------------------------------
        -- Dependencies: (1) C_LD_WIDTH + C_LD_OFFSET <= C_REG_WIDTH
        --               (2) C_AD_WIDTH + C_AD_OFFSET <= C_REG_WIDTH
        ------------------------------------------------------------------------
    );
    port (
        CK       : in  std_logic;
        RST      : in  std_logic; -- Reset to C_RESET_VALUE. (Overrides OP,LOAD)
        Q        : out std_logic_vector(0 to C_REG_WIDTH-1);
        LD       : in  std_logic_vector(0 to C_LD_WIDTH-1); -- Load data.
        AD       : in  std_logic_vector(0 to C_AD_WIDTH-1); -- Arith data.
        LOAD     : in  std_logic;  -- Enable for the load op, Q <= LD.
        OP       : in  std_logic   -- Enable for the arith op, Q <= Q + AD.
                                   -- (Q <= Q - AD if C_ADD_SUB_NOT = false.)
                                   -- (Overrrides LOAD.)
    );
end ld_arith_reg;
  

library unisim;
use unisim.all;

library ieee;
use ieee.numeric_std.all;

architecture imp of ld_arith_reg is

    component MULT_AND
       port(
          LO :  out   std_ulogic;
          I1 :  in    std_ulogic;
          I0 :  in    std_ulogic);
    end component;

    component MUXCY is
      port (
        DI : in  std_logic;
        CI : in  std_logic;
        S  : in  std_logic;
        O  : out std_logic);
    end component MUXCY;

    component XORCY is
      port (
        LI : in  std_logic;
        CI : in  std_logic;
        O  : out std_logic);
    end component XORCY;

    component FDRE is
      port (
        Q  : out std_logic;
        C  : in  std_logic;
        CE : in  std_logic;
        D  : in  std_logic;
        R  : in  std_logic
      );
    end component FDRE;
  
    component FDSE is
      port (
        Q  : out std_logic;
        C  : in  std_logic;
        CE : in  std_logic;
        D  : in  std_logic;
        S  : in  std_logic
      );
    end component FDSE;
  
    signal q_i,
           q_i_ns,
           xorcy_out,
           gen_cry_kill_n : std_logic_vector(0 to C_REG_WIDTH-1);
    signal cry : std_logic_vector(0 to C_REG_WIDTH);

begin

    -- synthesis translate_off

    assert C_LD_WIDTH + C_LD_OFFSET <= C_REG_WIDTH 
        report "ld_arith_reg, constraint does not hold: " &
               "C_LD_WIDTH + C_LD_OFFSET <= C_REG_WIDTH"
        severity error;
    assert C_AD_WIDTH + C_AD_OFFSET <= C_REG_WIDTH 
        report "ld_arith_reg, constraint does not hold: " &
               "C_AD_WIDTH + C_AD_OFFSET <= C_REG_WIDTH"
        severity error;

    -- synthesis translate_on

    Q <= q_i;

    cry(C_REG_WIDTH) <= '0' when C_ADD_SUB_NOT else OP;

    PERBIT_GEN: for j in C_REG_WIDTH-1 downto 0 generate
        signal load_bit, arith_bit, CE : std_logic;
    begin

        ------------------------------------------------------------------------
        -- Assign to load_bit either zero or the bit from input port LD.
        ------------------------------------------------------------------------
        D_ZERO_GEN: if    j > C_REG_WIDTH - 1 - C_LD_OFFSET 
                       or j < C_REG_WIDTH - C_LD_WIDTH - C_LD_OFFSET generate
            load_bit <= '0';
        end generate;
        D_NON_ZERO_GEN: if    j <= C_REG_WIDTH - 1 - C_LD_OFFSET 
                          and j >= C_REG_WIDTH - C_LD_OFFSET - C_LD_WIDTH
        generate
            load_bit <= LD(j - (C_REG_WIDTH - C_LD_WIDTH - C_LD_OFFSET));
        end generate;

        ------------------------------------------------------------------------
        -- Assign to arith_bit either zero or the bit from input port AD.
        ------------------------------------------------------------------------
        AD_ZERO_GEN: if    j > C_REG_WIDTH - 1 - C_AD_OFFSET 
                        or j < C_REG_WIDTH - C_AD_WIDTH - C_AD_OFFSET
        generate
            arith_bit <= '0';
        end generate;
        AD_NON_ZERO_GEN: if    j <= C_REG_WIDTH - 1 - C_AD_OFFSET 
                           and j >= C_REG_WIDTH - C_AD_OFFSET - C_AD_WIDTH
        generate
            arith_bit <= AD(j - (C_REG_WIDTH - C_AD_WIDTH - C_AD_OFFSET));
        end generate;


        ------------------------------------------------------------------------
        -- LUT output generation.
        -- Adder case
        ------------------------------------------------------------------------
        Q_I_GEN_ADD: if C_ADD_SUB_NOT generate
            q_i_ns(j) <= q_i(j) xor  arith_bit when  OP = '1' else load_bit;
        end generate;
        ------------------------------------------------------------------------
        -- Subtractor case
        ------------------------------------------------------------------------
        Q_I_GEN_SUB: if not C_ADD_SUB_NOT generate
            q_i_ns(j) <= q_i(j) xnor arith_bit when  OP = '1' else load_bit;
        end generate;


        ------------------------------------------------------------------------
        -- Kill carries (borrows) for loads but
        -- generate or kill carries (borrows) for add (sub).
        ------------------------------------------------------------------------
        MULT_AND_i1: MULT_AND
           port map (
              LO => gen_cry_kill_n(j),
              I1 => OP,
              I0 => Q_i(j)
           );

        ------------------------------------------------------------------------
        -- Propagate the carry (borrow) out.
        ------------------------------------------------------------------------
        MUXCY_i1: MUXCY
          port map (
            DI => gen_cry_kill_n(j),
            CI => cry(j+1),
            S  => q_i_ns(j),
            O  => cry(j)
          );

        ------------------------------------------------------------------------
        -- Apply the effect of carry (borrow) in.
        ------------------------------------------------------------------------
        XORCY_i1: XORCY
          port map (
            LI => q_i_ns(j),
            CI => cry(j+1),
            O  =>  xorcy_out(j)
          );


        CE <= LOAD or OP;


        ------------------------------------------------------------------------
        -- Generate either a resettable or setable FF for bit j, depending
        -- on C_RESET_VALUE at bit j.
        ------------------------------------------------------------------------
        FF_RST0_GEN: if C_RESET_VALUE(j) = '0' generate
            FDRE_i1: FDRE
              port map (
                Q  => q_i(j),
                C  => CK,
                CE => CE,
                D  => xorcy_out(j),
                R  => RST
              );
        end generate;
        FF_RST1_GEN: if C_RESET_VALUE(j) = '1' generate
            FDSE_i1: FDSE
              port map (
                Q  => q_i(j),
                C  => CK,
                CE => CE,
                D  => xorcy_out(j),
                S  => RST
              );
        end generate;

      end generate;

end imp;


-- mux_onehot_f - arch and entity
-------------------------------------------------------------------------------
--
-- *************************************************************************
-- **                                                                     **
-- ** DISCLAIMER OF LIABILITY                                             **
-- **                                                                     **
-- ** This text/file contains proprietary, confidential                   **
-- ** information of Xilinx, Inc., is distributed under                   **
-- ** license from Xilinx, Inc., and may be used, copied                  **
-- ** and/or disclosed only pursuant to the terms of a valid              **
-- ** license agreement with Xilinx, Inc. Xilinx hereby                   **
-- ** grants you a license to use this text/file solely for               **
-- ** design, simulation, implementation and creation of                  **
-- ** design files limited to Xilinx devices or technologies.             **
-- ** Use with non-Xilinx devices or technologies is expressly            **
-- ** prohibited and immediately terminates your license unless           **
-- ** covered by a separate agreement.                                    **
-- **                                                                     **
-- ** Xilinx is providing this design, code, or information               **
-- ** "as-is" solely for use in developing programs and                   **
-- ** solutions for Xilinx devices, with no obligation on the             **
-- ** part of Xilinx to provide support. By providing this design,        **
-- ** code, or information as one possible implementation of              **
-- ** this feature, application or standard, Xilinx is making no          **
-- ** representation that this implementation is free from any            **
-- ** claims of infringement. You are responsible for obtaining           **
-- ** any rights you may require for your implementation.                 **
-- ** Xilinx expressly disclaims any warranty whatsoever with             **
-- ** respect to the adequacy of the implementation, including            **
-- ** but not limited to any warranties or representations that this      **
-- ** implementation is free from claims of infringement, implied         **
-- ** warranties of merchantability or fitness for a particular           **
-- ** purpose.                                                            **
-- **                                                                     **
-- ** Xilinx products are not intended for use in life support            **
-- ** appliances, devices, or systems. Use in such applications is        **
-- ** expressly prohibited.                                               **
-- **                                                                     **
-- ** Any modifications that are made to the Source Code are              **
-- ** done at the user’s sole risk and will be unsupported.               **
-- ** The Xilinx Support Hotline does not have access to source           **
-- ** code and therefore cannot answer specific questions related         **
-- ** to source HDL. The Xilinx Hotline support of original source        **
-- ** code IP shall only address issues and questions related             **
-- ** to the standard Netlist version of the core (and thus               **
-- ** indirectly, the original core source).                              **
-- **                                                                     **
-- ** Copyright (c) 2005-2010 Xilinx, Inc. All rights reserved.           **
-- **                                                                     **
-- ** This copyright and support notice must be retained as part          **
-- ** of this text at all times.                                          **
-- **                                                                     **
-- *************************************************************************
--
-------------------------------------------------------------------------------
-- Filename:        mux_onehot_f.vhd
--
-- Description:     Parameterizable multiplexer with one hot select lines.
--
--                  Please refer to the entity interface while reading the
--                  remainder of this description.
--
--                  If n is the index of the single select line of S(0 to C_NB-1)
--                  that is asserted, then
--
--                      Y(0 to C_DW-1) <= D(n*C_DW to n*C_DW + C_DW -1)
--
--                  That is, Y selects the nth group of C_DW consecutive
--                  bits of D.
--
--                  Note that C_NB = 1 is handled as a special case in which
--                  Y <= D, without regard to the select line, S.
--
--                  The Implementation depends on the C_FAMILY parameter.
--                  If the target family supports the needed primitives,
--                  a carry-chain structure will be implemented. Otherwise,
--                  an implementation dependent on synthesis inferral will
--                  be generated.
--
-------------------------------------------------------------------------------
-- Structure:   
--      mux_onehot_f
--------------------------------------------------------------------------------
-- Author:      FLO
-- History:
--  FLO             11/30/05      -- First version derived from mux_onehot.vhd
--                                -- by BLT and ALS.
--
-- ~~~~~~
--
--     DET     1/17/2008     v4_0
-- ~~~~~~
--     - Incorporated new disclaimer header
-- ^^^^^^
--
---------------------------------------------------------------------------------
-- Naming Conventions:
--      active low signals:                     "*_n"
--      clock signals:                          "clk", "clk_div#", "clk_#x" 
--      reset signals:                          "rst", "rst_n" 
--      generics:                               "C_*" 
--      user defined types:                     "*_TYPE" 
--      state machine next state:               "*_ns" 
--      state machine current state:            "*_cs" 
--      combinatorial signals:                  "*_cmb" 
--      pipelined or register delay signals:    "*_d#" 
--      counter signals:                        "*cnt*"
--      clock enable signals:                   "*_ce" 
--      internal version of output port         "*_i"
--      device pins:                            "*_pin" 
--      ports:                                  - Names begin with Uppercase 
--      processes:                              "*_PROCESS" 
--      component instantiations:               "<ENTITY_>I_<#|FUNC>
-------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;

-------------------------------------------------------------------------------
-- Generic and Port Declaration
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-- Definition of Generics and Ports
--
--      C_DW: Data width of buses entering the mux. Valid range is 1 to 256.
--      C_NB: Number of data buses entering the mux. Valid range is 1 to 64.
--
--      input D           -- input data bus
--      input S           -- input select bus
--      output Y          -- output bus
--
--      The input data is represented by a one-dimensional bus that is made up
--      of all of the data buses concatenated together. For example, a 4 to 1
--      mux with 2 bit data buses (C_DW=2,C_NB=4) is represented by:
--          
--        D = (Bus0Data0, Bus0Data1, Bus1Data0, Bus1Data1, Bus2Data0, Bus2Data1,
--             Bus3Data0, Bus3Data1)
--      
--        Y = (Bus0Data0, Bus0Data1) if S(0)=1 else
--            (Bus1Data0, Bus1Data1) if S(1)=1 else
--            (Bus2Data0, Bus2Data1) if S(2)=1 else
--            (Bus3Data0, Bus3Data1) if S(3)=1 
--
--        Only one bit of S should be asserted at a time.
--
-------------------------------------------------------------------------------

entity mux_onehot_f is 
   generic( C_DW: integer := 32;
            C_NB: integer := 5;
            C_FAMILY : string := "virtexe");
   port(
      D: in std_logic_vector(0 to C_DW*C_NB-1);
      S: in std_logic_vector(0 to C_NB-1);
      Y: out std_logic_vector(0 to C_DW-1));

end mux_onehot_f;

library unisim;
use     unisim.all; -- Make unisim entities available for default binding.
architecture imp of mux_onehot_f is

    constant NLS : natural := 6;--native_lut_size(fam_as_string => C_FAMILY,
                                  --            no_lut_return_val => 2*C_NB);

    function lut_val(D, S : std_logic_vector) return std_logic is
        variable rn : std_logic := '0';
    begin
        for i in D'range loop
            rn := rn or (S(i) and D(i));
        end loop;
        return not rn;
    end;

    function min(i, j : integer) return integer is
    begin
        if i < j then return i; else return j; end if;
    end;

-----------------------------------------------------------------------------
-- Signal and Type Declarations
-------------------------------------------------------------------------------
signal Dreord:      std_logic_vector(0 to C_DW*C_NB-1);
signal sel:         std_logic_vector(0 to C_DW*C_NB-1);

-------------------------------------------------------------------------------
-- Component Declarations
-------------------------------------------------------------------------------
component MUXCY
    port
    (
        O : out std_ulogic;
        CI : in std_ulogic;
        DI : in std_ulogic;
        S : in std_ulogic
    );
end component;

begin

-- Reorder data buses

WA_GEN : if C_DW > 0 generate -- XST WA
REORD: process( D )
variable m,n: integer;
begin
for m in 0 to C_DW-1 loop
  for n in 0 to C_NB-1 loop
    Dreord( m*C_NB+n) <= D( n*C_DW+m );
  end loop;
end loop;
end process REORD;
end generate;

-------------------------------------------------------------------------------
-- REPSELS_PROCESS
-------------------------------------------------------------------------------
-- The one-hot select bus contains 1-bit for each bus. To more easily
-- parameterize the carry chains and reduce loading on the select bus, these 
-- signals are replicated into a bus that replicates the select bits for the 
-- data width of the busses
-------------------------------------------------------------------------------
REPSELS_PROCESS : process ( S )
variable i, j   : integer;
begin
    -- loop through all data bits and busses
    for i in 0 to C_DW-1 loop
        for j in 0 to C_NB-1 loop
            sel(i*C_NB+j) <= S(j);
        end loop;
    end loop;
end process REPSELS_PROCESS;


GEN: if C_NB > 1 generate
    constant BPL : positive := NLS / 2; -- Buses per LUT is the native lut
                                        -- size divided by two.signals per bus.
    constant NUMLUTS : positive := (C_NB+(BPL-1))/BPL;
begin

    DATA_WIDTH_GEN: for i in 0 to C_DW-1 generate
        signal cyout  : std_logic_vector(0 to NUMLUTS);
        signal lutout : std_logic_vector(0 to NUMLUTS-1);
    begin

        cyout(0) <= '0';

        NUM_BUSES_GEN: for j in 0 to NUMLUTS - 1 generate
            constant BTL : positive := min(BPL, C_NB - j*BPL);
            -- Number of Buses This Lut (for last LUT this may be less than BPL)
        begin
            lutout(j) <= lut_val(D => Dreord(i*C_NB+j*BPL to i*C_NB+j*BPL+BTL-1),
                                 S =>    sel(i*C_NB+j*BPL to i*C_NB+j*BPL+BTL-1)
                                );

            MUXCY_GEN : if NUMLUTS > 1 generate
            MUXCY_I : component MUXCY
                port map (CI=>cyout(j),
                          DI=> '1',
                          S=>lutout(j),
                          O=>cyout(j+1));
            end generate;

        end generate;

    Y(i) <= cyout(NUMLUTS) when NUMLUTS > 1 else not lutout(0); -- If just one
                                            -- LUT, then take value from
                                            -- lutout rather than cyout.
    end generate;
end generate;


ONE_GEN: if C_NB = 1 generate
    Y <= D;
end generate;

end imp;



-------------------------------------------------------------------------------
-- mac_pkg - Package
-------------------------------------------------------------------------------
--  ***************************************************************************
--  ** DISCLAIMER OF LIABILITY                                               **
--  **                                                                       **
--  **  This file contains proprietary and confidential information of       **
--  **  Xilinx, Inc. ("Xilinx"), that is distributed under a license         **
--  **  from Xilinx, and may be used, copied and/or disclosed only           **
--  **  pursuant to the terms of a valid license agreement with Xilinx.      **
--  **                                                                       **
--  **  XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION                **
--  **  ("MATERIALS") "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER           **
--  **  EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING WITHOUT                  **
--  **  LIMITATION, ANY WARRANTY WITH RESPECT TO NONINFRINGEMENT,            **
--  **  MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE. Xilinx        **
--  **  does not warrant that functions included in the Materials will       **
--  **  meet the requirements of Licensee, or that the operation of the      **
--  **  Materials will be uninterrupted or error-free, or that defects       **
--  **  in the Materials will be corrected. Furthermore, Xilinx does         **
--  **  not warrant or make any representations regarding use, or the        **
--  **  results of the use, of the Materials in terms of correctness,        **
--  **  accuracy, reliability or otherwise.                                  **
--  **                                                                       **
--  **  Xilinx products are not designed or intended to be fail-safe,        **
--  **  or for use in any application requiring fail-safe performance,       **
--  **  such as life-support or safety devices or systems, Class III         **
--  **  medical devices, nuclear facilities, applications related to         **
--  **  the deployment of airbags, or any other applications that could      **
--  **  lead to death, personal injury or severe property or                 **
--  **  environmental damage (individually and collectively, "critical       **
--  **  applications"). Customer assumes the sole risk and liability         **
--  **  of any use of Xilinx products in critical applications,              **
--  **  subject only to applicable laws and regulations governing            **
--  **  limitations on product liability.                                    **
--  **                                                                       **
--  **  Copyright 2010 Xilinx, Inc.                                          **
--  **  All rights reserved.                                                 **
--  **                                                                       **
--  **  This disclaimer and copyright notice must be retained as part        **
--  **  of this file at all times.                                           **
--  ***************************************************************************
--
-------------------------------------------------------------------------------
-- Filename     : mac_pkg.vhd
-- Version      : v2.0
-- Description  : This file contains the constants used in the design of the
--                Ethernet MAC.
-- VHDL-Standard: VHDL'93
-------------------------------------------------------------------------------
-- Structure:   
--
--  axi_ethernetlite.vhd
--      \
--      \-- axi_interface.vhd
--      \-- xemac.vhd
--           \
--           \-- mdio_if.vhd
--           \-- emac_dpram.vhd                     
--           \    \                     
--           \    \-- RAMB16_S4_S36
--           \                          
--           \    
--           \-- emac.vhd                     
--                \                     
--                \-- MacAddrRAM                   
--                \-- receive.vhd
--                \      rx_statemachine.vhd
--                \      rx_intrfce.vhd
--                \         async_fifo_fg.vhd
--                \      crcgenrx.vhd
--                \                     
--                \-- transmit.vhd
--                       crcgentx.vhd
--                          crcnibshiftreg
--                       tx_intrfce.vhd
--                          async_fifo_fg.vhd
--                       tx_statemachine.vhd
--                       deferral.vhd
--                          cntr5bit.vhd
--                          defer_state.vhd
--                       bocntr.vhd
--                          lfsr16.vhd
--                       msh_cnt.vhd
--                       ld_arith_reg.vhd
--
-------------------------------------------------------------------------------
-- Author:    PVK
-- History:    
-- PVK        06/07/2010     First Version
-- ^^^^^^
-- First version.  
-- ~~~~~~
-------------------------------------------------------------------------------
-- Naming Conventions:
--      active low signals:                     "*_n"
--      clock signals:                          "clk", "clk_div#", "clk_#x" 
--      reset signals:                          "rst", "rst_n" 
--      generics:                               "C_*" 
--      user defined types:                     "*_TYPE" 
--      state machine next state:               "*_ns" 
--      state machine current state:            "*_cs" 
--      combinatorial signals:                  "*_com" 
--      pipelined or register delay signals:    "*_d#" 
--      counter signals:                        "*cnt*"
--      clock enable signals:                   "*_ce" 
--      internal version of output port         "*_i"
--      device pins:                            "*_pin" 
--      ports:                                  - Names begin with Uppercase 
--      processes:                              "*_PROCESS" 
--      component instantiations:               "<ENTITY_>I_<#|FUNC>
-------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;

package mac_pkg is


  type tx_state_name is 
    (idle,txEnCheck,loadByteCnt,checkByteCnt,checkByteCntOvrWrtSrcAdr,
     requestFifoRd,requestFifoRdOvrWrtSrcAdr,waitFifoEmpty,decByteCnt,
     decByteCntOvrWrtSrcAdr,checkBusFifoFull,checkBusFifoFullOvrWrtSrcAdr,
     loadBusFifo,loadBusFifoOvrWrtSrcAdr,txDone,preamble,SFD,loadBusFifoSrcAdr,
     loadBusFifoJam,checkBusFifoFullSrcAdr,loadBusFifoPad,checkBusFifoFullPad,
     loadBusFifoCrc,checkBusFifoFullCrc,checkBusFifoFullJam,lateCollision,
     excessDeferal,collisionRetry,cleanPacketCheckByteCnt,
     cleanPacketRequestFifoRd,cleanPacketDecByteCnt,retryWaitFifoEmpty,
     retryReset,tlrRead,checkBusFifoFullSFD,txDone2);



-------------------------------------------------------------------------------
-- Constant Declarations
-------------------------------------------------------------------------------

  constant XEMAC_MAJOR_VERSION : std_logic_vector(0 to 3) := "0001"; -- 1
                                 -- binary encoded major version number
    
  constant XEMAC_MINOR_VERSION : std_logic_vector(0 to 6) := "0000000";-- 00
                                 -- binary encoded minor version number   
    
  constant XEMAC_REVISION      : std_logic_vector(0 to 4)      := "00001";-- rev b
                                 -- binary encoded revision letter. a = "00000" z = "11001"    
    
  constant XEMAC_BLOCK_TYPE    : std_logic_vector(0 to 7) := "00000001";-- 1
                                 -- value that indentifies this device as a 10/100 Ethernet IP
    
  constant RESET_ACTIVE        : std_logic := '1'; 
                                -- the RESET_ACTIVE constant should denote the
                                -- logic level of an active reset
  constant MinimumPacketLength : std_logic_vector (0 to 15) := "0000000000111100";
                                -- 60 = 3c in hex
    
  constant MAXENetPktLength    : natural   := 1500;
  constant MINENetPktLength    : natural   := 46;
-------------------------------------------------------------------------------
-- these constants give data and address bus sizes for 3 widths of data
-- such that the largest ethernet frame will be addressable
-------------------------------------------------------------------------------

  constant dbw4  : natural := 4;
  constant abw4  : natural := 12;
  constant dbw8  : natural := 8;
  constant abw8  : natural := 11;
  constant dbw16 : natural := 16;
  constant abw16 : natural := 10;
  
  constant CRCDataWidth : natural := 32;  -- crc output data width
  
  subtype UNSIGNED30BIT is std_logic_vector (0 to 29);
  subtype UNSIGNED29BIT is std_logic_vector (0 to 28);
  subtype UNSIGNED27BIT is std_logic_vector (0 to 26);
  subtype UNSIGNED24BIT is std_logic_vector (0 to 23);
  subtype UNSIGNED23BIT is std_logic_vector (0 to 22);
  subtype UNSIGNED22BIT is std_logic_vector (0 to 21);
  subtype UNSIGNED21BIT is std_logic_vector (0 to 20);
  subtype UNSIGNED20BIT is std_logic_vector (0 to 19);
  subtype UNSIGNED18BIT is std_logic_vector (0 to 17);
  subtype UNSIGNED16BIT is std_logic_vector (0 to 15);
  subtype UNSIGNED12BIT is std_logic_vector (0 to 11);
  subtype UNSIGNED11BIT is std_logic_vector (0 to 10);
  subtype UNSIGNED10BIT is std_logic_vector (0 to 9);
  subtype UNSIGNED9BIT  is std_logic_vector (0 to 8);
  subtype UNSIGNED8BIT  is std_logic_vector (0 to 7);
  subtype UNSIGNED6BIT  is std_logic_vector (0 to 5);
    
  subtype ENetAddr      is std_logic_vector (47 downto 0);  -- ethernet address
  subtype IPAddress     is std_logic_vector(31 downto 0);


  subtype TwoBit is std_logic_vector (1 downto 0);  -- half a Nibble    
  subtype Nibble is std_logic_vector (3 downto 0);  -- half a byte
  subtype Byte   is std_logic_vector (7 downto 0);  -- single byte
  subtype Monk   is std_logic_vector (8 downto 0);  -- monkey (500, for 512)
  subtype Deck   is std_logic_vector (9 downto 0);  -- a 10 digit binary number
  subtype Word   is std_logic_vector (15 downto 0); -- double byte
  subtype DWord  is std_logic_vector (31 downto 0); -- quadruple byte


  subtype CRCData is std_logic_vector(CRCDataWidth - 1 downto 0);
  
-------------------------------------------------------------------------------
-- these standard types are used for the address bus declarations
-------------------------------------------------------------------------------
 
  subtype NibbleAddress is std_logic_vector(abw4 - 1 downto 0);
  subtype ByteAddress   is std_logic_vector(abw8 - 1 downto 0);
  subtype WordAddress   is std_logic_vector(abw16 - 1 downto 0);
  
  function getENetAddr (eaddr : string)           return ENetAddr;
  function revBitOrder (arg   : std_logic_vector) return std_logic_vector; 
  function convENetAddr(arg   : ENetAddr)         return bit_vector;   
                                                         -- By ben  06/28/2000
  function revNibOrder(arg    : std_logic_vector) return std_logic_vector; 
                                                         -- by ben 07/04/2000
  function getIPAddr (ipaddr  : string)           return IPAddress;
  function allZeroes (inp     : std_logic_vector) return boolean;
  function allOnes (inp       : std_logic_vector) return boolean;
  function zExtend (arg1      : std_logic_vector; size : natural) 
                                                     return std_logic_vector;
  function maxNat (arg1, arg2 : natural)          return natural;
  function netOrder (arg      : Word)             return Word; --by Ying
  function netOrder (arg      : bit_vector(15 downto 0)) return bit_vector;

  function GetInitString4 (   idx : integer;
     init_00 : bit_vector(15 downto 0);   init_01 : bit_vector(15 downto 0);
     init_02 : bit_vector(15 downto 0);   init_03 : bit_vector(15 downto 0))
     return string;

  function GetInitVector4 (   idx : integer;
     init_00 : bit_vector(15 downto 0);   init_01 : bit_vector(15 downto 0);
     init_02 : bit_vector(15 downto 0);   init_03 : bit_vector(15 downto 0))
     return bit_vector;
  function to_string (bv : bit_vector) return string;
  function to_string (b : bit) return string;   
  function to_character (bv : bit_vector(3 downto 0)) return character;
  
end mac_pkg;

package body mac_pkg is

-- coverage off

  -- Convert 4-bit vector to a character
  function to_character (
    bv : bit_vector(3 downto 0))
    return character is
  begin  -- to_character
    case bv is
      when b"0000" => return '0';
      when b"0001" => return '1';
      when b"0010" => return '2';
      when b"0011" => return '3';
      when b"0100" => return '4';
      when b"0101" => return '5';
      when b"0110" => return '6';
      when b"0111" => return '7';
      when b"1000" => return '8';
      when b"1001" => return '9';
      when b"1010" => return 'a';
      when b"1011" => return 'b';
      when b"1100" => return 'c';
      when b"1101" => return 'd';
      when b"1110" => return 'e';
      when b"1111" => return 'f';
    end case;
  end to_character;
  
  -- Convert n-bits vector to n/4-character string
  function to_string (bv : bit_vector) return string is
    constant strlen : integer := bv'length / 4;
    variable str : string(1 to strlen);
    begin  -- to_string
    for i in 0 to strlen - 1 loop
      str(strlen-i) := to_character(bv((i * 4) + 3 downto (i * 4)));
    end loop;  -- i
    return str;
  end to_string;


  -- Convert 1-bit  to 1-character string
  function to_string (b : bit) return string is
  begin
    case b is
      when '0'    => return "0";
      when '1'    => return "1";
      when others => assert false report "unrecognised bit value" severity failure;
    end case;
    return "0";
  end to_string;
  
  function netOrder (arg : bit_vector(15 downto 0))
      return bit_vector is
    variable res : bit_vector(15 downto 0);
  begin  -- netOrder
    res(15 downto 12) := arg(11 downto 8);
    res(11 downto 8) := arg(15 downto 12);
    res(7 downto 4) := arg(3 downto 0);
    res(3 downto 0) := arg(7 downto 4);
    return res;
  end netOrder;

  -- Generate the label string for LUT ROM 16x4 from the init strings
    function GetInitString4 (   idx : integer;
       init_00 : bit_vector(15 downto 0);  init_01 : bit_vector(15 downto 0);
       init_02 : bit_vector(15 downto 0);  init_03 : bit_vector(15 downto 0))
       return string is
      variable bitvalue : bit_vector(15 downto 0) ;
      begin
        bitvalue(0)  := INIT_00(idx+12);  bitvalue(1)  := INIT_00(idx+8);
        bitvalue(2)  := INIT_00(idx+4);   bitvalue(3)  := INIT_00(idx);
        bitvalue(4)  := INIT_01(idx+12);  bitvalue(5)  := INIT_01(idx+8);
        bitvalue(6)  := INIT_01(idx+4);   bitvalue(7)  := INIT_01(idx);
        bitvalue(8)  := INIT_02(idx+12);  bitvalue(9)  := INIT_02(idx+8);
        bitvalue(10)  := INIT_02(idx+4);  bitvalue(11)  := INIT_02(idx);
        bitvalue(12)  := INIT_03(idx+12); bitvalue(13)  := INIT_03(idx+8);
        bitvalue(14)  := INIT_03(idx+4);  bitvalue(15)  := INIT_03(idx);
        return to_string(bitvalue);
    end function GetInitString4;


  -- Generate the generic init vector for the LUT ROM 16x4 from the
  -- init strings
  function GetInitVector4(    idx : integer;
        init_00 : bit_vector(15 downto 0); init_01 : bit_vector(15 downto 0);
        init_02 : bit_vector(15 downto 0); init_03 : bit_vector(15 downto 0))
         return bit_vector is
      variable bitvalue : bit_vector(15 downto 0) ;
      begin
        bitvalue(0)  := INIT_00(idx+12);  bitvalue(1)  := INIT_00(idx+8);
        bitvalue(2)  := INIT_00(idx+4);   bitvalue(3)  := INIT_00(idx);
        bitvalue(4)  := INIT_01(idx+12);  bitvalue(5)  := INIT_01(idx+8);
        bitvalue(6)  := INIT_01(idx+4);   bitvalue(7)  := INIT_01(idx);
        bitvalue(8)  := INIT_02(idx+12);  bitvalue(9)  := INIT_02(idx+8);
        bitvalue(10)  := INIT_02(idx+4);  bitvalue(11)  := INIT_02(idx);
        bitvalue(12)  := INIT_03(idx+12); bitvalue(13)  := INIT_03(idx+8);
        bitvalue(14)  := INIT_03(idx+4);  bitvalue(15)  := INIT_03(idx);
        return bitvalue;
  end function GetInitVector4;
  
  function conv_std_logic_vector (ch : character) return std_logic_vector is
  begin
    case ch is
      when '0'    => return "0000";
      when '1'    => return "0001";
      when '2'    => return "0010";
      when '3'    => return "0011";
      when '4'    => return "0100";
      when '5'    => return "0101";
      when '6'    => return "0110";
      when '7'    => return "0111";
      when '8'    => return "1000";
      when '9'    => return "1001";
      when 'a'    => return "1010";
      when 'b'    => return "1011";
      when 'c'    => return "1100";
      when 'd'    => return "1101";
      when 'e'    => return "1110";
      when 'f'    => return "1111";
      when others => assert false report "unrecognised character" 
                                                         severity failure;
    end case;
    return "0000";
  end conv_std_logic_vector;

  function getENetAddr (eaddr : string) return ENetAddr is
    variable tmp : ENetAddr := (others => '0');
    variable bptr : natural  := 0;
    variable nptr : natural  := 0;
    variable indx : natural  := 0;
  begin  -- getENetAddr
    tmp := (others => '0');
    bptr := 0;
    nptr := 0;
    indx := 0;
    if eaddr'length = 17 then
      lp0  : for i in eaddr'reverse_range loop
        -- lsbyte first
        if eaddr(i) = ':'  then
          bptr := bptr + 1;
          nptr := 0;
        else
          indx := (bptr * 8) + (nptr * 4);
          tmp(indx + 3 downto indx) := conv_std_logic_vector(eaddr(i));
          nptr := nptr + 1;
        end if;
      end loop lp0;
    else
      assert false report "ethernet address format is 01 : 23 : 45 : 67 : 89 : ab msb- > lsb" severity failure;
    end if;

    return tmp;

  end getENetAddr;

-------------------------------------------------------------------------------
-- A function which can change the order of ENetAddr to
-- the order of smallrom init   --   BY ben 06/28
-------------------------------------------------------------------------------


function convENetAddr(arg: ENetAddr) return bit_vector is
variable tmp : std_logic_vector(63 downto 0) :=(others => '0');
begin
 lp0:  for i in 0 to 11 loop
      tmp(59-i) := arg(3 + i*4);
      tmp(43 -i) := arg(2 + i*4);
      tmp(27 -i) := arg(1 + i*4);
      tmp(11 -i)    := arg(i*4);
 end loop lp0;     
return to_bitvector(tmp);
end convENetAddr;

-------------------------------------------------------------------------------
-- A function which can reverse the bit order
-- order   --   BY ben 07/04
-------------------------------------------------------------------------------

  function revBitOrder( arg : std_logic_vector) return std_logic_vector is  -- By ben 07/04/2000
    variable tmp            : std_logic_vector(arg'range);
  begin
    lp0                     : for i in arg'range loop
      tmp(arg'high - i) := arg(i);
    end loop lp0;
    return tmp;
  end revBitOrder;
  
-------------------------------------------------------------------------------
-- A function which can swap the Nibble order
-- order   --   BY ben 07/04
-------------------------------------------------------------------------------

  function swapNibbles (
    arg : std_logic_vector)
    return std_logic_vector is
    variable tmp : std_logic_vector(arg'length -1 downto 0);
    variable j : integer;
  begin  -- swapNibbles
    for i in 0 to (arg'length / 8) - 1 loop
      j := i * 8;
      tmp(j + 3 downto j) := arg(j + 7 downto j + 4);
      tmp(j + 7 downto j + 4) := arg(j + 3 downto j);
    end loop;  -- i
    return tmp;
  end swapNibbles;

-------------------------------------------------------------------------------
-- A function which can reverse the Nibble order
-- order   --   BY ben 07/04
-------------------------------------------------------------------------------

function revNibOrder( arg : std_logic_vector) return std_logic_vector is   -- By ben 07/04/2000
 variable tmp : std_logic_vector(arg'high downto 0); -- length is numNubs
 variable numNibs : integer;
 begin
  numNibs := arg'length/4;
  lp0: for i in 0 to numNibs -1 loop
       tmp( (4*(numNibs-i)-1) downto 4*(numNibs-i-1) ) := arg( (4*i+3) downto 4*i);
  end loop lp0;
  return tmp ;
end revNibOrder;

-------------------------------------------------------------------------------
-- Afunction to parse IP address
------------------------------------------------------------------------------- 
 
  function getIPAddr (ipaddr : string) return IPAddress is
    variable tmp             : IPAddress := (others => '0');
    variable bptr            : natural   := 3;
    variable nptr            : natural   := 2;
    variable indx            : natural   := 0;
  begin
    bptr                                 := 3;
    nptr                                 := 2;
    indx                                 := 0;
    -- similar to above, take a fixed length string and parse it for
    -- expected characters.  We anticipate hex numbers if the format,
    --   hh.hh.hh.hh
    if ipaddr'length = 11 then
      for i in ipaddr'range loop
        if ipaddr(i) = '.' then
          bptr                           := bptr - 1;
          nptr                           := 2;
        else
          indx                           := (bptr * 8) + ((nptr - 1) * 4);
          tmp(indx + 3 downto indx)      := conv_std_logic_vector(ipaddr(i));
          nptr                           := nptr - 1;
        end if;
      end loop;  -- i
    else
      assert false report "IP address format is 01.23.45.67 msb- > lsb" severity failure;
    end if;

    return tmp;

  end getIPAddr;

-------------------------------------------------------------------------------
-- couple of useful functions, move them to utils eventually
-------------------------------------------------------------------------------
  function allZeroes (inp : std_logic_vector) return boolean is
    variable t : boolean := true;
  begin
    t := true;  -- for synopsys
    for i in inp'range loop
      if inp(i) = '1' then
        t := false;
      end if;
    end loop;
    return t;
  end allZeroes;

  function allOnes (inp : std_logic_vector) return boolean is
    variable t : boolean := true;
  begin
    t := true;  -- for synopsys
    for i in inp'range loop
      if inp(i) = '0' then
        t := false;
      end if;
    end loop;
    return t;
  end allOnes;

-------------------------------------------------------------------------------
-- returns the maximum of two naturals
-------------------------------------------------------------------------------
  function maxNat (arg1, arg2 : natural)
    return natural is
  begin  -- maxNat
    if arg1 >= arg2 then
      return arg1;
    else
      return arg2;
    end if;
  end maxNat;

-------------------------------------------------------------------------------
-- zero extend a std_logic_vector
-------------------------------------------------------------------------------
  function zExtend (
    arg1            : std_logic_vector;
    size            : natural)
    return std_logic_vector is
    variable result : std_logic_vector(size - 1 downto 0);
  begin  -- extend
    result := (others => '0');
    result(arg1'length - 1 downto 0) := arg1;
    return result;
  end zExtend;
  
-------------------------------------------------------------------------------
-- Switch the word order between Correct-Word-Order and Net-Word-Order.
-- If arg is (Hex)1234, 
-- then netOrder(arg) is (Hex)2143.
-------------------------------------------------------------------------------
--  function netOrder (arg : Word)
--      return Word is
--  begin  -- netOrder
--      return arg(11 downto 8) & arg(15 downto 12) &
--             arg(3 downto 0) & arg(7 downto 4);
--  end netOrder;
--  function netOrder (arg : std_logic_vector (15 downto 0))
--      return std_logic_vector is
--    variable res : std_logic_vector (15 downto 0);
--  begin  -- netOrder
--    res(15 downto 12) := arg(11 downto 8);
--    res(11 downto 8) := arg(15 downto 12);
--    res(7 downto 4) := arg(3 downto 0);
--    res(3 downto 0) := arg(7 downto 4);
--    return res;
--  end netOrder;
  function netOrder (arg : Word)
      return Word is
    variable res : Word;--(15 downto 0);
  begin  -- netOrder
    res(15 downto 12) := arg(11 downto 8);
    res(11 downto 8) := arg(15 downto 12);
    res(7 downto 4) := arg(3 downto 0);
    res(3 downto 0) := arg(7 downto 4);
    return res;
  end netOrder;

-- coverage on
  
end mac_pkg;


-------------------------------------------------------------------------------
-- lfsr16 - entity/architecture pair
-------------------------------------------------------------------------------
--  ***************************************************************************
--  ** DISCLAIMER OF LIABILITY                                               **
--  **                                                                       **
--  **  This file contains proprietary and confidential information of       **
--  **  Xilinx, Inc. ("Xilinx"), that is distributed under a license         **
--  **  from Xilinx, and may be used, copied and/or disclosed only           **
--  **  pursuant to the terms of a valid license agreement with Xilinx.      **
--  **                                                                       **
--  **  XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION                **
--  **  ("MATERIALS") "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER           **
--  **  EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING WITHOUT                  **
--  **  LIMITATION, ANY WARRANTY WITH RESPECT TO NONINFRINGEMENT,            **
--  **  MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE. Xilinx        **
--  **  does not warrant that functions included in the Materials will       **
--  **  meet the requirements of Licensee, or that the operation of the      **
--  **  Materials will be uninterrupted or error-free, or that defects       **
--  **  in the Materials will be corrected. Furthermore, Xilinx does         **
--  **  not warrant or make any representations regarding use, or the        **
--  **  results of the use, of the Materials in terms of correctness,        **
--  **  accuracy, reliability or otherwise.                                  **
--  **                                                                       **
--  **  Xilinx products are not designed or intended to be fail-safe,        **
--  **  or for use in any application requiring fail-safe performance,       **
--  **  such as life-support or safety devices or systems, Class III         **
--  **  medical devices, nuclear facilities, applications related to         **
--  **  the deployment of airbags, or any other applications that could      **
--  **  lead to death, personal injury or severe property or                 **
--  **  environmental damage (individually and collectively, "critical       **
--  **  applications"). Customer assumes the sole risk and liability         **
--  **  of any use of Xilinx products in critical applications,              **
--  **  subject only to applicable laws and regulations governing            **
--  **  limitations on product liability.                                    **
--  **                                                                       **
--  **  Copyright 2010 Xilinx, Inc.                                          **
--  **  All rights reserved.                                                 **
--  **                                                                       **
--  **  This disclaimer and copyright notice must be retained as part        **
--  **  of this file at all times.                                           **
--  ***************************************************************************
--
-------------------------------------------------------------------------------
-- Filename     : lfsr16.vhd
-- Version      : v2.0
-- Description  : This is a 15 bit random number generator.
--                In simulation we need to reset first, then give the enable signal.
-- VHDL-Standard: VHDL'93
-------------------------------------------------------------------------------
-- Structure:   
--
--  axi_ethernetlite.vhd
--      \
--      \-- axi_interface.vhd
--      \-- xemac.vhd
--           \
--           \-- mdio_if.vhd
--           \-- emac_dpram.vhd                     
--           \    \                     
--           \    \-- RAMB16_S4_S36
--           \                          
--           \    
--           \-- emac.vhd                     
--                \                     
--                \-- MacAddrRAM                   
--                \-- receive.vhd
--                \      rx_statemachine.vhd
--                \      rx_intrfce.vhd
--                \         async_fifo_fg.vhd
--                \      crcgenrx.vhd
--                \                     
--                \-- transmit.vhd
--                       crcgentx.vhd
--                          crcnibshiftreg
--                       tx_intrfce.vhd
--                          async_fifo_fg.vhd
--                       tx_statemachine.vhd
--                       deferral.vhd
--                          cntr5bit.vhd
--                          defer_state.vhd
--                       bocntr.vhd
--                          lfsr16.vhd
--                       msh_cnt.vhd
--                       ld_arith_reg.vhd
--
-------------------------------------------------------------------------------
-- Author:    PVK
-- History:    
-- PVK        06/07/2010     First Version
-- ^^^^^^
-- First version.  
-- ~~~~~~
-------------------------------------------------------------------------------
-- Naming Conventions:
--      active low signals:                     "*_n"
--      clock signals:                          "Clk", "clk_div#", "clk_#x" 
--      reset signals:                          "Rst", "rst_n" 
--      generics:                               "C_*" 
--      user defined types:                     "*_TYPE" 
--      state machine next state:               "*_ns" 
--      state machine current state:            "*_cs" 
--      combinatorial signals:                  "*_com" 
--      pipelined or register delay signals:    "*_d#" 
--      counter signals:                        "*cnt*"
--      clock enable signals:                   "*_ce" 
--      internal version of output port         "*_i"
--      device pins:                            "*_pin" 
--      ports:                                  - Names begin with Uppercase 
--      processes:                              "*_PROCESS" 
--      component instantiations:               "<ENTITY_>I_<#|FUNC>
-------------------------------------------------------------------------------
-- 
-- This is a 15 bit random number generator.
-- In simulation we need to reset first, then give the enable signal.
-- 
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

-------------------------------------------------------------------------------
-- Vcomponents from unisim library is used for FIFO instatiation
-- function declarations
-------------------------------------------------------------------------------
library unisim;
use unisim.Vcomponents.all;

-------------------------------------------------------------------------------
-- axi_ethernetlite_v3_0_18 library is used for axi_ethernetlite_v3_0_18 
-- component declarations
-------------------------------------------------------------------------------
library axi_ethernetlite_v3_0_18;
use axi_ethernetlite_v3_0_18.mac_pkg.all;

-- synopsys translate_off
-- Library XilinxCoreLib;
-- synopsys translate_on

-------------------------------------------------------------------------------
-- Port Declaration
-------------------------------------------------------------------------------
-- Definition of Ports:
--
--  Clk          -- System Clock
--  Rst          -- System Reset
--  Clken        -- Clock enable
--  Enbl         -- LFSR enable
--  Shftout      -- Shift data output
-------------------------------------------------------------------------------
-- ENTITY
-------------------------------------------------------------------------------
entity lfsr16 is
  port 
       (
        Clk     : in  std_logic; 
        Rst     : in  std_logic; 
        Clken   : in  std_logic; -- tx Clk based. Assumed to be 2.5 or 25 MHz
        Enbl    : in  std_logic; 
        Shftout : out std_logic
        );
end lfsr16;
-------------------------------------------------------------------------------
-- Definition of Generics:
--          No Generics were used for this Entity.
--
-- Definition of Ports:
--         
-------------------------------------------------------------------------------
architecture imp of lfsr16 is

attribute DowngradeIPIdentifiedWarnings: string;

attribute DowngradeIPIdentifiedWarnings of imp : architecture is "yes";

-------------------------------------------------------------------------------
--  Constant Declarations
-------------------------------------------------------------------------------
-- Constants used in this design are found in mac_pkg.vhd

-------------------------------------------------------------------------------
-- Signal and Type Declarations
-------------------------------------------------------------------------------
  signal   Bit1           : std_logic;
  signal   Bit15          : std_logic;
  signal   Bit14          : std_logic;
  signal   XNORGateOutput : std_logic;
  signal   zero           : std_logic;
  signal   one            : std_logic;
  signal   combo_enbl     : std_logic;
-------------------------------------------------------------------------------
-- Component Declarations
-------------------------------------------------------------------------------
-- The following components are the building blocks of the 16 bit lfsr

  component SRL16E
   -- synthesis translate_off 
       generic (INIT : bit_vector := X"0000"); 
   -- synthesis translate_on 
    port (
      Q   : out std_logic;
      A0  : in  std_logic;              -- Set the address to 12.
      A1  : in  std_logic;
      A2  : in  std_logic;
      A3  : in  std_logic;
      D   : in  std_logic;
      Clk : in  std_logic;
      CE  : in  std_logic);
  end component;
  
begin

  zero <= '0';
  one  <= '1';

  SHREG0 : SRL16E
   -- synthesis translate_off 
   generic map (INIT => X"5a5a") 
   -- synthesis translate_on 
    port map(
      Q   => Bit14,
      A0  => zero,
      A1  => zero,
      A2  => one,
      A3  => one,
      D   => Bit1,
      CE  => combo_enbl,
      Clk => Clk);
      combo_enbl <= Enbl and Clken;
      
-------------------------------------------------------------------------------
-- determine bit 15 value
-------------------------------------------------------------------------------
  REG0_PROCESS:process(Clk)
  begin
    if Clk'event and Clk = '1' then
      if Rst = '1' then
        Bit15 <= '0';
      elsif combo_enbl = '1' then
        Bit15 <= Bit14;
      end if;
    end if;
  end process REG0_PROCESS;
      
-------------------------------------------------------------------------------
-- determine bit 1 value
-------------------------------------------------------------------------------
  REG1_PROCESS:process(Clk)
  begin
    if Clk'event and Clk = '1' then
      if Rst = '1' then
        Bit1 <= '0';
      elsif combo_enbl = '1' then
        Bit1 <= XNORGateOutput;
      end if;
    end if;
  end process REG1_PROCESS;


  XNORGateOutput <= Bit14 XNOR Bit15; 
  
  Shftout        <= Bit1;
  
end imp;


-------------------------------------------------------------------------------
-- defer_state - entity/architecture pair
-------------------------------------------------------------------------------
--  ***************************************************************************
--  ** DISCLAIMER OF LIABILITY                                               **
--  **                                                                       **
--  **  This file contains proprietary and confidential information of       **
--  **  Xilinx, Inc. ("Xilinx"), that is distributed under a license         **
--  **  from Xilinx, and may be used, copied and/or disclosed only           **
--  **  pursuant to the terms of a valid license agreement with Xilinx.      **
--  **                                                                       **
--  **  XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION                **
--  **  ("MATERIALS") "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER           **
--  **  EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING WITHOUT                  **
--  **  LIMITATION, ANY WARRANTY WITH RESPECT TO NONINFRINGEMENT,            **
--  **  MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE. Xilinx        **
--  **  does not warrant that functions included in the Materials will       **
--  **  meet the requirements of Licensee, or that the operation of the      **
--  **  Materials will be uninterrupted or error-free, or that defects       **
--  **  in the Materials will be corrected. Furthermore, Xilinx does         **
--  **  not warrant or make any representations regarding use, or the        **
--  **  results of the use, of the Materials in terms of correctness,        **
--  **  accuracy, reliability or otherwise.                                  **
--  **                                                                       **
--  **  Xilinx products are not designed or intended to be fail-safe,        **
--  **  or for use in any application requiring fail-safe performance,       **
--  **  such as life-support or safety devices or systems, Class III         **
--  **  medical devices, nuclear facilities, applications related to         **
--  **  the deployment of airbags, or any other applications that could      **
--  **  lead to death, personal injury or severe property or                 **
--  **  environmental damage (individually and collectively, "critical       **
--  **  applications"). Customer assumes the sole risk and liability         **
--  **  of any use of Xilinx products in critical applications,              **
--  **  subject only to applicable laws and regulations governing            **
--  **  limitations on product liability.                                    **
--  **                                                                       **
--  **  Copyright 2010 Xilinx, Inc.                                          **
--  **  All rights reserved.                                                 **
--  **                                                                       **
--  **  This disclaimer and copyright notice must be retained as part        **
--  **  of this file at all times.                                           **
--  ***************************************************************************
--
-------------------------------------------------------------------------------
-- Filename     : defer_state.vhd
-- Version      : v2.0
-- Description  : This file contains the transmit deferral state machine.
-- VHDL-Standard: VHDL'93
-------------------------------------------------------------------------------
-- Structure:   
--
--  axi_ethernetlite.vhd
--      \
--      \-- axi_interface.vhd
--      \-- xemac.vhd
--           \
--           \-- mdio_if.vhd
--           \-- emac_dpram.vhd                     
--           \    \                     
--           \    \-- RAMB16_S4_S36
--           \                          
--           \    
--           \-- emac.vhd                     
--                \                     
--                \-- MacAddrRAM                   
--                \-- receive.vhd
--                \      rx_statemachine.vhd
--                \      rx_intrfce.vhd
--                \         async_fifo_fg.vhd
--                \      crcgenrx.vhd
--                \                     
--                \-- transmit.vhd
--                       crcgentx.vhd
--                          crcnibshiftreg
--                       tx_intrfce.vhd
--                          async_fifo_fg.vhd
--                       tx_statemachine.vhd
--                       deferral.vhd
--                          cntr5bit.vhd
--                          defer_state.vhd
--                       bocntr.vhd
--                          lfsr16.vhd
--                       msh_cnt.vhd
--                       ld_arith_reg.vhd
--
-------------------------------------------------------------------------------
-- Author:    PVK
-- History:    
-- PVK        06/07/2010     First Version
-- ^^^^^^
-- First version.  
-- ~~~~~~
-------------------------------------------------------------------------------
-- Naming Conventions:
--      active low signals:                     "*_n"
--      clock signals:                          "Clk", "clk_div#", "clk_#x" 
--      reset signals:                          "Rst", "rst_n" 
--      generics:                               "C_*" 
--      user defined types:                     "*_TYPE" 
--      state machine next state:               "*_ns" 
--      state machine current state:            "*_cs" 
--      combinatorial signals:                  "*_com" 
--      pipelined or register delay signals:    "*_d#" 
--      counter signals:                        "*cnt*"
--      clock enable signals:                   "*_ce" 
--      internal version of output port         "*_i"
--      device pins:                            "*_pin" 
--      ports:                                  - Names begin with Uppercase 
--      processes:                              "*_PROCESS" 
--      component instantiations:               "<ENTITY_>I_<#|FUNC>
-------------------------------------------------------------------------------
-- 
library ieee;
use ieee.std_logic_1164.all;

-------------------------------------------------------------------------------
-- axi_ethernetlite_v3_0_18 library is used for axi_ethernetlite_v3_0_18 
-- component declarations
-------------------------------------------------------------------------------
library axi_ethernetlite_v3_0_18;
use axi_ethernetlite_v3_0_18.mac_pkg.all;

-- synopsys translate_off
-- Library XilinxCoreLib;
-- synopsys translate_on

-------------------------------------------------------------------------------
-- Port Declaration
-------------------------------------------------------------------------------
-- Definition of Ports:
--
--  Clk          -- System Clock
--  Rst          -- System Reset
--  TxEn         -- Transmit enable
--  Txrst        -- Transmit reset
--  Ifgp2Done    -- Interframe gap2 done
--  Ifgp1Done    -- Interframe gap1 done
--  BackingOff   -- Backing off 
--  Crs          -- Carrier sense
--  Full_half_n  -- Full/Half duplex indicator
--  Deferring    -- Deffering for the tx data
--  CntrEnbl     -- Counter enable 
--  CntrLd       -- Counter load
-------------------------------------------------------------------------------
-- ENTITY
-------------------------------------------------------------------------------
entity defer_state is
  port (
        Clk         : in  std_logic;
        Rst         : in  std_logic;
        TxEn        : in  std_logic; 
        Txrst       : in  std_logic;
        Ifgp2Done   : in  std_logic;
        Ifgp1Done   : in  std_logic;
        BackingOff  : in  std_logic;
        Crs         : in  std_logic;
        Full_half_n : in  std_logic;        
        Deferring   : out std_logic;
        CntrEnbl    : out std_logic;
        CntrLd      : out std_logic
        );
 
end defer_state;

-------------------------------------------------------------------------------
-- Definition of Generics:
--          No Generics were used for this Entity.
--
-- Definition of Ports:
--         
-------------------------------------------------------------------------------

architecture implementation of defer_state is

attribute DowngradeIPIdentifiedWarnings: string;

attribute DowngradeIPIdentifiedWarnings of implementation : architecture is "yes";

-------------------------------------------------------------------------------
--  Constant Declarations
-------------------------------------------------------------------------------
-- Constants used in this design are found in mac_pkg.vhd
-------------------------------------------------------------------------------
-- Signal and Type Declarations
-------------------------------------------------------------------------------
 
  type StateName is (loadCntr,startIfgp1Cnt,startIfgp2Cnt,cntDone);
  signal thisState               : StateName;
  signal nextState               : StateName;  
   
-------------------------------------------------------------------------------
-- Component Declarations
-------------------------------------------------------------------------------
-- The following components are the building blocks of the tx state machine
 
begin

   ----------------------------------------------------------------------------
   -- FSMR Process
   ----------------------------------------------------------------------------
   -- An FSM that deals with transmitting data
   ----------------------------------------------------------------------------
   FSMR : process (Clk)
   begin  --
      if (Clk'event and Clk = '1') then     -- rising clock edge
         if (Rst = '1' or Txrst = '1') then
            thisState <= loadCntr;
         else
            thisState <= nextState;
         end if;
      end if;
   end process FSMR;

   ----------------------------------------------------------------------------
   -- FSMC Process
   ----------------------------------------------------------------------------
   FSMC : process (thisState,TxEn,Ifgp2Done,Ifgp1Done,BackingOff,Crs,
                   Full_half_n)
   begin  --
      case thisState is
         when loadCntr =>
            if (((TxEn = '0') and (Full_half_n = '1')) or
                ((Crs = '0') and (Full_half_n = '0') and 
                 (BackingOff = '0'))) and 
                  Ifgp1Done = '0'  and Ifgp2Done = '0' then
               nextState <= startIfgp1Cnt;
            else
               nextState <= loadCntr; -- wait for end of transmission
            end if;
            
         when startIfgp1Cnt =>
            if (((Crs = '1') and (Full_half_n = '0')) or
                ((BackingOff = '1') and (Full_half_n = '0'))) then
               nextState <= loadCntr;
            elsif (Ifgp1Done = '1') then  -- gap done
               nextState <= startIfgp2Cnt;
            else
               nextState <= startIfgp1Cnt; -- still counting
            end if;
            

         when startIfgp2Cnt =>
            -- Added check for CRS to reset counter in when CRS goes low.
            if (((Crs = '1') and (Full_half_n = '0')) or
                ((BackingOff = '1') and (Full_half_n = '0'))) then
               nextState <= loadCntr;
            elsif (Ifgp2Done = '1') then  -- gap done
               nextState <= cntDone;
            else
               nextState <= startIfgp2Cnt; -- still counting
            end if;        
           
         when cntDone =>
            if (TxEn = '1' or Crs = '1') then  -- transmission started
               nextState <= loadCntr;
            else
               nextState <= cntDone;
            end if;        
           
      -- coverage off
         when others  => null;
            nextState <= loadCntr;
      -- coverage on
      
     end case;
   end process FSMC;
   
   ----------------------------------------------------------------------------
   -- FSMD Process
   ----------------------------------------------------------------------------
   FSMD : process(thisState)
   begin  --
      if ((thisState =  loadCntr) or (thisState =  startIfgp1Cnt) or 
          (thisState =  startIfgp2Cnt)) then
         Deferring <= '1';
      else
         Deferring <= '0';
      end if;
      
      if ((thisState = startIfgp1Cnt) or (thisState =  startIfgp2Cnt)) then
         CntrEnbl   <= '1';
      else
         CntrEnbl   <= '0';
      end if;
      
      if (thisState = loadCntr) then
         CntrLd   <= '1';
      else
         CntrLd   <= '0';
      end if;
   end process FSMD;   

end implementation;



-------------------------------------------------------------------------------
-- crcnibshiftreg - entity/architecture pair
-------------------------------------------------------------------------------
--  ***************************************************************************
--  ** DISCLAIMER OF LIABILITY                                               **
--  **                                                                       **
--  **  This file contains proprietary and confidential information of       **
--  **  Xilinx, Inc. ("Xilinx"), that is distributed under a license         **
--  **  from Xilinx, and may be used, copied and/or disclosed only           **
--  **  pursuant to the terms of a valid license agreement with Xilinx.      **
--  **                                                                       **
--  **  XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION                **
--  **  ("MATERIALS") "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER           **
--  **  EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING WITHOUT                  **
--  **  LIMITATION, ANY WARRANTY WITH RESPECT TO NONINFRINGEMENT,            **
--  **  MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE. Xilinx        **
--  **  does not warrant that functions included in the Materials will       **
--  **  meet the requirements of Licensee, or that the operation of the      **
--  **  Materials will be uninterrupted or error-free, or that defects       **
--  **  in the Materials will be corrected. Furthermore, Xilinx does         **
--  **  not warrant or make any representations regarding use, or the        **
--  **  results of the use, of the Materials in terms of correctness,        **
--  **  accuracy, reliability or otherwise.                                  **
--  **                                                                       **
--  **  Xilinx products are not designed or intended to be fail-safe,        **
--  **  or for use in any application requiring fail-safe performance,       **
--  **  such as life-support or safety devices or systems, Class III         **
--  **  medical devices, nuclear facilities, applications related to         **
--  **  the deployment of airbags, or any other applications that could      **
--  **  lead to death, personal injury or severe property or                 **
--  **  environmental damage (individually and collectively, "critical       **
--  **  applications"). Customer assumes the sole risk and liability         **
--  **  of any use of Xilinx products in critical applications,              **
--  **  subject only to applicable laws and regulations governing            **
--  **  limitations on product liability.                                    **
--  **                                                                       **
--  **  Copyright 2010 Xilinx, Inc.                                          **
--  **  All rights reserved.                                                 **
--  **                                                                       **
--  **  This disclaimer and copyright notice must be retained as part        **
--  **  of this file at all times.                                           **
--  ***************************************************************************
--
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-- Filename     : crcnibshiftreg.vhd
-- Version      : v2.0
-- Description  : CRC Nible Shift Register
--
-- VHDL-Standard:   VHDL'93
-------------------------------------------------------------------------------
-- Structure:   
--
--  axi_ethernetlite.vhd
--      \
--      \-- axi_interface.vhd
--      \-- xemac.vhd
--           \
--           \-- mdio_if.vhd
--           \-- emac_dpram.vhd                     
--           \    \                     
--           \    \-- RAMB16_S4_S36
--           \                          
--           \    
--           \-- emac.vhd                     
--                \                     
--                \-- MacAddrRAM                   
--                \-- receive.vhd
--                \      rx_statemachine.vhd
--                \      rx_intrfce.vhd
--                \         async_fifo_fg.vhd
--                \      crcgenrx.vhd
--                \                     
--                \-- transmit.vhd
--                       crcgentx.vhd
--                          crcnibshiftreg
--                       tx_intrfce.vhd
--                          async_fifo_fg.vhd
--                       tx_statemachine.vhd
--                       deferral.vhd
--                          cntr5bit.vhd
--                          defer_state.vhd
--                       bocntr.vhd
--                          lfsr16.vhd
--                       msh_cnt.vhd
--                       ld_arith_reg.vhd
--
-------------------------------------------------------------------------------
-- Author:    PVK
-- History:    
-- PVK        06/07/2010     First Version
-- ^^^^^^
-- First version.  
-- ~~~~~~
-------------------------------------------------------------------------------
-- Naming Conventions:
--      active low signals:                     "*_n"
--      clock signals:                          "Clk", "clk_div#", "clk_#x" 
--      reset signals:                          "Rst", "rst_n" 
--      generics:                               "C_*" 
--      user defined types:                     "*_TYPE" 
--      state machine next state:               "*_ns" 
--      state machine current state:            "*_cs" 
--      combinatorial signals:                  "*_com" 
--      pipelined or register delay signals:    "*_d#" 
--      counter signals:                        "*cnt*"
--      clock enable signals:                   "*_ce" 
--      internal version of output port         "*_i"
--      device pins:                            "*_pin" 
--      ports:                                  - Names begin with Uppercase 
--      processes:                              "*_PROCESS" 
--      component instantiations:               "<ENTITY_>I_<#|FUNC>
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;

-------------------------------------------------------------------------------
-- Definition of Ports:
--
--  Clk      -- System Clock
--  Rst      -- System Reset
--  Clke     -- Clock enable
--  Din      -- Data in 
--  Load     -- Data load
--  Shift    -- Data shift enable
--  Dout     -- data out
-------------------------------------------------------------------------------
-- ENTITY
-------------------------------------------------------------------------------
entity crcnibshiftreg is

  port (
    Clk   : in  std_logic;
    Rst   : in  std_logic;
    Clken : in  std_logic;
    Din   : in  std_logic_vector(31 downto 0);
    Load  : in  std_logic;
    Shift : in  std_logic; 
    Dout  : out std_logic_vector(31 downto 0)
    );
end crcnibshiftreg;

architecture implementation of crcnibshiftreg is

attribute DowngradeIPIdentifiedWarnings: string;

attribute DowngradeIPIdentifiedWarnings of implementation : architecture is "yes";

-------------------------------------------------------------------------------
--  Signal and Type Declarations
-------------------------------------------------------------------------------
signal nibData  : std_logic_vector (31 downto 0);
  
begin

   ----------------------------------------------------------------------------
   -- PROCESS : SHIFTER 
   ----------------------------------------------------------------------------
   -- The process shifts the nibble data when shift is enabled.
   ----------------------------------------------------------------------------
   SHIFTER : process (Clk)
   begin  --
  
      if (Clk'event and Clk = '1') then
         if Rst = '1' then
            nibData <= (others => '0');        
         elsif (Clken = '1') then
            if (Load = '1') then
               nibData <= Din;
            elsif (Shift = '1') then
               nibData(3 downto 0)   <= nibData(7 downto 4);
               nibData(7 downto 4)   <= nibData(11 downto 8);
               nibData(11 downto 8)  <= nibData(15 downto 12);
               nibData(15 downto 12) <= nibData(19 downto 16);
               nibData(19 downto 16) <= nibData(23 downto 20);
               nibData(23 downto 20) <= nibData(27 downto 24);
               nibData(27 downto 24) <= nibData(31 downto 28);
               nibData(31 downto 28) <= (others => '0');
            end if;
         end if;
      end if;
   end process SHIFTER;
   
   Dout <= nibData;
  
end implementation;
  


-------------------------------------------------------------------------------
-- cntr5bit - entity/architecture pair
-------------------------------------------------------------------------------
--  ***************************************************************************
--  ** DISCLAIMER OF LIABILITY                                               **
--  **                                                                       **
--  **  This file contains proprietary and confidential information of       **
--  **  Xilinx, Inc. ("Xilinx"), that is distributed under a license         **
--  **  from Xilinx, and may be used, copied and/or disclosed only           **
--  **  pursuant to the terms of a valid license agreement with Xilinx.      **
--  **                                                                       **
--  **  XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION                **
--  **  ("MATERIALS") "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER           **
--  **  EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING WITHOUT                  **
--  **  LIMITATION, ANY WARRANTY WITH RESPECT TO NONINFRINGEMENT,            **
--  **  MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE. Xilinx        **
--  **  does not warrant that functions included in the Materials will       **
--  **  meet the requirements of Licensee, or that the operation of the      **
--  **  Materials will be uninterrupted or error-free, or that defects       **
--  **  in the Materials will be corrected. Furthermore, Xilinx does         **
--  **  not warrant or make any representations regarding use, or the        **
--  **  results of the use, of the Materials in terms of correctness,        **
--  **  accuracy, reliability or otherwise.                                  **
--  **                                                                       **
--  **  Xilinx products are not designed or intended to be fail-safe,        **
--  **  or for use in any application requiring fail-safe performance,       **
--  **  such as life-support or safety devices or systems, Class III         **
--  **  medical devices, nuclear facilities, applications related to         **
--  **  the deployment of airbags, or any other applications that could      **
--  **  lead to death, personal injury or severe property or                 **
--  **  environmental damage (individually and collectively, "critical       **
--  **  applications"). Customer assumes the sole risk and liability         **
--  **  of any use of Xilinx products in critical applications,              **
--  **  subject only to applicable laws and regulations governing            **
--  **  limitations on product liability.                                    **
--  **                                                                       **
--  **  Copyright 2010 Xilinx, Inc.                                          **
--  **  All rights reserved.                                                 **
--  **                                                                       **
--  **  This disclaimer and copyright notice must be retained as part        **
--  **  of this file at all times.                                           **
--  ***************************************************************************
--
-------------------------------------------------------------------------------
-- Filename     : cntr5bit.vhd
-- Version      : v2.0
-- 
-- Description  : This file contains the a 5 bit resetable, loadable
--                down counter by 1.
-- VHDL-Standard:   VHDL'93
-------------------------------------------------------------------------------
-- Structure:   
--
--  axi_ethernetlite.vhd
--      \
--      \-- axi_interface.vhd
--      \-- xemac.vhd
--           \
--           \-- mdio_if.vhd
--           \-- emac_dpram.vhd                     
--           \    \                     
--           \    \-- RAMB16_S4_S36
--           \                          
--           \    
--           \-- emac.vhd                     
--                \                     
--                \-- MacAddrRAM                   
--                \-- receive.vhd
--                \      rx_statemachine.vhd
--                \      rx_intrfce.vhd
--                \         async_fifo_fg.vhd
--                \      crcgenrx.vhd
--                \                     
--                \-- transmit.vhd
--                       crcgentx.vhd
--                          crcnibshiftreg
--                       tx_intrfce.vhd
--                          async_fifo_fg.vhd
--                       tx_statemachine.vhd
--                       deferral.vhd
--                          cntr5bit.vhd
--                          defer_state.vhd
--                       bocntr.vhd
--                          lfsr16.vhd
--                       msh_cnt.vhd
--                       ld_arith_reg.vhd
--
-------------------------------------------------------------------------------
-- Author:    PVK
-- History:    
-- PVK        06/07/2010     First Version
-- ^^^^^^
-- First version.  
-- ~~~~~~
-------------------------------------------------------------------------------
-- Naming Conventions:
--      active low signals:                     "*_n"
--      clock signals:                          "Clk", "clk_div#", "clk_#x" 
--      reset signals:                          "Rst", "rst_n" 
--      generics:                               "C_*" 
--      user defined types:                     "*_TYPE" 
--      state machine next state:               "*_ns" 
--      state machine current state:            "*_cs" 
--      combinatorial signals:                  "*_com" 
--      pipelined or register delay signals:    "*_d#" 
--      counter signals:                        "*cnt*"
--      clock enable signals:                   "*_ce" 
--      internal version of output port         "*_i"
--      device pins:                            "*_pin" 
--      ports:                                  - Names begin with Uppercase 
--      processes:                              "*_PROCESS" 
--      component instantiations:               "<ENTITY_>I_<#|FUNC>
-------------------------------------------------------------------------------
-- 
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;

-------------------------------------------------------------------------------
-- axi_ethernetlite_v3_0_18 library is used for axi_ethernetlite_v3_0_18 
-- component declarations
-------------------------------------------------------------------------------
library axi_ethernetlite_v3_0_18;
use axi_ethernetlite_v3_0_18.mac_pkg.all;

-------------------------------------------------------------------------------
-- Port Declaration
-------------------------------------------------------------------------------
-- Definition of Ports:
--
--  Clk       -- System Clock
--  Rst       -- System Reset
--  Cntout    -- Counter output
--  En        -- Counter enable 
--  Ld        -- Counter load enable
--  Load_in   -- Counter load data
--  Zero      -- Terminal count
-------------------------------------------------------------------------------
-- ENTITY
-------------------------------------------------------------------------------
entity cntr5bit is

  port (
    Clk     : in  std_logic;                  -- input clock
    Rst     : in  std_logic;                  -- reset counter
    Cntout  : out std_logic_vector (0 to 4);
    En      : in  std_logic;                  -- counter down enable by 1
    Ld      : in  std_logic;                  -- load enable
    Load_in : in  std_logic_vector (0 to 4);  -- load input value
    Zero    : out std_logic                  -- terminal count
  );
end cntr5bit;

architecture implementation of cntr5bit is

attribute DowngradeIPIdentifiedWarnings: string;

attribute DowngradeIPIdentifiedWarnings of implementation : architecture is "yes";

-------------------------------------------------------------------------------
--  Constant Declarations
-------------------------------------------------------------------------------
-- Constants used in this design are found in mac_pkg.vhd
-------------------------------------------------------------------------------
-- Signal and Type Declarations
-------------------------------------------------------------------------------
signal count : std_logic_vector(0 to 4);
signal zero_i : std_logic;

begin
Cntout <= count;
 -------------------------------------------------------------------------------
 -- INT_count_PROCESS
 -------------------------------------------------------------------------------
 -- This process assigns the internal control register signals to the out port
 -------------------------------------------------------------------------------
  INT_COUNT_PROCESS1: process (Clk)
  begin
    if (Clk'event and Clk = '1') then
      if (Rst = RESET_ACTIVE) then
        count <= (others => '1');
      elsif (Ld = '1') then
        count <= Load_in;
      elsif (En = '1' and zero_i = '0') then
          count <= count - 1;
      else
          null;
      end if;
    end if;
  end process INT_COUNT_PROCESS1;    
  
  INT_COUNT_PROCESS2: process (Clk)
  begin

    if (Clk'event and Clk = '1') then
      if (Rst = RESET_ACTIVE) then
        zero_i <= '1';
      else
        if (count = "00001") then
        zero_i <= '1';
        else
        zero_i <= '0';
        end if;
      end if;
    end if;
  end process INT_COUNT_PROCESS2;
  
  Zero <= zero_i;

end implementation;


-------------------------------------------------------------------------------
-- tx_statemachine - entity/architecture pair
-------------------------------------------------------------------------------
--  ***************************************************************************
--  ** DISCLAIMER OF LIABILITY                                               **
--  **                                                                       **
--  **  This file contains proprietary and confidential information of       **
--  **  Xilinx, Inc. ("Xilinx"), that is distributed under a license         **
--  **  from Xilinx, and may be used, copied and/or disclosed only           **
--  **  pursuant to the terms of a valid license agreement with Xilinx.      **
--  **                                                                       **
--  **  XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION                **
--  **  ("MATERIALS") "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER           **
--  **  EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING WITHOUT                  **
--  **  LIMITATION, ANY WARRANTY WITH RESPECT TO NONINFRINGEMENT,            **
--  **  MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE. Xilinx        **
--  **  does not warrant that functions included in the Materials will       **
--  **  meet the requirements of Licensee, or that the operation of the      **
--  **  Materials will be uninterrupted or error-free, or that defects       **
--  **  in the Materials will be corrected. Furthermore, Xilinx does         **
--  **  not warrant or make any representations regarding use, or the        **
--  **  results of the use, of the Materials in terms of correctness,        **
--  **  accuracy, reliability or otherwise.                                  **
--  **                                                                       **
--  **  Xilinx products are not designed or intended to be fail-safe,        **
--  **  or for use in any application requiring fail-safe performance,       **
--  **  such as life-support or safety devices or systems, Class III         **
--  **  medical devices, nuclear facilities, applications related to         **
--  **  the deployment of airbags, or any other applications that could      **
--  **  lead to death, personal injury or severe property or                 **
--  **  environmental damage (individually and collectively, "critical       **
--  **  applications"). Customer assumes the sole risk and liability         **
--  **  of any use of Xilinx products in critical applications,              **
--  **  subject only to applicable laws and regulations governing            **
--  **  limitations on product liability.                                    **
--  **                                                                       **
--  **  Copyright 2010 Xilinx, Inc.                                          **
--  **  All rights reserved.                                                 **
--  **                                                                       **
--  **  This disclaimer and copyright notice must be retained as part        **
--  **  of this file at all times.                                           **
--  ***************************************************************************
--
-------------------------------------------------------------------------------
-- Filename     : tx_statemachine.vhd
-- Version      : v2.0
-- Description  : This file contains the transmit control state machine.
-- VHDL-Standard: VHDL'93
-------------------------------------------------------------------------------
-- Structure:   
--
--  axi_ethernetlite.vhd
--      \
--      \-- axi_interface.vhd
--      \-- xemac.vhd
--           \
--           \-- mdio_if.vhd
--           \-- emac_dpram.vhd                     
--           \    \                     
--           \    \-- RAMB16_S4_S36
--           \                          
--           \    
--           \-- emac.vhd                     
--                \                     
--                \-- MacAddrRAM                   
--                \-- receive.vhd
--                \      rx_statemachine.vhd
--                \      rx_intrfce.vhd
--                \         async_fifo_fg.vhd
--                \      crcgenrx.vhd
--                \                     
--                \-- transmit.vhd
--                       crcgentx.vhd
--                          crcnibshiftreg
--                       tx_intrfce.vhd
--                          async_fifo_fg.vhd
--                       tx_statemachine.vhd
--                       deferral.vhd
--                          cntr5bit.vhd
--                          defer_state.vhd
--                       bocntr.vhd
--                          lfsr16.vhd
--                       msh_cnt.vhd
--                       ld_arith_reg.vhd
--
-------------------------------------------------------------------------------
-- Author:    PVK
-- History:    
-- PVK        06/07/2010     First Version
-- ^^^^^^
-- First version.  
-- ~~~~~~
-------------------------------------------------------------------------------
-- Naming Conventions:
--      active low signals:                     "*_n"
--      clock signals:                          "Clk", "clk_div#", "clk_#x" 
--      reset signals:                          "Rst", "rst_n" 
--      generics:                               "C_*" 
--      user defined types:                     "*_TYPE" 
--      state machine next state:               "*_ns" 
--      state machine current state:            "*_cs" 
--      combinatorial signals:                  "*_com" 
--      pipelined or register delay signals:    "*_d#" 
--      counter signals:                        "*cnt*"
--      clock enable signals:                   "*_ce" 
--      internal version of output port         "*_i"
--      device pins:                            "*_pin" 
--      ports:                                  - Names begin with Uppercase 
--      processes:                              "*_PROCESS" 
--      component instantiations:               "<ENTITY_>I_<#|FUNC>
-------------------------------------------------------------------------------
-- 
library ieee;
use ieee.std_logic_1164.all;

-------------------------------------------------------------------------------
-- axi_ethernetlite_v3_0_18 library is used for axi_ethernetlite_v3_0_18 
-- component declarations
-------------------------------------------------------------------------------
library axi_ethernetlite_v3_0_18;
use axi_ethernetlite_v3_0_18.all;

-- synopsys translate_off
-- Library XilinxCoreLib;
--library simprim;
-- synopsys translate_on

-------------------------------------------------------------------------------
-- Vcomponents from unisim library is used for FIFO instatiation
-- function declarations
-------------------------------------------------------------------------------
library unisim;
use unisim.Vcomponents.all;

-------------------------------------------------------------------------------
-- Definition of Generics:
-------------------------------------------------------------------------------
--  C_DUPLEX               -- 1 = full duplex, 0 = half duplex
-------------------------------------------------------------------------------
-- Definition of Ports:
--
--  Clk                   -- System Clock
--  Rst                   -- System Reset
--  TxClkEn               -- Transmit clocl enable
--  Jam_rst               -- Jam reset
--  TxRst                 -- Transmit reset
--  Deferring             -- Deffering 
--  ColRetryCnt           -- Collision retry coun
--  ColWindowNibCnt       -- Collision window nibble count
--  JamTxNibCnt           -- TX Jam nibble count
--  TxNibbleCnt           -- TX Nibble count
--  BusFifoWrNibbleCnt    -- Bus FIFO write nibble count
--  CrcCnt                -- CRC count
--  BusFifoFull           -- Bus FIFO full
--  BusFifoEmpty          -- Bus FIFO empty
--  PhyCollision          -- Phy collision
--  Tx_pong_ping_l        -- TX Ping/Pong buffer enable
--  InitBackoff           -- Initialize back off
--  TxRetryRst            -- TX retry reset
--  TxExcessDefrlRst      -- TX excess defer reset
--  TxLateColnRst         -- TX late collision reset
--  TxColRetryCntRst_n    -- TX collision retry counter reset
--  TxColRetryCntEnbl     -- TX collision retry counter enable
--  TxNibbleCntRst        -- TX nibble counter reset
--  TxEnNibbleCnt         -- TX nibble count
--  TxNibbleCntLd         -- TX nibble counter load
--  BusFifoWrCntRst       -- Bus FIFO write counter reset
--  BusFifoWrCntEn        -- Bus FIFO write counter enable
--  EnblPre               -- Enable Preamble
--  EnblSFD               -- Enable SFD
--  EnblData              -- Enable Data  
--  EnblJam               -- Enable Jam
--  EnblCRC               -- Enable CRC 
--  BusFifoWr             -- Bus FIFO write enable
--  Phytx_en              -- PHY transmit enable
--  TxCrcEn               -- TX CRC enable
--  TxCrcShftOutEn        -- TX CRC shift out enable
--  Tx_addr_en            -- TX buffer address enable  
--  Tx_start              -- Trasnmit start
--  Tx_done               -- Transmit done
--  Tx_idle               -- Transmit idle
--  Tx_DPM_ce             -- TX buffer chip enable
--  Tx_DPM_wr_data        -- TX buffer write data
--  Tx_DPM_wr_rd_n        -- TX buffer write/read enable
--  Enblclear             -- Enable clear 
--  Transmit_start        -- Transmit start
--  Mac_program_start     -- MAC Program start
--  Mac_addr_ram_we       -- MAC Address RAM write enable
--  Mac_addr_ram_addr_wr  -- MAC Address RAM write address
--  Pre_sfd_done          -- Pre SFD done
-------------------------------------------------------------------------------
-- ENTITY
-------------------------------------------------------------------------------
entity tx_statemachine is
  generic 
     (
       C_DUPLEX : integer := 1
       -- 1 = full duplex, 0 = half duplex   
     );
  port 
    (
    Clk                  : in  std_logic;
    Rst                  : in  std_logic;
    TxClkEn              : in  std_logic; 
    Jam_rst              : out std_logic;
    TxRst                : in  std_logic;
    Deferring            : in  std_logic;        
    ColRetryCnt          : in  std_logic_vector (0 to 4);
    ColWindowNibCnt      : in  std_logic_vector (0 to 7);
    JamTxNibCnt          : in  std_logic_vector (0 to 3);
    TxNibbleCnt          : in  std_logic_vector (0 to 11);
    BusFifoWrNibbleCnt   : in  std_logic_vector (0 to 11);
    CrcCnt               : in  std_logic_vector (0 to 3);
    BusFifoFull          : in  std_logic;
    BusFifoEmpty         : in  std_logic;
    PhyCollision         : in  std_logic;
    Tx_pong_ping_l       : in  std_logic;
    InitBackoff          : out std_logic;
    TxRetryRst           : out std_logic;
    TxExcessDefrlRst     : out std_logic;
    TxLateColnRst        : out std_logic;          
    TxColRetryCntRst_n   : out std_logic;
    TxColRetryCntEnbl    : out std_logic;
    TxNibbleCntRst       : out std_logic;
    TxEnNibbleCnt        : out std_logic;
    TxNibbleCntLd        : out std_logic;
    BusFifoWrCntRst      : out std_logic;
    BusFifoWrCntEn       : out std_logic;
    EnblPre              : out std_logic;
    EnblSFD              : out std_logic;
    EnblData             : out std_logic;
    EnblJam              : out std_logic;
    EnblCRC              : out std_logic;
    BusFifoWr            : out std_logic;
    Phytx_en             : out std_logic;
    TxCrcEn              : out std_logic;
    TxCrcShftOutEn       : out std_logic;
    Tx_addr_en           : out std_logic;
    Tx_start             : out std_logic;
    Tx_done              : out std_logic;
    Tx_idle              : out std_logic;
    Tx_DPM_ce            : out std_logic;
    Tx_DPM_wr_data       : out std_logic_vector (0 to 3);
    Tx_DPM_wr_rd_n       : out std_logic;
    Enblclear            : out std_logic;
    Transmit_start       : in  std_logic;
    Mac_program_start    : in  std_logic;
    Mac_addr_ram_we      : out std_logic;
    Mac_addr_ram_addr_wr : out std_logic_vector(0 to 3);      
    Pre_sfd_done         : out std_logic
    );
 
end tx_statemachine;

-------------------------------------------------------------------------------
-- Definition of Generics:
--          No Generics were used for this Entity.
--
-- Definition of Ports:
--         
-------------------------------------------------------------------------------

architecture implementation of tx_statemachine is

attribute DowngradeIPIdentifiedWarnings: string;

attribute DowngradeIPIdentifiedWarnings of implementation : architecture is "yes";

-------------------------------------------------------------------------------
--  Constant Declarations
-------------------------------------------------------------------------------
-- Constants used in this design are found in mac_pkg.vhd
-------------------------------------------------------------------------------
-- Signal and Type Declarations
-------------------------------------------------------------------------------
signal idle                         : std_logic; -- state  0
signal lngthDelay1                  : std_logic; -- state  5
signal lngthDelay2                  : std_logic; -- state  6
signal ldLngthCntr                  : std_logic; -- state  7
signal preamble                     : std_logic; -- state  8
signal checkBusFifoFullSFD          : std_logic; -- state  9
signal SFD                          : std_logic; -- state 10
signal checkBusFifoFull             : std_logic; -- state 11
signal loadBusFifo                  : std_logic; -- state 12
signal checkCrc                     : std_logic; -- state 13  
signal checkBusFifoFullCrc          : std_logic; -- state 14
signal loadBusFifoCrc               : std_logic; -- state 15
signal waitFifoEmpty                : std_logic; -- state 16
signal txDone                       : std_logic; -- state 17
signal checkBusFifoFullJam          : std_logic; -- state 18
signal loadBusFifoJam               : std_logic; -- state 19
signal half_dup_error               : std_logic; -- state 20
signal collisionRetry               : std_logic; -- state 21
signal retryWaitFifoEmpty           : std_logic; -- state 22
signal retryReset                   : std_logic; -- state 23
signal txDone2                      : std_logic; -- state 24
signal txDonePause                  : std_logic; -- state 25
signal chgMacAdr1                   : std_logic; -- state 26
signal chgMacAdr2                   : std_logic; -- state 27
signal chgMacAdr3                   : std_logic; -- state 28
signal chgMacAdr4                   : std_logic; -- state 29
signal chgMacAdr5                   : std_logic; -- state 30
signal chgMacAdr6                   : std_logic; -- state 31
signal chgMacAdr7                   : std_logic; -- state 32
signal chgMacAdr8                   : std_logic; -- state 33
signal chgMacAdr9                   : std_logic; -- state 34
signal chgMacAdr10                  : std_logic; -- state 35
signal chgMacAdr11                  : std_logic; -- state 36
signal chgMacAdr12                  : std_logic; -- state 37
signal chgMacAdr13                  : std_logic; -- state 38
signal chgMacAdr14                  : std_logic; -- state 39
signal idle_D                       : std_logic; -- state  0
signal txLngthRdNib1_D              : std_logic; -- state  1
signal lngthDelay1_D                : std_logic; -- state  5
signal lngthDelay2_D                : std_logic; -- state  6
signal ldLngthCntr_D                : std_logic; -- state  7
signal preamble_D                   : std_logic; -- state  8
signal checkBusFifoFullSFD_D        : std_logic; -- state  9
signal SFD_D                        : std_logic; -- state 10
signal checkBusFifoFull_D           : std_logic; -- state 11
signal loadBusFifo_D                : std_logic; -- state 12
signal checkCrc_D                   : std_logic; -- state 13  
signal checkBusFifoFullCrc_D        : std_logic; -- state 14
signal loadBusFifoCrc_D             : std_logic; -- state 15
signal waitFifoEmpty_D              : std_logic; -- state 16
signal txDone_D                     : std_logic; -- state 17
signal checkBusFifoFullJam_D        : std_logic; -- state 18
signal loadBusFifoJam_D             : std_logic; -- state 19
signal half_dup_error_D             : std_logic; -- state 20
signal collisionRetry_D             : std_logic; -- state 21
signal retryWaitFifoEmpty_D         : std_logic; -- state 22
signal retryReset_D                 : std_logic; -- state 23
signal txDone2_D                    : std_logic; -- state 24
signal txDonePause_D                : std_logic; -- state 25
signal chgMacAdr1_D                 : std_logic; -- state 26
signal chgMacAdr2_D                 : std_logic; -- state 27
signal chgMacAdr3_D                 : std_logic; -- state 28
signal chgMacAdr4_D                 : std_logic; -- state 29
signal chgMacAdr5_D                 : std_logic; -- state 30
signal chgMacAdr6_D                 : std_logic; -- state 31
signal chgMacAdr7_D                 : std_logic; -- state 32
signal chgMacAdr8_D                 : std_logic; -- state 33
signal chgMacAdr9_D                 : std_logic; -- state 34
signal chgMacAdr10_D                : std_logic; -- state 35
signal chgMacAdr11_D                : std_logic; -- state 36
signal chgMacAdr12_D                : std_logic; -- state 37
signal chgMacAdr13_D                : std_logic; -- state 38
signal chgMacAdr14_D                : std_logic; -- state 39
signal txNibbleCntRst_i             : std_logic; 
signal txEnNibbleCnt_i              : std_logic;
signal txNibbleCntLd_i              : std_logic;
signal busFifoWr_i                  : std_logic;
signal phytx_en_i                   : std_logic;
signal phytx_en_i_n                 : std_logic;
signal txCrcEn_i                    : std_logic;
signal retrying_i                   : std_logic; 
signal phytx_en_reg                 : std_logic;
signal busFifoWrCntRst_reg          : std_logic;
signal retrying_reg                 : std_logic;
signal txCrcEn_reg                  : std_logic;
signal busFifoWrCntRst_i            : std_logic;
signal state_machine_rst            : std_logic;
signal full_half_n                  : std_logic;
signal goto_idle                    : std_logic; -- state  0
signal stay_idle                    : std_logic; -- state  0
signal goto_txLngthRdNib1_1         : std_logic; -- state  1
signal goto_txLngthRdNib1_2         : std_logic; -- state  1
signal goto_lngthDelay1             : std_logic; -- state  5
signal goto_lngthDelay2             : std_logic; -- state  6
signal goto_ldLngthCntr             : std_logic; -- state  7
signal stay_ldLngthCntr             : std_logic; -- state  7
signal goto_preamble                : std_logic; -- state  8
signal stay_preamble                : std_logic; -- state  8
signal goto_checkBusFifoFullSFD     : std_logic; -- state  9 
signal stay_checkBusFifoFullSFD     : std_logic; -- state  9
signal goto_SFD                     : std_logic; -- state 10 
signal stay_SFD                     : std_logic; -- state 10 
signal goto_checkBusFifoFull_1      : std_logic; -- state 11 
signal goto_checkBusFifoFull_2      : std_logic; -- state 11
signal stay_checkBusFifoFull        : std_logic; -- state 11
signal goto_loadBusFifo             : std_logic; -- state 12
signal goto_checkCrc                : std_logic; -- state 13 
signal goto_checkBusFifoFullCrc_1   : std_logic; -- state 14
signal goto_checkBusFifoFullCrc_2   : std_logic; -- state 14
signal stay_checkBusFifoFullCrc     : std_logic; -- state 14
signal goto_loadBusFifoCrc_1        : std_logic; -- state 15
signal goto_waitFifoEmpty_2         : std_logic; -- state 16
signal stay_waitFifoEmpty           : std_logic; -- state 16
signal goto_txDone_1                : std_logic; -- state 17
signal goto_txDone_2                : std_logic; -- state 17
signal goto_checkBusFifoFullJam_1   : std_logic; -- state 18
signal goto_checkBusFifoFullJam_2   : std_logic; -- state 18
signal stay_checkBusFifoFullJam     : std_logic; -- state 18
signal goto_loadBusFifoJam          : std_logic; -- state 19
signal goto_half_dup_error_1        : std_logic; -- state 20
signal goto_half_dup_error_2        : std_logic; -- state 20
signal goto_collisionRetry          : std_logic; -- state 21
signal goto_retryWaitFifoEmpty      : std_logic; -- state 22
signal stay_retryWaitFifoEmpty      : std_logic; -- state 22
signal goto_retryReset              : std_logic; -- state 23
signal goto_txDone2                 : std_logic; -- state 24
signal goto_txDonePause             : std_logic; -- state 25
signal goto_chgMacAdr1              : std_logic; -- state 26
signal goto_chgMacAdr2              : std_logic; -- state 27
signal goto_chgMacAdr3              : std_logic; -- state 28
signal goto_chgMacAdr4              : std_logic; -- state 29
signal goto_chgMacAdr5              : std_logic; -- state 30
signal goto_chgMacAdr6              : std_logic; -- state 31
signal goto_chgMacAdr7              : std_logic; -- state 32
signal goto_chgMacAdr8              : std_logic; -- state 33
signal goto_chgMacAdr9              : std_logic; -- state 34
signal goto_chgMacAdr10             : std_logic; -- state 35
signal goto_chgMacAdr11             : std_logic; -- state 36
signal goto_chgMacAdr12             : std_logic; -- state 37
signal goto_chgMacAdr13             : std_logic; -- state 38
signal goto_chgMacAdr14             : std_logic; -- state 39
signal txNibbleCnt_is_1             : std_logic;
signal busFifoWrNibbleCnt_is_14     : std_logic;
signal busFifoWrNibbleCnt_not_14    : std_logic;   
signal busFifoWrNibbleCnt_is_15     : std_logic;
signal busFifoWrNibbleCnt_not_15    : std_logic;
signal crcCnt_not_0                 : std_logic;
signal crcCnt_is_0                  : std_logic;
signal jamTxNibCnt_not_0            : std_logic;
signal jamTxNibCnt_is_0             : std_logic;
signal colWindowNibCnt_not_0        : std_logic;
signal colWindowNibCnt_is_0         : std_logic;
signal colRetryCnt_is_15            : std_logic;
signal pre_SFD_zero                 : std_logic;
signal waitdone_pre_sfd             : std_logic;
signal transmit_start_reg           : std_logic;
signal mac_program_start_reg        : std_logic;
  

-------------------------------------------------------------------------------
-- Component Declarations
-------------------------------------------------------------------------------
-- The following components are the building blocks of the tx state machine

component FDR
  port 
   (
    Q : out std_logic;
    C : in std_logic;
    D : in std_logic;
    R : in std_logic
    );
end component;

component FDS
  port 
   (
    Q : out std_logic;
    C : in std_logic;
    D : in std_logic;
    S : in std_logic
    );
end component;

component FDRE
  port
   (
    Q  : out std_logic;
    C  : in std_logic;
    CE : in std_logic;
    D  : in std_logic;
    R  : in std_logic
    );
end component;

begin

   Tx_DPM_wr_data <= (others => '0'); 
   -- Trnasmit Done indicator
   -- added txDone for ping pong control
   Tx_done          <= txDone and not retrying_reg; 
                        
   
   -- Full/Half duplex indicator
   full_half_n      <= '1'when C_DUPLEX = 1 else '0'; 
   
   -- Wait for Pre SFD
   --waitdone_pre_sfd <= PhyCollision and not(full_half_n) and not(pre_sfd_zero);
   Pre_sfd_done     <= pre_SFD_zero;
   
   -- PHY tx enable
   phytx_en_i_n     <= not(phytx_en_i);


   ----------------------------------------------------------------------------
   -- Signal Assignment
   ----------------------------------------------------------------------------
   TxNibbleCntRst      <= txNibbleCntRst_i;       
   TxEnNibbleCnt       <= txEnNibbleCnt_i;
   TxNibbleCntLd       <= txNibbleCntLd_i;
   
   BusFifoWr           <= busFifoWr_i;
   Phytx_en            <= phytx_en_i;
   TxCrcEn             <= txCrcEn_i;

   BusFifoWrCntRst     <= busFifoWrCntRst_i;
                  
   ----------------------------------------------------------------------------
   -- Pre SFD Counter
   ----------------------------------------------------------------------------
   PRE_SFD_count: entity axi_ethernetlite_v3_0_18.cntr5bit
     port map
            (
             cntout  =>  open,
             Clk     =>  Clk, 
             Rst     =>  Rst,
             en      =>  TxClkEn,
             ld      =>  phytx_en_i_n,
             load_in =>  "10011",
             zero    =>  pre_SFD_zero
             );  
   
   -- State machine reset
   state_machine_rst <= Rst;
   
   ----------------------------------------------------------------------------
   -- Counter enable generation
   ----------------------------------------------------------------------------
   -- Transmit Nibble Counte=1
   txNibbleCnt_is_1 <= not(TxNibbleCnt(0)) and not(TxNibbleCnt(1)) and 
                       not(TxNibbleCnt(2)) and not(TxNibbleCnt(3)) and 
                       not(TxNibbleCnt(4)) and not(TxNibbleCnt(5)) and
                       not(TxNibbleCnt(6)) and not(TxNibbleCnt(7)) and 
                       not(TxNibbleCnt(8)) and not(TxNibbleCnt(9)) and
                       not(TxNibbleCnt(10))and TxNibbleCnt(11);
   
   -- Bus FIFO write Nibble Counte=14
   busFifoWrNibbleCnt_is_14 <= BusFifoWrNibbleCnt(8) and 
                               BusFifoWrNibbleCnt(9) and 
                               BusFifoWrNibbleCnt(10) and 
                               not(BusFifoWrNibbleCnt(11));
                               
   -- Bus FIFO write Nibble Counte/=14
   busFifoWrNibbleCnt_not_14 <= not(busFifoWrNibbleCnt_is_14);
     
   -- Bus FIFO write Nibble Counte=15
   busFifoWrNibbleCnt_is_15 <= (BusFifoWrNibbleCnt(8)  and 
                                BusFifoWrNibbleCnt(9)  and 
                                BusFifoWrNibbleCnt(10) and 
                                BusFifoWrNibbleCnt(11));
                                
   -- Bus FIFO write Nibble Counte/=15
   busFifoWrNibbleCnt_not_15 <= not(busFifoWrNibbleCnt_is_15);
   
   -- CRC Count/=0
   crcCnt_not_0 <= CrcCnt(0) or CrcCnt(1) or CrcCnt(2) or CrcCnt(3);      
   
   -- CRC Count=0
   crcCnt_is_0  <= not crcCnt_not_0;  
   
   -- Jam Transmit Nibble count/=0
   jamTxNibCnt_not_0 <= JamTxNibCnt(0) or JamTxNibCnt(1) or JamTxNibCnt(2) or 
                        JamTxNibCnt(3);  
   
   -- Jam Transmit Nibble count=0
   jamTxNibCnt_is_0  <= not(jamTxNibCnt_not_0);
   
   -- Collision windo Nibble count/=0
   colWindowNibCnt_not_0 <= ColWindowNibCnt(0) or ColWindowNibCnt(1) or 
                            ColWindowNibCnt(2) or ColWindowNibCnt(3) or 
                            ColWindowNibCnt(4) or ColWindowNibCnt(5) or 
                            ColWindowNibCnt(6) or ColWindowNibCnt(7); 
                            
   -- Collision windo Nibble count=0
   colWindowNibCnt_is_0 <= not(colWindowNibCnt_not_0);
   
   -- Collision retry count=15
   colRetryCnt_is_15    <= not(ColRetryCnt(0)) and ColRetryCnt(1) and 
                                ColRetryCnt(2) and ColRetryCnt(3) and 
                                ColRetryCnt(4);    

   ----------------------------------------------------------------------------
   -- idle state
   ----------------------------------------------------------------------------    
   goto_idle <= txDonePause;
   

   stay_idle <= idle and not(Transmit_start) and not Mac_program_start; 
   
   
   
   idle_D <= goto_idle or stay_idle;
   
   ----------------------------------------------------------------------------
   -- idle state
   ----------------------------------------------------------------------------    
   STATE0A: FDS
     port map 
       (
       Q => idle,             --[out]
       C => Clk,              --[in]
       D => idle_D,           --[in]
       S => state_machine_rst --[in]
      );
     
   Tx_idle <= idle;  
   
   ----------------------------------------------------------------------------
   -- txLngthRdNib1 state
   ----------------------------------------------------------------------------
   --goto_txLngthRdNib1_1 <= idle and Transmit_start and not transmit_start_reg;

   goto_txLngthRdNib1_1 <= idle and 
                           ((transmit_start and not transmit_start_reg)
                            or 
                            (transmit_start and retrying_reg));
   
   goto_txLngthRdNib1_2 <= retryReset;
   
   txLngthRdNib1_D      <= goto_txLngthRdNib1_1 or goto_txLngthRdNib1_2;
     
     
   goto_lngthDelay1 <= txLngthRdNib1_D;   
   ----------------------------------------------------------------------------
   -- lngthDelay1 state
   ----------------------------------------------------------------------------    
   lngthDelay1_D    <= goto_lngthDelay1;
     
   STATE5A: FDR
     port map 
      (
       Q => lngthDelay1,      --[out]
       C => Clk,              --[in]
       D => lngthDelay1_D,    --[in]
       R => state_machine_rst --[in]
      );    
   ----------------------------------------------------------------------------
   -- lngthDelay2 state
   ----------------------------------------------------------------------------    
   goto_lngthDelay2 <= lngthDelay1;
   
   lngthDelay2_D    <= goto_lngthDelay2;
     
   STATE6A: FDR
     port map 
      (
       Q => lngthDelay2,      --[out]
       C => Clk,              --[in]
       D => lngthDelay2_D,    --[in]
       R => state_machine_rst --[in]
      );    
           
   ----------------------------------------------------------------------------
   -- ldLngthCntr state
   ----------------------------------------------------------------------------    
   goto_ldLngthCntr <= lngthDelay1;
   
   stay_ldLngthCntr <= ldLngthCntr and Deferring;
   
   ldLngthCntr_D    <= goto_ldLngthCntr or stay_ldLngthCntr;
     
   STATE7A: FDR
     port map 
      (
       Q => ldLngthCntr,      --[out]
       C => Clk,              --[in]
       D => ldLngthCntr_D,    --[in]
       R => state_machine_rst --[in]
      );    
     
   ----------------------------------------------------------------------------
   -- preamble state
   ----------------------------------------------------------------------------      
   goto_preamble <= (ldLngthCntr and (not(Deferring)));
     
   stay_preamble <= preamble and busFifoWrNibbleCnt_not_14;
   
   preamble_D    <= goto_preamble or stay_preamble;
   
   STATE8A: FDR
     port map 
      (
       Q => preamble,         --[out]
       C => Clk,              --[in]
       D => preamble_D,       --[in]
       R => state_machine_rst --[in]
      );  
   ----------------------------------------------------------------------------
   -- checkBusFifoFullSFD state
   ----------------------------------------------------------------------------      
   goto_checkBusFifoFullSFD <= preamble and busFifoWrNibbleCnt_is_14;
   
   stay_checkBusFifoFullSFD <= checkBusFifoFullSFD and BusFifoFull;
   
   checkBusFifoFullSFD_D    <= goto_checkBusFifoFullSFD or
                               stay_checkBusFifoFullSFD;
   STATE9A: FDR
     port map 
      (
       Q => checkBusFifoFullSFD,   --[out]
       C => Clk,                   --[in]
       D => checkBusFifoFullSFD_D, --[in]
       R => state_machine_rst      --[in]
      );
   ----------------------------------------------------------------------------
   -- SFD state
   ----------------------------------------------------------------------------
   goto_SFD <= checkBusFifoFullSFD and not (BusFifoFull);
   
   stay_SFD <= SFD and busFifoWrNibbleCnt_not_15;
   
   SFD_D    <= goto_SFD or stay_SFD;
   
   STATE10A: FDR
     port map 
      (
       Q => SFD,              --[out]
       C => Clk,              --[in]
       D => SFD_D,            --[in]
       R => state_machine_rst --[in]
       );
   ----------------------------------------------------------------------------
   -- checkBusFifoFull state
   ----------------------------------------------------------------------------
   goto_checkBusFifoFull_1 <= loadBusFifo and not(goto_checkCrc) and
                              not(goto_checkBusFifoFullJam_1);
   
   goto_checkBusFifoFull_2 <= SFD and busFifoWrNibbleCnt_is_15;
     
   stay_checkBusFifoFull   <= checkBusFifoFull and BusFifoFull and
                              not (goto_checkBusFifoFullJam_1);
   
   checkBusFifoFull_D      <= goto_checkBusFifoFull_1 or
                              goto_checkBusFifoFull_2 or
                              stay_checkBusFifoFull;
   
   STATE11A: FDR
     port map 
      (
       Q => checkBusFifoFull,   --[out]
       C => Clk,                --[in]
       D => checkBusFifoFull_D, --[in]
       R => state_machine_rst   --[in]
       );    
   ----------------------------------------------------------------------------
   -- loadBusFifo state
   ----------------------------------------------------------------------------   
   goto_loadBusFifo <= checkBusFifoFull and not(BusFifoFull) and 
                       not(goto_checkCrc) and not(goto_checkBusFifoFullJam_1);
   
   loadBusFifo_D    <= goto_loadBusFifo;
   
   STATE12A: FDR
     port map 
      (
       Q => loadBusFifo,      --[out]
       C => Clk,              --[in]
       D => loadBusFifo_D,    --[in]
       R => state_machine_rst --[in]
       );  
   ----------------------------------------------------------------------------
   -- checkCrc state
   ----------------------------------------------------------------------------    
   goto_checkCrc <= loadBusFifo and txNibbleCnt_is_1 and 
                    not(goto_checkBusFifoFullJam_1);
     
   checkCrc_D    <= goto_checkCrc;
     
   STATE13A: FDR
     port map 
      (
       Q => checkCrc,         --[out]
       C => Clk,              --[in]
       D => checkCrc_D,       --[in]
       R => state_machine_rst --[in]
      );      
   ----------------------------------------------------------------------------
   -- checkBusFifoFullCrc state
   ----------------------------------------------------------------------------
   goto_checkBusFifoFullCrc_1 <=  checkCrc and not(goto_checkBusFifoFullJam_1);
   
   goto_checkBusFifoFullCrc_2 <= loadBusFifoCrc and 
                                 not(goto_checkBusFifoFullJam_1);
   
   stay_checkBusFifoFullCrc   <=  checkBusFifoFullCrc and BusFifoFull and
                                not(goto_checkBusFifoFullJam_1);
   
   checkBusFifoFullCrc_D      <= goto_checkBusFifoFullCrc_1 or 
                                 goto_checkBusFifoFullCrc_2 or 
                                 stay_checkBusFifoFullCrc;  
   
   STATE14A: FDR
     port map 
      (
       Q => checkBusFifoFullCrc,   --[out]
       C => Clk,                   --[in]
       D => checkBusFifoFullCrc_D, --[in]
       R => state_machine_rst      --[in]
      );  
   ----------------------------------------------------------------------------
   -- loadBusFifoCrc state
   ----------------------------------------------------------------------------      
   goto_loadBusFifoCrc_1 <= checkBusFifoFullCrc and not(BusFifoFull) and
                            crcCnt_not_0 and not(goto_checkBusFifoFullJam_1);
     
   loadBusFifoCrc_D      <= goto_loadBusFifoCrc_1;  
     
   STATE15A: FDR
     port map 
      (
       Q => loadBusFifoCrc,   --[out]
       C => Clk,              --[in]
       D => loadBusFifoCrc_D, --[in]
       R => state_machine_rst --[in]
      );  
   ----------------------------------------------------------------------------
   -- waitFifoEmpty state
   ----------------------------------------------------------------------------       
   
   goto_waitFifoEmpty_2 <= checkBusFifoFullCrc and crcCnt_is_0 and
                          not(BusFifoFull) and not(goto_checkBusFifoFullJam_1);
   
   stay_waitFifoEmpty   <= waitFifoEmpty and not(BusFifoEmpty) and
                           not(goto_checkBusFifoFullJam_1);
   
   waitFifoEmpty_D      <= goto_waitFifoEmpty_2 or stay_waitFifoEmpty;
     
   STATE16A: FDR
     port map 
      (
       Q => waitFifoEmpty,    --[out]
       C => Clk,              --[in]
       D => waitFifoEmpty_D,  --[in]
       R => state_machine_rst --[in]
      );  
   ----------------------------------------------------------------------------
   -- txDone state
   ----------------------------------------------------------------------------   
   goto_txDone_1 <= waitFifoEmpty and BusFifoEmpty and 
                    not(goto_checkBusFifoFullJam_1);
   
   goto_txDone_2 <= half_dup_error or chgMacAdr14;
   
   txDone_D      <= goto_txDone_1 or goto_txDone_2;
     
   STATE17A: FDR
     port map 
      (
       Q => txDone,           --[out]
       C => Clk,              --[in]
       D => txDone_D,         --[in]
       R => state_machine_rst --[in]
      );  
   ----------------------------------------------------------------------------
   -- checkBusFifoFullJam state
   ----------------------------------------------------------------------------    
   goto_checkBusFifoFullJam_1 <= (checkBusFifoFull or loadBusFifo or checkCrc 
                                  or checkBusFifoFullCrc or waitFifoEmpty) and
                                  PhyCollision and not(full_half_n);
   
   goto_checkBusFifoFullJam_2 <= loadBusFifoJam;
     
   stay_checkBusFifoFullJam   <= checkBusFifoFullJam and (BusFifoFull or
                               not(pre_SFD_zero));
   
   checkBusFifoFullJam_D      <= goto_checkBusFifoFullJam_1 or
                                 goto_checkBusFifoFullJam_2 or 
                                 stay_checkBusFifoFullJam;
   
   STATE18A: FDR
     port map 
      (
       Q => checkBusFifoFullJam,   --[out]
       C => Clk,                   --[in]
       D => checkBusFifoFullJam_D, --[in]
       R => state_machine_rst      --[in]
       );  
   ----------------------------------------------------------------------------
   -- loadBusFifoJam state
   ----------------------------------------------------------------------------    
   goto_loadBusFifoJam <= checkBusFifoFullJam and 
                          not(stay_checkBusFifoFullJam) and 
                          jamTxNibCnt_not_0;
   
   loadBusFifoJam_D    <= goto_loadBusFifoJam;
   
   STATE19A: FDR
     port map 
      (
       Q => loadBusFifoJam,   --[out]
       C => Clk,              --[in]
       D => loadBusFifoJam_D, --[in]
       R => state_machine_rst --[in]
      );  
   ----------------------------------------------------------------------------
   -- half_dup_error state
   ----------------------------------------------------------------------------  
   goto_half_dup_error_1 <= checkBusFifoFullJam and not(BusFifoFull or
                            not(pre_SFD_zero)) and jamTxNibCnt_is_0 and
                            colWindowNibCnt_not_0 and colRetryCnt_is_15;
     
   goto_half_dup_error_2 <= checkBusFifoFullJam and not(BusFifoFull or
                            not(pre_SFD_zero)) and jamTxNibCnt_is_0 and 
                            colWindowNibCnt_is_0;
     
   half_dup_error_D      <= goto_half_dup_error_1 or goto_half_dup_error_2;
   
   STATE20A: FDR
     port map 
      (
       Q => half_dup_error,   --[out]
       C => Clk,              --[in]
       D => half_dup_error_D, --[in]
       R => state_machine_rst --[in]
      );
   
   ----------------------------------------------------------------------------
   -- collisionRetry state
   ----------------------------------------------------------------------------   
   goto_collisionRetry <= checkBusFifoFullJam and not(stay_checkBusFifoFullJam)
                          and not(goto_half_dup_error_1) and 
                          not(goto_half_dup_error_2) and
                          not(goto_loadBusFifoJam);
     
   collisionRetry_D    <= goto_collisionRetry;
   
   STATE21A: FDR
     port map 
      (
       Q => collisionRetry,   --[out]
       C => Clk,              --[in]
       D => collisionRetry_D, --[in]
       R => state_machine_rst --[in]
      );  
   
   ----------------------------------------------------------------------------
   -- retryWaitFifoEmpty state
   ----------------------------------------------------------------------------   
   goto_retryWaitFifoEmpty <= collisionRetry;
   
   stay_retryWaitFifoEmpty <= retryWaitFifoEmpty and not(BusFifoEmpty);
     
   retryWaitFifoEmpty_D <= goto_retryWaitFifoEmpty or stay_retryWaitFifoEmpty;
   
   STATE22A: FDR
     port map 
      (
       Q => retryWaitFifoEmpty,   --[out]
       C => Clk,                  --[in]
       D => retryWaitFifoEmpty_D, --[in]
       R => state_machine_rst     --[in]
      );
   
   ----------------------------------------------------------------------------
   -- retryReset state
   ----------------------------------------------------------------------------   
   goto_retryReset <= retryWaitFifoEmpty and BusFifoEmpty;
     
   retryReset_D    <= goto_retryReset;
   
   STATE23A: FDR
     port map 
      (
       Q => retryReset,       --[out]
       C => Clk,              --[in]
       D => retryReset_D,     --[in]
       R => state_machine_rst --[in]
      );  
   
   ----------------------------------------------------------------------------
   -- txDone2 state
   ----------------------------------------------------------------------------   
   goto_txDone2 <= txDone;
   
   txDone2_D    <= goto_txDone2;
   
   STATE24A: FDR
     port map 
      (
       Q => txDone2,          --[out]
       C => Clk,              --[in]
       D => txDone2_D,        --[in]
       R => state_machine_rst --[in]
      );  
   
   ----------------------------------------------------------------------------
   -- txDonePause state
   ----------------------------------------------------------------------------   
   goto_txDonePause <= txDone2;
   
   txDonePause_D    <= goto_txDonePause;
     
   STATE25A: FDR
     port map 
      (
       Q => txDonePause,      --[out]
       C => Clk,              --[in]
       D => txDonePause_D,    --[in]
       R => state_machine_rst --[in]
      );    
   
   ----------------------------------------------------------------------------
   -- chgMacAdr1 state
   ----------------------------------------------------------------------------    
   goto_chgMacAdr1 <= idle and Mac_program_start and not mac_program_start_reg;
   
   chgMacAdr1_D    <= goto_chgMacAdr1 ;
   
   STATE26A: FDR
     port map 
      (
       Q => chgMacAdr1,       --[out]
       C => Clk,              --[in]
       D => chgMacAdr1_D,     --[in]
       R => state_machine_rst --[in]
      );  
   
   ----------------------------------------------------------------------------
   -- chgMacAdr2 state
   ----------------------------------------------------------------------------    
   goto_chgMacAdr2 <= chgMacAdr1;
   
   chgMacAdr2_D    <= goto_chgMacAdr2 ;
   
   STATE27A: FDR
     port map 
      (
       Q => chgMacAdr2,       --[out]
       C => Clk,              --[in]
       D => chgMacAdr2_D,     --[in]
       R => state_machine_rst --[in]
      );  
   
   ----------------------------------------------------------------------------
   -- chgMacAdr3 state
   ----------------------------------------------------------------------------    
   goto_chgMacAdr3 <= chgMacAdr2;
   
   chgMacAdr3_D    <= goto_chgMacAdr3 ;
     
   STATE28A: FDR
     port map 
      (
       Q => chgMacAdr3,       --[out]
       C => Clk,              --[in]
       D => chgMacAdr3_D,     --[in]
       R => state_machine_rst --[in]
      );    
   
   ----------------------------------------------------------------------------
   -- chgMacAdr4 state
   ----------------------------------------------------------------------------    
   goto_chgMacAdr4 <= chgMacAdr3;
   
   chgMacAdr4_D    <= goto_chgMacAdr4 ;
     
   STATE29A: FDR
     port map 
      (
       Q => chgMacAdr4,       --[out]
       C => Clk,              --[in]
       D => chgMacAdr4_D,     --[in]
       R => state_machine_rst --[in]
       );  
   
   ----------------------------------------------------------------------------
   -- chgMacAdr5 state
   ----------------------------------------------------------------------------    
   goto_chgMacAdr5 <= chgMacAdr4;
   
   chgMacAdr5_D    <= goto_chgMacAdr5 ;
     
   STATE30A: FDR
     port map 
      (
       Q => chgMacAdr5,       --[out]
       C => Clk,              --[in]
       D => chgMacAdr5_D,     --[in]
       R => state_machine_rst --[in]
      );   
   
   ----------------------------------------------------------------------------
   -- chgMacAdr6 state
   ----------------------------------------------------------------------------    
   goto_chgMacAdr6 <= chgMacAdr5;
   
   chgMacAdr6_D    <= goto_chgMacAdr6 ;
   
   STATE31A: FDR
     port map 
      (
       Q => chgMacAdr6,       --[out]
       C => Clk,              --[in]
       D => chgMacAdr6_D,     --[in]
       R => state_machine_rst --[in]
      );  
   
   ----------------------------------------------------------------------------
   -- chgMacAdr7 state
   ----------------------------------------------------------------------------    
   goto_chgMacAdr7 <= chgMacAdr6;
   
   chgMacAdr7_D    <= goto_chgMacAdr7 ;
   
   STATE32A: FDR
     port map 
      (
       Q => chgMacAdr7,       --[out]
       C => Clk,              --[in]
       D => chgMacAdr7_D,     --[in]
       R => state_machine_rst --[in]
      );    
   
   ----------------------------------------------------------------------------
   -- chgMacAdr8 state
   ----------------------------------------------------------------------------    
   goto_chgMacAdr8 <= chgMacAdr7;
   
   chgMacAdr8_D    <= goto_chgMacAdr8 ;
   
   STATE33A: FDR
     port map 
      (
       Q => chgMacAdr8,       --[out]
       C => Clk,              --[in]
       D => chgMacAdr8_D,     --[in]
       R => state_machine_rst --[in]
      );  
   
   ----------------------------------------------------------------------------
   -- chgMacAdr9 state
   ----------------------------------------------------------------------------    
   goto_chgMacAdr9 <= chgMacAdr8;
   
   chgMacAdr9_D    <= goto_chgMacAdr9 ;
   
   STATE34A: FDR
     port map 
      (
       Q => chgMacAdr9,       --[out]
       C => Clk,              --[in]
       D => chgMacAdr9_D,     --[in]
       R => state_machine_rst --[in]
      );     
   
   ----------------------------------------------------------------------------
   -- chgMacAdr10 state
   ----------------------------------------------------------------------------    
   goto_chgMacAdr10 <= chgMacAdr9;
   
   chgMacAdr10_D    <= goto_chgMacAdr10 ;
   
   STATE35A: FDR
     port map 
      (
       Q => chgMacAdr10,      --[out]
       C => Clk,              --[in]
       D => chgMacAdr10_D,    --[in]
       R => state_machine_rst --[in]
      );   
   
   ----------------------------------------------------------------------------
   -- chgMacAdr11 state
   ----------------------------------------------------------------------------    
   goto_chgMacAdr11 <= chgMacAdr10;
   
   chgMacAdr11_D    <= goto_chgMacAdr11 ;
   
   STATE36A: FDR
     port map 
      (
       Q => chgMacAdr11,      --[out]
       C => Clk,              --[in]
       D => chgMacAdr11_D,    --[in]
       R => state_machine_rst --[in]
      );  
   
   ----------------------------------------------------------------------------
   -- chgMacAdr12 state
   ----------------------------------------------------------------------------    
   goto_chgMacAdr12 <= chgMacAdr11;
   
   chgMacAdr12_D    <= goto_chgMacAdr12 ;
   
   STATE37A: FDR
     port map 
      (
       Q => chgMacAdr12,      --[out]
       C => Clk,              --[in]
       D => chgMacAdr12_D,    --[in]
       R => state_machine_rst --[in]
      );    
   
   ----------------------------------------------------------------------------
   -- chgMacAdr13 state
   ----------------------------------------------------------------------------    
   goto_chgMacAdr13 <= chgMacAdr12;
   
   chgMacAdr13_D    <= goto_chgMacAdr13 ;
     
   STATE38A: FDR
     port map 
      (
       Q => chgMacAdr13,      --[out]
       C => Clk,              --[in]
       D => chgMacAdr13_D,    --[in]
       R => state_machine_rst --[in]
      );  
   
   ----------------------------------------------------------------------------
   -- chgMacAdr14 state
   ----------------------------------------------------------------------------    
   goto_chgMacAdr14 <= chgMacAdr13;
   
   chgMacAdr14_D    <= goto_chgMacAdr14 ;
   
   STATE39A: FDR
     port map 
      (
       Q => chgMacAdr14,      --[out]
       C => Clk,              --[in]
       D => chgMacAdr14_D,    --[in]
       R => state_machine_rst --[in]
      );    
   ----------------------------------------------------------------------------   
   -- end of states
   ----------------------------------------------------------------------------   

   ----------------------------------------------------------------------------
   -- REG_PROCESS
   ----------------------------------------------------------------------------
   -- This process registers all the signals on the bus clock.
   ----------------------------------------------------------------------------
   REG_PROCESS : process (Clk)
   begin  --
     if (Clk'event and Clk = '1') then     -- rising clock edge
       if (Rst = '1') then
         phytx_en_reg          <= '0';
         busFifoWrCntRst_reg   <= '0';
         retrying_reg          <= '0';
         txCrcEn_reg           <= '0';   
         transmit_start_reg    <= '0';
         mac_program_start_reg <= '0';
       else
   
         phytx_en_reg          <= phytx_en_i;
         busFifoWrCntRst_reg   <= busFifoWrCntRst_i;
         retrying_reg          <= retrying_i;
         txCrcEn_reg           <= txCrcEn_i;
         transmit_start_reg    <= Transmit_start;
         mac_program_start_reg <= Mac_program_start; 
       end if;
     end if;
   end process REG_PROCESS;
 
   ----------------------------------------------------------------------------
   -- COMB_PROCESS
   ----------------------------------------------------------------------------
   -- This process generate control signals for the state machine.
   ----------------------------------------------------------------------------
   COMB_PROCESS : process (phytx_en_reg, busFifoWrCntRst_reg, 
                           txCrcEn_reg, txDone, idle, preamble, 
                           half_dup_error, checkBusFifoFull,
                           collisionRetry, retrying_reg, 
                           checkBusFifoFullCrc, SFD, loadBusFifoCrc, 
                           checkBusFifoFullSFD)
   begin
      
      -- Generate PHY Tx Enable
      if (txDone='1' or idle='1') then
         phytx_en_i <= '0';
      elsif (preamble = '1') then
         phytx_en_i <= '1';
      else
         phytx_en_i <= phytx_en_reg;
      end if;
      
      -- Generate BusFifo Write Counter reset
      if (half_dup_error='1' or txDone='1' or idle='1') then
         busFifoWrCntRst_i <= '1';
      elsif (preamble = '1') then
         busFifoWrCntRst_i <= '0';
      else
         busFifoWrCntRst_i <= busFifoWrCntRst_reg;
      end if;    
      
      -- Generate retry signal in case of collision
      if (collisionRetry='1') then
         retrying_i <= '1';
      elsif (idle = '1') then
         retrying_i <= '0';
      else
         retrying_i <= retrying_reg;
      end if;
     
      -- Generate transmit CRC enable
      if (checkBusFifoFull='1') then
         txCrcEn_i <= '1';
      elsif (checkBusFifoFullSFD='1' or checkBusFifoFullCRC='1' or SFD='1' or
             idle='1' or loadBusFifoCrc='1' or preamble='1') then
         txCrcEn_i <= '0';
      else
         txCrcEn_i <= txCrcEn_reg;
      end if;
   end process COMB_PROCESS;
   
   ----------------------------------------------------------------------------
   -- FSMD_PROCESS
   ----------------------------------------------------------------------------
   -- This process generate control signals for the state machine for 
   -- transmit operation
   ----------------------------------------------------------------------------
   FSMD_PROCESS : process(crcCnt_is_0, JamTxNibCnt, goto_checkBusFifoFullCrc_1,
                          pre_SFD_zero, checkBusFifoFullJam, full_half_n,
                          retryReset, txDonePause, loadBusFifo, loadBusFifoJam,
                          checkCrc, txDone2, chgMacAdr2, chgMacAdr3,
                          chgMacAdr4, chgMacAdr5, chgMacAdr6, chgMacAdr7, 
                          chgMacAdr8, chgMacAdr9, chgMacAdr10, chgMacAdr11, 
                          chgMacAdr12, chgMacAdr13, chgMacAdr14, chgMacAdr1, 
                          lngthDelay1, lngthDelay2, idle, checkBusFifoFull, 
                          txDone, ldLngthCntr,half_dup_error, collisionRetry, 
                          checkBusFifoFullCrc, loadBusFifoCrc, retrying_reg, 
                          preamble, SFD)
  
   begin

      -- Enable JAM reset
      if (checkBusFifoFullJam = '1' and pre_SFD_zero = '1' and 
          full_half_n = '0' and (JamTxNibCnt = "0111")) then
         Jam_rst <= '1';
      else
         Jam_rst <= '0';     
      end if;
   
      -- Bus FIFO write counte enable
      BusFifoWrCntEn      <= '1'; -- temp 
   
      -- Enable TX late collision reset
         TxLateColnRst <= '0';
      
      -- Enable TX deffer reset 
         TxExcessDefrlRst <= '0';        
      
      -- Enable back off and TX collision retry counter 
      if (collisionRetry = '1') then
         InitBackoff <= '1';
         TxColRetryCntEnbl <= '1';
      else
         InitBackoff <= '0';
         TxColRetryCntEnbl <= '0';
      end if;
      
      -- Enable TX retry reset
      if (retryReset = '1') or
         (txDonePause = '1') then -- clear up any built up garbage in async 
                                  -- FIFOs  at the end of a packet
        TxRetryRst <= '1';        
      else
        TxRetryRst <= '0';     
      end if;    
      
      -- Enable TX nibble counter reset
      if (idle = '1') then
         txNibbleCntRst_i <= '1';
      else
         txNibbleCntRst_i <= '0';
      end if;
      
      -- Enable TX collision retry reset
      if (idle = '1' and retrying_reg = '0') then
         TxColRetryCntRst_n   <= '0';
      else
         TxColRetryCntRst_n   <= '1';
      end if;
      
      -- Enable TX CRC counter shift 
      if ((checkBusFifoFullCrc = '1') or (loadBusFifoCrc = '1')) then
         TxCrcShftOutEn   <= '1';
      else
         TxCrcShftOutEn   <= '0';
      end if;
      
      -- Enable Preamble in the frame
      if (preamble = '1') then
         EnblPre   <= '1';
      else
         EnblPre   <= '0';
      end if;
      
      -- Enable SFD in the frame
      if (SFD = '1') then
         EnblSFD   <= '1';
      else
         EnblSFD   <= '0';
      end if;
      
      -- Enable Data in the frame
      if (loadBusFifo = '1') then
         EnblData <= '1';
      else
         EnblData <= '0';
      end if;   
      
      -- Enable CRC
      if (loadBusFifoCrc = '1') then
         EnblCRC <= '1';
      else
         EnblCRC <= '0';
      end if;
      
      -- Enable TX nibble counter load 
      if (SFD = '1') then
         txNibbleCntLd_i   <= '1';
      else
         txNibbleCntLd_i   <= '0';
      end if;
  
      -- Enable clear for TX interface FIFO
      if (checkBusFifoFullCrc = '1' and crcCnt_is_0  = '1') or
          ((checkBusFifoFullJam='1' or  loadBusFifoJam='1') 
            and pre_SFD_zero = '1'  and full_half_n = '0') or
            (collisionRetry = '1' ) or  (half_dup_error = '1') or
            (checkCrc = '1' and goto_checkBusFifoFullCrc_1 = '0') then
         Enblclear <= '1';
      else
         Enblclear <= '0';
      end if;
      
      -- Enable Bus FIFO write
      if ((loadBusFifo = '1') or
          (preamble = '1') or
          (SFD = '1') or
          (loadBusFifoCrc = '1')
          ) then
         busFifoWr_i   <= '1';
      else
         busFifoWr_i   <= '0';
      end if;
      
      -- Enable JAM TX nibble 
      if (loadBusFifo = '1') then
         txEnNibbleCnt_i  <= '1';
      else
         txEnNibbleCnt_i  <= '0';
      end if;   
      
 
      -- Enable TX buffer address increment
      if (loadBusFifo = '1') or (chgMacAdr2 = '1')  or (chgMacAdr3 = '1') or
         (chgMacAdr4 = '1')  or (chgMacAdr5 = '1')  or (chgMacAdr6 = '1') or
         (chgMacAdr7 = '1')  or (chgMacAdr8 = '1')  or (chgMacAdr9 = '1') or
         (chgMacAdr10 = '1') or (chgMacAdr11 = '1') or (chgMacAdr12 = '1') or
         (chgMacAdr13 = '1') or (chgMacAdr14 = '1') then
         Tx_addr_en <= '1'; 
      else
         Tx_addr_en <= '0';
      end if;   
      
      -- Generate TX start after preamble
      if (preamble = '1') or
         (chgMacAdr1 = '1') then
         Tx_start <= '1'; -- reset address to 0 for start of transmit
      else
         Tx_start <= '0';
      end if;
   
         
      -- TX DPM buffer CE
      if (idle = '1') or
         (lngthDelay1 = '1') or (lngthDelay2 = '1') or 
         (checkBusFifoFull = '1') or (ldLngthCntr = '1') or
         (txDone = '1') or (txDone2 = '1') or (txDonePause = '1') or
         (chgMacAdr1 = '1')  or (chgMacAdr2 = '1')  or (chgMacAdr3 = '1') or
         (chgMacAdr4 = '1')  or (chgMacAdr5 = '1')  or (chgMacAdr6 = '1') or
         (chgMacAdr7 = '1')  or (chgMacAdr8 = '1')  or (chgMacAdr9 = '1') or
         (chgMacAdr10 = '1') or (chgMacAdr11 = '1') or (chgMacAdr12 = '1') or
         (chgMacAdr13 = '1') or (chgMacAdr14 = '1') then
         Tx_DPM_ce <= '1';
      else
         Tx_DPM_ce <= '0';
      end if;

      -- Enable JAM 
      if (loadBusFifoJam = '1') then
         EnblJam <= '1';
      else
         EnblJam <= '0';
      end if;
     
     -- TX DPM write enable
     Tx_DPM_wr_rd_n <= '0';
   
                                    
  end process FSMD_PROCESS;


   ----------------------------------------------------------------------------
   -- OUTPUT_REG1
   ----------------------------------------------------------------------------
   -- This process generate mack address RAM write enable 
   ----------------------------------------------------------------------------
   OUTPUT_REG1:process (Clk)
   begin
      if (Clk'event and Clk='1') then
         if (Rst = '1') then
            Mac_addr_ram_we <= '0';
         elsif (idle_D = '1') then
            Mac_addr_ram_we <= '0';
         elsif (chgMacAdr3_D = '1')  or 
               (chgMacAdr4_D = '1')  or
               (chgMacAdr5_D = '1')  or
               (chgMacAdr6_D = '1')  or
               (chgMacAdr7_D = '1')  or
               (chgMacAdr8_D = '1')  or
               (chgMacAdr9_D = '1')  or
               (chgMacAdr10_D = '1') or
               (chgMacAdr11_D = '1') or
               (chgMacAdr12_D = '1') or
               (chgMacAdr13_D = '1') or
               (chgMacAdr14_D = '1') then
            Mac_addr_ram_we <= '1';     
         else
            Mac_addr_ram_we <= '0';
         end if;
      end if;
   end process OUTPUT_REG1;

    
   ----------------------------------------------------------------------------
   -- OUTPUT_REG2
   ----------------------------------------------------------------------------
   -- This process MAC Addr RAM write Adrress to update the MAC address of 
   -- EMACLite Core.
   ----------------------------------------------------------------------------
   OUTPUT_REG2:process (Clk)
   begin
 
      if (Clk'event and Clk='1') then
         if (Rst = '1') then
            Mac_addr_ram_addr_wr <= x"0";
         else
            if idle_D = '1' then
               Mac_addr_ram_addr_wr <= x"0";
            elsif chgMacAdr3_D = '1' then
               Mac_addr_ram_addr_wr <= x"0";             
            elsif chgMacAdr4_D = '1' then
               Mac_addr_ram_addr_wr <= x"1";             
            elsif chgMacAdr5_D = '1' then
               Mac_addr_ram_addr_wr <= x"2";             
            elsif chgMacAdr6_D = '1' then
               Mac_addr_ram_addr_wr <= x"3";             
            elsif chgMacAdr7_D = '1' then
               Mac_addr_ram_addr_wr <= x"4";             
            elsif chgMacAdr8_D = '1' then
               Mac_addr_ram_addr_wr <= x"5";             
            elsif chgMacAdr9_D = '1' then
               Mac_addr_ram_addr_wr <= x"6";             
            elsif chgMacAdr10_D = '1' then
               Mac_addr_ram_addr_wr <= x"7";             
            elsif chgMacAdr11_D = '1' then
               Mac_addr_ram_addr_wr <= x"8";             
            elsif chgMacAdr12_D = '1' then
               Mac_addr_ram_addr_wr <= x"9";             
            elsif chgMacAdr13_D = '1' then
               Mac_addr_ram_addr_wr <= x"a";             
            elsif chgMacAdr14_D = '1' then
               Mac_addr_ram_addr_wr <= x"b";             
            else
               Mac_addr_ram_addr_wr <= x"0";
            end if;
         end if;
      end if;
   end process OUTPUT_REG2;
    
 end implementation;
 


-------------------------------------------------------------------------------
-- tx_intrfce - entity/architecture pair
-------------------------------------------------------------------------------
--  ***************************************************************************
--  ** DISCLAIMER OF LIABILITY                                               **
--  **                                                                       **
--  **  This file contains proprietary and confidential information of       **
--  **  Xilinx, Inc. ("Xilinx"), that is distributed under a license         **
--  **  from Xilinx, and may be used, copied and/or disclosed only           **
--  **  pursuant to the terms of a valid license agreement with Xilinx.      **
--  **                                                                       **
--  **  XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION                **
--  **  ("MATERIALS") "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER           **
--  **  EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING WITHOUT                  **
--  **  LIMITATION, ANY WARRANTY WITH RESPECT TO NONINFRINGEMENT,            **
--  **  MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE. Xilinx        **
--  **  does not warrant that functions included in the Materials will       **
--  **  meet the requirements of Licensee, or that the operation of the      **
--  **  Materials will be uninterrupted or error-free, or that defects       **
--  **  in the Materials will be corrected. Furthermore, Xilinx does         **
--  **  not warrant or make any representations regarding use, or the        **
--  **  results of the use, of the Materials in terms of correctness,        **
--  **  accuracy, reliability or otherwise.                                  **
--  **                                                                       **
--  **  Xilinx products are not designed or intended to be fail-safe,        **
--  **  or for use in any application requiring fail-safe performance,       **
--  **  such as life-support or safety devices or systems, Class III         **
--  **  medical devices, nuclear facilities, applications related to         **
--  **  the deployment of airbags, or any other applications that could      **
--  **  lead to death, personal injury or severe property or                 **
--  **  environmental damage (individually and collectively, "critical       **
--  **  applications"). Customer assumes the sole risk and liability         **
--  **  of any use of Xilinx products in critical applications,              **
--  **  subject only to applicable laws and regulations governing            **
--  **  limitations on product liability.                                    **
--  **                                                                       **
--  **  Copyright 2010 Xilinx, Inc.                                          **
--  **  All rights reserved.                                                 **
--  **                                                                       **
--  **  This disclaimer and copyright notice must be retained as part        **
--  **  of this file at all times.                                           **
--  ***************************************************************************
--
-------------------------------------------------------------------------------
-- Filename     : tx_intrfce.vhd
-- Version      : v2.0
-- Description  : This is the ethernet transmit interface. 
-- VHDL-Standard: VHDL'93
-------------------------------------------------------------------------------
-- Structure:   
--
--  axi_ethernetlite.vhd
--      \
--      \-- axi_interface.vhd
--      \-- xemac.vhd
--           \
--           \-- mdio_if.vhd
--           \-- emac_dpram.vhd                     
--           \    \                     
--           \    \-- RAMB16_S4_S36
--           \                          
--           \    
--           \-- emac.vhd                     
--                \                     
--                \-- MacAddrRAM                   
--                \-- receive.vhd
--                \      rx_statemachine.vhd
--                \      rx_intrfce.vhd
--                \         async_fifo_fg.vhd
--                \      crcgenrx.vhd
--                \                     
--                \-- transmit.vhd
--                       crcgentx.vhd
--                          crcnibshiftreg
--                       tx_intrfce.vhd
--                          async_fifo_fg.vhd
--                       tx_statemachine.vhd
--                       deferral.vhd
--                          cntr5bit.vhd
--                          defer_state.vhd
--                       bocntr.vhd
--                          lfsr16.vhd
--                       msh_cnt.vhd
--                       ld_arith_reg.vhd
--

-------------------------------------------------------------------------------
-- Author:    PVK
-- History:    
-- PVK        06/07/2010     First Version
-- ^^^^^^
-- First version.  
-- ~~~~~~
-------------------------------------------------------------------------------
-- Naming Conventions:
--      active low signals:                     "*_n"
--      clock signals:                          "Clk", "clk_div#", "clk_#x" 
--      reset signals:                          "Rst", "rst_n" 
--      generics:                               "C_*" 
--      user defined types:                     "*_TYPE" 
--      state machine next state:               "*_ns" 
--      state machine current state:            "*_cs" 
--      combinatorial signals:                  "*_com" 
--      pipelined or register delay signals:    "*_d#" 
--      counter signals:                        "*cnt*"
--      clock enable signals:                   "*_ce" 
--      internal version of output port         "*_i"
--      device pins:                            "*_pin" 
--      ports:                                  - Names begin with Uppercase 
--      processes:                              "*_PROCESS" 
--      component instantiations:               "<ENTITY_>I_<#|FUNC>
-------------------------------------------------------------------------------
-- 
library ieee;
use ieee.std_logic_1164.all;

-------------------------------------------------------------------------------
-- axi_ethernetlite_v3_0_18 library is used for axi_ethernetlite_v3_0_18 
-- component declarations
-------------------------------------------------------------------------------
library axi_ethernetlite_v3_0_18;
use axi_ethernetlite_v3_0_18.all;

-------------------------------------------------------------------------------
library lib_cdc_v1_0_2;
library lib_fifo_v1_0_14;

--library fifo_generator_v11_0; --FIFO Hier
--use fifo_generator_v11_0.all;
-- synopsys translate_off
-- Library XilinxCoreLib;
--library simprim;
-- synopsys translate_on

-------------------------------------------------------------------------------
-- Vcomponents from unisim library is used for FIFO instatiation
-- function declarations
-------------------------------------------------------------------------------
library unisim;
use unisim.vcomponents.all;

-------------------------------------------------------------------------------
-- Definition of Ports:
--
--  Clk                 -- System Clock
--  Rst                 -- System Reset
--  Phy_tx_clk          -- PHY TX Clock
--  Emac_tx_wr_data     -- Ethernet transmit data
--  Tx_er               -- Transmit error
--  Phy_tx_en           -- Ethernet transmit enable
--  Tx_en               -- Transmit enable
--  Emac_tx_wr          -- TX FIFO write enable
--  Fifo_empty          -- TX FIFO empty
--  Fifo_almost_emp     -- TX FIFP almost empty
--  Fifo_full           -- TX FIFO full
--  Phy_tx_data         -- Ethernet transmit data
-------------------------------------------------------------------------------
-- ENTITY
-------------------------------------------------------------------------------
entity tx_intrfce is
  generic 
    (
    C_FAMILY          : string  := "virtex6"  
    );
  port 
    (
    Clk               : in  std_logic;
    Rst               : in  std_logic;
    Phy_tx_clk        : in  std_logic;
    Emac_tx_wr_data   : in  std_logic_vector (0 to 3);
    Tx_er             : in  std_logic;
    PhyTxEn           : in  std_logic;
    Tx_en             : in  std_logic;
    Emac_tx_wr        : in  std_logic;
    Fifo_empty        : out std_logic;
    Fifo_full         : out std_logic;
    Phy_tx_data       : out std_logic_vector (0 to 5)
    );
end tx_intrfce;

architecture implementation of tx_intrfce is

attribute DowngradeIPIdentifiedWarnings: string;

attribute DowngradeIPIdentifiedWarnings of implementation : architecture is "yes";

-------------------------------------------------------------------------------
--  Constant Declarations
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
--  Signal and Type Declarations
-------------------------------------------------------------------------------

signal bus_combo    : std_logic_vector (0 to 5);
signal fifo_empty_i : std_logic;
signal fifo_empty_c : std_logic;
-------------------------------------------------------------------------------
-- Component Declarations
-------------------------------------------------------------------------------
-- The following components are the building blocks of the EMAC
-------------------------------------------------------------------------------
component FDR
  port 
   (
    Q : out std_logic;
    C : in std_logic;
    D : in std_logic;
    R : in std_logic
   );
end component;

--FIFO HIER
--component async_fifo_eth
--  port (
--    rst     : in std_logic;
--    wr_clk  : in std_logic;
--    rd_clk  : in std_logic;
--    din     : in std_logic_vector(5 downto 0);
--    wr_en   : in std_logic;
--    rd_en   : in std_logic;
--    dout    : out std_logic_vector(5 downto 0);
--    full    : out std_logic;
--    empty   : out std_logic;
--    valid   : out std_logic
--  );
--end component;


begin


   I_TX_FIFO: entity lib_fifo_v1_0_14.async_fifo_fg
     generic map(
       C_ALLOW_2N_DEPTH   => 0,  -- New paramter to leverage FIFO Gen 2**N depth
       C_FAMILY           => C_FAMILY,  -- new for FIFO Gen
       C_DATA_WIDTH       => 6,
       C_ENABLE_RLOCS     => 0,  -- not supported in FG
       C_FIFO_DEPTH       => 16,
       C_HAS_ALMOST_EMPTY => 0,
       C_HAS_ALMOST_FULL  => 0,
       C_HAS_RD_ACK       => 1,
       C_HAS_RD_COUNT     => 0,
       C_EN_SAFETY_CKT    => 1,  
       C_HAS_RD_ERR       => 0,
       C_HAS_WR_ACK       => 0,
       C_HAS_WR_COUNT     => 0,
       C_HAS_WR_ERR       => 0,
       C_RD_ACK_LOW       => 0,
       C_RD_COUNT_WIDTH   => 2,
       C_RD_ERR_LOW       => 0,
       C_USE_BLOCKMEM     => 0,  -- 0 = distributed RAM, 1 = BRAM
       C_WR_ACK_LOW       => 0,
       C_WR_COUNT_WIDTH   => 2,
       C_WR_ERR_LOW       => 0,
       C_XPM_FIFO         => 1   
     )
     port map(
       Din            => bus_combo, 
       Wr_en          => Emac_tx_wr,
       Wr_clk         => Clk,
       Rd_en          => Tx_en,
       Rd_clk         => Phy_tx_clk,
       Ainit          => Rst,   
       Dout           => Phy_tx_data,
       Full           => Fifo_full,
       Empty          => fifo_empty_i,
       Almost_full    => open,
       Almost_empty   => open, 
       Wr_count       => open,
       Rd_count       => open,
       Rd_ack         => open,
       Rd_err         => open,
       Wr_ack         => open,
       Wr_err         => open
     );

-- I_TX_FIFO : async_fifo_eth
--   port map(
--    din            => bus_combo, 
--    wr_en          => Emac_tx_wr,
--    wr_clk         => Clk,
--    rd_en          => Tx_en,
--    rd_clk         => Phy_tx_clk,
--    rst            => Rst,   
--    dout           => Phy_tx_data,
--    full           => Fifo_full,
--    empty          => fifo_empty_i,
--    valid          => open    
--   );

   pipeIt: FDR
     port map
      (
       Q => Fifo_empty,   --[out]
       C => Clk,          --[in]
       D => fifo_empty_c, --[in]
       R => Rst           --[in]
      );
   ----------------------------------------------------------------------------
   -- CDC module for syncing tx_en_i in fifo_empty domain
   ----------------------------------------------------------------------------
  CDC_FIFO_EMPTY: entity  lib_cdc_v1_0_2.cdc_sync
  generic map (
    C_CDC_TYPE           => 1,
    C_RESET_STATE        => 0,
    C_SINGLE_BIT         => 1,
    C_FLOP_INPUT         => 0,
    C_VECTOR_WIDTH       => 1,
    C_MTBF_STAGES        => 3
            )
  port map(
    prmry_aclk            => '1',
    prmry_resetn          => '1',
    prmry_in              => fifo_empty_i,
    prmry_ack             => open,
    scndry_out            => fifo_empty_c,
    scndry_aclk           => Clk,
    scndry_resetn         => '1',
    prmry_vect_in         => (OTHERS => '0'),
    scndry_vect_out       => open
     );    

   bus_combo <= (Emac_tx_wr_data & Tx_er & PhyTxEn); 
           
end implementation;



-------------------------------------------------------------------------------
-- rx_statemachine - entity/architecture pair
-------------------------------------------------------------------------------
--  ***************************************************************************
--  ** DISCLAIMER OF LIABILITY                                               **
--  **                                                                       **
--  **  This file contains proprietary and confidential information of       **
--  **  Xilinx, Inc. ("Xilinx"), that is distributed under a license         **
--  **  from Xilinx, and may be used, copied and/or disclosed only           **
--  **  pursuant to the terms of a valid license agreement with Xilinx.      **
--  **                                                                       **
--  **  XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION                **
--  **  ("MATERIALS") "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER           **
--  **  EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING WITHOUT                  **
--  **  LIMITATION, ANY WARRANTY WITH RESPECT TO NONINFRINGEMENT,            **
--  **  MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE. Xilinx        **
--  **  does not warrant that functions included in the Materials will       **
--  **  meet the requirements of Licensee, or that the operation of the      **
--  **  Materials will be uninterrupted or error-free, or that defects       **
--  **  in the Materials will be corrected. Furthermore, Xilinx does         **
--  **  not warrant or make any representations regarding use, or the        **
--  **  results of the use, of the Materials in terms of correctness,        **
--  **  accuracy, reliability or otherwise.                                  **
--  **                                                                       **
--  **  Xilinx products are not designed or intended to be fail-safe,        **
--  **  or for use in any application requiring fail-safe performance,       **
--  **  such as life-support or safety devices or systems, Class III         **
--  **  medical devices, nuclear facilities, applications related to         **
--  **  the deployment of airbags, or any other applications that could      **
--  **  lead to death, personal injury or severe property or                 **
--  **  environmental damage (individually and collectively, "critical       **
--  **  applications"). Customer assumes the sole risk and liability         **
--  **  of any use of Xilinx products in critical applications,              **
--  **  subject only to applicable laws and regulations governing            **
--  **  limitations on product liability.                                    **
--  **                                                                       **
--  **  Copyright 2010 Xilinx, Inc.                                          **
--  **  All rights reserved.                                                 **
--  **                                                                       **
--  **  This disclaimer and copyright notice must be retained as part        **
--  **  of this file at all times.                                           **
--  ***************************************************************************
--
-------------------------------------------------------------------------------
-- Filename     : rx_statemachine.vhd
-- Version      : v2.0
-- Description  : This file contains the receive control state machine.
-- VHDL-Standard: VHDL'93
-------------------------------------------------------------------------------
-- Structure:   
--
--  axi_ethernetlite.vhd
--      \
--      \-- axi_interface.vhd
--      \-- xemac.vhd
--           \
--           \-- mdio_if.vhd
--           \-- emac_dpram.vhd                     
--           \    \                     
--           \    \-- RAMB16_S4_S36
--           \                          
--           \    
--           \-- emac.vhd                     
--                \                     
--                \-- MacAddrRAM                   
--                \-- receive.vhd
--                \      rx_statemachine.vhd
--                \      rx_intrfce.vhd
--                \         async_fifo_fg.vhd
--                \      crcgenrx.vhd
--                \                     
--                \-- transmit.vhd
--                       crcgentx.vhd
--                          crcnibshiftreg
--                       tx_intrfce.vhd
--                          async_fifo_fg.vhd
--                       tx_statemachine.vhd
--                       deferral.vhd
--                          cntr5bit.vhd
--                          defer_state.vhd
--                       bocntr.vhd
--                          lfsr16.vhd
--                       msh_cnt.vhd
--                       ld_arith_reg.vhd
--
-------------------------------------------------------------------------------
-- Author:    PVK
-- History:    
-- PVK        06/07/2010     First Version
-- ^^^^^^
-- First version.  
-- ~~~~~~
-------------------------------------------------------------------------------
-- Naming Conventions:
--      active low signals:                     "*_n"
--      clock signals:                          "clk", "clk_div#", "clk_#x" 
--      reset signals:                          "rst", "rst_n" 
--      generics:                               "C_*" 
--      user defined types:                     "*_TYPE" 
--      state machine next state:               "*_ns" 
--      state machine current state:            "*_cs" 
--      combinatorial signals:                  "*_com" 
--      pipelined or register delay signals:    "*_d#" 
--      counter signals:                        "*cnt*"
--      clock enable signals:                   "*_ce" 
--      internal version of output port         "*_i"
--      device pins:                            "*_pin" 
--      ports:                                  - Names begin with Uppercase 
--      processes:                              "*_PROCESS" 
--      component instantiations:               "<ENTITY_>I_<#|FUNC>
-------------------------------------------------------------------------------
-- 
library ieee;
use ieee.STD_LOGIC_1164.all;
use ieee.numeric_std.UNSIGNED;
use ieee.numeric_std."+";

-------------------------------------------------------------------------------
-- axi_ethernetlite_v3_0_18 library is used for axi_ethernetlite_v3_0_18 
-- component declarations
-------------------------------------------------------------------------------
library axi_ethernetlite_v3_0_18;
use axi_ethernetlite_v3_0_18.all;

-- synopsys translate_off
-- Library XilinxCoreLib;
--library simprim;
-- synopsys translate_on

-------------------------------------------------------------------------------
-- Vcomponents from unisim library is used for FIFO instatiation
-- function declarations
-------------------------------------------------------------------------------
library unisim;
use unisim.Vcomponents.all;

-------------------------------------------------------------------------------
-- Definition of Generics:
-------------------------------------------------------------------------------
-- C_DUPLEX               -- 1 = full duplex, 0 = half duplex
-------------------------------------------------------------------------------
-- Definition of Ports:
--
--  Clk                  -- System Clock
--  Rst                  -- System Reset
--  Emac_rx_rd_data      -- RX FIFO read data to controller
--  Rcv_en               -- Receive enable
--  RxBusFifoRdAck       -- RX FIFO read ack
--  BusFifoEmpty         -- RX FIFO empty
--  Collision            -- Collision detected
--  DataValid            -- Data valid from PHY
--  RxError              -- Receive error
--  BusFifoData          -- RX FIFO data
--  CrcOk                -- CRC correct in the receive data
--  BusFifoRd            -- RX FIFO read
--  RxAbortRst           -- Receive abort
--  RxCrcRst             -- Receive CRC reset
--  RxCrcEn              -- RX CRC enable
--  Rx_addr_en           -- Receive address enable
--  Rx_start             -- Receive start
--  Rx_done              -- Receive complete
--  Rx_pong_ping_l       -- RX Ping/Pong buffer enable
--  Rx_DPM_ce            -- RX buffer chip enable
--  Rx_DPM_wr_data       -- RX buffer write data
--  Rx_DPM_rd_data       -- RX buffer read data
--  Rx_DPM_wr_rd_n       -- RX buffer write read enable
--  Rx_idle              -- RX idle
--  Mac_addr_ram_addr_rd -- MAC Addr RAM read address
--  Mac_addr_ram_data    -- MAC Addr RAM read data
--  Rx_buffer_ready      -- RX buffer ready to accept new packet
-------------------------------------------------------------------------------
-- ENTITY
-------------------------------------------------------------------------------
entity rx_statemachine is
  generic (
    C_DUPLEX : integer := 1
      -- 1 = full duplex, 0 = half duplex    
    );
  port (
        Clk                    : in  std_logic;
        Rst                    : in  std_logic;
        Emac_rx_rd_data_d1     : in  std_logic_vector(0 to 5);   -- 03-26-04
        Receive_enable         : out std_logic;   -- 03-26-04
        RxBusFifoRdAck         : in  std_logic;  
        BusFifoEmpty           : in  std_logic;
        Collision              : in  std_logic;
        DataValid              : in  std_logic;
        RxError                : in  std_logic;
        BusFifoData            : in  std_logic_vector(0 to 3);
        CrcOk                  : in  std_logic;
        BusFifoRd              : out std_logic;
        RxAbortRst             : out std_logic;
        RxCrcRst               : out std_logic;
        RxCrcEn                : out std_logic;   
        Rx_addr_en             : out std_logic;
        Rx_start               : out std_logic;
        Rx_done                : out std_logic;
        Rx_pong_ping_l         : in  std_logic;
        Rx_DPM_ce              : out std_logic;
        Rx_DPM_wr_data         : out std_logic_vector (0 to 3);
        Rx_DPM_rd_data         : in  std_logic_vector (0 to 3);    
        Rx_DPM_wr_rd_n         : out std_logic;
        Rx_idle                : out std_logic;
        Mac_addr_ram_addr_rd   : out std_logic_vector(0 to 3);
        Mac_addr_ram_data      : in  std_logic_vector (0 to 3);
        Rx_buffer_ready        : in  std_logic 
    
        ); 
end rx_statemachine;


architecture imp of rx_statemachine is

attribute DowngradeIPIdentifiedWarnings: string;

attribute DowngradeIPIdentifiedWarnings of imp : architecture is "yes";

type bo2sl_type is array (boolean) of std_logic;
constant bo2sl : bo2sl_type := (false => '0', true => '1');
signal idle                         : std_logic; -- state 0
signal waitForSfd1                  : std_logic; -- state 1
signal sfd1CheckBusFifoEmpty        : std_logic; -- state 2
signal waitForSfd2                  : std_logic; -- state 3
signal startReadDestAdrNib          : std_logic; -- state 4
signal rdDestAddrNib_eq_0           : std_logic;
signal rdDestAddrNib_eq_12          : std_logic;
signal startReadDataNib             : std_logic; -- state 17
signal crcCheck                     : std_logic; -- state 18
signal rxDone                       : std_logic; -- state 20
signal receiveRst                   : std_logic; -- state 21
signal rxCollision                  : std_logic; -- state 22
signal idle_D                       : std_logic; -- state 0
signal waitForSfd1_D                : std_logic; -- state 1
signal sfd1CheckBusFifoEmpty_D      : std_logic; -- state 2
signal waitForSfd2_D                : std_logic; -- state 3
signal startReadDestAdrNib_D        : std_logic; -- state 4
signal startReadDataNib_D           : std_logic; -- state 17
signal crcCheck_D                   : std_logic; -- state 18
signal rxDone_D                     : std_logic; -- state 20
signal receiveRst_D                 : std_logic; -- state 21
signal rxCollision_D                : std_logic; -- state 22
signal goto_idle_1                  : std_logic; -- state 0
signal goto_idle_2                  : std_logic; -- state 0
signal goto_idle_3                  : std_logic; -- state 0
signal goto_idle_4                  : std_logic; -- state 0
signal goto_waitForSfd1             : std_logic; -- state 1
signal goto_sfd1CheckBusFifoEmpty_1 : std_logic; -- state 2
signal goto_sfd1CheckBusFifoEmpty_2 : std_logic; -- state 2
signal goto_waitForSfd2             : std_logic; -- state 3
signal goto_startReadDestAdrNib_1   : std_logic; -- state 4
signal goto_readDestAdrNib1         : std_logic; -- state 5
signal goto_startReadDataNib_2      : std_logic; -- state 17
signal goto_crcCheck                : std_logic; -- state 18
signal goto_rxDone_3                : std_logic; -- state 20
signal goto_receiveRst_1            : std_logic; -- state 21
signal goto_receiveRst_2            : std_logic; -- state 21
signal goto_receiveRst_3            : std_logic; -- state 21
signal goto_receiveRst_5            : std_logic; -- state 21
signal goto_receiveRst_9            : std_logic; -- state 21
signal goto_receiveRst_10           : std_logic; -- state 21
signal goto_receiveRst_14           : std_logic; -- state 21
signal goto_rxCollision_1           : std_logic; -- state 22
signal goto_rxCollision_2           : std_logic; -- state 22
signal goto_rxCollision_5           : std_logic; -- state 22
signal stay_idle                    : std_logic; -- state 0
signal stay_sfd1CheckBusFifoEmpty   : std_logic; -- state 2
signal stay_startReadDestAdrNib     : std_logic; -- state 4
signal stay_startReadDataNib        : std_logic; -- state 17
signal state_machine_rst            : std_logic;
signal full_half_n                  : std_logic;
signal checkingBroadcastAdr_i       : std_logic;
signal checkingBroadcastAdr_reg     : std_logic;
signal busFifoData_is_5             : std_logic;
signal busFifoData_is_13            : std_logic;
signal busFifoData_not_5            : std_logic;
signal busFifoData_not_13           : std_logic;
signal bcastAddrGood                : std_logic;
signal ucastAddrGood                : std_logic;
signal crcokr1                      : std_logic;
signal crcokin                      : std_logic;
signal rxCrcEn_i                    : std_logic;
signal mac_addr_ram_addr_rd_D       : std_logic_vector(0 to 3);
signal rdDestAddrNib_D_t            : std_logic_vector(0 to 3);
signal rdDestAddrNib_D_t_q          : std_logic_vector(0 to 3);
signal rxDone_i                     : std_logic;
signal preamble_valid               : std_logic;
signal preamble_error_reg           : std_logic;
signal preamble_error               : std_logic;
signal busFifoData_is_5_d1          : std_logic;
signal busFifoData_is_5_d2          : std_logic;
signal busFifoData_is_5_d3          : std_logic;
signal pkt_length_cnt               : integer range 0 to 127; 
signal crc_rst                      : std_logic;

component FDR
  port (
    Q : out std_logic;
    C : in std_logic;
    D : in std_logic;
    R : in std_logic
  );
end component;

component FDS
  port (
    Q : out std_logic;
    C : in std_logic;
    D : in std_logic;
    S : in std_logic
  );
end component;

component FDRE
  port (
    Q  : out std_logic;
    C  : in std_logic;
    CE : in std_logic;
    D  : in std_logic;
    R  : in std_logic
  );
end component;

-------------------------------------------------------------------------------
-- Begin architecture
-------------------------------------------------------------------------------
begin
  
   ----------------------------------------------------------------------------
   -- CRC check
   ----------------------------------------------------------------------------
   crcokin <= ((CrcOk      -- set
              or crcokr1)  -- keep
              and (not(rxCrcEn_i) or CrcOk)); -- clear when 0
      
   crcokdelay: FDR
     port map (
       Q => crcokr1, --[out]
       C => Clk,     --[in]
       D => crcokin, --[in]
       R => crc_rst  --[in]
     );
    
   -- Added this to reset CRCokr1 before starting the next packet reception.
   crc_rst <= Rst or (not CrcOk and crcokr1);    
         
   --   RX Complete indicator
   Rx_done     <= rxDone_i; -- added Rx_done output for ping pong control      
   
     
   -- Generate rxdone only if received framelength is greater than minimum 
   -- frame length
   rxDone_i <= '1' when rxDone='1' and pkt_length_cnt=0 else
               '0';
   
   -- Check start of Frame
   -- If receive data=5
   busFifoData_is_5    <= not(BusFifoData(0)) and BusFifoData(1) and 
                          not(BusFifoData(2)) and BusFifoData(3);
                          
   -- If receive data/=5
   busFifoData_not_5   <= not(busFifoData_is_5);
   
   -- If receive data=13
   busFifoData_is_13   <= BusFifoData(0) and BusFifoData(1) and 
                     not(BusFifoData(2)) and BusFifoData(3);
                     
   -- If receive data/=13
   busFifoData_not_13       <= not(busFifoData_is_13);
   
   -- State Machine Reset
   state_machine_rst <= Rst;
   
    
   ----------------------------------------------------------------------------
   -- idle state
   ----------------------------------------------------------------------------  
   goto_idle_1 <= rxDone;
   goto_idle_2 <= receiveRst;  
   goto_idle_3 <= waitForSfd1 and (not(DataValid) or busFifoData_not_5);
   goto_idle_4 <= waitForSfd2 and (not(DataValid) or 
                  (busFifoData_not_5 and busFifoData_not_13));      
     
   stay_idle <= idle and not(goto_waitForSfd1); 
   
   idle_D <= goto_idle_1 or goto_idle_2 or goto_idle_3 or goto_idle_4 
                         or stay_idle;
   
   state0a: FDS
     port map (
       Q => idle,             --[out]
       C => Clk,              --[in]
       D => idle_D,           --[in]
       S => state_machine_rst --[in]
     );
   ----------------------------------------------------------------------------
   -- waitForSfd1 state
   ----------------------------------------------------------------------------    
   goto_waitForSfd1 <= idle and (RxBusFifoRdAck or not(BusFifoEmpty)) 
                       and (Rx_buffer_ready);
     
     
   waitForSfd1_D <= goto_waitForSfd1;
   
   state1a: FDR
     port map (
       Q => waitForSfd1,      --[out]
       C => Clk,              --[in]
       D => waitForSfd1_D,    --[in]
       R => state_machine_rst --[in]
     );    
   Rx_idle <= idle or waitForSfd1;  
   ----------------------------------------------------------------------------
   -- sfd1CheckBusFifoEmpty state
   ----------------------------------------------------------------------------    
   goto_sfd1CheckBusFifoEmpty_1 <= waitForSfd1 and busFifoData_is_5 
                                               and DataValid;
   goto_sfd1CheckBusFifoEmpty_2 <= waitForSfd2 and busFifoData_is_5 
                                               and DataValid;
   
   stay_sfd1CheckBusFifoEmpty <= sfd1CheckBusFifoEmpty   and 
                                 not(goto_rxCollision_1) and
                                 not(goto_receiveRst_1)  and
                                 not(goto_waitForSfd2);
     
   sfd1CheckBusFifoEmpty_D <= goto_sfd1CheckBusFifoEmpty_1 or 
                              goto_sfd1CheckBusFifoEmpty_2 or 
                              stay_sfd1CheckBusFifoEmpty;
   
   state2a: FDR
     port map (
       Q => sfd1CheckBusFifoEmpty,    --[out]
       C => Clk,                      --[in]
       D => sfd1CheckBusFifoEmpty_D,  --[in]
       R => state_machine_rst         --[in]
     ); 
   ----------------------------------------------------------------------------
   -- waitForSfd2 state
   ----------------------------------------------------------------------------    
   goto_waitForSfd2 <= sfd1CheckBusFifoEmpty and not(goto_rxCollision_1) and 
                      not(goto_receiveRst_1) and (RxBusFifoRdAck or 
                                                  not(BusFifoEmpty)) and 
                                                  busFifoData_is_5;
   
   waitForSfd2_D <= goto_waitForSfd2;
   
   state3a: FDR
     port map (
       Q => waitForSfd2,      --[out]
       C => Clk,              --[in]
       D => waitForSfd2_D,    --[in]
       R => state_machine_rst --[in]
     );  
   
   
   ----------------------------------------------------------------------------
   --startReadDestAdrNib state
   ----------------------------------------------------------------------------    
   goto_startReadDestAdrNib_1 <= waitForSfd2 and busFifoData_is_13  
                                             and preamble_valid
                                             and DataValid;
   
   stay_startReadDestAdrNib <= startReadDestAdrNib     and 
                               not(goto_rxCollision_2) and
                               not(goto_receiveRst_2)  and 
                               not(goto_readDestAdrNib1);
     
   startReadDestAdrNib_D <= goto_startReadDestAdrNib_1  or 
                            stay_startReadDestAdrNib;
   
   state4a: FDR
     port map (
       Q => startReadDestAdrNib,    --[out]
       C => Clk,                    --[in]
       D => startReadDestAdrNib_D,  --[in]
       R => state_machine_rst       --[in]
     );
   ----------------------------------------------------------------------------
   --readDestAdrNib1 state
   ----------------------------------------------------------------------------    
   goto_readDestAdrNib1 <= startReadDestAdrNib     and 
                           not(goto_rxCollision_2) and 
                           not(goto_receiveRst_2)  and 
                           RxBusFifoRdAck;
     
   
   rdDestAddrNib_eq_0  <= bo2sl(rdDestAddrNib_D_t_q = "0000");
   rdDestAddrNib_eq_12 <= bo2sl(rdDestAddrNib_D_t_q = "1011");
   
   ----------------------------------------------------------------------------
   -- STATE_REG_PROCESS
   ----------------------------------------------------------------------------
   -- Registeting the read destination address.
   ----------------------------------------------------------------------------
   STATE_REG_PROCESS : process (Clk)
   begin
      if (Clk'event and Clk='1') then
         if (state_machine_rst = '1' or 
             goto_startReadDestAdrNib_1 = '1') then  
            rdDestAddrNib_D_t_q <= "0000";
         else
            rdDestAddrNib_D_t_q <= rdDestAddrNib_D_t;
         end if;
      end if;
   end process STATE_REG_PROCESS;
   
   
   ----------------------------------------------------------------------------
   -- FSM_CMB_PROCESS
   ----------------------------------------------------------------------------
   -- This process generate read destination address for the MAC address RAM
   -- for the received frame. 
   ----------------------------------------------------------------------------
   FSM_CMB_PROCESS : process (startReadDestAdrNib,goto_rxCollision_2,
            goto_receiveRst_2,RxBusFifoRdAck,goto_receiveRst_3,bcastAddrGood,
            ucastAddrGood,goto_receiveRst_5,
            rdDestAddrNib_D_t_q)
   begin
   ----
   
      rdDestAddrNib_D_t <= rdDestAddrNib_D_t_q;

      case (rdDestAddrNib_D_t_q) is
   
         when "0000" => 
         
            if (startReadDestAdrNib and not(goto_rxCollision_2) and 
                not(goto_receiveRst_2) and RxBusFifoRdAck) = '1' then
               rdDestAddrNib_D_t <= "0001";
            else
               rdDestAddrNib_D_t <= "0000";
            end if;
         
         when "0001" =>  
         
            if (RxBusFifoRdAck and (bcastAddrGood or ucastAddrGood) and
                not(goto_receiveRst_5) and not(goto_receiveRst_3)) = '1' then
               rdDestAddrNib_D_t <= "0010";
            elsif goto_receiveRst_5='1' or goto_receiveRst_3='1' then
               rdDestAddrNib_D_t <= "0000";
            else
                 rdDestAddrNib_D_t <= "0001";
            end if;  
                        
                          
         
         when "0010" =>  
            if (RxBusFifoRdAck and (bcastAddrGood or ucastAddrGood) and 
                not(goto_receiveRst_5) and not(goto_receiveRst_3)) = '1' then
               rdDestAddrNib_D_t <= "0011";
            elsif goto_receiveRst_5='1' or goto_receiveRst_3='1' then
               rdDestAddrNib_D_t <= "0000";
            else
               rdDestAddrNib_D_t <= "0010";
            end if;
                          
         
         when "0011" =>  
            if (RxBusFifoRdAck and (bcastAddrGood or ucastAddrGood) and 
                not(goto_receiveRst_5) and not(goto_receiveRst_3)) = '1' then
               rdDestAddrNib_D_t <= "0100";
            elsif goto_receiveRst_5='1' or goto_receiveRst_3='1' then
               rdDestAddrNib_D_t <= "0000";
            else
               rdDestAddrNib_D_t <= "0011";
            end if;
                          
         
         when "0100" =>  
            if (RxBusFifoRdAck and (bcastAddrGood or ucastAddrGood) and 
                not(goto_receiveRst_5) and not(goto_receiveRst_3)) = '1' then
               rdDestAddrNib_D_t <= "0101";
            elsif goto_receiveRst_5='1' or goto_receiveRst_3='1' then
               rdDestAddrNib_D_t <= "0000";
            else
               rdDestAddrNib_D_t <= "0100";
            end if;
                          
         
         when "0101" =>  
            if (RxBusFifoRdAck and (bcastAddrGood or ucastAddrGood) and 
                not(goto_receiveRst_5) and not(goto_receiveRst_3)) = '1' then
               rdDestAddrNib_D_t <= "0110";
            elsif goto_receiveRst_5='1' or goto_receiveRst_3='1' then
               rdDestAddrNib_D_t <= "0000";
            else
               rdDestAddrNib_D_t <= "0101";
            end if;
                          
         
         when "0110" =>  
            if (RxBusFifoRdAck and (bcastAddrGood or ucastAddrGood) and 
                not(goto_receiveRst_5) and not(goto_receiveRst_3)) = '1' then
               rdDestAddrNib_D_t <= "0111";
            elsif goto_receiveRst_5='1' or goto_receiveRst_3='1' then
               rdDestAddrNib_D_t <= "0000";
            else
               rdDestAddrNib_D_t <= "0110";
            end if;
                          
         when "0111" =>  
            if (RxBusFifoRdAck and (bcastAddrGood or ucastAddrGood) and 
                not(goto_receiveRst_5) and not(goto_receiveRst_3)) = '1' then
               rdDestAddrNib_D_t <= "1000";
            elsif goto_receiveRst_5='1' or goto_receiveRst_3='1' then
               rdDestAddrNib_D_t <= "0000";
            else
               rdDestAddrNib_D_t <= "0111";
             end if;
                          
         when "1000" =>  
            if (RxBusFifoRdAck and (bcastAddrGood or ucastAddrGood) and 
                not(goto_receiveRst_5) and not(goto_receiveRst_3)) = '1' then
               rdDestAddrNib_D_t <= "1001";
            elsif goto_receiveRst_5='1' or goto_receiveRst_3='1' then
               rdDestAddrNib_D_t <= "0000";
            else
               rdDestAddrNib_D_t <= "1000";
            end if;
         
         when "1001" =>  
            if (RxBusFifoRdAck and (bcastAddrGood or ucastAddrGood) and 
                not(goto_receiveRst_5) and not(goto_receiveRst_3)) = '1' then
               rdDestAddrNib_D_t <= "1010";
            elsif goto_receiveRst_5='1' or goto_receiveRst_3='1' then
               rdDestAddrNib_D_t <= "0000";
            else
               rdDestAddrNib_D_t <= "1001";
            end if;
                          
         when "1010" =>  
            if (RxBusFifoRdAck and (bcastAddrGood or ucastAddrGood) and 
                not(goto_receiveRst_5) and not(goto_receiveRst_3)) = '1' then
               rdDestAddrNib_D_t <= "1011";
            elsif goto_receiveRst_5='1' or goto_receiveRst_3='1' then
               rdDestAddrNib_D_t <= "0000";
            else
               rdDestAddrNib_D_t <= "1010";
            end if;
                          
         when "1011" =>  
            if (RxBusFifoRdAck and (bcastAddrGood or ucastAddrGood) and 
                not(goto_receiveRst_5) and not(goto_receiveRst_3)) = '1' then
               rdDestAddrNib_D_t <= "1100";
            elsif goto_receiveRst_5='1' or goto_receiveRst_3='1' then
               rdDestAddrNib_D_t <= "0000";
            else
               rdDestAddrNib_D_t <= "1011";
            end if;
                          
         when "1100" =>  
            if (RxBusFifoRdAck and (bcastAddrGood or ucastAddrGood) and 
                not(goto_receiveRst_5) and not(goto_receiveRst_3)) = '1' then
               rdDestAddrNib_D_t <= "0000";
            elsif goto_receiveRst_5='1' or goto_receiveRst_3='1' then
               rdDestAddrNib_D_t <= "0000";
            else
               rdDestAddrNib_D_t <= "1100";
            end if;
         
         
         when others =>  null;
        end case;
    end process FSM_CMB_PROCESS;
   
    
   ----------------------------------------------------------------------------
   --startReadDataNib state
   ----------------------------------------------------------------------------      
   goto_startReadDataNib_2 <= rdDestAddrNib_eq_12 and RxBusFifoRdAck and
     (bcastAddrGood or ucastAddrGood) and not(goto_receiveRst_5) and 
     not(goto_receiveRst_3);
   
   stay_startReadDataNib <= startReadDataNib and not(goto_rxCollision_5) 
     and not(goto_receiveRst_9) and DataValid;
     
   startReadDataNib_D <= goto_startReadDataNib_2
     or stay_startReadDataNib;
   
   state17a: FDR
     port map (
       Q => startReadDataNib,    --[out]
       C => Clk,                 --[in]
       D => startReadDataNib_D,  --[in]
       R => state_machine_rst    --[in]
     );  
   ----------------------------------------------------------------------------
   --crcCheck state
   ----------------------------------------------------------------------------      
   goto_crcCheck <= startReadDataNib and not(DataValid) ;
   
   goto_receiveRst_1 <= sfd1CheckBusFifoEmpty and not(goto_rxCollision_1) 
                                              and RxError;
   
   goto_receiveRst_2 <= startReadDestAdrNib   and not(goto_rxCollision_2) 
                                              and RxError;
   
   goto_receiveRst_9 <= startReadDataNib      and not(goto_rxCollision_5) 
                                              and RxError;

   crcCheck_D <= goto_crcCheck or goto_receiveRst_1 or 
                                  goto_receiveRst_2 or 
                                  goto_receiveRst_9;
   
   state18a: FDR
     port map (
       Q => crcCheck,         --[out]
       C => Clk,              --[in]
       D => crcCheck_D,       --[in]
       R => state_machine_rst --[in]
     );  

   -------------------------------------------------------------------------------
   --rxDone state
   -------------------------------------------------------------------------------      
   --goto_rxDone_3 <= writeFinalData ;
   goto_rxDone_3 <= crcCheck and crcokr1;
     
   rxDone_D <= goto_rxDone_3 ;
   
   
   state20a: FDR
     port map (
       Q => rxDone,           --[out]
       C => Clk,              --[in]
       D => rxDone_D,         --[in]
       R => state_machine_rst --[in]
     );  
   ----------------------------------------------------------------------------
   --rxCollision state
   ----------------------------------------------------------------------------    
   full_half_n <= '1'when C_DUPLEX = 1 else
                  '0';
   goto_rxCollision_1 <= sfd1CheckBusFifoEmpty and Collision 
                                               and not(full_half_n);
   
   goto_rxCollision_2 <= startReadDestAdrNib   and Collision 
                                               and not(full_half_n);
   
   goto_rxCollision_5 <= startReadDataNib      and Collision 
                                               and not(full_half_n);
     
   rxCollision_D <= goto_rxCollision_1 or goto_rxCollision_2 or 
                    goto_rxCollision_5;
   
   state21a: FDR
     port map (
       Q => rxCollision,       --[out]
       C => Clk,               --[in]
       D => rxCollision_D,     --[in]
       R => state_machine_rst  --[in]
     );  
   ----------------------------------------------------------------------------
   --receiveRst state
   ----------------------------------------------------------------------------      
   goto_receiveRst_3 <= not rdDestAddrNib_eq_0 and not(DataValid);
   
   goto_receiveRst_5 <= not rdDestAddrNib_eq_0 and 
                        not(BusFifoEmpty)      and 
                        not(bcastAddrGood or ucastAddrGood);
   
   goto_receiveRst_10<= crcCheck and not(crcokr1);
   goto_receiveRst_14<= rxCollision;

     
   receiveRst_D <= goto_receiveRst_3  or
                   goto_receiveRst_5  or 
                   goto_receiveRst_10 or 
                   goto_receiveRst_14 or
                   preamble_error_reg;
   
   state22a: FDR
     port map (
       Q => receiveRst,        --[out]
       C => Clk,               --[in]
       D => receiveRst_D,      --[in]
       R => state_machine_rst  --[in]
     );  
   ----------------------------------------------------------------------------   
   -- end of states
   ----------------------------------------------------------------------------   
   

   ----------------------------------------------------------------------------
   -- BROADCAST_ADDR_REG
   ----------------------------------------------------------------------------
   -- This process generate control signals for the state machine.
   ----------------------------------------------------------------------------
   BROADCAST_ADDR_REG : process (Clk)
   begin  --
      if (Clk'event and Clk = '1') then     -- rising clock edge
         if (Rst = '1') then
            checkingBroadcastAdr_reg <= '0';
         else
            checkingBroadcastAdr_reg <= checkingBroadcastAdr_i;          
         end if;
      end if;
   end process BROADCAST_ADDR_REG;
   
   ----------------------------------------------------------------------------
   -- RX_FSMD_PROCESS
   ----------------------------------------------------------------------------
   -- This process generate control signals for the state machine.
   ----------------------------------------------------------------------------
   RX_FSMD_PROCESS : process( DataValid,RxBusFifoRdAck,idle, 
                              startReadDestAdrNib, startReadDataNib, 
                              sfd1CheckBusFifoEmpty, rxDone, receiveRst, 
                              waitForSfd2, Emac_rx_rd_data_d1, 
                              checkingBroadcastAdr_reg, rdDestAddrNib_eq_0,
                              rdDestAddrNib_D_t_q) 
   begin
   
      -- Reset RX CRC in idle state
      if (idle = '1') then
         RxCrcRst           <=  '1';
      else
         RxCrcRst           <=  '0';
      end if;
      
      
      -- RX CRC enable
      if ((( startReadDestAdrNib or (not rdDestAddrNib_eq_0) or 
           (startReadDataNib and DataValid)) 
           and RxBusFifoRdAck) = '1')   then
         RxCrcEn     <=   '1';
         rxCrcEn_i   <=   '1';
      else
         RxCrcEn     <=   '0';
         rxCrcEn_i   <=   '0';
      end if; 
      
      -- RX buffer FIFO read enable
      if ((idle = '1') or
          (sfd1CheckBusFifoEmpty = '1') or 
          (not rdDestAddrNib_eq_0 = '1') or
          (rxDone = '1') or   -- 03-26-04
          (startReadDestAdrNib = '1') or
          (startReadDataNib = '1')) and (RxBusFifoRdAck = '0')then
         BusFifoRd          <=   '1';
      else
         BusFifoRd          <=   '0';
      end if; 
      
      -- RX abort reset
      if (receiveRst = '1') then
         RxAbortRst <= '1';
      else
         RxAbortRst <= '0';
      end if;   

      
      -- RX buffer address enable
      if RxBusFifoRdAck = '1' and
         (
          (startReadDestAdrNib = '1') or  -- 03-26-04
          (not rdDestAddrNib_eq_0 = '1') or
          (startReadDataNib = '1')
         ) then
         Rx_addr_en <= '1'; --enable address increment
      else
         Rx_addr_en <= '0';
      end if; 
      
      -- Generate RX start after SFD is detected
      if (waitForSfd2 = '1')then
         Rx_start <= '1'; -- reset address to 0 for start of receive
      else
         Rx_start <= '0';
      end if;
      
      -- RX buffer chip enable
      if (idle = '1') or
         ((
           (startReadDestAdrNib = '1') or  -- 03-26-04
           (not rdDestAddrNib_eq_0 = '1') or
           (startReadDataNib = '1')
          ) and (RxBusFifoRdAck = '1')
         ) then
         Rx_DPM_ce <= '1';
      else
         Rx_DPM_ce <= '0';
      end if;
      
      -- RX buffer read/write enable
      if (startReadDestAdrNib = '1') or  -- 03-26-04
         (not rdDestAddrNib_eq_0 = '1') or
         (startReadDataNib = '1') then
         Rx_DPM_wr_rd_n <= '1';   
      else   
         Rx_DPM_wr_rd_n <= '0';
      end if;
      
      -- RX buffer chip enable
      if (idle = '1') then
         checkingBroadcastAdr_i <= '0';  -- reset
      -- 06-09-04 Use delayed data for compare
      elsif (rdDestAddrNib_D_t_q = x"1" and 
             Emac_rx_rd_data_d1(0 to 3) = x"f") then   
         checkingBroadcastAdr_i <= '1'; -- set
      else
         checkingBroadcastAdr_i <= checkingBroadcastAdr_reg; -- stay the same
      end if;   
   
   end process RX_FSMD_PROCESS;
   
   -- write data to Receive DPRAM
   Rx_DPM_wr_data  <= BusFifoData;
   
   ----------------------------------------------------------------------------
   -- MARAR_PROC
   ----------------------------------------------------------------------------
   -- This process generate MAC RAM address to get mac addres to compare with
   -- incoming frame destination address
   ----------------------------------------------------------------------------
   MARAR_PROC : process (rdDestAddrNib_D_t, idle_D, startReadDestAdrNib_D)
   begin
      case rdDestAddrNib_D_t is
         when "0001" => mac_addr_ram_addr_rd_D <= x"0";
         when "0010" => mac_addr_ram_addr_rd_D <= x"1";
         when "0011" => mac_addr_ram_addr_rd_D <= x"2";
         when "0100" => mac_addr_ram_addr_rd_D <= x"3";
         when "0101" => mac_addr_ram_addr_rd_D <= x"4";
         when "0110" => mac_addr_ram_addr_rd_D <= x"5";
         when "0111" => mac_addr_ram_addr_rd_D <= x"6";
         when "1000" => mac_addr_ram_addr_rd_D <= x"7";
         when "1001" => mac_addr_ram_addr_rd_D <= x"8";
         when "1010" => mac_addr_ram_addr_rd_D <= x"9";
         when "1011" => mac_addr_ram_addr_rd_D <= x"a";
         when "1100" => mac_addr_ram_addr_rd_D <= x"b";
         when others => mac_addr_ram_addr_rd_D <= x"0";
      end case;
      
      -- Reset the address in idle or start of new frame
      if (idle_D or startReadDestAdrNib_D) = '1' then
         mac_addr_ram_addr_rd_D <= x"0";
      end if;
   end process MARAR_PROC;
   
   ----------------------------------------------------------------------------
   -- OUTPUT_REG
   ----------------------------------------------------------------------------
   -- Registerit the mac_addr_ram_addr_rd
   ----------------------------------------------------------------------------
   OUTPUT_REG:process (Clk)
   begin
      if Clk'event and Clk = '1' then
         if Rst = '1' then
            Mac_addr_ram_addr_rd <= (others => '0');
         else
            Mac_addr_ram_addr_rd <= mac_addr_ram_addr_rd_D;
         end if;
      end if;
   end process OUTPUT_REG;
   
   ----------------------------------------------------------------------------
   -- Check if the incoming packet is broadcast packet
   ----------------------------------------------------------------------------
   bcastAddrGood <= '1' when checkingBroadcastAdr_i = '1' and 
                             Emac_rx_rd_data_d1(0 to 3) = x"F" else -- 03-26-04
                    '0';   
   
   ----------------------------------------------------------------------------
   -- Check if the incoming packet is unicast and address matches to core 
   -- MAC address
   ----------------------------------------------------------------------------
   ucastAddrGood <= '1' when checkingBroadcastAdr_i = '0' and 
                            (Emac_rx_rd_data_d1(0 to 3) = Mac_addr_ram_data) 
                                                              else  -- 03-26-04
                    '0';    
                    
   -- Genarate Receive enable
   Receive_enable <= not(crcCheck or rxDone or receiveRst); 
   
   
   ----------------------------------------------------------------------------
   -- PROCESS : PKT_LENGTH_COUNTER 
   ----------------------------------------------------------------------------
   -- This counter is used to check if the receive packet length is greater 
   -- minimum packet length (64 byte - 128 nibble)
   ----------------------------------------------------------------------------
   PKT_LENGTH_COUNTER : process(Clk)
   begin
      if (Clk'event and Clk = '1') then
         if (Rst = '1' or preamble_error_reg = '1' ) then
            pkt_length_cnt <= 0;
         elsif goto_readDestAdrNib1 = '1' then  -- load the counter for 
            pkt_length_cnt <= 127;              -- minimum packet length
         elsif (rxCrcEn_i='1') then             -- Enable Down Counter   
            if (pkt_length_cnt = 0) then  
               pkt_length_cnt <= 0;
            else
               pkt_length_cnt <= pkt_length_cnt - 1;
            end if;
         end if;
      end if;
   end process;


   ----------------------------------------------------------------------------
   -- PROCESS : SFD_CHECK_REG 
   ----------------------------------------------------------------------------
   -- This process registers the preamble nibble to checl if atleast last 2 
   -- preamble nibbles are valid before the SFD nibble. 
   ----------------------------------------------------------------------------
   SFD_CHECK_REG : process(Clk)
   begin
      if (Clk'event and Clk = '1') then
         if (Rst = '1' ) then
            busFifoData_is_5_d1 <= '0';
            busFifoData_is_5_d2 <= '0';
            busFifoData_is_5_d3 <= '0';
         elsif RxBusFifoRdAck = '1' then  
            busFifoData_is_5_d1 <= busFifoData_is_5;
            busFifoData_is_5_d2 <= busFifoData_is_5_d1;
            busFifoData_is_5_d3 <= busFifoData_is_5_d2;
         end if;
      end if;
   end process;
   

   preamble: FDR
     port map (
       Q => preamble_error_reg, --[out]
       C => Clk,                --[in]
       D => preamble_error,     --[in]
       R => state_machine_rst   --[in]
     );  

   -- Premable valid
   preamble_valid <= (busFifoData_is_5_d1)  and 
                      busFifoData_is_13; 

   -- Premable Error 
   preamble_error <=  (not busFifoData_is_5    and 
                           busFifoData_is_5_d1 and 
                       not busFifoData_is_13) and waitForSfd2 ; 

end imp;


-------------------------------------------------------------------------------
-- rx_intrfce - entity/architecture pair
-------------------------------------------------------------------------------
--  ***************************************************************************
--  ** DISCLAIMER OF LIABILITY                                               **
--  **                                                                       **
--  **  This file contains proprietary and confidential information of       **
--  **  Xilinx, Inc. ("Xilinx"), that is distributed under a license         **
--  **  from Xilinx, and may be used, copied and/or disclosed only           **
--  **  pursuant to the terms of a valid license agreement with Xilinx.      **
--  **                                                                       **
--  **  XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION                **
--  **  ("MATERIALS") "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER           **
--  **  EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING WITHOUT                  **
--  **  LIMITATION, ANY WARRANTY WITH RESPECT TO NONINFRINGEMENT,            **
--  **  MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE. Xilinx        **
--  **  does not warrant that functions included in the Materials will       **
--  **  meet the requirements of Licensee, or that the operation of the      **
--  **  Materials will be uninterrupted or error-free, or that defects       **
--  **  in the Materials will be corrected. Furthermore, Xilinx does         **
--  **  not warrant or make any representations regarding use, or the        **
--  **  results of the use, of the Materials in terms of correctness,        **
--  **  accuracy, reliability or otherwise.                                  **
--  **                                                                       **
--  **  Xilinx products are not designed or intended to be fail-safe,        **
--  **  or for use in any application requiring fail-safe performance,       **
--  **  such as life-support or safety devices or systems, Class III         **
--  **  medical devices, nuclear facilities, applications related to         **
--  **  the deployment of airbags, or any other applications that could      **
--  **  lead to death, personal injury or severe property or                 **
--  **  environmental damage (individually and collectively, "critical       **
--  **  applications"). Customer assumes the sole risk and liability         **
--  **  of any use of Xilinx products in critical applications,              **
--  **  subject only to applicable laws and regulations governing            **
--  **  limitations on product liability.                                    **
--  **                                                                       **
--  **  Copyright 2010 Xilinx, Inc.                                          **
--  **  All rights reserved.                                                 **
--  **                                                                       **
--  **  This disclaimer and copyright notice must be retained as part        **
--  **  of this file at all times.                                           **
--  ***************************************************************************
--
-------------------------------------------------------------------------------
-- Filename     : rx_intrfce.vhd
-- Version      : v2.0
-- Description  : This is the ethernet receive interface. 
-- VHDL-Standard: VHDL'93
-------------------------------------------------------------------------------
-- Structure:   
--
--  axi_ethernetlite.vhd
--      \
--      \-- axi_interface.vhd
--      \-- xemac.vhd
--           \
--           \-- mdio_if.vhd
--           \-- emac_dpram.vhd                     
--           \    \                     
--           \    \-- RAMB16_S4_S36
--           \                          
--           \    
--           \-- emac.vhd                     
--                \                     
--                \-- MacAddrRAM                   
--                \-- receive.vhd
--                \      rx_statemachine.vhd
--                \      rx_intrfce.vhd
--                \         async_fifo_fg.vhd
--                \      crcgenrx.vhd
--                \                     
--                \-- transmit.vhd
--                       crcgentx.vhd
--                          crcnibshiftreg
--                       tx_intrfce.vhd
--                          async_fifo_fg.vhd
--                       tx_statemachine.vhd
--                       deferral.vhd
--                          cntr5bit.vhd
--                          defer_state.vhd
--                       bocntr.vhd
--                          lfsr16.vhd
--                       msh_cnt.vhd
--                       ld_arith_reg.vhd
--
-------------------------------------------------------------------------------
-- Author:    PVK
-- History:    
-- PVK        06/07/2010     First Version
-- ^^^^^^
-- First version.  
-- ~~~~~~
-------------------------------------------------------------------------------
-- Naming Conventions:
--      active low signals:                     "*_n"
--      clock signals:                          "clk", "clk_div#", "clk_#x" 
--      reset signals:                          "rst", "rst_n" 
--      generics:                               "C_*" 
--      user defined types:                     "*_TYPE" 
--      state machine next state:               "*_ns" 
--      state machine current state:            "*_cs" 
--      combinatorial signals:                  "*_com" 
--      pipelined or register delay signals:    "*_d#" 
--      counter signals:                        "*cnt*"
--      clock enable signals:                   "*_ce" 
--      internal version of output port         "*_i"
--      device pins:                            "*_pin" 
--      ports:                                  - Names begin with Uppercase 
--      processes:                              "*_PROCESS" 
--      component instantiations:               "<ENTITY_>I_<#|FUNC>
-------------------------------------------------------------------------------
-- 
library ieee;
use ieee.std_logic_1164.all;

-------------------------------------------------------------------------------
-- axi_ethernetlite_v3_0_18 library is used for axi_ethernetlite_v3_0_18 
-- component declarations
-------------------------------------------------------------------------------
library axi_ethernetlite_v3_0_18;
use axi_ethernetlite_v3_0_18.all;

-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
library lib_cdc_v1_0_2;
use lib_cdc_v1_0_2.all;

library lib_fifo_v1_0_14;
use lib_fifo_v1_0_14.all;

--library fifo_generator_v11_0; -- FIFO HIER
--use fifo_generator_v11_0.all;
-- synopsys translate_off
-- Library XilinxCoreLib;
library unisim;
--library simprim;
-- synopsys translate_on

-------------------------------------------------------------------------------
-- Definition of Ports:
--
--  Clk                 -- System Clock
--  Rst                 -- System Reset
--  Phy_rx_clk          -- PHY RX Clock
--  InternalWrapEn      -- Internal wrap enable
--  Phy_rx_er           -- Receive error
--  Phy_dv              -- Ethernet receive enable
--  Phy_rx_data         -- Ethernet receive data
--  Rcv_en              -- Receive enable
--  Fifo_empty          -- RX FIFO empty
--  Fifo_full           -- RX FIFO full
--  Emac_rx_rd          -- RX FIFO Read enable
--  Emac_rx_rd_data     -- RX FIFO read data to controller
--  RdAck               -- RX FIFO read ack
-------------------------------------------------------------------------------
-- ENTITY
-------------------------------------------------------------------------------
entity rx_intrfce is
  generic 
    (
    C_FAMILY        : string  := "virtex6"  
    );
  port (
    Clk             : in std_logic;
    Rst             : in std_logic;
    Phy_rx_clk      : in std_logic;
    InternalWrapEn  : in std_logic;
    Phy_rx_er       : in std_logic;
    Phy_dv          : in std_logic;
    Phy_rx_data     : in std_logic_vector (0 to 3);
    Rcv_en          : in std_logic;
    Fifo_empty      : out std_logic;
    Fifo_full       : out std_logic;
    Emac_rx_rd      : in std_logic;
    Emac_rx_rd_data : out std_logic_vector (0 to 5);
    RdAck           : out std_logic
    );
end rx_intrfce;

-------------------------------------------------------------------------------
-- Definition of Generics:
--          No Generics were used for this Entity.
--
-- Definition of Ports:
--         
-------------------------------------------------------------------------------

architecture implementation of rx_intrfce is

attribute DowngradeIPIdentifiedWarnings: string;

attribute DowngradeIPIdentifiedWarnings of implementation : architecture is "yes";

-------------------------------------------------------------------------------
--  Constant Declarations
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
--  Signal and Type Declarations
-------------------------------------------------------------------------------

signal rxBusCombo      : std_logic_vector (0 to 5);
signal rx_wr_en        : std_logic;
signal rx_data         : std_logic_vector (0 to 5);
signal rx_fifo_full    : std_logic;
signal rx_fifo_empty   : std_logic;
signal rx_rd_ack       : std_logic;
signal rst_s           : std_logic;

-------------------------------------------------------------------------------
-- Component Declarations
-------------------------------------------------------------------------------
-- The following components are the building blocks of the EMAC
-------------------------------------------------------------------------------
--FIFI HIER
--component async_fifo_eth
--  port (
--    rst     : in std_logic;
--    wr_clk  : in std_logic;
--    rd_clk  : in std_logic;
--    din     : in std_logic_vector(5 downto 0);
--    wr_en   : in std_logic;
--    rd_en   : in std_logic;
--    dout    : out std_logic_vector(5 downto 0);
--    full    : out std_logic;
--    empty   : out std_logic;
--    valid   : out std_logic
--  );
--end component;


begin
   ----------------------------------------------------------------------------
   -- CDC module for syncing reset in wr clk domain
   ----------------------------------------------------------------------------
  CDC_FIFO_RST: entity  lib_cdc_v1_0_2.cdc_sync
  generic map (
    C_CDC_TYPE           => 1,
    C_RESET_STATE        => 0,
    C_SINGLE_BIT         => 1,
    C_FLOP_INPUT         => 0,
    C_VECTOR_WIDTH       => 1,
    C_MTBF_STAGES        => 4
            )
  port map(
    prmry_aclk            => '1',
    prmry_resetn          => '1',
    prmry_in              => Rst,
    prmry_ack             => open,
    scndry_out            => rst_s,
    scndry_aclk           => Phy_rx_clk,
    scndry_resetn         => '1',
    prmry_vect_in         => (OTHERS => '0'),
    scndry_vect_out       => open
     );

   I_RX_FIFO: entity lib_fifo_v1_0_14.async_fifo_fg
     generic map(
       C_ALLOW_2N_DEPTH   => 0,  -- New paramter to leverage FIFO Gen 2**N depth
       C_FAMILY           => C_FAMILY,  -- new for FIFO Gen
       C_DATA_WIDTH       => 6,
       C_ENABLE_RLOCS     => 0,  -- not supported in FG
       C_FIFO_DEPTH       => 16,
       C_HAS_ALMOST_EMPTY => 0,
       C_HAS_ALMOST_FULL  => 0,
       C_HAS_RD_ACK       => 1,
       C_HAS_RD_COUNT     => 0,
       C_EN_SAFETY_CKT    => 1,  
       C_HAS_RD_ERR       => 0,
       C_HAS_WR_ACK       => 0,
       C_HAS_WR_COUNT     => 0,
       C_HAS_WR_ERR       => 0,
       C_RD_ACK_LOW       => 0,
       C_RD_COUNT_WIDTH   => 2,
       C_RD_ERR_LOW       => 0,
       C_USE_BLOCKMEM     => 0,  -- 0 = distributed RAM, 1 = BRAM
       C_WR_ACK_LOW       => 0,
       C_WR_COUNT_WIDTH   => 2,
       C_WR_ERR_LOW       => 0,
       C_XPM_FIFO	  => 1
     )
     port map(
       Din            => rxBusCombo, 
       Wr_en          => rx_wr_en,
       Wr_clk         => Phy_rx_clk,
       Rd_en          => Emac_rx_rd,
       Rd_clk         => Clk,
       Ainit          => rst_s,   
       Dout           => rx_data,
       Full           => rx_fifo_full,
       Empty          => rx_fifo_empty,
       Almost_full    => open,
       Almost_empty   => open, 
       Wr_count       => open,
       Rd_count       => open,
       Rd_ack         => rx_rd_ack,
       Rd_err         => open,
       Wr_ack         => open,
       Wr_err         => open
     );

-- FIFO HIER
-- I_RX_FIFO : async_fifo_eth
--   port map(    
--       din            => rxBusCombo, 
--       wr_en          => rx_wr_en,
--       wr_clk         => Phy_rx_clk,
--       rd_en          => Emac_rx_rd,
--       rd_clk         => Clk,
--       rst            => Rst,   
--       dout           => rx_data,
--       full           => rx_fifo_full,
--       empty          => rx_fifo_empty,
--       valid          => rx_rd_ack
--   );

rxBusCombo      <= (Phy_rx_data & Phy_dv & Phy_rx_er);
Emac_rx_rd_data <= rx_data; 
RdAck           <= rx_rd_ack; 
Fifo_full       <= rx_fifo_full; 
Fifo_empty      <= rx_fifo_empty; 
--rx_wr_en        <= Rcv_en; 
rx_wr_en        <= not(rx_fifo_full); -- having this as Rcv_en is generated in lite_clock domain and passing to FIFO working in rx_clk domain

           
end implementation;


-------------------------------------------------------------------------------
-- ram16x4 - entity/architecture pair
-------------------------------------------------------------------------------
--  ***************************************************************************
--  ** DISCLAIMER OF LIABILITY                                               **
--  **                                                                       **
--  **  This file contains proprietary and confidential information of       **
--  **  Xilinx, Inc. ("Xilinx"), that is distributed under a license         **
--  **  from Xilinx, and may be used, copied and/or disclosed only           **
--  **  pursuant to the terms of a valid license agreement with Xilinx.      **
--  **                                                                       **
--  **  XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION                **
--  **  ("MATERIALS") "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER           **
--  **  EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING WITHOUT                  **
--  **  LIMITATION, ANY WARRANTY WITH RESPECT TO NONINFRINGEMENT,            **
--  **  MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE. Xilinx        **
--  **  does not warrant that functions included in the Materials will       **
--  **  meet the requirements of Licensee, or that the operation of the      **
--  **  Materials will be uninterrupted or error-free, or that defects       **
--  **  in the Materials will be corrected. Furthermore, Xilinx does         **
--  **  not warrant or make any representations regarding use, or the        **
--  **  results of the use, of the Materials in terms of correctness,        **
--  **  accuracy, reliability or otherwise.                                  **
--  **                                                                       **
--  **  Xilinx products are not designed or intended to be fail-safe,        **
--  **  or for use in any application requiring fail-safe performance,       **
--  **  such as life-support or safety devices or systems, Class III         **
--  **  medical devices, nuclear facilities, applications related to         **
--  **  the deployment of airbags, or any other applications that could      **
--  **  lead to death, personal injury or severe property or                 **
--  **  environmental damage (individually and collectively, "critical       **
--  **  applications"). Customer assumes the sole risk and liability         **
--  **  of any use of Xilinx products in critical applications,              **
--  **  subject only to applicable laws and regulations governing            **
--  **  limitations on product liability.                                    **
--  **                                                                       **
--  **  Copyright 2007, 2008, 2009 Xilinx, Inc.                              **
--  **  All rights reserved.                                                 **
--  **                                                                       **
--  **  This disclaimer and copyright notice must be retained as part        **
--  **  of this file at all times.                                           **
--  ***************************************************************************
--
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-- Filename   : ram16x4.vhd
-- Version    : v4.00.a
-- Description: This is a LUT RAM design to provide 4 bits wide and 16 bits
--              deep memory structue. The initial string for rom16x4 is 
--              specially designed to ease the initialization of this memory.  
--              The initialization value is taken from the "INIT_XX" string.  
--              Each string is read in the standard Xilinx format, which is to
--              take the right-most character as the least significant bit. 
--              INIT_00 is for address 0 to address 3, INIT_01 is for address 
--              4 to address 7, ..., INIT_03 is for address 12 to address 15.
--              Uses 16 LUTs (16 RAM16x1)
-- VHDL-Standard:  VHDL'93
-------------------------------------------------------------------------------
-- Structure:   
--
--  axi_ethernetlite.vhd
--      \
--      \-- axi_interface.vhd
--      \-- xemac.vhd
--           \
--           \-- mdio_if.vhd
--           \-- emac_dpram.vhd                     
--           \    \                     
--           \    \-- RAMB16_S4_S36
--           \                          
--           \    
--           \-- emac.vhd                     
--                \                     
--                \-- MacAddrRAM                   
--                \-- receive.vhd
--                \      rx_statemachine.vhd
--                \      rx_intrfce.vhd
--                \         async_fifo_fg.vhd
--                \      crcgenrx.vhd
--                \                     
--                \-- transmit.vhd
--                       crcgentx.vhd
--                          crcnibshiftreg
--                       tx_intrfce.vhd
--                          async_fifo_fg.vhd
--                       tx_statemachine.vhd
--                       deferral.vhd
--                          cntr5bit.vhd
--                          defer_state.vhd
--                       bocntr.vhd
--                          lfsr16.vhd
--                       msh_cnt.vhd
--                       ld_arith_reg.vhd
--
-------------------------------------------------------------------------------
-- Author:    PVK
-- History:    
-- PVK        06/07/2010     First Version
-- ^^^^^^
-- First version.  
-- ~~~~~~
-------------------------------------------------------------------------------
-- Naming Conventions:
--      active low signals:                     "*_n"
--      clock signals:                          "clk", "clk_div#", "clk_#x" 
--      reset signals:                          "rst", "rst_n" 
--      generics:                               "C_*" 
--      user defined types:                     "*_TYPE" 
--      state machine next state:               "*_ns" 
--      state machine current state:            "*_cs" 
--      combinatorial signals:                  "*_com" 
--      pipelined or register delay signals:    "*_d#" 
--      counter signals:                        "*cnt*"
--      clock enable signals:                   "*_ce" 
--      internal version of output port         "*_i"
--      device pins:                            "*_pin" 
--      ports:                                  - Names begin with Uppercase 
--      processes:                              "*_PROCESS" 
--      component instantiations:               "<ENTITY_>I_<#|FUNC>
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

-------------------------------------------------------------------------------
-- axi_ethernetlite_v3_0_18 library is used for axi_ethernetlite_v3_0_18 
-- component declarations
-------------------------------------------------------------------------------
library axi_ethernetlite_v3_0_18;
use axi_ethernetlite_v3_0_18.mac_pkg.all;

-- synopsys translate_off
-- Library XilinxCoreLib;
--library simprim;
-- synopsys translate_on

-------------------------------------------------------------------------------
-- Vcomponents from unisim library is used for FIFO instatiation
-- function declarations
-------------------------------------------------------------------------------
library unisim;
use unisim.Vcomponents.all;

-------------------------------------------------------------------------------
-- Port Declaration
-------------------------------------------------------------------------------
entity ram16x4 is
  generic(
      INIT_00  : bit_vector(15 downto 0)  :=x"0000";-- for addr(3 downto 0)
      INIT_01  : bit_vector(15 downto 0)  :=x"0000";-- for addr(7 downto 4)
      INIT_02  : bit_vector(15 downto 0)  :=x"0000";-- for addr(11 downto 8)
      INIT_03  : bit_vector(15 downto 0)  :=x"0000" -- for addr(15 downto 12)
      );
  port(
    Addr : in  std_logic_vector(3 downto 0);
    D    : in  std_logic_vector(3 downto 0);
    We   : in  std_logic;
    Clk  : in  std_logic;
    Q    : out std_logic_vector(3 downto 0));
end entity ram16x4 ;


-------------------------------------------------------------------------------
-- Architecture
-------------------------------------------------------------------------------  
architecture imp of ram16x4 is

attribute DowngradeIPIdentifiedWarnings: string;

attribute DowngradeIPIdentifiedWarnings of imp : architecture is "yes";

  attribute INIT : string ;
  attribute INIT of ram16x1_0 : label is GetInitString4(0, INIT_00,INIT_01,
      INIT_02, INIT_03);
  attribute INIT of ram16x1_1 : label is GetInitString4(1, INIT_00,INIT_01,
      INIT_02, INIT_03);
  attribute INIT of ram16x1_2 : label is GetInitString4(2, INIT_00,INIT_01,
      INIT_02, INIT_03);
  attribute INIT of ram16x1_3 : label is GetInitString4(3, INIT_00,INIT_01,
      INIT_02, INIT_03);

-------------------------------------------------------------------------------
-- Component Declarations
-------------------------------------------------------------------------------
  component ram16x1s
    -- synthesis translate_off
    -- synopsys translate_off
    generic ( init : bit_vector);
    -- synopsys translate_on
    -- synthesis translate_on
    port (
      a0   : in  std_logic;
      a1   : in  std_logic;
      a2   : in  std_logic;
      a3   : in  std_logic;
      d    : in  std_logic;
      we   : in  std_logic;
      wclk : in std_logic;
      o    : out std_logic);
   end component;


begin

  -----------------------------------------------------------------------------
  -- RAM 0 
  -----------------------------------------------------------------------------
  ram16x1_0 : ram16x1s
    -- synthesis translate_off
    -- synopsys translate_off
    generic map (init => GetInitVector4(0, INIT_00,INIT_01,
      INIT_02, INIT_03))
    -- synopsys translate_on
    -- synthesis translate_on
    port map (a0 => Addr(0), a1 => Addr(1), a2 => Addr(2), a3 => Addr(3),
               d => D(0), we => We, wclk => Clk, o => Q(0));

  -----------------------------------------------------------------------------
  -- RAM 1 
  -----------------------------------------------------------------------------
  ram16x1_1 : ram16x1s
    -- synthesis translate_off
    -- synopsys translate_off
    generic map (init => GetInitVector4(1, INIT_00,INIT_01,
      INIT_02, INIT_03))
    -- synopsys translate_on
    -- synthesis translate_on
    port map (a0 => Addr(0), a1 => Addr(1), a2 => Addr(2), a3 => Addr(3),
               d => D(1), we => We, wclk => Clk, o => Q(1));

  -----------------------------------------------------------------------------
  -- RAM 2 
  -----------------------------------------------------------------------------
  ram16x1_2 : ram16x1s
    -- synthesis translate_off
    -- synopsys translate_off
    generic map (init => GetInitVector4(2, INIT_00,INIT_01,
      INIT_02, INIT_03))
    -- synopsys translate_on
    -- synthesis translate_on
    port map (a0 => Addr(0), a1 => Addr(1), a2 => Addr(2), a3 => Addr(3),
               d => D(2), we => We, wclk => Clk, o => Q(2));

  -----------------------------------------------------------------------------
  -- RAM 3 
  -----------------------------------------------------------------------------
  ram16x1_3 : ram16x1s
    -- synthesis translate_off
    -- synopsys translate_off
    generic map (init => GetInitVector4(3, INIT_00,INIT_01,
      INIT_02, INIT_03))
    -- synopsys translate_on
    -- synthesis translate_on
    port map (a0 => Addr(0), a1 => Addr(1), a2 => Addr(2), a3 => Addr(3),
               d => D(3), we => We, wclk => Clk, o => Q(3));


end imp;


-------------------------------------------------------------------------------
-- msh_cnt - entity/architecture pair
-------------------------------------------------------------------------------
--  ***************************************************************************
--  ** DISCLAIMER OF LIABILITY                                               **
--  **                                                                       **
--  **  This file contains proprietary and confidential information of       **
--  **  Xilinx, Inc. ("Xilinx"), that is distributed under a license         **
--  **  from Xilinx, and may be used, copied and/or disclosed only           **
--  **  pursuant to the terms of a valid license agreement with Xilinx.      **
--  **                                                                       **
--  **  XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION                **
--  **  ("MATERIALS") "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER           **
--  **  EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING WITHOUT                  **
--  **  LIMITATION, ANY WARRANTY WITH RESPECT TO NONINFRINGEMENT,            **
--  **  MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE. Xilinx        **
--  **  does not warrant that functions included in the Materials will       **
--  **  meet the requirements of Licensee, or that the operation of the      **
--  **  Materials will be uninterrupted or error-free, or that defects       **
--  **  in the Materials will be corrected. Furthermore, Xilinx does         **
--  **  not warrant or make any representations regarding use, or the        **
--  **  results of the use, of the Materials in terms of correctness,        **
--  **  accuracy, reliability or otherwise.                                  **
--  **                                                                       **
--  **  Xilinx products are not designed or intended to be fail-safe,        **
--  **  or for use in any application requiring fail-safe performance,       **
--  **  such as life-support or safety devices or systems, Class III         **
--  **  medical devices, nuclear facilities, applications related to         **
--  **  the deployment of airbags, or any other applications that could      **
--  **  lead to death, personal injury or severe property or                 **
--  **  environmental damage (individually and collectively, "critical       **
--  **  applications"). Customer assumes the sole risk and liability         **
--  **  of any use of Xilinx products in critical applications,              **
--  **  subject only to applicable laws and regulations governing            **
--  **  limitations on product liability.                                    **
--  **                                                                       **
--  **  Copyright 2010 Xilinx, Inc.                                          **
--  **  All rights reserved.                                                 **
--  **                                                                       **
--  **  This disclaimer and copyright notice must be retained as part        **
--  **  of this file at all times.                                           **
--  ***************************************************************************
--
-------------------------------------------------------------------------------
-- Filename     : msh_cnt.vhd
-- Version      : v2.0
-- Description  : A register that can be loaded and added to or subtracted from
--                (but not both). The width of the register is specified
--                with a generic. The load value and the arith
--                value, i.e. the value to be added (subtracted), may be of
--                lesser width than the register and may be
--                offset from the LSB position. (Uncovered positions
--                load or add (subtract) zero.) The register can be
--                reset, via the Rst signal, to a freely selectable value.
--                The register is defined in terms of big-endian bit ordering.
-- VHDL-Standard: VHDL'93
-------------------------------------------------------------------------------
-- Structure:   
--
--  axi_ethernetlite.vhd
--      \
--      \-- axi_interface.vhd
--      \-- xemac.vhd
--           \
--           \-- mdio_if.vhd
--           \-- emac_dpram.vhd                     
--           \    \                     
--           \    \-- RAMB16_S4_S36
--           \                          
--           \    
--           \-- emac.vhd                     
--                \                     
--                \-- MacAddrRAM                   
--                \-- receive.vhd
--                \      rx_statemachine.vhd
--                \      rx_intrfce.vhd
--                \         async_fifo_fg.vhd
--                \      crcgenrx.vhd
--                \                     
--                \-- transmit.vhd
--                       crcgentx.vhd
--                          crcnibshiftreg
--                       tx_intrfce.vhd
--                          async_fifo_fg.vhd
--                       tx_statemachine.vhd
--                       deferral.vhd
--                          cntr5bit.vhd
--                          defer_state.vhd
--                       bocntr.vhd
--                          lfsr16.vhd
--                       msh_cnt.vhd
--                       ld_arith_reg.vhd
--
-------------------------------------------------------------------------------
-- Author:    PVK
-- History:    
-- PVK        06/07/2010     First Version
-- ^^^^^^
-- First version.  
-- ~~~~~~
-------------------------------------------------------------------------------
-- Naming Conventions:
--      active low signals:                     "*_n"
--      clock signals:                          "clk", "clk_div#", "clk_#x" 
--      reset signals:                          "Rst", "rst_n" 
--      generics:                               "C_*" 
--      user defined types:                     "*_TYPE" 
--      state machine next state:               "*_ns" 
--      state machine current state:            "*_cs" 
--      combinatorial signals:                  "*_com" 
--      pipelined or register delay signals:    "*_d#" 
--      counter signals:                        "*cnt*"
--      clock enable signals:                   "*_ce" 
--      internal version of output port         "*_i"
--      device pins:                            "*_pin" 
--      ports:                                  - Names begin with Uppercase 
--      processes:                              "*_PROCESS" 
--      component instantiations:               "<ENTITY_>I_<#|FUNC>
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;

-------------------------------------------------------------------------------
-- Vcomponents from unisim library is used for FIFO instatiation
-- function declarations
-------------------------------------------------------------------------------
library unisim;
use unisim.Vcomponents.all;


-------------------------------------------------------------------------------
-- Definition of Generics:
-------------------------------------------------------------------------------
-- C_ADD_SUB_NOT    -- 1 = Arith Add, 0 = Arith Substract
-- C_REG_WIDTH      -- Width of data
-- C_RESET_VALUE    -- Default value for the operation. Must be specified.
-------------------------------------------------------------------------------
-- Port Declaration
-------------------------------------------------------------------------------
-- Definition of Ports:
--
--  Clk             -- System Clock
--  Rst             -- System Reset
--  Q               -- Counter data out
--  Z               -- Indicates '0' when decrementing
--  LD              -- Counter load data
--  AD              -- Counter load arithmatic data
--  LOAD            -- Counter load enable
--  OP              -- Counter arith operation enable
-------------------------------------------------------------------------------
-- ENTITY
-------------------------------------------------------------------------------
entity msh_cnt is
    generic 
        (
        -----------------------------------------------------------------------
        -- True if the arithmetic operation is add, false if subtract.
        C_ADD_SUB_NOT : boolean := false;
        -----------------------------------------------------------------------
        -- Width of the register.
        C_REG_WIDTH   : natural := 8;
        -----------------------------------------------------------------------
        -- Reset value. (No default, must be specified in the instantiation.)
        C_RESET_VALUE : std_logic_vector
        -----------------------------------------------------------------------
        );
    port 
       (
        Clk    : in  std_logic;
        Rst    : in  std_logic; -- Reset to C_RESET_VALUE. (Overrides OP,LOAD)
        Q      : out std_logic_vector(0 to C_REG_WIDTH-1);
        Z      : out std_logic; -- indicates 0 when decrementing
        LD     : in  std_logic_vector(0 to C_REG_WIDTH-1); -- Load data.
        AD     : in  std_logic_vector(0 to C_REG_WIDTH-1); -- Arith data.
        LOAD   : in  std_logic;  -- Enable for the load op, Q <= LD.
        OP     : in  std_logic   -- Enable for the arith op, Q <= Q + AD.
                                   -- (Q <= Q - AD if C_ADD_SUB_NOT = false.)
                                   -- (Overrrides LOAD.)
        );
end msh_cnt;


architecture imp of msh_cnt is

attribute DowngradeIPIdentifiedWarnings: string;

attribute DowngradeIPIdentifiedWarnings of imp : architecture is "yes";

-------------------------------------------------------------------------------
-- Component Declarations
-------------------------------------------------------------------------------
    component MULT_AND
       port
         (
          LO :  out   std_ulogic;
          I1 :  in    std_ulogic;
          I0 :  in    std_ulogic
          );
    end component;

    component MUXCY_L is
      port 
        (
        DI : in  std_logic;
        CI : in  std_logic;
        S  : in  std_logic;
        LO : out std_logic
        );
    end component MUXCY_L;

    component XORCY is
      port 
       (
        LI : in  std_logic;
        CI : in  std_logic;
        O  : out std_logic
        );
    end component XORCY;

    component FDRE is
      port 
       (
        Q  : out std_logic;
        C  : in  std_logic;
        CE : in  std_logic;
        D  : in  std_logic;
        R  : in  std_logic
       );
    end component FDRE;
  
    component FDSE is
      port 
       (
        Q  : out std_logic;
        C  : in  std_logic;
        CE : in  std_logic;
        D  : in  std_logic;
        S  : in  std_logic
       );
    end component FDSE;
  
    signal q_i            : std_logic_vector(0 to C_REG_WIDTH-1);
    signal q_i_ns         : std_logic_vector(0 to C_REG_WIDTH-1);
    signal xorcy_out      : std_logic_vector(0 to C_REG_WIDTH-1);
    signal gen_cry_kill_n : std_logic_vector(0 to C_REG_WIDTH-1);
    signal cry            : std_logic_vector(0 to C_REG_WIDTH);
    signal z_i            : std_logic;

begin

    Q <= q_i;

    cry(C_REG_WIDTH) <= '0' when C_ADD_SUB_NOT else OP;

    PERBIT_GEN: for j in C_REG_WIDTH-1 downto 0 generate
        signal load_bit, arith_bit, ClkEn : std_logic;
    begin

        -----------------------------------------------------------------------
        -- Assign to load_bit the bit from input port LD.
        -----------------------------------------------------------------------
           load_bit <= LD(j);
        -----------------------------------------------------------------------
        -- Assign to arith_bit the bit from input port AD.
        -----------------------------------------------------------------------
            arith_bit <= AD(j);
        -----------------------------------------------------------------------
        -- LUT output generation.
        -- Adder case
        -----------------------------------------------------------------------
        Q_I_GEN_ADD: if C_ADD_SUB_NOT generate
            q_i_ns(j) <= q_i(j) xor  arith_bit when  OP = '1' else load_bit;
        end generate;
        -----------------------------------------------------------------------
        -- Subtractor case
        -----------------------------------------------------------------------
        Q_I_GEN_SUB: if not C_ADD_SUB_NOT generate
            q_i_ns(j) <= q_i(j) xnor arith_bit when  OP = '1' else load_bit;
        end generate;

        -----------------------------------------------------------------------
        -- Kill carries (borrows) for loads but
        -- generate or kill carries (borrows) for add (sub).
        -----------------------------------------------------------------------
        MULT_AND_i1: MULT_AND
           port map 
             (
              LO => gen_cry_kill_n(j),
              I1 => OP,
              I0 => Q_i(j)
             );

        -----------------------------------------------------------------------
        -- Propagate the carry (borrow) out.
        -----------------------------------------------------------------------
        MUXCY_L_i1: MUXCY_L
          port map 
           (
            DI => gen_cry_kill_n(j),
            CI => cry(j+1),
            S  => q_i_ns(j),
            LO => cry(j)
           );

        -----------------------------------------------------------------------
        -- Apply the effect of carry (borrow) in.
        -----------------------------------------------------------------------
        XORCY_i1: XORCY
          port map 
            (
            LI => q_i_ns(j),
            CI => cry(j+1),
            O  =>  xorcy_out(j)
            );

        STOP_AT_0_SUB: if not C_ADD_SUB_NOT generate
          ClkEn <= (LOAD or OP) when (not (conv_integer(q_i) = 0)) else '0';
        end generate STOP_AT_0_SUB;
        
        STOP_AT_MSB_ADD : if C_ADD_SUB_NOT generate
          ClkEn <= LOAD or OP;
        end generate STOP_AT_MSB_ADD;
        
        -----------------------------------------------------------------------
        -- Generate either a resettable or setable FF for bit j, depending
        -- on C_RESET_VALUE at bit j.
        -----------------------------------------------------------------------
        FF_RST0_GEN: if C_RESET_VALUE(j) = '0' generate
            FDRE_i1: FDRE
              port map 
                (
                Q  => q_i(j),
                C  => Clk,
                CE => ClkEn,
                D  => xorcy_out(j),
                R  => Rst
                );
        end generate;

        FF_RST1_GEN: if C_RESET_VALUE(j) = '1' generate
            FDSE_i1: FDSE
              port map 
               (
                Q  => q_i(j),
                C  => Clk,
                CE => ClkEn,
                D  => xorcy_out(j),
                S  => Rst
               );
        end generate;

      end generate;
      
      z_i <= '1' when ((conv_integer(q_i) = 1)) else '0';

      z_ff: FDSE
        port map 
         (
          Q  => Z,
          C  => Clk,
          CE => '1',
          D  => z_i,
          S  => Rst
          );

end imp;


-------------------------------------------------------------------------------
-- deferral - entity/architecture pair
-------------------------------------------------------------------------------
--  ***************************************************************************
--  ** DISCLAIMER OF LIABILITY                                               **
--  **                                                                       **
--  **  This file contains proprietary and confidential information of       **
--  **  Xilinx, Inc. ("Xilinx"), that is distributed under a license         **
--  **  from Xilinx, and may be used, copied and/or disclosed only           **
--  **  pursuant to the terms of a valid license agreement with Xilinx.      **
--  **                                                                       **
--  **  XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION                **
--  **  ("MATERIALS") "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER           **
--  **  EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING WITHOUT                  **
--  **  LIMITATION, ANY WARRANTY WITH RESPECT TO NONINFRINGEMENT,            **
--  **  MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE. Xilinx        **
--  **  does not warrant that functions included in the Materials will       **
--  **  meet the requirements of Licensee, or that the operation of the      **
--  **  Materials will be uninterrupted or error-free, or that defects       **
--  **  in the Materials will be corrected. Furthermore, Xilinx does         **
--  **  not warrant or make any representations regarding use, or the        **
--  **  results of the use, of the Materials in terms of correctness,        **
--  **  accuracy, reliability or otherwise.                                  **
--  **                                                                       **
--  **  Xilinx products are not designed or intended to be fail-safe,        **
--  **  or for use in any application requiring fail-safe performance,       **
--  **  such as life-support or safety devices or systems, Class III         **
--  **  medical devices, nuclear facilities, applications related to         **
--  **  the deployment of airbags, or any other applications that could      **
--  **  lead to death, personal injury or severe property or                 **
--  **  environmental damage (individually and collectively, "critical       **
--  **  applications"). Customer assumes the sole risk and liability         **
--  **  of any use of Xilinx products in critical applications,              **
--  **  subject only to applicable laws and regulations governing            **
--  **  limitations on product liability.                                    **
--  **                                                                       **
--  **  Copyright 2010 Xilinx, Inc.                                          **
--  **  All rights reserved.                                                 **
--  **                                                                       **
--  **  This disclaimer and copyright notice must be retained as part        **
--  **  of this file at all times.                                           **
--  ***************************************************************************
--
-------------------------------------------------------------------------------
-- Filename     : deferral.vhd
-- Version      : v2.0
-- Description  : This file contains the transmit deferral control.
-- VHDL-Standard: VHDL'93
-------------------------------------------------------------------------------
-- Structure:   
--
--  axi_ethernetlite.vhd
--      \
--      \-- axi_interface.vhd
--      \-- xemac.vhd
--           \
--           \-- mdio_if.vhd
--           \-- emac_dpram.vhd                     
--           \    \                     
--           \    \-- RAMB16_S4_S36
--           \                          
--           \    
--           \-- emac.vhd                     
--                \                     
--                \-- MacAddrRAM                   
--                \-- receive.vhd
--                \      rx_statemachine.vhd
--                \      rx_intrfce.vhd
--                \         async_fifo_fg.vhd
--                \      crcgenrx.vhd
--                \                     
--                \-- transmit.vhd
--                       crcgentx.vhd
--                          crcnibshiftreg
--                       tx_intrfce.vhd
--                          async_fifo_fg.vhd
--                       tx_statemachine.vhd
--                       deferral.vhd
--                          cntr5bit.vhd
--                          defer_state.vhd
--                       bocntr.vhd
--                          lfsr16.vhd
--                       msh_cnt.vhd
--                       ld_arith_reg.vhd
--
-------------------------------------------------------------------------------
-- Author:    PVK
-- History:    
-- PVK        06/07/2010     First Version
-- ^^^^^^
-- First version.  
-- ~~~~~~
-------------------------------------------------------------------------------
-- Naming Conventions:
--      active low signals:                     "*_n"
--      clock signals:                          "Clk", "clk_div#", "clk_#x" 
--      reset signals:                          "Rst", "rst_n" 
--      generics:                               "C_*" 
--      user defined types:                     "*_TYPE" 
--      state machine next state:               "*_ns" 
--      state machine current state:            "*_cs" 
--      combinatorial signals:                  "*_com" 
--      pipelined or register delay signals:    "*_d#" 
--      counter signals:                        "*cnt*"
--      clock enable signals:                   "*_ce" 
--      internal version of output port         "*_i"
--      device pins:                            "*_pin" 
--      ports:                                  - Names begin with Uppercase 
--      processes:                              "*_PROCESS" 
--      component instantiations:               "<ENTITY_>I_<#|FUNC>
-------------------------------------------------------------------------------
-- 
library ieee;
use ieee.std_logic_1164.all;

-------------------------------------------------------------------------------
-- axi_ethernetlite_v3_0_18 library is used for axi_ethernetlite_v3_0_18 
-- component declarations
-------------------------------------------------------------------------------
library axi_ethernetlite_v3_0_18;
use axi_ethernetlite_v3_0_18.mac_pkg.all;

-- synopsys translate_off
-- Library XilinxCoreLib;
-- synopsys translate_on

-------------------------------------------------------------------------------
-- Port Declaration
-------------------------------------------------------------------------------
-- Definition of Ports:
--
--  Clk          -- System Clock
--  Rst          -- System Reset
--  TxEn         -- Transmit enable
--  Txrst        -- Transmit reset
--  Tx_clk_en    -- Transmit clock enable
--  BackingOff   -- Backing off 
--  Crs          -- Carrier sense
--  Full_half_n  -- Full/Half duplex indicator
--  Ifgp1        -- Interframe gap delay
--  Ifgp2        -- Interframe gap delay
--  Deferring    -- Deffering for the tx data
-------------------------------------------------------------------------------
-- ENTITY
-------------------------------------------------------------------------------
entity deferral is
  port 
       (
        Clk         : in std_logic;
        Rst         : in std_logic;
        TxEn        : in std_logic; 
        Txrst       : in std_logic;
        Tx_clk_en   : in std_logic;
        BackingOff  : in std_logic;
        Crs         : in std_logic;
        Full_half_n : in std_logic;
        Ifgp1       : in std_logic_vector(0 to 4);
        Ifgp2       : in std_logic_vector(0 to 4);        
        Deferring   : out std_logic
        );
end deferral;

-------------------------------------------------------------------------------
-- Definition of Generics:
--          No Generics were used for this Entity.
--
-- Definition of Ports:
--         
-------------------------------------------------------------------------------

architecture implementation of deferral is

attribute DowngradeIPIdentifiedWarnings: string;

attribute DowngradeIPIdentifiedWarnings of implementation : architecture is "yes";

-------------------------------------------------------------------------------
-- Signal and Type Declarations
-------------------------------------------------------------------------------
  signal cntrLd_i     : std_logic;
  signal cntrEn       : std_logic;
  signal comboCntrEn  : std_logic;
  signal comboCntrEn2 : std_logic;
  signal ifgp1_zero   : std_logic;
  signal ifgp2_zero   : std_logic;
  signal comboRst     : std_logic;
   
begin

comboRst     <= Rst or Txrst;
comboCntrEn  <= Tx_clk_en and cntrEn;
comboCntrEn2 <= Tx_clk_en and cntrEn and ifgp1_zero;
-------------------------------------------------------------------------------
-- Ifgp1 counter
-------------------------------------------------------------------------------
  inst_ifgp1_count: entity axi_ethernetlite_v3_0_18.cntr5bit
    port map
           (
            Clk     =>  Clk, 
            Rst     =>  comboRst,
            En      =>  comboCntrEn,
            Ld      =>  cntrLd_i,
            Load_in =>  Ifgp1,
            Zero    =>  ifgp1_zero
            );  

-------------------------------------------------------------------------------
-- Ifgp2 counter
-------------------------------------------------------------------------------
  inst_ifgp2_count: entity axi_ethernetlite_v3_0_18.cntr5bit
    port map
           (
            Clk     =>  Clk, 
            Rst     =>  comboRst,
            En      =>  comboCntrEn2,
            Ld      =>  cntrLd_i,
            Load_in =>  Ifgp2,
            Zero    =>  ifgp2_zero
            );

-------------------------------------------------------------------------------
-- deferral state machine
-------------------------------------------------------------------------------
  inst_deferral_state: entity axi_ethernetlite_v3_0_18.defer_state
    port map
            (
             Clk         =>  Clk, 
             Rst         =>  Rst,
             TxEn        =>  TxEn,
             Txrst       =>  Txrst, 
             Ifgp2Done   =>  ifgp2_zero,
             Ifgp1Done   =>  ifgp1_zero,
             BackingOff  =>  BackingOff,
             Crs         =>  Crs,
             Full_half_n =>  Full_half_n,
             Deferring   =>  Deferring,
             CntrEnbl    =>  cntrEn,
             CntrLd      =>  cntrLd_i
             );   


end implementation;


-------------------------------------------------------------------------------
-- crcgentx - entity/architecture pair
-------------------------------------------------------------------------------
--  ***************************************************************************
--  ** DISCLAIMER OF LIABILITY                                               **
--  **                                                                       **
--  **  This file contains proprietary and confidential information of       **
--  **  Xilinx, Inc. ("Xilinx"), that is distributed under a license         **
--  **  from Xilinx, and may be used, copied and/or disclosed only           **
--  **  pursuant to the terms of a valid license agreement with Xilinx.      **
--  **                                                                       **
--  **  XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION                **
--  **  ("MATERIALS") "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER           **
--  **  EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING WITHOUT                  **
--  **  LIMITATION, ANY WARRANTY WITH RESPECT TO NONINFRINGEMENT,            **
--  **  MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE. Xilinx        **
--  **  does not warrant that functions included in the Materials will       **
--  **  meet the requirements of Licensee, or that the operation of the      **
--  **  Materials will be uninterrupted or error-free, or that defects       **
--  **  in the Materials will be corrected. Furthermore, Xilinx does         **
--  **  not warrant or make any representations regarding use, or the        **
--  **  results of the use, of the Materials in terms of correctness,        **
--  **  accuracy, reliability or otherwise.                                  **
--  **                                                                       **
--  **  Xilinx products are not designed or intended to be fail-safe,        **
--  **  or for use in any application requiring fail-safe performance,       **
--  **  such as life-support or safety devices or systems, Class III         **
--  **  medical devices, nuclear facilities, applications related to         **
--  **  the deployment of airbags, or any other applications that could      **
--  **  lead to death, personal injury or severe property or                 **
--  **  environmental damage (individually and collectively, "critical       **
--  **  applications"). Customer assumes the sole risk and liability         **
--  **  of any use of Xilinx products in critical applications,              **
--  **  subject only to applicable laws and regulations governing            **
--  **  limitations on product liability.                                    **
--  **                                                                       **
--  **  Copyright 2010 Xilinx, Inc.                                          **
--  **  All rights reserved.                                                 **
--  **                                                                       **
--  **  This disclaimer and copyright notice must be retained as part        **
--  **  of this file at all times.                                           **
--  ***************************************************************************
--
-------------------------------------------------------------------------------
-- Filename    : crcgentx.vhd
-- Version     : v4.00.a
-- Description : This module does an 4-bit parallel CRC generation. 
--               The polynomial is that specified for IEEE 802.3 (ethernet)
--               LANs and other standards.
--
--               I. Functionality:
--               1. The module does an 4-bit parallel CRC generation.  
--               2. The module provides a synchronous 4-bit per clock load and
--                  unload function.
--               3. The polynomial is that specified for 802.3 LANs and other
--                  standards.
--                  The polynomial computed is:
--                  G(x)=X**32+X**26+X**23+X**22+X**16+X**12+X**11+X**10+X**8
--                       +X**7+X**5+X**4+X** >2+X+1
-- 
--              II. Module I/O
--              Inputs: Clk, Clken, RESET, LOAD, COMPUTE, DATA_IN[3:0]
--              outputs: CRC_OK, DATA_OUT[3:0], CRC[31:0]
--
--              III.Truth Table:        
-- 
--              Clken  RESET  COMPUTE   LOAD   | DATA_OUT 
--              ------------------------------------------ 
--               0      X      X        X      | No change
--               1      0      0        0      | No change
--               1      1      X        X      | 0xFFFF (all ones) 
--               1      0      X        1      | load and shift 1 nibble of crc
--               1      0      1        0      | Compute CRC
--
--               0      0      1        1      | unload 4 byte crc 
--                                               NOT IMPLEMENTED)
-- 
--              Loading and unloading of the 32-bit CRC register is done one
--              nibble at a time by asserting LOAD and Clken.  The Data on
--              data_in is shifted into the the LSB of the CRC register. The
--              MSB of the CRC register is available on data_out. 
--
--              Signals ending in _n are active low.
--
-- Copyright 1997 VAutomation Inc. Nashua NH USA (603) 882-2282.
-- Modification for 4 Bit done by Ronald Hecht @ Xilinx Inc.
-- This software is provided AS-IS and free of charge with the restriction that
-- this copyright notice remain in all copies of the Source Code at all times.
-- Visit HTTP://www.vautomation.com for more information on our cores.
-------------------------------------------------------------------------------
-- We add a nibble shift register into this module. 
-- This module contains two parts.
--
-- 1. parallel_crc function which is a function will calculate the crc value.
-- 2. nibShitReg is a nibble shift register which has two operations
--    when DataEn goes high it will act as a normal register, 
--    when OutEn goes high it will stop load new Data and shift out current
--    register Data by nibbles. 
--
-- Some specification on module and port
-- 
-- 1. For nibShiftReg, give initial value to all zeros. This is because the initial
--    value for parallel_crc need to be all ones. Because we put a not on both 
--    side of nibShiftReg, so we need to set it's value to all zeros at the
--    beginning. 
-- 2. Don't shift the nibShiftReg at the first OutEn clock cycle, because the 
--    first nibble is already there.
-- 
-- THE INTERFACE REQUIREMENTS OF THIS MODULE
--
-- Rst        reset everything to initial value. We must give this reset 
--              before we use crc module, otherwise the result will incorrect.
-- dataClk      For use with mactx module, it will be 2.5 MHZ.
-- Data         Input Data from other module in nibbles.
-- DataEn       Enable crcgenrx. Make sure your enable and first Data can be 
--              captured at the beginning of Data stream. 
-- crcOk        At the end of Data stream, this will go high if the crc is 
--              correct.
--
-- VHDL-Standard:  VHDL'93
-------------------------------------------------------------------------------
-- Structure:   
--
--  axi_ethernetlite.vhd
--      \
--      \-- xemac.vhd
--           \
--           \-- axi_ipif_interface.vhd
--           \-- mdio_if.vhd
--           \-- emac_dpram.vhd                     
--           \    \                     
--           \    \-- RAMB16_S4_S36
--           \                          
--           \    
--           \-- emac.vhd                     
--                \                     
--                \                     
--                \-- receive.vhd
--                \      rx_statemachine.vhd
--                \      rx_intrfce.vhd
--                \         ethernetlite_v3_0_dmem_v2.edn
--                \      crcgenrx.vhd
--                \                     
--                \-- transmit.vhd
--                       crcgentx.vhd
--                          crcnibshiftreg
--                       tx_intrfce.vhd
--                          ethernetlite_v3_0_dmem_v2.edn
--                       tx_statemachine.vhd
--                       deferral.vhd
--                          cntr5bit.vhd
--                          defer_state.vhd
--                       bocntr.vhd
--                          lfsr16.vhd
--                       msh_cnt.vhd
--                       ld_arith_reg.vhd
--
-------------------------------------------------------------------------------
-- Author:    PVK
-- History:    
-- PVK        06/07/2010     First Version
-- ^^^^^^
-- First version.  
-- ~~~~~~
-------------------------------------------------------------------------------
-- Naming Conventions:
--      active low signals:                     "*_n"
--      clock signals:                          "Clk", "clk_div#", "clk_#x" 
--      reset signals:                          "Rst", "rst_n" 
--      generics:                               "C_*" 
--      user defined types:                     "*_TYPE" 
--      state machine next state:               "*_ns" 
--      state machine current state:            "*_cs" 
--      combinatorial signals:                  "*_com" 
--      pipelined or register delay signals:    "*_d#" 
--      counter signals:                        "*cnt*"
--      clock enable signals:                   "*_ce" 
--      internal version of output port         "*_i"
--      device pins:                            "*_pin" 
--      ports:                                  - Names begin with Uppercase 
--      processes:                              "*_PROCESS" 
--      component instantiations:               "<ENTITY_>I_<#|FUNC>
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;

-------------------------------------------------------------------------------
-- axi_ethernetlite_v3_0_18 library is used for axi_ethernetlite_v3_0_18 
-- component declarations
-------------------------------------------------------------------------------
library axi_ethernetlite_v3_0_18;
use axi_ethernetlite_v3_0_18.mac_pkg.all;

-------------------------------------------------------------------------------
-- Definition of Ports:
--
--  Clk      -- System Clock
--  Rst      -- System Reset
--  Clke     -- Clock enable
--  Data     -- Data in 
--  DataEn   -- Data valid
--  OutEn    -- Dataout enable
--  CrcNibs  -- CRC nibble out
-------------------------------------------------------------------------------
-- ENTITY
-------------------------------------------------------------------------------
entity crcgentx is

  port 
       (
        Clk       : in  std_logic;
        Rst       : in  std_logic;
        Clken     : in  std_logic;
        Data      : in  std_logic_vector(3 downto 0);
        DataEn    : in  std_logic;
        OutEn     : in  std_logic;        -- NSR shift out enable
        CrcNibs   : out std_logic_vector(3 downto 0)
        );
end crcgentx;


architecture arch1 of crcgentx is  

attribute DowngradeIPIdentifiedWarnings: string;

attribute DowngradeIPIdentifiedWarnings of arch1 : architecture is "yes";

  constant CRC_REMAINDER : std_logic_vector(31 downto 0) :=
    "11000111000001001101110101111011";  -- 0xC704DD7B

  function parallel_crc (crc_in  : std_logic_vector(31 downto 0);
                         data_in : std_logic_vector(3 downto 0)
                         ) return std_logic_vector is
    variable c, crc_out          : std_logic_vector(31 downto 0);
    variable x                   : std_logic_vector (31 downto 28);
    variable d                   : std_logic_vector (3 downto 0);
  begin
-- Because the equations are long I am keeping the name of the incoming
-- CRC and the XOR vector short.

    c := crc_in;
    d := data_in;

-- the first thing that a parallel CRC needs to do is to develop the
-- vector formed by XORing the input vector by the current CRC. This
-- vector is then used during the CRC calculation.

    x := (c(31) xor d(3)) & (c(30) xor d(2)) &
         (c(29) xor d(1)) & (c(28) xor d(0));

-- The parellel CRC is a function of the X vector and the current CRC.
    crc_out :=
      (c(27) ) &
      (c(26) ) &
      (c(25) xor x(31) ) &
      (c(24) xor x(30) ) &
      (c(23) xor x(29) ) &
      (c(22) xor x(31) xor x(28) ) &
      (c(21) xor x(31) xor x(30) ) &
      (c(20) xor x(30) xor x(29) ) &
      (c(19) xor x(29) xor x(28) ) &
      (c(18) xor x(28) ) &
      (c(17) ) &
      (c(16) ) &
      (c(15) xor x(31) ) &
      (c(14) xor x(30) ) &
      (c(13) xor x(29) ) &
      (c(12) xor x(28) ) &
      (c(11) xor x(31) ) &
      (c(10) xor x(31) xor x(30) ) &
      (c(9 ) xor x(31) xor x(30) xor x(29) ) &
      (c(8 ) xor x(30) xor x(29) xor x(28) ) &
      (c(7 ) xor x(31) xor x(29) xor x(28) ) &
      (c(6 ) xor x(31) xor x(30) xor x(28) ) &
      (c(5 ) xor x(30) xor x(29) ) &
      (c(4 ) xor x(31) xor x(29) xor x(28) ) &
      (c(3 ) xor x(31) xor x(30) xor x(28) ) &
      (c(2 ) xor x(30) xor x(29) ) &
      (c(1 ) xor x(31) xor x(29) xor x(28) ) &
      (c(0 ) xor x(31) xor x(30) xor x(28) ) &
      ( x(31) xor x(30) xor x(29) ) &
      ( x(30) xor x(29) xor x(28) ) &
      ( x(29) xor x(28) ) &
      ( x(28) );

    return(crc_out);

  end parallel_crc;

---------------------------------------------------------
-- A function which can reverse the bit order
-- order   --   BY ben 07/04
---------------------------------------------------------

  function revBitOrder( arg : std_logic_vector) return std_logic_vector is  -- By ben 07/04/2000
    variable tmp            : std_logic_vector(arg'range);
  begin
    lp0                     : for i in arg'range loop
      tmp(arg'high - i) := arg(i);
    end loop lp0;
    return tmp;
  end revBitOrder;

  signal regDataIn, regDataOut, crcFuncIn, crcFuncOut: std_logic_vector(31 downto 0);
  signal data_transpose : std_logic_vector(3 downto 0);
  signal shiftEnable : std_logic;

--  component crcnibshiftreg
--    port (
--      Clk     : in  std_logic;
--      Clken   : in  std_logic;
--      Rst     : in  std_logic;
--      din     : in  std_logic_vector(31 downto 0);
--      load    : in  std_logic;
--      shift   : in  std_logic;
--      dout    : out std_logic_vector(31 downto 0)
--      );
--  end component;

begin  ----------------------------------------------------------------------


-----------------------------------------------------------------------------
-- This nibble shift register act as a normal register when DataEn is
-- high. When shiftEnable goes high, this register will stop load Data
-- and begin to shift Data out in nibbles. 
-- Rember to check the initial value of this register which should be 
-- all '0', otherwise the initial value for parallel_crc will not be 
-- all '1'. This is related with the functions we put on input and output 
-- of this register. 
-----------------------------------------------------------------------------

  NSR : entity axi_ethernetlite_v3_0_18.crcnibshiftreg
    port map
     (
      Clk     => Clk,
      Clken   => Clken,
      Rst     => Rst,
      Din     => regDataIn,
      Load    => DataEn,
      Shift   => shiftEnable,
      Dout    => regDataOut
      );

  shiftEnable    <= OutEn and not DataEn;
  crcFuncOut     <= parallel_crc(crcFuncIn,data_transpose);
---------------------------------------------------------------------------------
-- These two sets of functions at input/output are balanced and the synthesis
-- tool will optimize them. The purpose is to let the register have all the Data 
-- in right order before shift them. 
---------------------------------------------------------------------------------
  regDataIn      <= not revBitOrder(crcFuncOut);
  crcFuncIn      <= not revBitOrder(regDataOut);
  CrcNibs        <= regDataOut(3 downto 0); 

  data_transpose <= Data(0) & Data(1) & Data(2) & Data(3);

  
end arch1;


-------------------------------------------------------------------------------
-- crcgenrx - entity/architecture pair
-------------------------------------------------------------------------------
--  ***************************************************************************
--  ** DISCLAIMER OF LIABILITY                                               **
--  **                                                                       **
--  **  This file contains proprietary and confidential information of       **
--  **  Xilinx, Inc. ("Xilinx"), that is distributed under a license         **
--  **  from Xilinx, and may be used, copied and/or disclosed only           **
--  **  pursuant to the terms of a valid license agreement with Xilinx.      **
--  **                                                                       **
--  **  XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION                **
--  **  ("MATERIALS") "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER           **
--  **  EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING WITHOUT                  **
--  **  LIMITATION, ANY WARRANTY WITH RESPECT TO NONINFRINGEMENT,            **
--  **  MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE. Xilinx        **
--  **  does not warrant that functions included in the Materials will       **
--  **  meet the requirements of Licensee, or that the operation of the      **
--  **  Materials will be uninterrupted or error-free, or that defects       **
--  **  in the Materials will be corrected. Furthermore, Xilinx does         **
--  **  not warrant or make any representations regarding use, or the        **
--  **  results of the use, of the Materials in terms of correctness,        **
--  **  accuracy, reliability or otherwise.                                  **
--  **                                                                       **
--  **  Xilinx products are not designed or intended to be fail-safe,        **
--  **  or for use in any application requiring fail-safe performance,       **
--  **  such as life-support or safety devices or systems, Class III         **
--  **  medical devices, nuclear facilities, applications related to         **
--  **  the deployment of airbags, or any other applications that could      **
--  **  lead to death, personal injury or severe property or                 **
--  **  environmental damage (individually and collectively, "critical       **
--  **  applications"). Customer assumes the sole risk and liability         **
--  **  of any use of Xilinx products in critical applications,              **
--  **  subject only to applicable laws and regulations governing            **
--  **  limitations on product liability.                                    **
--  **                                                                       **
--  **  Copyright 2010 Xilinx, Inc.                                          **
--  **  All rights reserved.                                                 **
--  **                                                                       **
--  **  This disclaimer and copyright notice must be retained as part        **
--  **  of this file at all times.                                           **
--  ***************************************************************************
--
-------------------------------------------------------------------------------
-- Filename    : crcgenrx.vhd
-- Version     : v4.00.a
-- Description : This module does an 4-bit parallel CRC generation. 
--               The polynomial is that specified for IEEE 802.3 (ethernet)
--               LANs and other standards.
--
--               I. Functionality:
--               1. The module does an 4-bit parallel CRC generation.  
--               2. The module provides a synchronous 4-bit per clock load and
--                  unload function.
--               3. The polynomial is that specified for 802.3 LANs and other
--                  standards.
--                  The polynomial computed is:
--                  G(x)=X**32+X**26+X**23+X**22+X**16+X**12+X**11+X**10+X**8
--                       +X**7+X**5+X**4+X** >2+X+1
-- 
--              II. Module I/O
--              Inputs: Clk, CLKEN, Rst, LOAD, COMPUTE, DATA_IN[3:0]
--              outputs: CRC_OK, DATA_OUT[3:0], CRC[31:0]
--
--              III.Truth Table:        
-- 
--              CLKEN  Rst  COMPUTE   LOAD   | DATA_OUT 
--              ------------------------------------------ 
--               0      X      X        X      | No change
--               1      0      0        0      | No change
--               1      1      X        X      | 0xFFFF (all ones) 
--               1      0      X        1      | load and shift 1 nibble of crc
--               1      0      1        0      | Compute CRC
--
--               0      0      1        1      | unload 4 byte crc 
--                                               NOT IMPLEMENTED)
-- 
--              Loading and unloading of the 32-bit CRC register is done one
--              nibble at a time by asserting LOAD and CLKEN.  The Data on
--              data_in is shifted into the the LSB of the CRC register. The
--              MSB of the CRC register is available on data_out. 
--
--              Signals ending in _n are active low.
--
-- Copyright 1997 VAutomation Inc. Nashua NH USA (603) 882-2282.
-- Modification for 4 Bit done by Ronald Hecht @ Xilinx Inc.
-- This software is provided AS-IS and free of charge with the restriction that
-- this copyright notice remain in all copies of the Source Code at all times.
-- Visit HTTP://www.vautomation.com for more information on our cores.
-------------------------------------------------------------------------------
-- We remove the 32 bits register which restore the crc value in the old code.
-- For receive part we only need to know the crc is ok or not, so remove the 
-- register for restoring crc value will save some resources.
--
-- THE INTERFACE REQUIREMENTS OF THIS MODULE
--
-- Rst        reset everything to initial value. We must give this reset 
--              before we use crc module, otherwise the result will incorrect.
-- Clk      For use with mactx module, it will be 2.5 MHZ.
-- Data         Input Data from other module in nibbles.
-- DataEn       Enable crcgenrx. Make sure your enable and first Data can be 
--              captured at the beginning of Data stream. 
-- CrcOk        At the end of Data stream, this will go high if the crc is 
--              correct.
--
-- VHDL-Standard:  VHDL'93
-------------------------------------------------------------------------------
-- Structure:   
--
--  axi_ethernetlite.vhd
--      \
--      \-- xemac.vhd
--           \
--           \-- axi_ipif_interface.vhd
--           \-- mdio_if.vhd
--           \-- emac_dpram.vhd                     
--           \    \                     
--           \    \-- RAMB16_S4_S36
--           \                          
--           \    
--           \-- emac.vhd                     
--                \                     
--                \                     
--                \-- receive.vhd
--                \      rx_statemachine.vhd
--                \      rx_intrfce.vhd
--                \         ethernetlite_v3_0_dmem_v2.edn
--                \      crcgenrx.vhd
--                \                     
--                \-- transmit.vhd
--                       crcgentx.vhd
--                          crcnibshiftreg
--                       tx_intrfce.vhd
--                          ethernetlite_v3_0_dmem_v2.edn
--                       tx_statemachine.vhd
--                       deferral.vhd
--                          cntr5bit.vhd
--                          defer_state.vhd
--                       bocntr.vhd
--                          lfsr16.vhd
--                       msh_cnt.vhd
--                       ld_arith_reg.vhd
--
-------------------------------------------------------------------------------
-- Author:    PVK
-- History:    
-- PVK        06/07/2010     First Version
-- ^^^^^^
-- First version.  
-- ~~~~~~
-------------------------------------------------------------------------------
-- Naming Conventions:
--      active low signals:                     "*_n"
--      clock signals:                          "Clk", "clk_div#", "clk_#x" 
--      reset signals:                          "Rst", "rst_n" 
--      generics:                               "C_*" 
--      user defined types:                     "*_TYPE" 
--      state machine next state:               "*_ns" 
--      state machine current state:            "*_cs" 
--      combinatorial signals:                  "*_com" 
--      pipelined or register delay signals:    "*_d#" 
--      counter signals:                        "*cnt*"
--      clock enable signals:                   "*_ce" 
--      internal version of output port         "*_i"
--      device pins:                            "*_pin" 
--      ports:                                  - Names begin with Uppercase 
--      processes:                              "*_PROCESS" 
--      component instantiations:               "<ENTITY_>I_<#|FUNC>
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;

-------------------------------------------------------------------------------
-- axi_ethernetlite_v3_0_18 library is used for axi_ethernetlite_v3_0_18 
-- component declarations
-------------------------------------------------------------------------------
library axi_ethernetlite_v3_0_18;
use axi_ethernetlite_v3_0_18.mac_pkg.all;

-------------------------------------------------------------------------------
-- Definition of Ports:
--
--  Clk      -- System Clock
--  Rst      -- System Reset
--  Data     -- Data in 
--  DataEn   -- Data enable
--  CrcOk    -- CRC valid
-------------------------------------------------------------------------------
-- ENTITY
-------------------------------------------------------------------------------
entity crcgenrx is

    port 
         (
          Clk     : in std_logic;
          Rst     : in std_logic;
          Data    : in std_logic_vector(3 downto 0);
          DataEn  : in std_logic;
          CrcOk   : out std_logic
          );

end crcgenrx;

architecture arch1 of crcgenrx is 

attribute DowngradeIPIdentifiedWarnings: string;

attribute DowngradeIPIdentifiedWarnings of arch1 : architecture is "yes";

    constant CRC_REMAINDER         : std_logic_vector(31 downto 0) :=
                                      "11000111000001001101110101111011";
                                       -- 0xC704DD7B

    signal crc_local               : std_logic_vector(31 downto 0);  -- local version
    signal data_transpose          : std_logic_vector(3 downto 0);

    function parallel_crc (crc_in  : std_logic_vector(31 downto 0);
                         data_in   : std_logic_vector(3 downto 0)
                         ) return std_logic_vector is
    variable c, crc_out            : std_logic_vector(31 downto 0);
    variable x                     : std_logic_vector (31 downto 28);
    variable d                     : std_logic_vector (3 downto 0);
  begin
-- Because the equations are long I am keeping the name of the incoming
-- CRC and the XOR vector short.

    c := crc_in;
    d := data_in;

-- the first thing that a parallel CRC needs to do is to develop the
-- vector formed by XORing the input vector by the current CRC. This
-- vector is then used during the CRC calculation.

    x := (c(31) xor d(3)) & (c(30) xor d(2)) &
         (c(29) xor d(1)) & (c(28) xor d(0));

-- The parellel CRC is a function of the X vector and the current CRC.
    crc_out :=
      (c(27) ) &
      (c(26) ) &
      (c(25) xor x(31) ) &
      (c(24) xor x(30) ) &
      (c(23) xor x(29) ) &
      (c(22) xor x(31) xor x(28) ) &
      (c(21) xor x(31) xor x(30) ) &
      (c(20) xor x(30) xor x(29) ) &
      (c(19) xor x(29) xor x(28) ) &
      (c(18) xor x(28) ) &
      (c(17) ) &
      (c(16) ) &
      (c(15) xor x(31) ) &
      (c(14) xor x(30) ) &
      (c(13) xor x(29) ) &
      (c(12) xor x(28) ) &
      (c(11) xor x(31) ) &
      (c(10) xor x(31) xor x(30) ) &
      (c(9 ) xor x(31) xor x(30) xor x(29) ) &
      (c(8 ) xor x(30) xor x(29) xor x(28) ) &
      (c(7 ) xor x(31) xor x(29) xor x(28) ) &
      (c(6 ) xor x(31) xor x(30) xor x(28) ) &
      (c(5 ) xor x(30) xor x(29) ) &
      (c(4 ) xor x(31) xor x(29) xor x(28) ) &
      (c(3 ) xor x(31) xor x(30) xor x(28) ) &
      (c(2 ) xor x(30) xor x(29) ) &
      (c(1 ) xor x(31) xor x(29) xor x(28) ) &
      (c(0 ) xor x(31) xor x(30) xor x(28) ) &
      ( x(31) xor x(30) xor x(29) ) &
      ( x(30) xor x(29) xor x(28) ) &
      ( x(29) xor x(28) ) &
      ( x(28) );


    return(crc_out);

  end parallel_crc;

begin  ------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Xilinx modification, use asynchronous clear
-------------------------------------------------------------------------------
  data_transpose <= Data(0) & Data(1) & Data(2) & Data(3); 
                    -- Reverse the bit order

  -- Create the 32 Flip flops (with clock enable flops)
  CRC_REG : process(Clk)    
  begin
    if Clk'event and Clk = '1' then
      if Rst = '1' then
         crc_local   <= (others=>'1');
        elsif  DataEn = '1' then
         crc_local <= parallel_crc(crc_local, data_transpose);
      end if;
    end if;
  end process CRC_REG;

-------------------------------------------------------------------------------
-- Xilinx modification, remove reset from mux
-------------------------------------------------------------------------------

  CrcOk <= '1' when crc_local = CRC_REMAINDER else '0';
                                        -- This is a 32-bit wide AND
                                        -- function, so proper
                                        -- attention should be paid
                                        -- when synthesizing to
                                        -- achieve good results.  If
                                        -- there are cycles available
                                        -- pipeling this gate would be
                                        -- appropriate.

end arch1;


-------------------------------------------------------------------------------
-- bocntr - entity/architecture pair
-------------------------------------------------------------------------------
--  ***************************************************************************
--  ** DISCLAIMER OF LIABILITY                                               **
--  **                                                                       **
--  **  This file contains proprietary and confidential information of       **
--  **  Xilinx, Inc. ("Xilinx"), that is distributed under a license         **
--  **  from Xilinx, and may be used, copied and/or disclosed only           **
--  **  pursuant to the terms of a valid license agreement with Xilinx.      **
--  **                                                                       **
--  **  XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION                **
--  **  ("MATERIALS") "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER           **
--  **  EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING WITHOUT                  **
--  **  LIMITATION, ANY WARRANTY WITH RESPECT TO NONINFRINGEMENT,            **
--  **  MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE. Xilinx        **
--  **  does not warrant that functions included in the Materials will       **
--  **  meet the requirements of Licensee, or that the operation of the      **
--  **  Materials will be uninterrupted or error-free, or that defects       **
--  **  in the Materials will be corrected. Furthermore, Xilinx does         **
--  **  not warrant or make any representations regarding use, or the        **
--  **  results of the use, of the Materials in terms of correctness,        **
--  **  accuracy, reliability or otherwise.                                  **
--  **                                                                       **
--  **  Xilinx products are not designed or intended to be fail-safe,        **
--  **  or for use in any application requiring fail-safe performance,       **
--  **  such as life-support or safety devices or systems, Class III         **
--  **  medical devices, nuclear facilities, applications related to         **
--  **  the deployment of airbags, or any other applications that could      **
--  **  lead to death, personal injury or severe property or                 **
--  **  environmental damage (individually and collectively, "critical       **
--  **  applications"). Customer assumes the sole risk and liability         **
--  **  of any use of Xilinx products in critical applications,              **
--  **  subject only to applicable laws and regulations governing            **
--  **  limitations on product liability.                                    **
--  **                                                                       **
--  **  Copyright 2010 Xilinx, Inc.                                          **
--  **  All rights reserved.                                                 **
--  **                                                                       **
--  **  This disclaimer and copyright notice must be retained as part        **
--  **  of this file at all times.                                           **
--  ***************************************************************************
--
-------------------------------------------------------------------------------
-- Filename     : bocntr.vhd
-- Version      : v2.0
-- Description  : This is the transmit collision back off counter
--                the back off delay for retry n (1 <= n <= 16) is defined as
--                delay where delay is a uniformly distributed integer number
--                of slot times (512 bit times) defined as 
--                0 <= delay <= 2^k where k is min(n, 10) i.e., k is equal
--                to the retry attempt up to 10 and then remains at 10 for 
--                retry attempts 11 through 16.  So the delay for retry 1
--                would be 0, 1, or 2 slot times.  The delay for retry 2
--                would be 0, 1, 2, 3, or 4 slot times.
--
-- VHDL-Standard: VHDL'93
-------------------------------------------------------------------------------
-- Structure:   
--
--  axi_ethernetlite.vhd
--      \
--      \-- axi_interface.vhd
--      \-- xemac.vhd
--           \
--           \-- mdio_if.vhd
--           \-- emac_dpram.vhd                     
--           \    \                     
--           \    \-- RAMB16_S4_S36
--           \                          
--           \    
--           \-- emac.vhd                     
--                \                     
--                \-- MacAddrRAM                   
--                \-- receive.vhd
--                \      rx_statemachine.vhd
--                \      rx_intrfce.vhd
--                \         async_fifo_fg.vhd
--                \      crcgenrx.vhd
--                \                     
--                \-- transmit.vhd
--                       crcgentx.vhd
--                          crcnibshiftreg
--                       tx_intrfce.vhd
--                          async_fifo_fg.vhd
--                       tx_statemachine.vhd
--                       deferral.vhd
--                          cntr5bit.vhd
--                          defer_state.vhd
--                       bocntr.vhd
--                          lfsr16.vhd
--                       msh_cnt.vhd
--                       ld_arith_reg.vhd
--
-------------------------------------------------------------------------------
-- Author:    PVK
-- History:    
-- PVK        06/07/2010     First Version
-- ^^^^^^
-- First version.  
-- ~~~~~~
-------------------------------------------------------------------------------
-- Naming Conventions:
--      active low signals:                     "*_n"
--      clock signals:                          "Clk", "clk_div#", "clk_#x" 
--      reset signals:                          "Rst", "rst_n" 
--      generics:                               "C_*" 
--      user defined types:                     "*_TYPE" 
--      state machine next state:               "*_ns" 
--      state machine current state:            "*_cs" 
--      combinatorial signals:                  "*_com" 
--      pipelined or register delay signals:    "*_d#" 
--      counter signals:                        "*cnt*"
--      clock enable signals:                   "*_ce" 
--      internal version of output port         "*_i"
--      device pins:                            "*_pin" 
--      ports:                                  - Names begin with Uppercase 
--      processes:                              "*_PROCESS" 
--      component instantiations:               "<ENTITY_>I_<#|FUNC>
-------------------------------------------------------------------------------
-- 
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

-------------------------------------------------------------------------------
-- axi_ethernetlite_v3_0_18 library is used for axi_ethernetlite_v3_0_18 
-- component declarations
-------------------------------------------------------------------------------
library axi_ethernetlite_v3_0_18;
use axi_ethernetlite_v3_0_18.mac_pkg.all;

-- synopsys translate_off
-- Library XilinxCoreLib;
-- synopsys translate_on

-------------------------------------------------------------------------------
-- Port Declaration
-------------------------------------------------------------------------------
-- Definition of Ports:
--
--  Clk          -- System Clock
--  Rst          -- System Reset
--  Clken        -- Clock enable
--  InitBackoff  -- Backoff initialized
--  RetryCnt     -- Retry count
--  BackingOff   -- Backing off from transmit
-------------------------------------------------------------------------------
-- ENTITY
-------------------------------------------------------------------------------
entity bocntr is

  port (
    Clk         : in  std_logic; -- tx Clk based (2.5 or 25 MHz)
    Clken       : in  std_logic;  
    Rst         : in  std_logic;
    InitBackoff : in  std_logic;  
    RetryCnt    : in std_logic_vector(0 to 4);    
    BackingOff  : out std_logic);

end bocntr;

-------------------------------------------------------------------------------
-- Definition of Generics:
--          No Generics were used for this Entity.
--
-- Definition of Ports:
--         
-------------------------------------------------------------------------------

architecture implementation of bocntr is

attribute DowngradeIPIdentifiedWarnings: string;

attribute DowngradeIPIdentifiedWarnings of implementation : architecture is "yes";

-------------------------------------------------------------------------------
--  Constant Declarations
-------------------------------------------------------------------------------
-- Constants used in this design are found in mac_pkg.vhd
-------------------------------------------------------------------------------
-- Signal and Type Declarations
-------------------------------------------------------------------------------

  type StateName is (idle, shifting, inBackoff);

  signal thisState          : StateName;
  signal nextState          : StateName;
  signal initBackoffLtch    : std_logic;
  signal initBackoffLtchRst : std_logic;
  signal backingOff_i       : std_logic;
  signal lfsrOut            : std_logic;
  signal slotCntRst         : std_logic;
  signal slotCntEnbl        : std_logic;
  signal slotCnt            : std_logic_vector(0 to 6);
  signal backOffCntLd       : std_logic;
  signal backOffCntEnbl     : std_logic;
  signal backOffCnt         : std_logic_vector(0 to 9);
  signal shftCntLd          : std_logic;
  signal shftCntEnbl        : std_logic;
  signal shftCnt            : std_logic_vector(0 to 3);
  signal shftRst            : std_logic;
  signal shftEnbl           : std_logic;
  signal shftData           : std_logic_vector(0 to 9);
  signal slotDone           : std_logic;
  signal numRetries         : std_logic_vector(0 to 3);
  
-------------------------------------------------------------------------------
-- Component Declarations
-------------------------------------------------------------------------------

begin

  LFSRP : entity axi_ethernetlite_v3_0_18.lfsr16
    port map(
             Rst     => Rst,
             Clk     => Clk,
             Clken   => Clken,
             Enbl    => shftEnbl,
             Shftout => lfsrOut);

   numRetries <= "1010" when (((RetryCnt(1) = '1') and     -- 8 or larger and
                             ((RetryCnt(3) = '1') or       -- 10, 11, 14, 15 or
                             (RetryCnt(2) = '1'))) or      -- 12 thru 15
                             (RetryCnt(0) = '1')) else     -- 12 thru 15
                             RetryCnt(1 to 4);             -- 9 or less
 -------------------------------------------------------------------------------
 -- INT_SHFT_PROCESS
 -------------------------------------------------------------------------------
  INT_SHFT_PROCESS : process (Clk)
  begin
    if (Clk'event and Clk = '1') then
      if (Clken = '1') then
        if shftRst = '1' then
          shftData <= (others => '0');
        elsif (shftEnbl = '1') then
          shftData(9) <= lfsrOut;
          shftData(8) <= shftData(9);
          shftData(7) <= shftData(8);
          shftData(6) <= shftData(7);
          shftData(5) <= shftData(6);
          shftData(4) <= shftData(5);
          shftData(3) <= shftData(4);
          shftData(2) <= shftData(3);
          shftData(1) <= shftData(2);
          shftData(0) <= shftData(1);
        -- coverage off
        else
         null;
        -- coverage on
        end if;
      end if;
    end if;
  end process INT_SHFT_PROCESS;

 -------------------------------------------------------------------------------
 -- INT_SLOT_COUNT_PROCESS
 -------------------------------------------------------------------------------
  INT_SLOT_COUNT_PROCESS: process (Clk)
  begin
    if (Clk'event and Clk = '1') then
      if (Clken = '1') then
        if ((slotCntRst = '1') or (slotDone = '1')) then
          slotCnt <= "1111111";
        elsif (slotCntEnbl = '1' and not(slotCnt = "0000000")) then
          slotCnt <= slotCnt - 1;
    -- coverage off
        else
          null;
    -- coverage on
        end if;
      end if;
    end if;
  end process INT_SLOT_COUNT_PROCESS;

 -------------------------------------------------------------------------------
 -- INT_BACKOFF_COUNT_PROCESS
 -------------------------------------------------------------------------------
  INT_BACKOFF_COUNT_PROCESS: process (Clk)
  begin  -- 
    if (Clk'event and Clk = '1') then
      if (Clken = '1') then
        if (backOffCntLd = '1') then
          backOffCnt <= shftData;
        elsif (backOffCntEnbl = '1' and not(backOffCnt = "0000000000") and 
              (slotDone = '1')) then
          backOffCnt <= backOffCnt - 1;
        -- coverage off
        else
        null;
        -- coverage on

        end if;
      end if;
    end if;
  end process INT_BACKOFF_COUNT_PROCESS;  

 -------------------------------------------------------------------------------
 -- INT_SHIFT_COUNT_PROCESS
 -------------------------------------------------------------------------------
  INT_SHIFT_COUNT_PROCESS: process (Clk)
  begin  -- 
    if (Clk'event and Clk = '1') then
      if (Clken = '1') then
        if (shftCntLd = '1') then
          shftCnt <= numRetries;
        elsif (shftCntEnbl = '1' and not(shftCnt = "0000")) then
          shftCnt <= shftCnt - 1;
        -- coverage off
        else
        null;
        -- coverage on
        end if;
      end if;
    end if;
  end process INT_SHIFT_COUNT_PROCESS;  

 -------------------------------------------------------------------------------
 -- INT_BACKOFFDONE_PROCESS
 -------------------------------------------------------------------------------
  INT_BACKOFFDONE_PROCESS: process (Clk)
  begin  -- 
    if (Clk'event and Clk = '1') then
      if (Rst = '1') then
        backingOff_i <= '0';
      elsif (InitBackoff = '1') then
        backingOff_i <= '1';
      elsif ((backOffCntEnbl = '1') and  (backOffCnt = "000000000")) then
        backingOff_i <= '0';
      -- coverage off
      else
       null;
      -- coverage on
      end if;
    end if;
  end process INT_BACKOFFDONE_PROCESS;

  BackingOff <= backingOff_i;

 -------------------------------------------------------------------------------
 -- INT_SLOT_TIME_DONE_PROCESS
 -------------------------------------------------------------------------------
  INT_SLOT_TIME_DONE_PROCESS: process (Clk)
  begin  -- 
    if (Clk'event and Clk = '1') then
      if (Rst = '1') then
        slotDone <= '0';
      elsif (slotCntEnbl = '0') then
        slotDone <= '0';
      elsif ((slotDone = '1') and (Clken = '1')) then
        slotDone <= '0';
      elsif ((slotCntEnbl = '1') and  (slotCnt = "0000000")) then
        slotDone <= '1';
      else
        null;
      end if;
    end if;
  end process INT_SLOT_TIME_DONE_PROCESS;
  
 -------------------------------------------------------------------------------
 -- INT_LATCH_PROCESS
 -------------------------------------------------------------------------------
  INT_LATCH_PROCESS: process (Clk)
  begin  -- 
    if (Clk'event and Clk = '1') then
      if (Rst = '1') then
        initBackoffLtch <= '0';
      elsif (InitBackoff = '1') then
        initBackoffLtch <= '1';
      elsif (initBackoffLtchRst = '1') then
        initBackoffLtch <= '0';
      -- coverage off
      else
       null;
      -- coverage on
      -- coverage on
      end if;
    end if;
  end process INT_LATCH_PROCESS;
  
-------------------------------------------------------------------------------
-- An FSM that deals with backing off
-------------------------------------------------------------------------------
  FSMR : process (Clk)
  begin  --
    if (Clk'event and Clk = '1') then     -- rising clock edge
       if (Rst = '1') then
          thisState <= idle;
        elsif (Clken = '1') then
          thisState <= nextState;
       end if;
    end if;
  end process FSMR;

-------------------------------------------------------------------------------
-- State Machine
-------------------------------------------------------------------------------
  FSMC : process (thisState,initBackoffLtch,shftCnt,backOffCnt)
  begin  --
    case thisState is
      when idle =>
        if (initBackoffLtch = '1') then
          nextState <= shifting;
        else
          nextState <= idle;
        end if;
      when shifting =>
        if (shftCnt = "0000") then
          nextState <= inBackoff;
        else
          nextState <= shifting;
        end if;
      when inBackoff =>
        if (backOffCnt = "000000000") then
          nextState <= idle;
        else
          nextState <= inBackoff;
        end if;              
      -- coverage off
      when others         => null;
        nextState <= idle;
      -- coverage on
    end case;
  end process FSMC;

-------------------------------------------------------------------------------
-- State Machine Control signals generation
-------------------------------------------------------------------------------
  FSMD : process(thisState)
  
  begin
         
   if (thisState =  idle) then
     shftRst <= '1';
     shftCntLd <= '1';
   else
     shftRst <= '0';
     shftCntLd <= '0';
   end if;
   
   if ((thisState = idle) or (thisState =  shifting)) then
     slotCntRst   <= '1';
     backOffCntLd <= '1';
   else
     slotCntRst   <= '0';
     backOffCntLd <= '0';
   end if;

   if (thisState =  shifting) then
     shftCntEnbl <= '1';
     shftEnbl <= '1';
     initBackoffLtchRst <= '1';
   else
     shftCntEnbl <= '0';
     shftEnbl <= '0';
     initBackoffLtchRst <= '0';
   end if;
   
   if (thisState =  inBackoff) then
     slotCntEnbl <= '1';
     backOffCntEnbl <= '1';
   else
     slotCntEnbl <= '0';
     backOffCntEnbl <= '0';
   end if;
   
  end process FSMD;   
  
end implementation;


-------------------------------------------------------------------------------
-- transmit - entity/architecture pair
-------------------------------------------------------------------------------
--  ***************************************************************************
--  ** DISCLAIMER OF LIABILITY                                               **
--  **                                                                       **
--  **  This file contains proprietary and confidential information of       **
--  **  Xilinx, Inc. ("Xilinx"), that is distributed under a license         **
--  **  from Xilinx, and may be used, copied and/or disclosed only           **
--  **  pursuant to the terms of a valid license agreement with Xilinx.      **
--  **                                                                       **
--  **  XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION                **
--  **  ("MATERIALS") "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER           **
--  **  EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING WITHOUT                  **
--  **  LIMITATION, ANY WARRANTY WITH RESPECT TO NONINFRINGEMENT,            **
--  **  MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE. Xilinx        **
--  **  does not warrant that functions included in the Materials will       **
--  **  meet the requirements of Licensee, or that the operation of the      **
--  **  Materials will be uninterrupted or error-free, or that defects       **
--  **  in the Materials will be corrected. Furthermore, Xilinx does         **
--  **  not warrant or make any representations regarding use, or the        **
--  **  results of the use, of the Materials in terms of correctness,        **
--  **  accuracy, reliability or otherwise.                                  **
--  **                                                                       **
--  **  Xilinx products are not designed or intended to be fail-safe,        **
--  **  or for use in any application requiring fail-safe performance,       **
--  **  such as life-support or safety devices or systems, Class III         **
--  **  medical devices, nuclear facilities, applications related to         **
--  **  the deployment of airbags, or any other applications that could      **
--  **  lead to death, personal injury or severe property or                 **
--  **  environmental damage (individually and collectively, "critical       **
--  **  applications"). Customer assumes the sole risk and liability         **
--  **  of any use of Xilinx products in critical applications,              **
--  **  subject only to applicable laws and regulations governing            **
--  **  limitations on product liability.                                    **
--  **                                                                       **
--  **  Copyright 2010 Xilinx, Inc.                                          **
--  **  All rights reserved.                                                 **
--  **                                                                       **
--  **  This disclaimer and copyright notice must be retained as part        **
--  **  of this file at all times.                                           **
--  ***************************************************************************
--
-------------------------------------------------------------------------------
-- Filename     : transmit.vhd
-- Version      : v2.0
-- Description  : This is the transmit path portion of the design
-- VHDL-Standard: VHDL'93
-------------------------------------------------------------------------------
-- Structure:   
--
--  axi_ethernetlite.vhd
--      \
--      \-- axi_interface.vhd
--      \-- xemac.vhd
--           \
--           \-- mdio_if.vhd
--           \-- emac_dpram.vhd                     
--           \    \                     
--           \    \-- RAMB16_S4_S36
--           \                          
--           \    
--           \-- emac.vhd                     
--                \                     
--                \-- MacAddrRAM                   
--                \-- receive.vhd
--                \      rx_statemachine.vhd
--                \      rx_intrfce.vhd
--                \         async_fifo_fg.vhd
--                \      crcgenrx.vhd
--                \                     
--                \-- transmit.vhd
--                       crcgentx.vhd
--                          crcnibshiftreg
--                       tx_intrfce.vhd
--                          async_fifo_fg.vhd
--                       tx_statemachine.vhd
--                       deferral.vhd
--                          cntr5bit.vhd
--                          defer_state.vhd
--                       bocntr.vhd
--                          lfsr16.vhd
--                       msh_cnt.vhd
--                       ld_arith_reg.vhd
--
-------------------------------------------------------------------------------
-- Author:    PVK
-- History:    
-- PVK        06/07/2010     First Version
-- ^^^^^^
-- First version.  
-- ~~~~~~
-------------------------------------------------------------------------------
-- Naming Conventions:
--      active low signals:                     "*_n"
--      clock signals:                          "Clk", "clk_div#", "clk_#x" 
--      reset signals:                          "Rst", "rst_n" 
--      generics:                               "C_*" 
--      user defined types:                     "*_TYPE" 
--      state machine next state:               "*_ns" 
--      state machine current state:            "*_cs" 
--      combinatorial signals:                  "*_com" 
--      pipelined or register delay signals:    "*_d#" 
--      counter signals:                        "*cnt*"
--      clock enable signals:                   "*_ce" 
--      internal version of output port         "*_i"
--      device pins:                            "*_pin" 
--      ports:                                  - Names begin with Uppercase 
--      processes:                              "*_PROCESS" 
--      component instantiations:               "<ENTITY_>I_<#|FUNC>
-------------------------------------------------------------------------------
-- 
library ieee;
use ieee.std_logic_1164.all;
--use ieee.std_logic_misc.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;
--use ieee.numeric_std."-";

-------------------------------------------------------------------------------
-- axi_ethernetlite_v3_0_18 library is used for axi_ethernetlite_v3_0_18 
-- component declarations
-------------------------------------------------------------------------------
library axi_ethernetlite_v3_0_18;
use axi_ethernetlite_v3_0_18.mac_pkg.all;

-------------------------------------------------------------------------------
library lib_cdc_v1_0_2;
use lib_cdc_v1_0_2.all;

-- synopsys translate_off
-- Library XilinxCoreLib;
--library simprim;
-- synopsys translate_on

library unisim;
use unisim.Vcomponents.all;

-------------------------------------------------------------------------------
-- Definition of Generics:
-------------------------------------------------------------------------------
--  C_DUPLEX               -- 1 = full duplex, 0 = half duplex
--  C_FAMILY               -- Target device family 
-------------------------------------------------------------------------------
-- Definition of Ports:
--
--  Clk                   -- System Clock
--  Rst                   -- System Reset
--  NibbleLength          -- Transmit frame nibble length
--  NibbleLength_orig     -- Transmit nibble length before pkt adjustment
--  En_pad                -- Enable padding
--  TxClkEn               -- Transmit clock enable
--  Phy_tx_clk            -- PHY TX Clock
--  Phy_crs               -- Ethernet carrier sense
--  Phy_col               -- Ethernet collision indicator
--  Phy_tx_en             -- Ethernet transmit enable
--  Phy_tx_data           -- Ethernet transmit data
--  Tx_addr_en            -- TX buffer address enable
--  Tx_start              -- TX start
--  Tx_done               -- TX complete
--  Tx_pong_ping_l        -- TX Ping/Pong buffer enable
--  Tx_idle               -- TX idle
--  Tx_DPM_ce             -- TX buffer chip enable
--  Tx_DPM_wr_data        -- TX buffer write data
--  Tx_DPM_rd_data        -- TX buffer read data
--  Tx_DPM_wr_rd_n        -- TX buffer write/read enable
--  Transmit_start        -- Transmit start
--  Mac_program_start     -- MAC Program start
--  Mac_addr_ram_we       -- MAC Address RAM write enable
--  Mac_addr_ram_addr_wr  -- MAC Address RAM write address
-------------------------------------------------------------------------------
-- ENTITY
-------------------------------------------------------------------------------
entity transmit is
  generic 
   (
    C_DUPLEX : integer := 1; -- 1 = full duplex, 0 = half duplex   
    C_FAMILY : string  := "virtex6"  
   );
  port 
   ( 
    Clk                  : in  std_logic;
    Rst                  : in  std_logic;
    NibbleLength         : in  std_logic_vector(0 to 11);
    NibbleLength_orig    : in  std_logic_vector(0 to 11);
    En_pad               : in  std_logic; 
    TxClkEn              : in  std_logic;
    Phy_tx_clk           : in  std_logic;
    Phy_crs              : in  std_logic;
    Phy_col              : in  std_logic;
    Phy_tx_en            : out std_logic;
    Phy_tx_data          : out std_logic_vector (0 to 3);
    Tx_addr_en           : out std_logic;
    Tx_start             : out std_logic;
    Tx_done              : out std_logic;
    Tx_pong_ping_l       : in  std_logic;
    Tx_idle              : out std_logic;
    Tx_DPM_ce            : out std_logic;
    Tx_DPM_wr_data       : out std_logic_vector (0 to 3);
    Tx_DPM_rd_data       : in  std_logic_vector (0 to 3);
    Tx_DPM_wr_rd_n       : out std_logic;
    Transmit_start       : in  std_logic;
    Mac_program_start    : in  std_logic;
    Mac_addr_ram_we      : out std_logic;
    Mac_addr_ram_addr_wr : out std_logic_vector(0 to 3)    
        
    );
end transmit;

-------------------------------------------------------------------------------
-- Definition of Generics:
--          No Generics were used for this Entity.
--
-- Definition of Ports:
--         
-------------------------------------------------------------------------------

architecture imp of transmit is

attribute DowngradeIPIdentifiedWarnings: string;

attribute DowngradeIPIdentifiedWarnings of imp : architecture is "yes";

-------------------------------------------------------------------------------
--  Constant Declarations
-------------------------------------------------------------------------------
constant logic_one               :std_logic := '1';

-------------------------------------------------------------------------------
--  Signal and Type Declarations
-------------------------------------------------------------------------------
signal tx_d_rst                  : std_logic;
signal full_half_n               : std_logic;
signal bus_combo                 : std_logic_vector (0 to 5);
signal txChannelReset            : std_logic;
signal emac_tx_wr_i              : std_logic;
signal txfifo_full               : std_logic;
signal txfifo_empty              : std_logic;
signal tx_en_i                   : std_logic;
signal tx_en_i_tx_clk            : std_logic;
signal fifo_tx_en                : std_logic;
signal axi_fifo_tx_en                : std_logic;
signal txNibbleCntLd             : std_logic;
signal txNibbleCntEn             : std_logic;
signal txNibbleCntRst            : std_logic;
signal txComboNibbleCntRst       : std_logic;
--signal phy_tx_en_i               : std_logic;
signal phy_tx_en_i_p             : std_logic;
signal axi_phy_tx_en_i_p             : std_logic;
signal deferring                 : std_logic;
signal txBusFifoWrCntRst         : std_logic;
signal txBusFifoWrCntEn          : std_logic;
signal txComboBusFifoWrCntRst    : std_logic;
signal txComboBusFifoWrCntEn     : std_logic;
signal txComboColRetryCntRst_n   : std_logic;
signal txComboBusFifoRst         : std_logic;
signal txColRetryCntRst_n        : std_logic;
signal enblPreamble              : std_logic;
signal enblSFD                   : std_logic;
signal enblData                  : std_logic;
signal enblJam                   : std_logic;
signal enblCRC                   : std_logic;
signal txCntEnbl                 : std_logic;
signal txColRetryCntEnbl         : std_logic;
signal jamTxComboNibCntEnbl      : std_logic;
signal txRetryRst                : std_logic;
signal txLateColnRst             : std_logic;
signal initBackoff               : std_logic;
signal backingOff_i              : std_logic;
signal txCrcShftOutEn            : std_logic;
signal txCrcEn                   : std_logic;
signal crcComboRst               : std_logic;
signal emac_tx_wr_data_i         : std_logic_vector (0 to 3);
signal crcCnt                    : std_logic_vector(0 to 3);
signal collisionRetryCnt         : std_logic_vector(0 to 4);
signal jamTxNibbleCnt            : std_logic_vector(0 to 3);
signal colWindowNibbleCnt        : std_logic_vector(0 to 7);
signal prb                       : std_logic_vector(0 to 3);
signal sfd                       : std_logic_vector(0 to 3);
signal jam                       : std_logic_vector(0 to 3);
signal crc                       : std_logic_vector(0 to 3);
signal currentTxBusFifoWrCnt     : std_logic_vector(0 to 11);
signal currentTxNibbleCnt        : std_logic_vector (0 to 11);
signal phy_tx_en_n               : std_logic;
signal txComboColRetryCntRst     : std_logic;
signal phy_tx_en_i_n             : std_logic;
signal jam_rst                   : std_logic;
signal txExcessDefrlRst          : std_logic;
signal enblclear                 : std_logic;
signal tx_en_mod                 : std_logic;
signal emac_tx_wr_mod            : std_logic;
signal pre_sfd_done              : std_logic;
signal mux_in_data               : std_logic_vector (0 to 6*4-1);
signal mux_in_sel                : std_logic_vector (0 to 5);
signal transmit_data             : std_logic_vector (0 to 3);
signal txNibbleCnt_pad           : unsigned (0 to 11);
signal tx_idle_i                 : std_logic;
signal emac_tx_wr_data_d1        : std_logic_vector (0 to 3);
signal emac_tx_wr_d1             : std_logic;
signal txcrcen_d1                : std_logic;


-------------------------------------------------------------------------------
-- Component Declarations
-------------------------------------------------------------------------------
component FDRE
  port 
   (
    Q  : out std_logic;
    C  : in std_logic;
    CE : in std_logic;
    D  : in std_logic;
    R  : in std_logic
   );
end component;

component FDCE
  port 
   (
    Q   : out std_logic;
    C   : in std_logic;
    CE  : in std_logic;
    CLR : in std_logic;
    D   : in std_logic
    );
end component;  


begin

   ----------------------------------------------------------------------------
   -- tx crc generator
   ----------------------------------------------------------------------------
   INST_CRCGENTX: entity axi_ethernetlite_v3_0_18.crcgentx
      port map 
         (
         Clk     => Clk,
         Rst     => crcComboRst,
         Clken   => emac_tx_wr_d1,
         Data    => emac_tx_wr_data_d1,
         DataEn  => txcrcen_d1,
         OutEn   => txCrcShftOutEn,
         CrcNibs => crc
      );      
               
   crcComboRst <= Rst or not (tx_en_i); -- having tx_en form same clock domain as Clk
--   crcComboRst <= Rst or not (fifo_tx_en);
   
   ----------------------------------------------------------------------------
   -- tx interface contains the ethernet tx fifo
   ----------------------------------------------------------------------------
   INST_TX_INTRFCE: entity axi_ethernetlite_v3_0_18.tx_intrfce
      generic map
         (
         C_FAMILY => C_FAMILY
         )
      port map 
         (     
         Clk               => Clk,
         Rst               => txComboBusFifoRst,
         Phy_tx_clk        => Phy_tx_clk,
         Emac_tx_wr_data   => emac_tx_wr_data_i,
         Tx_er             => '0',      
         PhyTxEn           => tx_en_mod,
         Tx_en             => fifo_tx_en,
         Fifo_empty        => txfifo_empty,
         Fifo_full         => txfifo_full,
         Emac_tx_wr        => emac_tx_wr_mod,
         Phy_tx_data       => bus_combo
       );    
               
   txComboBusFifoRst <= Rst or txRetryRst or
                       (jam_rst and not(full_half_n) and 
                        Phy_col and pre_sfd_done);
   
   jam               <= "0110"; -- arbitrary
   prb               <= "0101"; -- transmitted as 1010
   sfd               <= "1101"; -- transmitted as 1011
   
   

   ----------------------------------------------------------------------------
   -- PHY output selection 
   ----------------------------------------------------------------------------
   mux_in_sel  <= enblPreamble & enblSFD & enblData & enblJam & enblCRC & 
                                                                logic_one;
                                                                
   mux_in_data <= prb & sfd & transmit_data & jam & crc & "0000";
   
   transmit_data <= "0000" when (txNibbleCnt_pad = 0) else
                    Tx_DPM_rd_data;                 
   
   Tx_idle <= tx_idle_i;
   ----------------------------------------------------------------------------
   -- Multiplexing PHY transmit data
   ----------------------------------------------------------------------------
   ONR_HOT_MUX:entity axi_ethernetlite_v3_0_18.mux_onehot_f
   
      generic map 
         ( 
         C_DW => 4, 
         C_NB => 6,
         C_FAMILY => C_FAMILY
        )
   
     port map 
         (
         D => mux_in_data, 
         S => mux_in_sel, 
         Y => emac_tx_wr_data_i
        );
   
   
   ----------------------------------------------------------------------------
   -- PHY transmit data
   ----------------------------------------------------------------------------
   Phy_tx_data <= "0110" when (Phy_col = '1' or 
                               not(jamTxNibbleCnt(0) = '1' and
                                   jamTxNibbleCnt(1) = '0' and 
                                   jamTxNibbleCnt(2) = '0' and
                                   jamTxNibbleCnt(3) = '1')) and 
                               full_half_n = '0' and 
                               not (jamTxNibbleCnt = "0000") and 
                               pre_sfd_done = '1'                    else
                  
                  "0000" when jamTxNibbleCnt = "0000" and 
                              full_half_n = '0'                      else
                  "0000" when axi_phy_tx_en_i_p = '0'                        else            
                  bus_combo(0 to 3);
   
   ----------------------------------------------------------------------------
   -- PHY transmit enable
   ----------------------------------------------------------------------------
   Phy_tx_en   <= '1' when (Phy_col = '1' or 
                            not(jamTxNibbleCnt(0) = '1' and
                                jamTxNibbleCnt(1) = '0' and 
                                jamTxNibbleCnt(2) = '0' and
                                jamTxNibbleCnt(3) = '1')) and 
                            full_half_n = '0' and
                            not (jamTxNibbleCnt = "0000") and 
                            pre_sfd_done = '1'                       else 
                  
                  '0' when jamTxNibbleCnt = "0000" and 
                           full_half_n = '0'                         else
                  '0' when axi_phy_tx_en_i_p = '0'                           else
                  bus_combo(5);
   
   ----------------------------------------------------------------------------
   -- transmit packet fifo read nibble counter
   ----------------------------------------------------------------------------
   INST_TXNIBBLECOUNT: entity axi_ethernetlite_v3_0_18.ld_arith_reg
      generic  map
        (
         C_ADD_SUB_NOT => false,
         C_REG_WIDTH   => 12,
         C_RESET_VALUE => "000000000000",
         C_LD_WIDTH    => 12,
         C_LD_OFFSET   => 0,
         C_AD_WIDTH    => 12,
         C_AD_OFFSET   => 0
        )
      port map
        (
         CK       => Clk,
         Rst      => txComboNibbleCntRst,
         Q        => currentTxNibbleCnt,
         LD       => NibbleLength,
         AD       => "000000000001",
         LOAD     => txNibbleCntLd,
         OP       => txNibbleCntEn
        );
       
   txComboNibbleCntRst <= Rst or txNibbleCntRst or txRetryRst;
                                  
   
   ----------------------------------------------------------------------------
   -- PROCESS : PKT_TX_PAD_COUNTER 
   ----------------------------------------------------------------------------
   -- This counter is used to check if the receive packet length is greater 
   -- minimum packet length (64 byte - 128 nibble)
   ----------------------------------------------------------------------------
   PKT_TX_PAD_COUNTER : process(Clk)
   begin
      if (Clk'event and Clk='1') then
         if (Rst=RESET_ACTIVE) then
            txNibbleCnt_pad <= (others=>'0');
         elsif (enblSFD='1') then  -- load the counter for minimum 
            txNibbleCnt_pad <= unsigned(NibbleLength_orig); -- packet length
         elsif (enblData='1' and En_pad='1') then        -- Enable Down Counter
            if (txNibbleCnt_pad=0 ) then  
               txNibbleCnt_pad <= (others=>'0');
            else
               txNibbleCnt_pad <= txNibbleCnt_pad-1;
            end if;
         end if;
      end if;
   end process PKT_TX_PAD_COUNTER;
   
   ----------------------------------------------------------------------------
   -- transmit state machine
   ----------------------------------------------------------------------------
   INST_TX_STATE_MACHINE: entity axi_ethernetlite_v3_0_18.tx_statemachine
      generic map 
        (
         C_DUPLEX             => C_DUPLEX
        )    
      port map
        (
         Clk                  =>  Clk,
         Rst                  =>  txChannelReset,
         TxClkEn              =>  TxClkEn,
         Jam_rst              =>  jam_rst,
         TxRst                =>  txChannelReset, 
         Deferring            =>  deferring,
         ColRetryCnt          =>  collisionRetryCnt,
         ColWindowNibCnt      =>  colWindowNibbleCnt,
         JamTxNibCnt          =>  jamTxNibbleCnt,
         TxNibbleCnt          =>  currentTxNibbleCnt,
         BusFifoWrNibbleCnt   =>  currentTxBusFifoWrCnt,
         CrcCnt               =>  crcCnt,
         BusFifoFull          =>  txfifo_full, 
         BusFifoEmpty         =>  txfifo_empty,
         PhyCollision         =>  Phy_col,
         InitBackoff          =>  initBackoff,
         TxRetryRst           =>  txRetryRst,
         TxExcessDefrlRst     =>  txExcessDefrlRst,
         TxLateColnRst        =>  txLateColnRst,
         TxColRetryCntRst_n   =>  txColRetryCntRst_n,
         TxColRetryCntEnbl    =>  txColRetryCntEnbl,
         TxNibbleCntRst       =>  txNibbleCntRst,
         TxEnNibbleCnt        =>  txNibbleCntEn, 
         TxNibbleCntLd        =>  txNibbleCntLd, 
         BusFifoWrCntRst      =>  txBusFifoWrCntRst,
         BusFifoWrCntEn       =>  txBusFifoWrCntEn,
         EnblPre              =>  enblPreamble,
         EnblSFD              =>  enblSFD,
         EnblData             =>  enblData,
         EnblJam              =>  enblJam,
         EnblCRC              =>  enblCRC,
         BusFifoWr            =>  emac_tx_wr_i,
         Phytx_en             =>  tx_en_i, 
         TxCrcEn              =>  txCrcEn,
         TxCrcShftOutEn       =>  txCrcShftOutEn,
         Tx_addr_en           =>  Tx_addr_en,          
         Tx_start             =>  Tx_start, 
         Tx_done              =>  Tx_done,
         Tx_pong_ping_l       =>  Tx_pong_ping_l,
         Tx_idle              =>  tx_idle_i,
         Tx_DPM_ce            =>  Tx_DPM_ce,           
         Tx_DPM_wr_data       =>  Tx_DPM_wr_data,      
         Tx_DPM_wr_rd_n       =>  Tx_DPM_wr_rd_n,      
         Enblclear            =>  enblclear,
         Transmit_start       =>  Transmit_start,
         Mac_program_start    =>  Mac_program_start,
         Mac_addr_ram_we      =>  Mac_addr_ram_we,     
         Mac_addr_ram_addr_wr =>  Mac_addr_ram_addr_wr,
         Pre_sfd_done         =>  pre_sfd_done
        );
        
   ----------------------------------------------------------------------------
   -- deferral control
   ----------------------------------------------------------------------------
   full_half_n <= '1'when C_DUPLEX = 1 else
                  '0';  
                  
   INST_DEFERRAL_CONTROL: entity axi_ethernetlite_v3_0_18.deferral
      port map
        (
         Clk         => Clk,
         Rst         => Rst,
         TxEn        => tx_en_i, 
         TxRst       => txChannelReset,
         Tx_clk_en   => TxClkEn,
         BackingOff  => backingOff_i,
         Crs         => Phy_crs,
         Full_half_n => full_half_n,
         Ifgp1       => "10000",
         Ifgp2       => "01000",
         Deferring   => deferring
       );            
              
   ----------------------------------------------------------------------------
   -- transmit bus fifo write nibble counter
   ----------------------------------------------------------------------------
   INST_TXBUSFIFOWRITENIBBLECOUNT: entity axi_ethernetlite_v3_0_18.ld_arith_reg
      generic  map
        (
         C_ADD_SUB_NOT => true,
         C_REG_WIDTH   => 12,
         C_RESET_VALUE => "000000000000",
         C_LD_WIDTH    => 12,
         C_LD_OFFSET   => 0,
         C_AD_WIDTH    => 12,
         C_AD_OFFSET   => 0
        )
      port map
        (
         CK       => Clk,
         Rst      => txComboBusFifoWrCntRst,
         Q        =>  currentTxBusFifoWrCnt,
         LD       => "000000000000",
         AD       => "000000000001",
         LOAD     => '0',
         OP       => txComboBusFifoWrCntEn
        );
       
   txComboBusFifoWrCntRst <= Rst or txBusFifoWrCntRst
                               or txRetryRst;
   txComboBusFifoWrCntEn  <= txBusFifoWrCntEn and emac_tx_wr_i;
   
   ----------------------------------------------------------------------------
   -- crc down counter
   ----------------------------------------------------------------------------
   phy_tx_en_n <= not(tx_en_i); -- modified to have this in lite clock domain
   
   INST_CRCCOUNTER: entity axi_ethernetlite_v3_0_18.ld_arith_reg
      generic  map
        (
         C_ADD_SUB_NOT => false,
         C_REG_WIDTH   => 4,
         C_RESET_VALUE => "1000",
         C_LD_WIDTH    => 4,
         C_LD_OFFSET   => 0,
         C_AD_WIDTH    => 4,
         C_AD_OFFSET   => 0
        )
     
      port map
        (
         CK       => Clk,
         Rst      => Rst,
         Q        =>  crcCnt,
         LD       => "1000",
         AD       => "0001",
         LOAD     => phy_tx_en_n,
         OP       => enblCRC
       );
   
   ----------------------------------------------------------------------------
   -- Full Duplex mode operation
   ----------------------------------------------------------------------------
   TX3_NOT_GENERATE: if(C_DUPLEX = 1) generate --Set outputs to zero
     
   begin
     
      collisionRetryCnt  <= (others=> '0');
      colWindowNibbleCnt <= (others=> '0');
      jamTxNibbleCnt     <= (others=> '0');
      backingOff_i       <= '0';
      
   end generate TX3_NOT_GENERATE;
   
   ----------------------------------------------------------------------------
   -- Half Duplex mode operation
   ----------------------------------------------------------------------------
   tx3_generate: if(C_DUPLEX = 0) generate --Include collision cnts when 1  
   
   ----------------------------------------------------------------------------
   -- transmit collision retry down counter
   ----------------------------------------------------------------------------
   INST_COLRETRYCNT: entity axi_ethernetlite_v3_0_18.msh_cnt
      generic map
        (
         C_ADD_SUB_NOT => true,
         C_REG_WIDTH   => 5,
         C_RESET_VALUE => "00000"
       )
     
      port map
        (
         Clk      => Clk,
         Rst      => txComboColRetryCntRst,
         Q        => collisionRetryCnt,
         Z        => open,
         LD       => "00000",
         AD       => "00001",
         LOAD     => '0',
         OP       => txColRetryCntEnbl
       );
               
   txComboColRetryCntRst_n <= not(Rst) and txColRetryCntRst_n;
   txComboColRetryCntRst   <= not txComboColRetryCntRst_n;                          
   
   ----------------------------------------------------------------------------
   -- transmit collision window nibble down counter
   ----------------------------------------------------------------------------
   INST_COLWINDOWNIBCNT: entity axi_ethernetlite_v3_0_18.msh_cnt
      generic  map
        (
         C_ADD_SUB_NOT => false,
         C_REG_WIDTH   => 8,
         C_RESET_VALUE => "10001111"
       )
     
      port map
        (
         Clk      => Clk,
         Rst      => phy_tx_en_i_n,
         Q        => colWindowNibbleCnt,
         Z        => open,
         LD       => "10001111",
         AD       => "00000001",
         LOAD     => '0',
         OP       => txCntEnbl
       );   
       
   phy_tx_en_i_n <= not(axi_phy_tx_en_i_p);    
   
   ----------------------------------------------------------------------------
   -- jam transmit nibble down counter
   ----------------------------------------------------------------------------
   INST_JAMTXNIBCNT: entity axi_ethernetlite_v3_0_18.msh_cnt
      generic  map
        (
         C_ADD_SUB_NOT => false,
         C_REG_WIDTH   => 4,
         C_RESET_VALUE => "1001"
       )
     
      port map
        (
         Clk      => Clk,
         Rst      => phy_tx_en_i_n,
         Q        => jamTxNibbleCnt,
         Z        => open,
         LD       => "1001",
         AD       => "0001",
         LOAD     => '0',
         OP       => jamTxComboNibCntEnbl
       );   
       
   ----------------------------------------------------------------------------
   -- tx collision back off counter
   ----------------------------------------------------------------------------
   INST_BOCNT: entity axi_ethernetlite_v3_0_18.bocntr
      port map
        (
         Clk         => Clk,
         Clken       => TxClkEn,
         Rst         => Rst,
         InitBackoff => initBackoff,
         RetryCnt    => collisionRetryCnt,
         BackingOff  => backingOff_i
         ); 
         
   end generate tx3_generate;   
 

   ----------------------------------------------------------------------------
   -- CDC module for syncing tx_en_i in PHY_tx_clk domain
   ----------------------------------------------------------------------------
  CDC_TX_EN: entity  lib_cdc_v1_0_2.cdc_sync
  generic map (
    C_CDC_TYPE           => 1,
    C_RESET_STATE        => 0,
    C_SINGLE_BIT         => 1,
    C_FLOP_INPUT         => 0,
    C_VECTOR_WIDTH       => 1,
    C_MTBF_STAGES        => 2
            )
  port map(
    prmry_aclk            => '1',
    prmry_resetn          => '1',
    prmry_in              => tx_en_i,
    prmry_ack             => open,
    scndry_out            => tx_en_i_tx_clk,
    scndry_aclk           => Phy_tx_clk,
    scndry_resetn         => '1',
    prmry_vect_in         => (OTHERS => '0'),
    scndry_vect_out       => open
     );
---   ----------------------------------------------------------------------------
---   -- CDC module for syncing phy_tx_en_i in Clk domain
---   ----------------------------------------------------------------------------
---  CDC_PHY_TX_EN: entity  lib_cdc_v1_0_2.cdc_sync
---  generic map (
---    C_CDC_TYPE           => 1,
---    C_RESET_STATE        => 0,
---    C_SINGLE_BIT         => 1,
---    C_FLOP_INPUT         => 0,
---    C_VECTOR_WIDTH       => 1,
---    C_MTBF_STAGES        => 4
---            )
---  port map(
---    prmry_aclk            => '1',
---    prmry_resetn          => '1',
---    prmry_in              => phy_tx_en_i_p,
---    prmry_ack             => open,
---    scndry_out            => phy_tx_en_i,
---    scndry_aclk           => Clk,
---    scndry_resetn         => '1',
---    prmry_vect_in         => (OTHERS => '0'),
---    scndry_vect_out       => open
---     );     
---   ----------------------------------------------------------------------------
   -- CDC module for syncing rst in tx_clk domain
   ----------------------------------------------------------------------------
  CDC_PHY_TX_RST: entity  lib_cdc_v1_0_2.cdc_sync
  generic map (
    C_CDC_TYPE           => 1,
    C_RESET_STATE        => 0,
    C_SINGLE_BIT         => 1,
    C_FLOP_INPUT         => 0,
    C_VECTOR_WIDTH       => 1,
    C_MTBF_STAGES        => 2
            )
  port map(
    prmry_aclk            => '1',
    prmry_resetn          => '1',
    prmry_in              => Rst,
    prmry_ack             => open,
    scndry_out            => tx_d_rst,
    scndry_aclk           => Phy_tx_clk,
    scndry_resetn         => '1',
    prmry_vect_in         => (OTHERS => '0'),
    scndry_vect_out       => open
     );
   ----------------------------------------------------------------------------
   -- INT_tx_en_PROCESS
   ----------------------------------------------------------------------------
   -- This process assigns the outputs the phy enable
   ----------------------------------------------------------------------------
   INT_TX_EN_PROCESS: process (Phy_tx_clk)
   begin  -- 
      if (Phy_tx_clk'event and Phy_tx_clk = '1') then
         if (tx_d_rst = RESET_ACTIVE) then
            phy_tx_en_i_p <= '0';
            fifo_tx_en  <= '0';
         else
            fifo_tx_en  <= tx_en_i_tx_clk; -- having cdc sync for tx_en_i for MTBF
            phy_tx_en_i_p <= fifo_tx_en and tx_en_i_tx_clk;
--            fifo_tx_en  <= tx_en_i;
--            phy_tx_en_i <= fifo_tx_en and tx_en_i;
         end if;
      end if;
   end process INT_TX_EN_PROCESS;            
  

   AXI_INT_TX_EN_PROCESS: process (Clk)
   begin  -- 
      if (Clk'event and Clk = '1') then
         if (Rst = RESET_ACTIVE) then
            axi_fifo_tx_en  <= '0';
            axi_phy_tx_en_i_p <= '0';
         else
            axi_fifo_tx_en  <= tx_en_i;
            axi_phy_tx_en_i_p <= axi_fifo_tx_en and tx_en_i;
         end if;
      end if;
   end process AXI_INT_TX_EN_PROCESS;            
   


 
   emac_tx_wr_mod <= emac_tx_wr_i or enblclear;
   
   tx_en_mod      <= '0' when enblclear = '1' else
                     tx_en_i;
   txChannelReset <= Rst;
   
   
   txCntEnbl      <= TxClkEn and axi_phy_tx_en_i_p and 
                             not(not(full_half_n) and Phy_col);
   
   ----------------------------------------------------------------------------
   -- jam transmit nibble down counter enable
   ----------------------------------------------------------------------------
   jamTxComboNibCntEnbl <= (Phy_col or not(jamTxNibbleCnt(0) and
                            not(jamTxNibbleCnt(1)) and 
                            not(jamTxNibbleCnt(2)) and
                            jamTxNibbleCnt(3))) and 
                            pre_sfd_done and TxClkEn and not(full_half_n);


   ----------------------------------------------------------------------------
   -- INT_CRC_DATA_REG_PROCESS
   ----------------------------------------------------------------------------
   -- This process registers the emac data going to CRCgen Module to break long
   -- timing path. 
   ----------------------------------------------------------------------------
   INT_CRC_DATA_REG_PROCESS: process (Clk)
   begin  -- 
      if (Clk'event and Clk = '1') then
         if (Rst = RESET_ACTIVE) then
            emac_tx_wr_data_d1 <= (others=>'0');
            emac_tx_wr_d1 <= '0';
            txcrcen_d1    <= '0';
         else
            emac_tx_wr_data_d1  <= emac_tx_wr_data_i;
            emac_tx_wr_d1 <= emac_tx_wr_i;
            txcrcen_d1    <= txCrcEn;
         end if;
      end if;
   end process INT_CRC_DATA_REG_PROCESS;            
  
end imp;


-------------------------------------------------------------------------------
-- receive - entity/architecture pair
-------------------------------------------------------------------------------
--  ***************************************************************************
--  ** DISCLAIMER OF LIABILITY                                               **
--  **                                                                       **
--  **  This file contains proprietary and confidential information of       **
--  **  Xilinx, Inc. ("Xilinx"), that is distributed under a license         **
--  **  from Xilinx, and may be used, copied and/or disclosed only           **
--  **  pursuant to the terms of a valid license agreement with Xilinx.      **
--  **                                                                       **
--  **  XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION                **
--  **  ("MATERIALS") "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER           **
--  **  EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING WITHOUT                  **
--  **  LIMITATION, ANY WARRANTY WITH RESPECT TO NONINFRINGEMENT,            **
--  **  MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE. Xilinx        **
--  **  does not warrant that functions included in the Materials will       **
--  **  meet the requirements of Licensee, or that the operation of the      **
--  **  Materials will be uninterrupted or error-free, or that defects       **
--  **  in the Materials will be corrected. Furthermore, Xilinx does         **
--  **  not warrant or make any representations regarding use, or the        **
--  **  results of the use, of the Materials in terms of correctness,        **
--  **  accuracy, reliability or otherwise.                                  **
--  **                                                                       **
--  **  Xilinx products are not designed or intended to be fail-safe,        **
--  **  or for use in any application requiring fail-safe performance,       **
--  **  such as life-support or safety devices or systems, Class III         **
--  **  medical devices, nuclear facilities, applications related to         **
--  **  the deployment of airbags, or any other applications that could      **
--  **  lead to death, personal injury or severe property or                 **
--  **  environmental damage (individually and collectively, "critical       **
--  **  applications"). Customer assumes the sole risk and liability         **
--  **  of any use of Xilinx products in critical applications,              **
--  **  subject only to applicable laws and regulations governing            **
--  **  limitations on product liability.                                    **
--  **                                                                       **
--  **  Copyright 2010 Xilinx, Inc.                                          **
--  **  All rights reserved.                                                 **
--  **                                                                       **
--  **  This disclaimer and copyright notice must be retained as part        **
--  **  of this file at all times.                                           **
--  ***************************************************************************
--
-------------------------------------------------------------------------------
-- Filename     : receive.vhd
-- Version      : v2.0
-- Description  : This is the receive path portion of the design
-- VHDL-Standard: VHDL'93
-------------------------------------------------------------------------------
-- Structure:   
--
--  axi_ethernetlite.vhd
--      \
--      \-- axi_interface.vhd
--      \-- xemac.vhd
--           \
--           \-- mdio_if.vhd
--           \-- emac_dpram.vhd                     
--           \    \                     
--           \    \-- RAMB16_S4_S36
--           \                          
--           \    
--           \-- emac.vhd                     
--                \                     
--                \-- MacAddrRAM                   
--                \-- receive.vhd
--                \      rx_statemachine.vhd
--                \      rx_intrfce.vhd
--                \         async_fifo_fg.vhd
--                \      crcgenrx.vhd
--                \                     
--                \-- transmit.vhd
--                       crcgentx.vhd
--                          crcnibshiftreg
--                       tx_intrfce.vhd
--                          async_fifo_fg.vhd
--                       tx_statemachine.vhd
--                       deferral.vhd
--                          cntr5bit.vhd
--                          defer_state.vhd
--                       bocntr.vhd
--                          lfsr16.vhd
--                       msh_cnt.vhd
--                       ld_arith_reg.vhd
--
-------------------------------------------------------------------------------
-- Author:    PVK
-- History:    
-- PVK        06/07/2010     First Version
-- ^^^^^^
-- First version.  
-- ~~~~~~
-------------------------------------------------------------------------------
-- Naming Conventions:
--      active low signals:                     "*_n"
--      clock signals:                          "Clk", "clk_div#", "clk_#x" 
--      reset signals:                          "Rst", "rst_n" 
--      generics:                               "C_*" 
--      user defined types:                     "*_TYPE" 
--      state machine next state:               "*_ns" 
--      state machine current state:            "*_cs" 
--      combinatorial signals:                  "*_com" 
--      pipelined or register delay signals:    "*_d#" 
--      counter signals:                        "*cnt*"
--      clock enable signals:                   "*_ce" 
--      internal version of output port         "*_i"
--      device pins:                            "*_pin" 
--      ports:                                  - Names begin with Uppercase 
--      processes:                              "*_PROCESS" 
--      component instantiations:               "<ENTITY_>I_<#|FUNC>
-------------------------------------------------------------------------------
-- 
library ieee;
use ieee.STD_LOGIC_1164.all;

-------------------------------------------------------------------------------
-- axi_ethernetlite_v3_0_18 library is used for axi_ethernetlite_v3_0_18 
-- component declarations
-------------------------------------------------------------------------------
library axi_ethernetlite_v3_0_18;
use axi_ethernetlite_v3_0_18.all;

-- synopsys translate_off
-- Library XilinxCoreLib;
library unisim;
--library simprim;
-- synopsys translate_on

-------------------------------------------------------------------------------
-- Definition of Generics:
-------------------------------------------------------------------------------
-- C_DUPLEX               -- 1 = full duplex, 0 = half duplex
-- C_FAMILY               -- Target device family 
-------------------------------------------------------------------------------
-- Definition of Ports:
--
--  Clk                  -- System Clock
--  Rst                  -- System Reset
--  Phy_rx_clk           -- Ethernet receive clock
--  Phy_dv               -- Ethernet receive enable
--  Phy_rx_data          -- Ethernet receive data
--  Phy_rx_col           -- Ethernet collision indicator
--  Phy_rx_er            -- Ethernet receive error
--  Rx_addr_en           -- RX buufer address enable
--  Rx_start             -- Receive start
--  Rx_done              -- Receive complete
--  Rx_pong_ping_l       -- RX Ping/Pong buffer enable
--  Rx_DPM_ce            -- RX buffer chip enable
--  Rx_DPM_wr_data       -- RX buffer write data
--  Rx_DPM_rd_data       -- RX buffer read data
--  Rx_DPM_wr_rd_n       -- RX buffer write read enable
--  Rx_idle              -- RX idle
--  Mac_addr_ram_addr_rd -- MAC Addr RAM read address
--  Mac_addr_ram_data    -- MAC Addr RAM read data
--  Rx_buffer_ready      -- RX buffer ready to accept new packet
-------------------------------------------------------------------------------
-- ENTITY
-------------------------------------------------------------------------------
entity receive is
  generic 
    (
    C_DUPLEX : integer := 1; 
      -- 1 = full duplex, 0 = half duplex    
    C_FAMILY             : string  := "virtex6"  
    );
  port 
    (
    Clk                  : in  std_logic;
    Rst                  : in  std_logic;
    Phy_rx_clk           : in  std_logic;
    Phy_dv               : in  std_logic;
    Phy_rx_data          : in  std_logic_vector (0 to 3);
    Phy_rx_col           : in  std_logic;
    Phy_rx_er            : in  std_logic;
    Rx_addr_en           : out std_logic;
    Rx_start             : out std_logic;
    Rx_done              : out std_logic;
    Rx_pong_ping_l       : in  std_logic;
    Rx_DPM_ce            : out std_logic;
    Rx_DPM_wr_data       : out std_logic_vector (0 to 3);
    Rx_DPM_rd_data       : in  std_logic_vector (0 to 3);    
    Rx_DPM_wr_rd_n       : out std_logic;
    Rx_idle              : out std_logic;
    Mac_addr_ram_addr_rd : out std_logic_vector(0 to 3);
    Mac_addr_ram_data    : in  std_logic_vector (0 to 3);
    Rx_buffer_ready      : in  std_logic
    );
end receive;

-------------------------------------------------------------------------------
-- Architecture
-------------------------------------------------------------------------------  
architecture imp of receive is

attribute DowngradeIPIdentifiedWarnings: string;

attribute DowngradeIPIdentifiedWarnings of imp : architecture is "yes";

-------------------------------------------------------------------------------
--  Constant Declarations
-------------------------------------------------------------------------------


-------------------------------------------------------------------------------
--  Signal and Type Declarations
-------------------------------------------------------------------------------
signal fifo_empty_i                  : std_logic;
signal fifo_full_i                   : std_logic;
signal emac_rx_rd_i                  : std_logic;
signal emac_rx_rd_data_i             : std_logic_vector(0 to 5);
signal emac_rx_rd_data_d1            : std_logic_vector(0 to 5); -- 03-26-04
signal rxAbortRst                    : std_logic;
signal rxChannelReset                : std_logic;
signal rxBusFifoRdAck                : std_logic;
signal rxComboCrcRst                 : std_logic;
signal rxComboCrcEn                  : std_logic;
signal crcOk_i                       : std_logic;
signal rxCrcRst                      : std_logic;
signal rxCrcEn                       : std_logic;
signal rxCrcEn_d1                    : std_logic; -- 03-26-04
signal receive_enable                : std_logic; -- 03-26-04
signal fifo_reset                    : std_logic; -- 03-26-04

begin

   ----------------------------------------------------------------------------
   -- rx control state machine
   ----------------------------------------------------------------------------
   INST_RX_STATE: entity axi_ethernetlite_v3_0_18.rx_statemachine
     generic map (
       C_DUPLEX             => C_DUPLEX
       )  
     port map (
       Clk                    => Clk,
       Rst                    => rxChannelReset,
       Emac_rx_rd_data_d1     => emac_rx_rd_data_d1, -- 03-26-04
       Receive_enable         => receive_enable, -- 03-26-04
       RxBusFifoRdAck         => rxBusFifoRdAck,
       BusFifoEmpty           => fifo_empty_i,
       Collision              => Phy_rx_col,
       DataValid              => emac_rx_rd_data_i(4),
       RxError                => emac_rx_rd_data_i(5),
       BusFifoData            => emac_rx_rd_data_i(0 to 3),
       CrcOk                  => crcOk_i,
       BusFifoRd              => emac_rx_rd_i,
       RxAbortRst             => rxAbortRst,
       RxCrcRst               => rxCrcRst,
       RxCrcEn                => rxCrcEn,
       Rx_addr_en             => Rx_addr_en,
       Rx_start               => Rx_start,
       Rx_done                => Rx_done,
       Rx_pong_ping_l         => Rx_pong_ping_l,
       Rx_DPM_ce              => Rx_DPM_ce,
       Rx_DPM_wr_data         => Rx_DPM_wr_data,
       Rx_DPM_rd_data         => Rx_DPM_rd_data,
       Rx_DPM_wr_rd_n         => Rx_DPM_wr_rd_n,
       Rx_idle                => Rx_idle,
       Mac_addr_ram_addr_rd   => Mac_addr_ram_addr_rd,
       Mac_addr_ram_data      => Mac_addr_ram_data,
       Rx_buffer_ready        => Rx_buffer_ready
      );      
   
   rxChannelReset <= Rst;
  
   ----------------------------------------------------------------------------
   -- rx interface contains the ethernet rx fifo
   ----------------------------------------------------------------------------
   INST_RX_INTRFCE: entity axi_ethernetlite_v3_0_18.rx_intrfce
     generic map (
       C_FAMILY => C_FAMILY
       )
     port map (    
       Clk             => Clk,
       Rst             => fifo_reset, 
       Phy_rx_clk      => Phy_rx_clk,
       InternalWrapEn  => '0',
       Phy_rx_er       => Phy_rx_er,
       Phy_dv          => Phy_dv,
       Phy_rx_data     => Phy_rx_data,
       Rcv_en          => receive_enable, 
       Fifo_empty      => fifo_empty_i,
       Fifo_full       => fifo_full_i,
       Emac_rx_rd      => emac_rx_rd_i,
       Emac_rx_rd_data => emac_rx_rd_data_i,
       RdAck           => rxBusFifoRdAck
       );    
      
   --fifo_reset <= Rst or not(receive_enable); -- 03-26-04
     fifo_reset <= Rst; -- removing cross clock passing of signal(receive_enable is genrated in lite_clock domain and going to fifo working in rx_clk domain)
   ----------------------------------------------------------------------------
   -- crc checker
   ----------------------------------------------------------------------------
   INST_CRCGENRX: entity axi_ethernetlite_v3_0_18.crcgenrx
     port map(
       Clk     => Clk,
       Rst     => rxComboCrcRst,
       Data    => emac_rx_rd_data_i(0 to 3),
       DataEn  => rxComboCrcEn,
       CrcOk   => crcOk_i);
   
   rxComboCrcRst <= Rst or rxCrcRst or rxAbortRst;
   rxComboCrcEn  <=  rxCrcEn_d1;                 
   
   ----------------------------------------------------------------------------
   -- REG_PROCESS
   ----------------------------------------------------------------------------
   -- This process registers the received read data and receive CRC enable.
   ----------------------------------------------------------------------------
   REG_PROCESS : process (Clk)
   begin  --
      if (Clk'event and Clk = '1') then     -- rising clock edge
         if (Rst = '1') then
            emac_rx_rd_data_d1 <= "000000";
            rxCrcEn_d1 <= '0';
         else
            emac_rx_rd_data_d1 <= emac_rx_rd_data_i;   
            rxCrcEn_d1 <= rxCrcEn;
         end if;
      end if;
   end process REG_PROCESS;

end imp;


-------------------------------------------------------------------------------
-- MacAddrRAM - entity/architecture pair
-------------------------------------------------------------------------------
--  ***************************************************************************
--  ** DISCLAIMER OF LIABILITY                                               **
--  **                                                                       **
--  **  This file contains proprietary and confidential information of       **
--  **  Xilinx, Inc. ("Xilinx"), that is distributed under a license         **
--  **  from Xilinx, and may be used, copied and/or disclosed only           **
--  **  pursuant to the terms of a valid license agreement with Xilinx.      **
--  **                                                                       **
--  **  XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION                **
--  **  ("MATERIALS") "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER           **
--  **  EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING WITHOUT                  **
--  **  LIMITATION, ANY WARRANTY WITH RESPECT TO NONINFRINGEMENT,            **
--  **  MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE. Xilinx        **
--  **  does not warrant that functions included in the Materials will       **
--  **  meet the requirements of Licensee, or that the operation of the      **
--  **  Materials will be uninterrupted or error-free, or that defects       **
--  **  in the Materials will be corrected. Furthermore, Xilinx does         **
--  **  not warrant or make any representations regarding use, or the        **
--  **  results of the use, of the Materials in terms of correctness,        **
--  **  accuracy, reliability or otherwise.                                  **
--  **                                                                       **
--  **  Xilinx products are not designed or intended to be fail-safe,        **
--  **  or for use in any application requiring fail-safe performance,       **
--  **  such as life-support or safety devices or systems, Class III         **
--  **  medical devices, nuclear facilities, applications related to         **
--  **  the deployment of airbags, or any other applications that could      **
--  **  lead to death, personal injury or severe property or                 **
--  **  environmental damage (individually and collectively, "critical       **
--  **  applications"). Customer assumes the sole risk and liability         **
--  **  of any use of Xilinx products in critical applications,              **
--  **  subject only to applicable laws and regulations governing            **
--  **  limitations on product liability.                                    **
--  **                                                                       **
--  **  Copyright 2010 Xilinx, Inc.                                          **
--  **  All rights reserved.                                                 **
--  **                                                                       **
--  **  This disclaimer and copyright notice must be retained as part        **
--  **  of this file at all times.                                           **
--  ***************************************************************************
--
-------------------------------------------------------------------------------
-- Filename     : macaddram.vhd
-- Version      : v2.0
-- Description  : Design file for the Ethernet Lite MAC. 
--         There is a rom used in the MII to store the MAC address
--
--         Note that the two nibbles in each word of the MAC address
--         are transposed in order to transmit to the network in the 
--         proper order.However, the generic value (MACAddr)of this 
--         ROM keeps the normal order.
--
--         Representation of each word in this ROM (list with address order)
--
--         Addr (3 downto 0)   : netOrder(MACAddr(47 downto 32))   e.g.: 0xafec
--         Addr (7 downto 4)   : netOrder(MACAddr(31 downto 16))   e.g.: 0xedfa
--         Addr (11 downto 8)  : netOrder(MACAddr(15 downto 0))    e.g.: 0xacef
--         Addr (15 downto 12) : netOrder(Filler)    e.g.: 0x0000
--
--         Uses 4 LUTs (4 rom16x1), 0 register
--
-- VHDL-Standard:  VHDL'93
-------------------------------------------------------------------------------
-- Structure:   
--
--  axi_ethernetlite.vhd
--      \
--      \-- axi_interface.vhd
--      \-- xemac.vhd
--           \
--           \-- mdio_if.vhd
--           \-- emac_dpram.vhd                     
--           \    \                     
--           \    \-- RAMB16_S4_S36
--           \                          
--           \    
--           \-- emac.vhd                     
--                \                     
--                \-- MacAddrRAM                   
--                \-- receive.vhd
--                \      rx_statemachine.vhd
--                \      rx_intrfce.vhd
--                \         async_fifo_fg.vhd
--                \      crcgenrx.vhd
--                \                     
--                \-- transmit.vhd
--                       crcgentx.vhd
--                          crcnibshiftreg
--                       tx_intrfce.vhd
--                          async_fifo_fg.vhd
--                       tx_statemachine.vhd
--                       deferral.vhd
--                          cntr5bit.vhd
--                          defer_state.vhd
--                       bocntr.vhd
--                          lfsr16.vhd
--                       msh_cnt.vhd
--                       ld_arith_reg.vhd
--
-------------------------------------------------------------------------------
-- Author:    PVK
-- History:    
-- PVK        06/07/2010     First Version
-- ^^^^^^
-- First version.  
-- ~~~~~~
-------------------------------------------------------------------------------
-- Naming Conventions:
--      active low signals:                     "*_n"
--      clock signals:                          "Clk", "clk_div#", "clk_#x" 
--      reset signals:                          "rst", "rst_n" 
--      generics:                               "C_*" 
--      user defined types:                     "*_TYPE" 
--      state machine next state:               "*_ns" 
--      state machine current state:            "*_cs" 
--      combinatorial signals:                  "*_com" 
--      pipelined or register delay signals:    "*_d#" 
--      counter signals:                        "*cnt*"
--      clock enable signals:                   "*_ce" 
--      internal version of output port         "*_i"
--      device pins:                            "*_pin" 
--      ports:                                  - Names begin with Uppercase 
--      processes:                              "*_PROCESS" 
--      component instantiations:               "<ENTITY_>I_<#|FUNC>
-------------------------------------------------------------------------------
-- 

library ieee;
use ieee.STD_LOGIC_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

-------------------------------------------------------------------------------
-- axi_ethernetlite_v3_0_18 library is used for axi_ethernetlite_v3_0_18 
-- component declarations
-------------------------------------------------------------------------------
library axi_ethernetlite_v3_0_18;
use axi_ethernetlite_v3_0_18.mac_pkg.all;

-------------------------------------------------------------------------------
-- synopsys translate_off
-- Library XilinxCoreLib;
--library simprim;
-- synopsys translate_on

-------------------------------------------------------------------------------
-- Vcomponents from unisim library is used for FIFO instatiation
-- function declarations
-------------------------------------------------------------------------------
library unisim;
use unisim.Vcomponents.all;
-------------------------------------------------------------------------------
-- Definition of Generics:
-------------------------------------------------------------------------------
-- 
-- MACAddr              -- MAC Address
-- Filler               -- Filler
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-- Definition of Ports:
--
--   Addr                 -- Address   
--   Dout                 -- Data output
--   Din                  -- Data input
--   We                   -- Write Enable
--   Clk                  -- Clock
-------------------------------------------------------------------------------
-- ENTITY
-------------------------------------------------------------------------------
entity MacAddrRAM is
  generic
      (MACAddr : bit_vector(47 downto 0) := x"ffffffffffaa";
                                                      -- use the normal order
       Filler  : bit_vector(15 downto 0) := x"0000");
  
  port(
       Addr    : in  std_logic_vector (3 downto 0);
       Dout    : out std_logic_vector (3 downto 0);
       Din     : in  std_logic_vector (3 downto 0);
       We      : in  std_logic;
       Clk     : in  std_logic 
       );
end MacAddrRAM;

architecture imp of MacAddrRAM is

attribute DowngradeIPIdentifiedWarnings: string;

attribute DowngradeIPIdentifiedWarnings of imp : architecture is "yes";

-------------------------------------------------------------------------------
--  Constant Declarations
-------------------------------------------------------------------------------
  
-- Constants used in this design are found in mac_pkg.vhd

-------------------------------------------------------------------------------
-- Component Declarations
-------------------------------------------------------------------------------

-- The following components are the building blocks of the EMAC
--component ram16x4
--  generic(
--      INIT_00  : bit_vector(15 downto 0)  :=x"0000";-- for Addr(3 downto 0)
--      INIT_01  : bit_vector(15 downto 0)  :=x"0000";-- for Addr(7 downto 4)
--      INIT_02  : bit_vector(15 downto 0)  :=x"0000";-- for Addr(11 downto 8)
--      INIT_03  : bit_vector(15 downto 0)  :=x"0000" -- for Addr(15 downto 12)
--      );
--  port(
--    Addr : in  std_logic_vector(3 downto 0);
--    D    : in  std_logic_vector(3 downto 0);
--    We   : in  std_logic;
--    Clk  : in  std_logic;
--    Q    : out std_logic_vector(3 downto 0));
--end component;

 begin

  ram16x4i: entity axi_ethernetlite_v3_0_18.ram16x4
     generic map
       (INIT_00 => netOrder(MACAddr(47 downto 32)),
        INIT_01 => netOrder(MACAddr(31 downto 16)),
        INIT_02 => netOrder(MACAddr(15 downto 0)),
        INIT_03 => netOrder(Filler)
        )
     port map
       (Addr => Addr,
        D    => Din,
        Q    => Dout,
        We   => We,
        Clk  => Clk
        );

end imp;


-------------------------------------------------------------------------------
-- mdio_if.vhd - entity/architecture pair
-------------------------------------------------------------------------------
--  ***************************************************************************
--  ** DISCLAIMER OF LIABILITY                                               **
--  **                                                                       **
--  **  This file contains proprietary and confidential information of       **
--  **  Xilinx, Inc. ("Xilinx"), that is distributed under a license         **
--  **  from Xilinx, and may be used, copied and/or disclosed only           **
--  **  pursuant to the terms of a valid license agreement with Xilinx.      **
--  **                                                                       **
--  **  XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION                **
--  **  ("MATERIALS") "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER           **
--  **  EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING WITHOUT                  **
--  **  LIMITATION, ANY WARRANTY WITH RESPECT TO NONINFRINGEMENT,            **
--  **  MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE. Xilinx        **
--  **  does not warrant that functions included in the Materials will       **
--  **  meet the requirements of Licensee, or that the operation of the      **
--  **  Materials will be uninterrupted or error-free, or that defects       **
--  **  in the Materials will be corrected. Furthermore, Xilinx does         **
--  **  not warrant or make any representations regarding use, or the        **
--  **  results of the use, of the Materials in terms of correctness,        **
--  **  accuracy, reliability or otherwise.                                  **
--  **                                                                       **
--  **  Xilinx products are not designed or intended to be fail-safe,        **
--  **  or for use in any application requiring fail-safe performance,       **
--  **  such as life-support or safety devices or systems, Class III         **
--  **  medical devices, nuclear facilities, applications related to         **
--  **  the deployment of airbags, or any other applications that could      **
--  **  lead to death, personal injury or severe property or                 **
--  **  environmental damage (individually and collectively, "critical       **
--  **  applications"). Customer assumes the sole risk and liability         **
--  **  of any use of Xilinx products in critical applications,              **
--  **  subject only to applicable laws and regulations governing            **
--  **  limitations on product liability.                                    **
--  **                                                                       **
--  **  Copyright 2010 Xilinx, Inc.                                          **
--  **  All rights reserved.                                                 **
--  **                                                                       **
--  **  This disclaimer and copyright notice must be retained as part        **
--  **  of this file at all times.                                           **
--  ***************************************************************************
--
-------------------------------------------------------------------------------
-- Filename     : mdio_if.vhd
-- Version      : v2.0
-- Description  : This entity provides the interface between the physical layer
--                management control, and the host interface through the MAC. 
-- VHDL-Standard: VHDL'93
-------------------------------------------------------------------------------
-- Structure:   
--
--  axi_ethernetlite.vhd
--      \
--      \-- axi_interface.vhd
--      \-- xemac.vhd
--           \
--           \-- mdio_if.vhd
--           \-- emac_dpram.vhd                     
--           \    \                     
--           \    \-- RAMB16_S4_S36
--           \                          
--           \    
--           \-- emac.vhd                     
--                \                     
--                \-- MacAddrRAM                   
--                \-- receive.vhd
--                \      rx_statemachine.vhd
--                \      rx_intrfce.vhd
--                \         async_fifo_fg.vhd
--                \      crcgenrx.vhd
--                \                     
--                \-- transmit.vhd
--                       crcgentx.vhd
--                          crcnibshiftreg
--                       tx_intrfce.vhd
--                          async_fifo_fg.vhd
--                       tx_statemachine.vhd
--                       deferral.vhd
--                          cntr5bit.vhd
--                          defer_state.vhd
--                       bocntr.vhd
--                          lfsr16.vhd
--                       msh_cnt.vhd
--                       ld_arith_reg.vhd
--
-------------------------------------------------------------------------------
-- Author:    PVK
-- History:    
-- PVK        06/07/2010     First Version
-- ^^^^^^
-- First version.  
-- ~~~~~~
-------------------------------------------------------------------------------
-- Naming Conventions:
--      active low signals:                     "*_n"
--      clock signals:                          "clk", "clk_div#", "clk_#x" 
--      reset signals:                          "rst", "rst_n" 
--      generics:                               "C_*" 
--      user defined types:                     "*_TYPE" 
--      state machine next state:               "*_ns" 
--      state machine current state:            "*_cs" 
--      combinatorial signals:                  "*_com" 
--      pipelined or register delay signals:    "*_d#" 
--      counter signals:                        "*cnt*"
--      clock enable signals:                   "*_ce" 
--      internal version of output port         "*_i"
--      device pins:                            "*_pin" 
--      ports:                                  - Names begin with Uppercase 
--      processes:                              "*_PROCESS" 
--      component instantiations:               "<ENTITY_>I_<#|FUNC>
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_arith.all;

library axi_ethernetlite_v3_0_18;
use axi_ethernetlite_v3_0_18.all;

-------------------------------------------------------------------------------
-- Definition of Ports:
--
--  Clk             -- System Clock
--  Rst             -- System Reset
--  MDIO_Clk        -- 2.5Mhz clock
--  MDIO_en         -- MDIO enable
--  MDIO_OP         -- MDIO OP code
--  MDIO_Req        -- MDIO transmission request
--  MDIO_PHY_AD     -- The physical layer address
--  MDIO_REG_AD     -- The individual register address
--  MDIO_WR_DATA    -- The data to be written on MDIO
--  MDIO_RD_DATA    -- The data read from MDIO
--  PHY_MDIO_I      -- MDIO Tri-state input from PHY
--  PHY_MDIO_O      -- MDIO Tri-state output to PHY
--  PHY_MDIO_T      -- MDIO Tri-state control
--  PHY_MDC         -- 2.5Mhz communication clock to PHY
--  MDIO_done       -- RX FIFO read ack
-------------------------------------------------------------------------------
-- ENTITY
-------------------------------------------------------------------------------
entity mdio_if is
   port (
      Clk            : in std_logic;   -- System Clock  
      Rst            : in std_logic;   -- System Reset  
      MDIO_Clk       : in std_logic;   -- 2.5Mhz clock
      MDIO_en        : in std_logic;   -- MDIO enable 
      MDIO_OP        : in std_logic;   -- MDIO OP code
      MDIO_Req       : in std_logic;   -- MDIO transmission request
      MDIO_PHY_AD    : in std_logic_vector(4 downto 0);    
                                       -- The physical layer address
      MDIO_REG_AD    : in std_logic_vector(4 downto 0);    
                                       -- The individual register address
      MDIO_WR_DATA   : in std_logic_vector(15 downto 0);   
                                       -- The data to be written on MDIO
      MDIO_RD_DATA   : out std_logic_vector(15 downto 0);  
                                       -- The data read from MDIO
      PHY_MDIO_I     : in std_logic;   -- MDIO Tri-state input from PHY 
      PHY_MDIO_O     : out std_logic;  -- MDIO Tri-state output to PHY
      PHY_MDIO_T     : out std_logic;  -- MDIO Tri-state control
      PHY_MDC        : out std_logic;  -- 2.5Mhz communication clock
      MDIO_done      : out std_logic   -- MDIO tranfer done indicator
   );
end mdio_if;


architecture imp of mdio_if is

attribute DowngradeIPIdentifiedWarnings: string;

attribute DowngradeIPIdentifiedWarnings of imp : architecture is "yes";

-------------------------------------------------------------------------------
--  Signal and Type Declarations
-------------------------------------------------------------------------------
type mdio_state_type is (IDLE, PREAMBLE, ST1, ST2, OP1, OP2, TA1, TA2,
                         PHY_ADDR, REG_ADDR, WRITE, READ, DONE);

signal mdio_state, next_state : mdio_state_type;

signal mdio_xfer_done : std_logic;   -- pulse to inidcate end of activity
signal mdio_idle      : std_logic;   -- internal READY signal
signal rd_data_en     : std_logic_vector(15 downto 0);   -- decoded write 
                                     -- MDIO_en for RD_DATA
signal mdio_en_reg    : std_logic;   -- MDIO_en signal latched at start of 
                                     -- transmission
signal mdio_o_cmb     : std_logic;   -- rising edge version of MDIO_OUT
signal mdio_t_comb    : std_logic;   -- combinatorial term to produce 
                                     -- MDIO_TRISTATE
signal mdio_clk_reg   : std_logic;   -- registering MDIO_Clk to use it as a 
                                     -- clock MDIO_en
signal mdio_in_reg1   : std_logic;   -- compensate in pipeline delay caused
                                     -- by using MDC as a clock MDIO_en 
signal mdio_in_reg2   : std_logic;   -- compensate in pipeline delay caused by
                                     -- using MDC as a clock MDIO_en 
signal clk_cnt           : integer range 0 to 32; -- Clk counter
signal ld_cnt_data_cmb   : integer range 0 to 32; -- Counter load comb
signal ld_cnt_data_reg   : integer range 0 to 32; -- Counter load reg
signal ld_cnt_en_cmb     : std_logic;       -- Counter load enable
signal clk_cnt_en        : std_logic;             -- Counter enable
signal mdc_falling       : std_logic;       -- MDC falling edge
signal mdc_rising        : std_logic;       -- MDC rising edge
signal ld_cnt_en_reg     : std_logic;       -- Counter load enable reg
   
begin

   ----------------------------------------------------------------------------
   -- PROCESS : INPUT_REG_CLK 
   ----------------------------------------------------------------------------
   -- Registering PHY_MDIO_I and MDC signals w.r.t SAXI clock.
   ----------------------------------------------------------------------------
   INPUT_REG_CLK: process(Clk)
   begin
      if (Clk'event and Clk = '1') then
         if (Rst = '1') then
            mdio_clk_reg <= '0';
            mdio_in_reg1 <= '0';
            mdio_in_reg2 <= '0';
         else 
            mdio_clk_reg <= MDIO_Clk;
            mdio_in_reg1 <= PHY_MDIO_I;
            mdio_in_reg2 <= mdio_in_reg1;
         end if;
      end if;
   end process INPUT_REG_CLK;

   -- Falling edge and rising edge generation of MDC clock
   mdc_falling <= not MDIO_Clk and mdio_clk_reg;
   mdc_rising  <= MDIO_Clk and not mdio_clk_reg;
   

   -- Enable MDC only when MDIO interface is enabled. 
   PHY_MDC <= MDIO_Clk; -- making the MDC clk contineous 
   --PHY_MDC <= MDIO_Clk and mdio_en_reg; 
   
   
   -- Informs MDIO interface about the MDIO transfer complete.
   MDIO_done <= mdio_xfer_done;
   
   ----------------------------------------------------------------------------
   -- PROCESS : REG_MDIO_en 
   ----------------------------------------------------------------------------
   -- Latch MDIO_en bit on falling edge of MDC and when MDIO master is IDLE.
   -- MDIO Master will complete the existing transfer even if MDIO interface 
   -- is disable in middle of the transaction.
   ----------------------------------------------------------------------------
   REG_MDIO_en : process(Clk)
   begin
      if (Clk'event and Clk = '1') then 
         if (Rst = '1') then
            mdio_en_reg <= '0';
         elsif mdc_falling='1' then 
            if mdio_idle = '1' then
               mdio_en_reg <= MDIO_en;
            end if;
         end if;
            
      end if;
   end process;
   
   ----------------------------------------------------------------------------
   -- PROCESS : PHY_MDIO_T_REG 
   ----------------------------------------------------------------------------
   -- The mdio_t_comb signal is driven high only for read operation starting
   -- from the Turn arround state. 
   -- It is driven on falling clock edge to match up with PHY_MDIO_O
   ----------------------------------------------------------------------------
   PHY_MDIO_T_REG : process(Clk)
   begin
      if (Clk'event and Clk = '1') then
         if (Rst = '1') then
            PHY_MDIO_T <= '1';
         elsif (mdc_falling='1') then -- falling edge of MDC
            PHY_MDIO_T <= mdio_t_comb;
         end if;
      end if;
   end process;
   
   ----------------------------------------------------------------------------
   -- PROCESS : PHY_MDIO_O_REG 
   ----------------------------------------------------------------------------
   -- Generating PHY_MDIO_O output singnal on falling edge of MDC
   ----------------------------------------------------------------------------
   PHY_MDIO_O_REG : process(Clk)
   begin
      if (Clk'event and Clk = '1') then
         if (Rst = '1') then
            PHY_MDIO_O <= '0';
         elsif (mdc_falling='1') then  -- falling edge of MDC
            PHY_MDIO_O <= mdio_o_cmb;
         end if;
      end if;
   end process;
   
   ----------------------------------------------------------------------------
   -- PROCESS : MDIO_IDLE_REG 
   ----------------------------------------------------------------------------
   -- The mdio_idle signal is used to indicate no activity on the MDIO.
   -- Set at reset amd at the end of transmission.
   -- Rst at start of transmission as long as device is MDIO_end
   ----------------------------------------------------------------------------
   MDIO_IDLE_REG : process(Clk)
   begin
      if (Clk'event and Clk = '1') then 
         if (Rst = '1') then
            mdio_idle <= '1';
         elsif (mdc_rising='1') then   -- rising edge of MDC
            if (mdio_xfer_done = '1') then
               mdio_idle <= '1';
            elsif (MDIO_Req = '1' and mdio_en_reg = '1') then
               mdio_idle <= '0';
            end if;
         end if;
      end if;
   end process ;
  
   ----------------------------------------------------------------------------
   -- PROCESS : MDIO_CAPTURE_DATA 
   ----------------------------------------------------------------------------
   -- This process captures registered PHY_MDIO_i input on rising edge of the 
   -- MDC clock. The rd_data_en signal is generated in MDIO State machine for 
   -- respective captured bit.
   ----------------------------------------------------------------------------
   MDIO_CAPTURE_DATA : for i in 15 downto 0 generate
      MDIO_DATA_IN : process(Clk)
      begin
         if (Clk'event and Clk = '1') then
            if (Rst = '1') then
               MDIO_RD_DATA(i) <= '0';
            elsif (mdc_rising='1') then  -- rising edge of MDC
               if(rd_data_en(i) = '1') then
                  MDIO_RD_DATA(i) <= mdio_in_reg2;
               end if;
            end if;
         end if;
      end process MDIO_DATA_IN;
   end generate;

   ----------------------------------------------------------------------------
   -- PROCESS : MDIO_DOWN_COUNTER 
   ----------------------------------------------------------------------------
   -- This counter is used in Preamble and PHY_ADDR and REG_ADDR state.
   -- This counter is loaded for the required values for each above states. 
   ----------------------------------------------------------------------------
   MDIO_DOWN_COUNTER : process(Clk)
   begin
      if (Clk'event and Clk = '1') then
         if (Rst = '1' ) then
            clk_cnt <= 0;
         elsif (mdc_rising='1') then  -- falling edge of MDC
            if (ld_cnt_en_reg = '1') then -- Load counter with load data
               clk_cnt <= ld_cnt_data_reg;
            elsif (clk_cnt_en='1') then -- Enable Down Counter
               clk_cnt <= clk_cnt - 1;
            end if;
         end if;
      end if;
   end process;

   ----------------------------------------------------------------------------
   -- PROCESS : MDIO_NEXT_STATE_GEN
   ----------------------------------------------------------------------------
   -- MDIO next state register process 
   ----------------------------------------------------------------------------
   MDIO_NEXT_STATE_GEN : process (Clk)
   begin
      if Clk'event and Clk = '1' then
         if (Rst = '1') then
            mdio_state <= IDLE;
        elsif (mdc_rising='1') then 
            mdio_state <= next_state;
        end if;   
      end if;
   end process MDIO_NEXT_STATE_GEN;

   ----------------------------------------------------------------------------
   -- PROCESS : MDIO_COMB_REG_GEN
   ----------------------------------------------------------------------------
   -- Combinational signal register process 
   ----------------------------------------------------------------------------
   MDIO_COMB_REG_GEN : process (Clk)
   begin
      if Clk'event and Clk = '1' then
         if (Rst = '1') then
            ld_cnt_data_reg <= 0;
            ld_cnt_en_reg   <= '0';
        else 
            ld_cnt_data_reg <= ld_cnt_data_cmb;
            ld_cnt_en_reg   <= ld_cnt_en_cmb;
        end if;   
      end if;
   end process MDIO_COMB_REG_GEN;

  ----------------------------------------------------------------------------
  -- PROCESS : MDIO_STATE_COMB 
  ----------------------------------------------------------------------------
  -- This process generates mdio_o_cmb signal in command and Write phase as 
  -- per the required MDIO protocol. This process also generate mdio_t_comb
  -- tristate signal and rd_data_en to capture the respective bit in Read 
  -- operation.
  ----------------------------------------------------------------------------
  MDIO_STATE_COMB : process (mdio_state, mdio_idle, clk_cnt, MDIO_OP,
                             MDIO_PHY_AD, MDIO_REG_AD, MDIO_WR_DATA)
  begin
     -- state machine defaults
     mdio_o_cmb     <= '1';
     rd_data_en     <= "0000000000000000";
     mdio_xfer_done <= '0';
     ld_cnt_en_cmb  <= '0';
     clk_cnt_en     <= '0'; 
     mdio_t_comb    <= '0';
     next_state     <= mdio_state;
     ld_cnt_data_cmb <= 0;
     
     case mdio_state is

        when IDLE =>
           mdio_o_cmb <= '1';
           mdio_t_comb <= '1';

           ld_cnt_en_cmb   <= '1';
           -- leave IDLE state when new mdio request is received.
           if mdio_idle = '0' then
              -- Load counter for 32-bit preamble
              ld_cnt_data_cmb <= 31;
              next_state  <= PREAMBLE;
           end if;

        when PREAMBLE =>
           clk_cnt_en     <= '1';
           -- Move to ST1 after 32-bit preamble.
           if clk_cnt = 0 then
              next_state  <= ST1;
              clk_cnt_en  <= '0';
           end if;

        when ST1 =>              -- Start Code-1
           mdio_o_cmb  <= '0';
           next_state  <= ST2;
           
        when ST2 =>              -- Start Code-2
           mdio_o_cmb  <= '1';
           next_state  <= OP1;

        when OP1 =>              -- Opcode-1
           next_state  <= OP2;
           if MDIO_OP='1' then
              mdio_o_cmb  <= '1';
           else
              mdio_o_cmb <= '0';                 
           end if;

        when OP2 =>              -- Opcode-2
           next_state  <= PHY_ADDR;
           -- Load counter for 5-bit PHYaddress transfer
           ld_cnt_data_cmb <= 4;
           ld_cnt_en_cmb   <= '1';
           if MDIO_OP='1' then
              mdio_o_cmb  <= '0';
           else
              mdio_o_cmb <= '1';                 
           end if;
                                 
        when PHY_ADDR =>         -- PHY Device Address
           clk_cnt_en  <= '1'; 
           mdio_o_cmb  <= MDIO_PHY_AD(clk_cnt);
           -- Send 5-bit PHY device address
           if clk_cnt=0 then
              next_state  <= REG_ADDR;
              -- Load counter for 5-bit REG address transfer
              ld_cnt_data_cmb <= 4;
              ld_cnt_en_cmb   <= '1';
           end if;

        when REG_ADDR =>         -- PHY Device Address
           clk_cnt_en    <= '1'; 
           mdio_o_cmb    <= MDIO_REG_AD(clk_cnt);
           -- Send 5-bit PHY Register address
           if clk_cnt=0 then
              next_state <= TA1;
              clk_cnt_en <= '0';
           end if;

        when TA1 =>              -- Turn Around Time-1
           mdio_o_cmb  <= '1';
           next_state  <= TA2;
           -- For Read operation generate high impedence on 
           -- MDIO bus
           if MDIO_OP='1' then
              mdio_t_comb <= '1';
           else
              mdio_t_comb <= '0';                 
           end if;

        when TA2 =>              -- Turn Around Time-2
           mdio_o_cmb  <= '0';
           -- Load the down counter for 16 bit data transfer
           ld_cnt_data_cmb <= 15;
           ld_cnt_en_cmb   <= '1';
           -- Move to Write state if opcode is '0'
           if MDIO_OP='0' then
              next_state  <= WRITE;
              mdio_t_comb <= '0';
           else
              next_state  <= READ;
              mdio_t_comb <= '1';                 
           end if;

        when WRITE =>            -- MDIO DATA Write
           clk_cnt_en <= '1'; 
           -- Send 16-bit Write Data on the MDIO data line
           mdio_o_cmb <= MDIO_WR_DATA(clk_cnt);
           -- Wait for 16 bit transfer
           if clk_cnt=0 then
              next_state <= DONE;
              clk_cnt_en <= '0';
           end if;

        when READ =>             -- MDIO DATA Read
           clk_cnt_en  <= '1'; 
           mdio_t_comb <= '1';
           -- Generate read data enable for respective bit
           rd_data_en(clk_cnt) <= '1';
           -- Wait for 16 bit transfer
           if clk_cnt=0 then
              next_state <= DONE;
              clk_cnt_en <= '0';
           end if;

        when DONE =>             -- MDIO Transfer Done
           mdio_o_cmb <= '1';
           mdio_t_comb <= '1';
           next_state <= IDLE;
           -- Mdio trasnfer complete
           mdio_xfer_done <= '1'; 

      -- coverage off
      when others =>
           next_state <= IDLE;
      -- coverage on

     end case;
  end process MDIO_STATE_COMB;

end imp;
         
         

       


-------------------------------------------------------------------------------
-- emac_dpram.vhd  - entity/architecture pair
-------------------------------------------------------------------------------
--  ***************************************************************************
--  ** DISCLAIMER OF LIABILITY                                               **
--  **                                                                       **
--  **  This file contains proprietary and confidential information of       **
--  **  Xilinx, Inc. ("Xilinx"), that is distributed under a license         **
--  **  from Xilinx, and may be used, copied and/or disclosed only           **
--  **  pursuant to the terms of a valid license agreement with Xilinx.      **
--  **                                                                       **
--  **  XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION                **
--  **  ("MATERIALS") "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER           **
--  **  EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING WITHOUT                  **
--  **  LIMITATION, ANY WARRANTY WITH RESPECT TO NONINFRINGEMENT,            **
--  **  MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE. Xilinx        **
--  **  does not warrant that functions included in the Materials will       **
--  **  meet the requirements of Licensee, or that the operation of the      **
--  **  Materials will be uninterrupted or error-free, or that defects       **
--  **  in the Materials will be corrected. Furthermore, Xilinx does         **
--  **  not warrant or make any representations regarding use, or the        **
--  **  results of the use, of the Materials in terms of correctness,        **
--  **  accuracy, reliability or otherwise.                                  **
--  **                                                                       **
--  **  Xilinx products are not designed or intended to be fail-safe,        **
--  **  or for use in any application requiring fail-safe performance,       **
--  **  such as life-support or safety devices or systems, Class III         **
--  **  medical devices, nuclear facilities, applications related to         **
--  **  the deployment of airbags, or any other applications that could      **
--  **  lead to death, personal injury or severe property or                 **
--  **  environmental damage (individually and collectively, "critical       **
--  **  applications"). Customer assumes the sole risk and liability         **
--  **  of any use of Xilinx products in critical applications,              **
--  **  subject only to applicable laws and regulations governing            **
--  **  limitations on product liability.                                    **
--  **                                                                       **
--  **  Copyright 2010 Xilinx, Inc.                                          **
--  **  All rights reserved.                                                 **
--  **                                                                       **
--  **  This disclaimer and copyright notice must be retained as part        **
--  **  of this file at all times.                                           **
--  ***************************************************************************
--
-------------------------------------------------------------------------------
-- Filename     : emac_dpram.vhd
-- Version      : v2.0
-- Description  : Realization of dprams   
--                  
-- VHDL-Standard: VHDL'93
-------------------------------------------------------------------------------
-- Structure:   
--
--  axi_ethernetlite.vhd
--      \
--      \-- axi_interface.vhd
--      \-- xemac.vhd
--           \
--           \-- mdio_if.vhd
--           \-- emac_dpram.vhd                     
--           \    \                     
--           \    \-- RAMB16_S4_S36
--           \                          
--           \    
--           \-- emac.vhd                     
--                \                     
--                \-- MacAddrRAM                   
--                \-- receive.vhd
--                \      rx_statemachine.vhd
--                \      rx_intrfce.vhd
--                \         async_fifo_fg.vhd
--                \      crcgenrx.vhd
--                \                     
--                \-- transmit.vhd
--                       crcgentx.vhd
--                          crcnibshiftreg
--                       tx_intrfce.vhd
--                          async_fifo_fg.vhd
--                       tx_statemachine.vhd
--                       deferral.vhd
--                          cntr5bit.vhd
--                          defer_state.vhd
--                       bocntr.vhd
--                          lfsr16.vhd
--                       msh_cnt.vhd
--                       ld_arith_reg.vhd
--
-------------------------------------------------------------------------------
-- Author:    PVK
-- History:    
-- PVK        06/07/2010     First Version
-- ^^^^^^
-- First version.  
-- ~~~~~~
-------------------------------------------------------------------------------
-- Naming Conventions:
--      active low signals:                     "*_n"
--      clock signals:                          "Clk", "clk_div#", "clk_#x" 
--      reset signals:                          "Rst", "rst_n" 
--      generics:                               "C_*" 
--      user defined types:                     "*_TYPE" 
--      state machine next state:               "*_ns" 
--      state machine current state:            "*_cs" 
--      combinatorial signals:                  "*_com" 
--      pipelined or register delay signals:    "*_d#" 
--      counter signals:                        "*cnt*"
--      clock enable signals:                   "*_ce" 
--      internal version of output port         "*_i"
--      device pins:                            "*_pin" 
--      ports:                                  - Names begin with Uppercase 
--      processes:                              "*_PROCESS" 
--      component instantiations:               "<ENTITY_>I_<#|FUNC>
-------------------------------------------------------------------------------
library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;
use IEEE.std_logic_unsigned.all;

-------------------------------------------------------------------------------
-- axi_ethernetlite_v3_0_18 library is used for axi_ethernetlite_v3_0_18 
-- component declarations
-------------------------------------------------------------------------------
library axi_ethernetlite_v3_0_18;
use axi_ethernetlite_v3_0_18.mac_pkg.all;

-------------------------------------------------------------------------------
library lib_bmg_v1_0_13;
use lib_bmg_v1_0_13.all;

library xpm;

-------------------------------------------------------------------------------
-- Vcomponents from unisim library is used for FIFO instatiation
-- function declarations
-------------------------------------------------------------------------------
library unisim;
use unisim.Vcomponents.all;  -- uses BRAM primitives 

-------------------------------------------------------------------------------
-- Definition of Generics:
-------------------------------------------------------------------------------
-- C_FAMILY             -- Target device family 
-------------------------------------------------------------------------------
-- Definition of Ports:
--
--  Clk                 -- System Clock
--  Rst                 -- System Reset
--  Ce_a                -- Port A enable
--  Wr_rd_n_a           -- Port A write/read enable
--  Adr_a               -- Port A address
--  Data_in_a           -- Port A data in
--  Data_out_a          -- Port A data out
--  Ce_b                -- Port B enable
--  Wr_rd_n_b           -- Port B write/read enable
--  Adr_b               -- Port B address
--  Data_in_b           -- Port B data in
--  Data_out_b          -- Port B data out
-------------------------------------------------------------------------------
-- ENTITY
-------------------------------------------------------------------------------
entity emac_dpram is
  generic 
   (
    C_FAMILY : string := "virtex6";
    C_SELECT_XPM : integer := 1
    );
  port 
    (   
    Clk        : in  std_logic;
    Rst        : in  std_logic;
    -- a Port signals
    Ce_a       : in  std_logic;
    Wr_rd_n_a  : in  std_logic;
    Adr_a      : in  std_logic_vector(11 downto 0);
    Data_in_a  : in  std_logic_vector(3 downto 0);
    Data_out_a : out std_logic_vector(3 downto 0);
    
    -- b Port Signals
    Ce_b       : in  std_logic;
    Wr_rd_n_b  : in  std_logic;
    Adr_b      : in  std_logic_vector(8 downto 0);
    Data_in_b  : in  std_logic_vector(31 downto 0);
    Data_out_b : out std_logic_vector(31 downto 0)
    );
end entity emac_dpram;

                
architecture imp of emac_dpram is

attribute DowngradeIPIdentifiedWarnings: string;

attribute DowngradeIPIdentifiedWarnings of imp : architecture is "yes";

--    component RAMB16_S4_S36
---- pragma translate_off
--      generic
--      (
--        WRITE_MODE_A : string := "READ_FIRST"; 
--      --WRITE_FIRST(default)/ READ_FIRST/ NO_CHANGE
--        WRITE_MODE_B : string := "READ_FIRST" 
--      --WRITE_FIRST(default)/ READ_FIRST/ NO_CHANGE
--      );
---- pragma translate_on
--      port (
--        DOA   : out std_logic_vector (3 downto 0);
--        DOB   : out std_logic_vector (31 downto 0);
--        DOPB  : out std_logic_vector (3 downto 0);
--        ADDRA : in std_logic_vector (11 downto 0);
--        CLKA  : in std_logic;
--        DIA   : in std_logic_vector (3 downto 0);
--        ENA   : in std_logic;
--        SSRA  : in std_logic;
--        WEA   : in std_logic;
--        ADDRB : in std_logic_vector (8 downto 0);
--        CLKB  : in std_logic;
--        DIB   : in std_logic_vector (31 downto 0);
--        DIPB  : in std_logic_vector (3 downto 0);
--        ENB   : in std_logic;
--        SSRB  : in std_logic;
--        WEB   : in std_logic
--      );
--    end component;
    
--constant create_v2_mem  : boolean   := supported(C_FAMILY, u_RAMB16_S4_S36);  

component  xpm_memory_tdpram
  generic (
  MEMORY_SIZE         : integer := 4096*32;
  MEMORY_PRIMITIVE    : string  :=  "blockram";
  CLOCKING_MODE       : string  :=  "common_clock";
  ECC_MODE            : string  :=  "no_ecc";
  MEMORY_INIT_FILE    : string  :=  "none";
  WAKEUP_TIME         : string  :=  "disable_sleep";
  MESSAGE_CONTROL     : integer :=  0;

  WRITE_DATA_WIDTH_A  : integer := 32;
  READ_DATA_WIDTH_A   : integer := 32;
  BYTE_WRITE_WIDTH_A  : integer :=  8;
  ADDR_WIDTH_A        : integer := 12; 
  READ_RESET_VALUE_A  : string  := "0";
  READ_LATENCY_A      : integer :=  1;
  WRITE_MODE_A        : string  := "read_first";

  WRITE_DATA_WIDTH_B  : integer := 32;
  READ_DATA_WIDTH_B   : integer := 32;
  BYTE_WRITE_WIDTH_B  : integer :=  8;
  ADDR_WIDTH_B        : integer := 12;
  READ_RESET_VALUE_B  : string  := "0";
  READ_LATENCY_B      : integer :=  1;
  WRITE_MODE_B        : string  := "read_first"

); 
  port (

  -- Common module ports
   sleep              : in std_logic;

  -- Port A module ports
   clka               : in std_logic;
   rsta               : in std_logic;
   ena                : in std_logic;
   regcea             : in std_logic;
   wea                : in std_logic_vector (0  downto 0);  -- (WRITE_DATA_WIDTH_A/BYTE_WRITE_WIDTH_A)-1:0]
   addra              : in std_logic_vector (10 downto 0);  -- [ADDR_WIDTH_A-1:0]   
   dina               : in std_logic_vector (3  downto 0);  -- [WRITE_DATA_WIDTH_A-1:0] 
   injectsbiterra     : in std_logic;
   injectdbiterra     : in std_logic;
   douta              : out std_logic_vector;                -- [READ_DATA_WIDTH_A-1:0]  
   sbiterra           : out std_logic;
   dbiterra           : out std_logic;

  -- Port B module ports
   clkb               : in std_logic;
   rstb               : in std_logic;
   enb                : in std_logic;
   regceb             : in std_logic;
   web                : in std_logic_vector (0  downto 0);  -- (WRITE_DATA_WIDTH_B/BYTE_WRITE_WIDTH_B)-1:0]
   addrb              : in std_logic_vector (8 downto 0);   -- [ADDR_WIDTH_B-1:0]   
   dinb               : in std_logic_vector (15 downto 0);  -- [WRITE_DATA_WIDTH_B-1:0] 
   injectsbiterrb     : in std_logic;
   injectdbiterrb     : in std_logic;
   doutb              : out std_logic_vector;                -- [READ_DATA_WIDTH_B-1:0]  
   sbiterrb           : out std_logic;
   dbiterrb           : out std_logic
  );
end component;
-------------------------------------------------------------------------------
--  Signal and Type Declarations
-------------------------------------------------------------------------------

signal ce_a_i          : std_logic; 
signal ce_b_i          : std_logic; 
--signal wr_rd_n_a_i     : std_logic; 
signal wr_rd_n_a_i     : std_logic_vector(0 downto 0); 
--signal wr_rd_n_b_i     : std_logic; 
signal wr_rd_n_b_i     : std_logic_vector(0 downto 0); 

signal port_b_data_in  : STD_LOGIC_VECTOR (31 downto 0);
signal port_b_data_out : STD_LOGIC_VECTOR (31 downto 0);        

--attribute WRITE_MODE_A : string;
--attribute WRITE_MODE_A of I_DPB16_4_9: label is "READ_FIRST";
--attribute WRITE_MODE_B : string;
--attribute WRITE_MODE_B of I_DPB16_4_9: label is "READ_FIRST"; 
signal ce_a_i_1          : std_logic; 
signal ce_b_i_1          : std_logic; 
signal wr_rd_n_a_i_1     : std_logic_vector(0 downto 0); 
signal wr_rd_n_b_i_1     : std_logic_vector(0 downto 0); 
signal Adr_a_1		 : std_logic_vector(10 downto 0);
signal Adr_b_1		 : std_logic_vector(8 downto 0);
signal Data_in_a_1	 : std_logic_vector(3 downto 0);
signal Data_in_b_1	 : std_logic_vector(15 downto 0);
signal Data_out_a_1	 : std_logic_vector(3 downto 0);
signal Data_out_b_1	 : std_logic_vector(15 downto 0);

signal select_2          : std_logic; 
signal ce_a_i_2          : std_logic; 
signal ce_b_i_2          : std_logic; 
signal wr_rd_n_a_i_2     : std_logic_vector(0 downto 0); 
signal wr_rd_n_b_i_2     : std_logic_vector(0 downto 0); 
signal Adr_a_2		 : std_logic_vector(10 downto 0);
signal Adr_b_2		 : std_logic_vector(8 downto 0);
signal Data_in_a_2	 : std_logic_vector(3 downto 0);
signal Data_in_b_2	 : std_logic_vector(15 downto 0);
signal Data_out_a_2	 : std_logic_vector(3 downto 0);
signal Data_out_b_2	 : std_logic_vector(15 downto 0);

    
    
  
begin  -- architecture

  ce_a_i <= Ce_a or Rst; 
  ce_b_i <= Ce_b or Rst; 
  wr_rd_n_a_i(0) <= Wr_rd_n_a and not(Rst); 
  wr_rd_n_b_i(0) <= Wr_rd_n_b and not(Rst); 

-------------------------------------------------------------------------------
-- Using VII 4096 x 4 : 2048 x 8 Dual Port Primitive
-------------------------------------------------------------------------------
 --     port_b_data_in(31) <= Data_in_b(0);
 --     port_b_data_in(30) <= Data_in_b(1);
 --     port_b_data_in(29) <= Data_in_b(2);
 --     port_b_data_in(28) <= Data_in_b(3);
 --     port_b_data_in(27) <= Data_in_b(4);
 --     port_b_data_in(26) <= Data_in_b(5);
 --     port_b_data_in(25) <= Data_in_b(6);
 --     port_b_data_in(24) <= Data_in_b(7);
 --     port_b_data_in(23) <= Data_in_b(8);
 --     port_b_data_in(22) <= Data_in_b(9);
 --     port_b_data_in(21) <= Data_in_b(10);
 --     port_b_data_in(20) <= Data_in_b(11);
 --     port_b_data_in(19) <= Data_in_b(12);
 --     port_b_data_in(18) <= Data_in_b(13);
 --     port_b_data_in(17) <= Data_in_b(14);
 --     port_b_data_in(16) <= Data_in_b(15);
 --     port_b_data_in(15) <= Data_in_b(16);
 --     port_b_data_in(14) <= Data_in_b(17);
 --     port_b_data_in(13) <= Data_in_b(18);
 --     port_b_data_in(12) <= Data_in_b(19);
 --     port_b_data_in(11) <= Data_in_b(20);
 --     port_b_data_in(10) <= Data_in_b(21);
 --     port_b_data_in(9)  <= Data_in_b(22);
 --     port_b_data_in(8)  <= Data_in_b(23);
 --     port_b_data_in(7)  <= Data_in_b(24);
 --     port_b_data_in(6)  <= Data_in_b(25);
 --     port_b_data_in(5)  <= Data_in_b(26);
 --     port_b_data_in(4)  <= Data_in_b(27);
 --     port_b_data_in(3)  <= Data_in_b(28);
 --     port_b_data_in(2)  <= Data_in_b(29);
 --     port_b_data_in(1)  <= Data_in_b(30);
 --     port_b_data_in(0)  <= Data_in_b(31);
 --
 --     Data_out_b(31) <= port_b_data_out(0);
 --     Data_out_b(30) <= port_b_data_out(1);
 --     Data_out_b(29) <= port_b_data_out(2);
 --     Data_out_b(28) <= port_b_data_out(3);
 --     Data_out_b(27) <= port_b_data_out(4);
 --     Data_out_b(26) <= port_b_data_out(5);
 --     Data_out_b(25) <= port_b_data_out(6);
 --     Data_out_b(24) <= port_b_data_out(7);  
 --     Data_out_b(23) <= port_b_data_out(8);
 --     Data_out_b(22) <= port_b_data_out(9);
 --     Data_out_b(21) <= port_b_data_out(10);
 --     Data_out_b(20) <= port_b_data_out(11);
 --     Data_out_b(19) <= port_b_data_out(12);
 --     Data_out_b(18) <= port_b_data_out(13);
 --     Data_out_b(17) <= port_b_data_out(14);
 --     Data_out_b(16) <= port_b_data_out(15);  
 --     Data_out_b(15) <= port_b_data_out(16);
 --     Data_out_b(14) <= port_b_data_out(17);
 --     Data_out_b(13) <= port_b_data_out(18);
 --     Data_out_b(12) <= port_b_data_out(19);
 --     Data_out_b(11) <= port_b_data_out(20);
 --     Data_out_b(10) <= port_b_data_out(21);
 --     Data_out_b(9)  <= port_b_data_out(22);
 --     Data_out_b(8)  <= port_b_data_out(23);  
 --     Data_out_b(7)  <= port_b_data_out(24);
 --     Data_out_b(6)  <= port_b_data_out(25);
 --     Data_out_b(5)  <= port_b_data_out(26);
 --     Data_out_b(4)  <= port_b_data_out(27);
 --     Data_out_b(3)  <= port_b_data_out(28);
 --     Data_out_b(2)  <= port_b_data_out(29);
 --     Data_out_b(1)  <= port_b_data_out(30);
 --     Data_out_b(0)  <= port_b_data_out(31);  
 --
 --
 --     I_DPB16_4_9: RAMB16_S4_S36
 --       port map (
 --         DOA   => Data_out_a,      --[out]
 --         DOB   => port_b_data_out, --[out]
 --         DOPB  => open,            --[out]
 --         ADDRA => Adr_a,           --[in]
 --         CLKA  => Clk,             --[in]
 --         DIA   => Data_in_a,       --[in]
 --         ENA   => ce_a_i,          --[in]
 --         SSRA  => Rst,             --[in]
 --         WEA   => wr_rd_n_a_i,     --[in]
 --         ADDRB => Adr_b,           --[in]
 --         CLKB  => Clk,             --[in]
 --         DIB   => port_b_data_in,  --[in]
 --         DIPB  => (others => '0'), --[in]
 --         ENB   => ce_b_i,          --[in]
 --         SSRB  => Rst,             --[in]
 --         WEB   => wr_rd_n_b_i      --[in]
 --       );
 --
      
--      I_DPB16_4_9: RAMB16_S4_S36
--        port map (
--          DOA   => Data_out_a,      --[out]
--          DOB   => Data_out_b,      --[out]
--          DOPB  => open,            --[out]
--          ADDRA => Adr_a,           --[in]
--          CLKA  => Clk,             --[in]
--          DIA   => Data_in_a,       --[in]
--          ENA   => ce_a_i,          --[in]
--          SSRA  => Rst,             --[in]
--          WEA   => wr_rd_n_a_i,     --[in]
--          ADDRB => Adr_b,           --[in]
--          CLKB  => Clk,             --[in]
--          DIB   => Data_in_b,       --[in]
--          DIPB  => (others => '0'), --[in]
--          ENB   => ce_b_i,          --[in]
--          SSRB  => Rst,             --[in]
--          WEB   => wr_rd_n_b_i      --[in]
--        );

xpm_mem_gen : if (C_SELECT_XPM = 1) generate
----Not supported in everest---------
--xpm_memory_inst: xpm_memory_tdpram
--
--   generic map (
--      MEMORY_SIZE             => 4096*4,
--      MEMORY_PRIMITIVE        => "blockram",
--      CLOCKING_MODE           => "common_clock",
--      ECC_MODE                => "no_ecc",
--      MEMORY_INIT_FILE        => "none",
--      WAKEUP_TIME             => "disable_sleep",
--      MESSAGE_CONTROL         =>  0,
--
--      WRITE_DATA_WIDTH_A      =>  4,
--      READ_DATA_WIDTH_A       =>  4,
--      BYTE_WRITE_WIDTH_A      =>  4,
--      ADDR_WIDTH_A            => 12, 
--      READ_RESET_VALUE_A      => "0",
--      READ_LATENCY_A          =>  1,
--      WRITE_MODE_A            => "read_first",
--
--      WRITE_DATA_WIDTH_B      => 32,
--      READ_DATA_WIDTH_B       => 32,
--      BYTE_WRITE_WIDTH_B      => 32,
--      ADDR_WIDTH_B            =>  9,
--      READ_RESET_VALUE_B      => "0",
--      READ_LATENCY_B          =>  1,
--      WRITE_MODE_B            => "read_first"
--      )
--      port map (
--       -- Common module ports
--      sleep                   =>  '0',
--    
--     -- Port A module ports
--      clka                    => Clk,
--      rsta                    => '0', 
--      ena                     => ce_a_i, 
--      regcea                  => '1',
--      wea                     => wr_rd_n_a_i,
--      addra                   => Adr_a,
--      dina                    => Data_in_a,
--      injectsbiterra          => '0',
--      injectdbiterra          => '0',
--      douta                   => Data_out_a,
--      sbiterra                => open,
--      dbiterra                => open,
--    
--     -- Port B module ports
--      clkb                    => Clk,
--      rstb                    => '0',
--      enb                     => ce_b_i,
--      regceb                  => '1',
--      web                     => wr_rd_n_b_i,
--      addrb                   => Adr_b,
--      dinb                    => Data_in_b,
--      injectsbiterrb          => '0',
--      injectdbiterrb          => '0',
--      doutb                   => Data_out_b,
--      sbiterrb                => open,
--      dbiterrb                => open
--      );
----Not supported in everest---------
-- For everest we are splitting the above xpm RAM into 2 RAMs to make the data width ratio supportable-----
ce_a_i_1 	<= ce_a_i and ((not Adr_a(2)) or Rst);
wr_rd_n_a_i_1 	<= wr_rd_n_a_i and (not Adr_a(2 downto 2));
Adr_a_1		<= Adr_a(11 downto 3) & Adr_a(1 downto 0);
Data_in_a_1	<= Data_in_a;

ce_b_i_1 	<= ce_b_i;
wr_rd_n_b_i_1	<= wr_rd_n_b_i;
Adr_b_1 	<= Adr_b;
Data_in_b_1	<= Data_in_b(15 downto 0);

ce_a_i_2 	<= ce_a_i and (Adr_a(2) or Rst);
wr_rd_n_a_i_2 	<= wr_rd_n_a_i and Adr_a(2 downto 2);
Adr_a_2		<= Adr_a(11 downto 3) & Adr_a(1 downto 0);
Data_in_a_2	<= Data_in_a;

ce_b_i_2 	<= ce_b_i;
wr_rd_n_b_i_2	<= wr_rd_n_b_i;
Adr_b_2 	<= Adr_b;
Data_in_b_2	<= Data_in_b(31 downto 16);

Data_out_a 	<= Data_out_a_2 when (select_2 = '1') else Data_out_a_1;
Data_out_b	<= Data_out_b_2 & Data_out_b_1;

   DOUT_A_PROC : process (Clk)
   begin  --   
      if (Clk'event and Clk = '1') then
         if (Rst = '1') then
		--Data_out_a	<= (others => '0' );
			select_2   <= '0';
         else           
		if (Adr_a(2) = '1') then
		--	Data_out_a <= Data_out_a_2;
			select_2   <= '1';
		else
		--	Data_out_a <= Data_out_a_1;
			select_2   <= '0';
		end if; 
         end if;
      end if;
   end process DOUT_A_PROC; 

xpm_memory_inst_1: xpm_memory_tdpram

   generic map (
      MEMORY_SIZE             => 4096*2,
      MEMORY_PRIMITIVE        => "blockram",
      CLOCKING_MODE           => "common_clock",
      ECC_MODE                => "no_ecc",
      MEMORY_INIT_FILE        => "none",
      WAKEUP_TIME             => "disable_sleep",
      MESSAGE_CONTROL         =>  0,

      WRITE_DATA_WIDTH_A      =>  4,
      READ_DATA_WIDTH_A       =>  4,
      BYTE_WRITE_WIDTH_A      =>  4,
      ADDR_WIDTH_A            => 11, 
      READ_RESET_VALUE_A      => "0",
      READ_LATENCY_A          =>  1,
      WRITE_MODE_A            => "read_first",

      WRITE_DATA_WIDTH_B      => 16,
      READ_DATA_WIDTH_B       => 16,
      BYTE_WRITE_WIDTH_B      => 16,
      ADDR_WIDTH_B            =>  9,
      READ_RESET_VALUE_B      => "0",
      READ_LATENCY_B          =>  1,
      WRITE_MODE_B            => "read_first"
      )
      port map (
       -- Common module ports
      sleep                   =>  '0',
    
     -- Port A module ports
      clka                    => Clk,
      rsta                    => '0', 
      ena                     => ce_a_i_1, 
      regcea                  => '1',
      wea                     => wr_rd_n_a_i_1,
      addra                   => Adr_a_1,
      dina                    => Data_in_a_1,
      injectsbiterra          => '0',
      injectdbiterra          => '0',
      douta                   => Data_out_a_1,
      sbiterra                => open,
      dbiterra                => open,
    
     -- Port B module ports
      clkb                    => Clk,
      rstb                    => '0',
      enb                     => ce_b_i_1,
      regceb                  => '1',
      web                     => wr_rd_n_b_i_1,
      addrb                   => Adr_b_1,
      dinb                    => Data_in_b_1,
      injectsbiterrb          => '0',
      injectdbiterrb          => '0',
      doutb                   => Data_out_b_1,
      sbiterrb                => open,
      dbiterrb                => open
      );

xpm_memory_inst_2: xpm_memory_tdpram

   generic map (
      MEMORY_SIZE             => 4096*2,
      MEMORY_PRIMITIVE        => "blockram",
      CLOCKING_MODE           => "common_clock",
      ECC_MODE                => "no_ecc",
      MEMORY_INIT_FILE        => "none",
      WAKEUP_TIME             => "disable_sleep",
      MESSAGE_CONTROL         =>  0,

      WRITE_DATA_WIDTH_A      =>  4,
      READ_DATA_WIDTH_A       =>  4,
      BYTE_WRITE_WIDTH_A      =>  4,
      ADDR_WIDTH_A            => 11, 
      READ_RESET_VALUE_A      => "0",
      READ_LATENCY_A          =>  1,
      WRITE_MODE_A            => "read_first",

      WRITE_DATA_WIDTH_B      => 16,
      READ_DATA_WIDTH_B       => 16,
      BYTE_WRITE_WIDTH_B      => 16,
      ADDR_WIDTH_B            =>  9,
      READ_RESET_VALUE_B      => "0",
      READ_LATENCY_B          =>  1,
      WRITE_MODE_B            => "read_first"
      )
      port map (
       -- Common module ports
      sleep                   =>  '0',
    
     -- Port A module ports
      clka                    => Clk,
      rsta                    => '0', 
      ena                     => ce_a_i_2, 
      regcea                  => '1',
      wea                     => wr_rd_n_a_i_2,
      addra                   => Adr_a_2,
      dina                    => Data_in_a_2,
      injectsbiterra          => '0',
      injectdbiterra          => '0',
      douta                   => Data_out_a_2,
      sbiterra                => open,
      dbiterra                => open,
    
     -- Port B module ports
      clkb                    => Clk,
      rstb                    => '0',
      enb                     => ce_b_i_2,
      regceb                  => '1',
      web                     => wr_rd_n_b_i_2,
      addrb                   => Adr_b_2,
      dinb                    => Data_in_b_2,
      injectsbiterrb          => '0',
      injectdbiterrb          => '0',
      doutb                   => Data_out_b_2,
      sbiterrb                => open,
      dbiterrb                => open
      );

end generate;

blk_mem_gen : if (C_SELECT_XPM = 0) generate
dpram_blkmem: entity lib_bmg_v1_0_13.blk_mem_gen_wrapper
   generic map (
      c_family                 => C_FAMILY,
      c_xdevicefamily          => C_FAMILY,
      c_mem_type               => 2,
      c_algorithm              => 1,
      c_prim_type              => 1,
      c_byte_size              => 8,   -- 8 or 9
      c_sim_collision_check    => "NONE",
      c_common_clk             => 1,   -- 0, 1
      c_disable_warn_bhv_coll  => 1,   -- 0, 1
      c_disable_warn_bhv_range => 1,   -- 0, 1
      c_load_init_file         => 0,
      c_init_file_name         => "no_coe_file_loaded",
      c_use_default_data       => 0,   -- 0, 1
      c_default_data           => "0", -- "..."
      -- Port A Specific Configurations
      c_has_mem_output_regs_a  => 0,   -- 0, 1
      c_has_mux_output_regs_a  => 0,   -- 0, 1
      c_write_width_a          => 4,   -- 1 to 1152
      c_read_width_a           => 4,   -- 1 to 1152
      c_write_depth_a          => 4096,  -- 2 to 9011200
      c_read_depth_a           => 4096,  -- 2 to 9011200
      c_addra_width            => 12,    -- 1 to 24
      c_write_mode_a           => "READ_FIRST",
      c_has_ena                => 1,   -- 0, 1
      c_has_regcea             => 1,   -- 0, 1
      c_has_ssra               => 0,   -- 0, 1
      c_sinita_val             => "0", --"..."
      c_use_byte_wea           => 0,   -- 0, 1
      c_wea_width              => 1,   -- 1 to 128
      -- Port B Specific Configurations
      c_has_mem_output_regs_b  => 0,   -- 0, 1
      c_has_mux_output_regs_b  => 0,   -- 0, 1
      c_write_width_b          => 32,  -- 1 to 1152
      c_read_width_b           => 32,  -- 1 to 1152
      c_write_depth_b          => 512,  -- 2 to 9011200
      c_read_depth_b           => 512,  -- 2 to 9011200
      c_addrb_width            => 9,    -- 1 to 24
      c_write_mode_b           => "READ_FIRST",
      c_has_enb                => 1,   -- 0, 1
      c_has_regceb             => 1,   -- 0, 1
      c_has_ssrb               => 0,   -- 0, 1
      c_sinitb_val             => "0", -- "..."
      c_use_byte_web           => 0,   -- 0, 1
      c_web_width              => 1,   -- 1 to 128
      -- Other Miscellaneous Configurations
      c_mux_pipeline_stages    => 0,   -- 0, 1, 2, 3
      c_use_ecc                => 0,
      c_use_ramb16bwer_rst_bhv => 0--,   --0, 1
      )
   port map (
      clka    => Clk,
      ssra    => '0',
      dina    => Data_in_a,
      addra   => Adr_a,
      ena     => ce_a_i,
      regcea  => '1',
      wea     => wr_rd_n_a_i,
      douta   => Data_out_a,

      clkb    => Clk,
      ssrb    => '0',
      dinb    => Data_in_b,
      addrb   => Adr_b,
      enb     => ce_b_i,
      regceb  => '1',
      web     => wr_rd_n_b_i,
      doutb   => Data_out_b,

      dbiterr => open,
      sbiterr => open );
end generate;
--assert (create_v2_mem)
--report "The primitive RAMB16_S4_S36 is not Supported by the Target device"
--severity FAILURE; 
   
end architecture imp;


-------------------------------------------------------------------------------
-- emac - entity/architecture pair
-------------------------------------------------------------------------------
--  ***************************************************************************
--  ** DISCLAIMER OF LIABILITY                                               **
--  **                                                                       **
--  **  This file contains proprietary and confidential information of       **
--  **  Xilinx, Inc. ("Xilinx"), that is distributed under a license         **
--  **  from Xilinx, and may be used, copied and/or disclosed only           **
--  **  pursuant to the terms of a valid license agreement with Xilinx.      **
--  **                                                                       **
--  **  XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION                **
--  **  ("MATERIALS") "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER           **
--  **  EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING WITHOUT                  **
--  **  LIMITATION, ANY WARRANTY WITH RESPECT TO NONINFRINGEMENT,            **
--  **  MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE. Xilinx        **
--  **  does not warrant that functions included in the Materials will       **
--  **  meet the requirements of Licensee, or that the operation of the      **
--  **  Materials will be uninterrupted or error-free, or that defects       **
--  **  in the Materials will be corrected. Furthermore, Xilinx does         **
--  **  not warrant or make any representations regarding use, or the        **
--  **  results of the use, of the Materials in terms of correctness,        **
--  **  accuracy, reliability or otherwise.                                  **
--  **                                                                       **
--  **  Xilinx products are not designed or intended to be fail-safe,        **
--  **  or for use in any application requiring fail-safe performance,       **
--  **  such as life-support or safety devices or systems, Class III         **
--  **  medical devices, nuclear facilities, applications related to         **
--  **  the deployment of airbags, or any other applications that could      **
--  **  lead to death, personal injury or severe property or                 **
--  **  environmental damage (individually and collectively, "critical       **
--  **  applications"). Customer assumes the sole risk and liability         **
--  **  of any use of Xilinx products in critical applications,              **
--  **  subject only to applicable laws and regulations governing            **
--  **  limitations on product liability.                                    **
--  **                                                                       **
--  **  Copyright 2010 Xilinx, Inc.                                          **
--  **  All rights reserved.                                                 **
--  **                                                                       **
--  **  This disclaimer and copyright notice must be retained as part        **
--  **  of this file at all times.                                           **
--  ***************************************************************************
--
-------------------------------------------------------------------------------
-- Filename     : emac.vhd
-- Version      : v2.0
-- Description  : Design file for the Ethernet Lite MAC. 
-- VHDL-Standard: VHDL'93
-------------------------------------------------------------------------------
-- Structure:   
--
--  axi_ethernetlite.vhd
--      \
--      \-- axi_interface.vhd
--      \-- xemac.vhd
--           \
--           \-- mdio_if.vhd
--           \-- emac_dpram.vhd                     
--           \    \                     
--           \    \-- RAMB16_S4_S36
--           \                          
--           \    
--           \-- emac.vhd                     
--                \                     
--                \-- MacAddrRAM                   
--                \-- receive.vhd
--                \      rx_statemachine.vhd
--                \      rx_intrfce.vhd
--                \         async_fifo_fg.vhd
--                \      crcgenrx.vhd
--                \                     
--                \-- transmit.vhd
--                       crcgentx.vhd
--                          crcnibshiftreg
--                       tx_intrfce.vhd
--                          async_fifo_fg.vhd
--                       tx_statemachine.vhd
--                       deferral.vhd
--                          cntr5bit.vhd
--                          defer_state.vhd
--                       bocntr.vhd
--                          lfsr16.vhd
--                       msh_cnt.vhd
--                       ld_arith_reg.vhd
--
-------------------------------------------------------------------------------
-- Author:    PVK
-- History:    
-- PVK        06/07/2010     First Version
-- ^^^^^^
-- First version.  
-- ~~~~~~
-------------------------------------------------------------------------------
-- Naming Conventions:
--      active low signals:                     "*_n"
--      clock signals:                          "Clk", "clk_div#", "clk_#x" 
--      reset signals:                          "Rst", "rst_n" 
--      generics:                               "C_*" 
--      user defined types:                     "*_TYPE" 
--      state machine next state:               "*_ns" 
--      state machine current state:            "*_cs" 
--      combinatorial signals:                  "*_com" 
--      pipelined or register delay signals:    "*_d#" 
--      counter signals:                        "*cnt*"
--      clock enable signals:                   "*_ce" 
--      internal version of output port         "*_i"
--      device pins:                            "*_pin" 
--      ports:                                  - Names begin with Uppercase 
--      processes:                              "*_PROCESS" 
--      component instantiations:               "<ENTITY_>I_<#|FUNC>
-------------------------------------------------------------------------------
-- 
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

-------------------------------------------------------------------------------
-- axi_ethernetlite_v3_0_18 library is used for axi_ethernetlite_v3_0_18 
-- component declarations
-------------------------------------------------------------------------------
library axi_ethernetlite_v3_0_18;
use axi_ethernetlite_v3_0_18.mac_pkg.all;
use axi_ethernetlite_v3_0_18.all;

-------------------------------------------------------------------------------
-- Vcomponents from unisim library is used for FIFO instatiation
-- function declarations
-------------------------------------------------------------------------------
library unisim;
use unisim.Vcomponents.all;

library lib_cdc_v1_0_2;
use lib_cdc_v1_0_2.all;

-------------------------------------------------------------------------------
-- Definition of Generics:
-------------------------------------------------------------------------------
--  C_DUPLEX               -- 1 = full duplex, 0 = half duplex
--  NODE_MAC               -- EMACLite MAC address
--  C_FAMILY               -- Target device family 
-------------------------------------------------------------------------------
-- Definition of Ports:
--
--  Clk                    -- System Clock
--  Rst                    -- System Reset
--  PHY_tx_clk             -- Ethernet tranmit clock
--  PHY_rx_clk             -- Ethernet receive clock
--  PHY_crs                -- Ethernet carrier sense
--  PHY_dv                 -- Ethernet receive data valid
--  PHY_rx_data            -- Ethernet receive data
--  PHY_col                -- Ethernet collision indicator
--  PHY_rx_er              -- Ethernet receive error
--  PHY_rst_n              -- Ethernet PHY Reset
--  PHY_tx_en              -- Ethernet transmit enable
--  PHY_tx_data            -- Ethernet transmit data
--  Tx_DPM_ce              -- TX buffer chip enable
--  Tx_DPM_adr             -- Tx buffer address
--  Tx_DPM_wr_data         -- TX buffer write data
--  Tx_DPM_rd_data         -- TX buffer read data
--  Tx_DPM_wr_rd_n         -- TX buffer write/read enable
--  Tx_done                -- Transmit done
--  Tx_pong_ping_l         -- TX Ping/Pong buffer enable
--  Tx_idle                -- Transmit idle
--  Rx_idle                -- Receive idle
--  Rx_DPM_ce              -- RX buffer chip enable
--  Rx_DPM_adr             -- RX buffer address
--  Rx_DPM_wr_data         -- RX buffer write data
--  Rx_DPM_rd_data         -- RX buffer read data
--  Rx_DPM_wr_rd_n         -- RX buffer write/read enable
--  Rx_done                -- Receive done
--  Rx_pong_ping_l         -- RX Ping/Pong buffer enable
--  Tx_packet_length       -- Transmit packet length
--  Transmit_start         -- Transmit Start
--  Mac_program_start      -- MAC Program start
--  Rx_buffer_ready        -- RX Buffer ready indicator
-------------------------------------------------------------------------------
-- ENTITY
-------------------------------------------------------------------------------
entity emac is
  generic (
    C_DUPLEX          : integer    := 1; 
      -- 1 = full duplex, 0 = half duplex       
    NODE_MAC          : bit_vector := x"00005e00FACE";
    C_FAMILY          : string     := "virtex6"
    );                
  port (              
    Clk               : in  std_logic;
    Rst               : in  std_logic;
    Phy_tx_clk        : in  std_logic;
    Phy_rx_clk        : in  std_logic;
    Phy_crs           : in  std_logic;
    Phy_dv            : in  std_logic;
    Phy_rx_data       : in  std_logic_vector (0 to 3);
    Phy_col           : in  std_logic;
    Phy_rx_er         : in  std_logic;
    Phy_tx_en         : out std_logic;
    Phy_tx_data       : out std_logic_vector (0 to 3);
    Tx_DPM_ce         : out std_logic;
    Tx_DPM_adr        : out std_logic_vector (0 to 11);
    Tx_DPM_wr_data    : out std_logic_vector (0 to 3);
    Tx_DPM_rd_data    : in  std_logic_vector (0 to 3);
    Tx_DPM_wr_rd_n    : out std_logic;
    Tx_done           : out std_logic;
    Tx_pong_ping_l    : in  std_logic;
    Tx_idle           : out std_logic;
    Rx_idle           : out std_logic;
    Rx_DPM_ce         : out std_logic;
    Rx_DPM_adr        : out std_logic_vector (0 to 11);
    Rx_DPM_wr_data    : out std_logic_vector (0 to 3);
    Rx_DPM_rd_data    : in  std_logic_vector (0 to 3);
    Rx_DPM_wr_rd_n    : out std_logic;
    Rx_done           : out std_logic;
    Rx_pong_ping_l    : in  std_logic;
    Tx_packet_length  : in  std_logic_vector(0 to 15);
    Transmit_start    : in  std_logic;
    Mac_program_start : in  std_logic;
    Rx_buffer_ready   : in  std_logic
    );
end emac;

architecture imp of emac is

attribute DowngradeIPIdentifiedWarnings: string;

attribute DowngradeIPIdentifiedWarnings of imp : architecture is "yes";

-------------------------------------------------------------------------------
--  Constant Declarations
-------------------------------------------------------------------------------
  signal phy_col_d1            : std_logic; -- added 3-03-05 MSH
  signal phy_crs_d1            : std_logic; -- added 3-03-05 MSH
  signal phy_col_d2            : std_logic; -- added 27-jul-2011
  signal phy_crs_d2            : std_logic; -- added 27-jul-2011
  signal rxbuffer_addr         : std_logic_vector(0 to 11);
  signal rx_addr_en            : std_logic;
  signal rx_start              : std_logic;
  signal txbuffer_addr         : std_logic_vector(0 to 11);
  signal tx_addr_en            : std_logic;
  signal tx_start              : std_logic;
  signal mac_addr_ram_addr     : std_logic_vector(0 to 3);
  signal mac_addr_ram_addr_rd  : std_logic_vector(0 to 3);
  signal mac_addr_ram_we       : std_logic;
  signal mac_addr_ram_addr_wr  : std_logic_vector(0 to 3);
  signal mac_addr_ram_data     : std_logic_vector(0 to 3);
  signal txClkEn               : std_logic;
  signal tx_clk_reg_d1         : std_logic;  
  signal tx_clk_reg_d2         : std_logic;
  signal tx_clk_reg_d3         : std_logic;
  signal mac_tx_frame_length   : std_logic_vector(0 to 15);
  signal nibbleLength          : std_logic_vector(0 to 11);
  signal nibbleLength_orig     : std_logic_vector(0 to 11);
  signal en_pad                : std_logic;
  signal Phy_tx_clk_axi_d      : std_logic;
-------------------------------------------------------------------------------
-- Component Declarations
-------------------------------------------------------------------------------

-- The following components are the building blocks of the EMAC

component FDR
  port (
    Q : out std_logic;
    C : in std_logic;
    D : in std_logic;
    R : in std_logic
    );
end component;



begin

   ----------------------------------------------------------------------------
   -- Receive Interface
   ----------------------------------------------------------------------------
   RX: entity axi_ethernetlite_v3_0_18.receive
     generic map
       (
       C_DUPLEX => C_DUPLEX,
       C_FAMILY => C_FAMILY   
       )    
     port map
       (   
       Clk                  => Clk,
       Rst                  => Rst,
       Phy_rx_clk           => Phy_rx_clk,
       Phy_dv               => Phy_dv,
       Phy_rx_data          => Phy_rx_data, 
       Phy_rx_col           => phy_col_d2, 
       Phy_rx_er            => Phy_rx_er,
       Rx_addr_en           => rx_addr_en,
       Rx_start             => rx_start,
       Rx_done              => Rx_done,
       Rx_pong_ping_l       => Rx_pong_ping_l,
       Rx_DPM_ce            => Rx_DPM_ce,
       Rx_DPM_wr_data       => Rx_DPM_wr_data,
       Rx_DPM_rd_data       => Rx_DPM_rd_data,
       Rx_DPM_wr_rd_n       => Rx_DPM_wr_rd_n,
       Rx_idle              => Rx_idle,
       Mac_addr_ram_addr_rd => mac_addr_ram_addr_rd,
       Mac_addr_ram_data    => mac_addr_ram_data,
       Rx_buffer_ready      => Rx_buffer_ready
   
       );      
   
   ----------------------------------------------------------------------------
   -- Transmit Interface
   ----------------------------------------------------------------------------
   TX: entity axi_ethernetlite_v3_0_18.transmit
     generic map
       (
       C_DUPLEX => C_DUPLEX,
       C_FAMILY => C_FAMILY
       )
     port map
       (
       Clk                   =>  Clk,
       Rst                   =>  Rst,
       NibbleLength          =>  nibbleLength,
       NibbleLength_orig     =>  nibbleLength_orig,
       En_pad                =>  en_pad,
       TxClkEn               =>  txClkEn,
       Phy_tx_clk            =>  Phy_tx_clk,
       Phy_crs               =>  phy_crs_d2, 
       Phy_col               =>  phy_col_d2, 
                    
       Phy_tx_en             =>  phy_tx_en,
       Phy_tx_data           =>  phy_tx_data,
       Tx_addr_en            =>  tx_addr_en,
       Tx_start              =>  tx_start,
       Tx_done               =>  Tx_done,
       Tx_pong_ping_l        =>  Tx_pong_ping_l,
       Tx_idle               =>  Tx_idle,
       Tx_DPM_ce             =>  Tx_DPM_ce,
       Tx_DPM_wr_data        =>  Tx_DPM_wr_data,
       Tx_DPM_rd_data        =>  Tx_DPM_rd_data,
       Tx_DPM_wr_rd_n        =>  Tx_DPM_wr_rd_n,
       Transmit_start        =>  Transmit_start,
       Mac_program_start     =>  Mac_program_start,
       Mac_addr_ram_we       =>  mac_addr_ram_we,
       Mac_addr_ram_addr_wr  =>  mac_addr_ram_addr_wr
                                 
       );              

   ----------------------------------------------------------------------------
   -- Registerign PHY Col
   ----------------------------------------------------------------------------
   COLLISION_SYNC_1: FDR             
     port map 
       (
       Q => phy_col_d1, --[out]
       C => Clk,        --[in]
       D => Phy_col,    --[in]
       R => Rst         --[in]
       );
   
   COLLISION_SYNC_2: FDR             
     port map 
       (
       Q => phy_col_d2, --[out]
       C => Clk,        --[in]
       D => phy_col_d1, --[in]
       R => Rst         --[in]
       );
   
   ----------------------------------------------------------------------------
   -- Registerign PHY CRs
   ----------------------------------------------------------------------------
   C_SENSE_SYNC_1: FDR               
     port map 
       (
       Q => phy_crs_d1, --[out]
       C => Clk,        --[in]
       D => Phy_crs,    --[in]
       R => Rst         --[in]
       );
             
   C_SENSE_SYNC_2: FDR               
     port map 
       (
       Q => phy_crs_d2, --[out]
       C => Clk,        --[in]
       D => phy_crs_d1, --[in]
       R => Rst         --[in]
       );
             
   ----------------------------------------------------------------------------
   -- MAC Address RAM
   ----------------------------------------------------------------------------
   NODEMACADDRRAMI: entity axi_ethernetlite_v3_0_18.MacAddrRAM
      generic map 
        (
        MACAddr  => NODE_MAC
        )
      port map 
        (
        addr     => mac_addr_ram_addr,
        dout     => mac_addr_ram_data,
        din      => Tx_DPM_rd_data,
        we       => mac_addr_ram_we,
        Clk      => Clk
        );
   
   mac_addr_ram_addr <= mac_addr_ram_addr_rd when mac_addr_ram_we = '0' else
                        mac_addr_ram_addr_wr;

   ----------------------------------------------------------------------------
   -- RX Address Counter for the RxBuffer
   ----------------------------------------------------------------------------
   RXADDRCNT: process (Clk)
     begin
       if Clk'event and Clk = '1' then
         if Rst = '1' then
           rxbuffer_addr <= (others => '0');
         elsif rx_start = '1' then
           rxbuffer_addr <= (others => '0');
         elsif rx_addr_en = '1' then
           rxbuffer_addr <= rxbuffer_addr + 1;
         end if;
       end if;
     end process RXADDRCNT;
   
   Rx_DPM_adr <= rxbuffer_addr;

   ----------------------------------------------------------------------------
   -- TX Address Counter for the TxBuffer (To Read)
   ----------------------------------------------------------------------------
   TXADDRCNT: process (Clk)
     begin
       if Clk'event and Clk = '1' then
         if Rst = '1' then
           txbuffer_addr <= (others => '0');
         elsif tx_start = '1' then
           txbuffer_addr <= (others => '0');
         elsif tx_addr_en = '1' then
           txbuffer_addr <= txbuffer_addr + 1;
         end if;
       end if;
     end process TXADDRCNT;

                   
   Tx_DPM_adr <= txbuffer_addr;
  
   ----------------------------------------------------------------------------
   -- CDC module for syncing phy_tx_clk in PHY_tx_clk domain
   ----------------------------------------------------------------------------
  CDC_TX_CLK: entity  lib_cdc_v1_0_2.cdc_sync
  generic map (
    C_CDC_TYPE           => 1,
    C_RESET_STATE        => 0,
    C_SINGLE_BIT         => 1,
    C_FLOP_INPUT         => 0,
    C_VECTOR_WIDTH       => 1,
    C_MTBF_STAGES        => 4
            )
  port map(
    prmry_aclk            => '1',
    prmry_resetn          => '1',
    prmry_in              => Phy_tx_clk,
    prmry_ack             => open,
    scndry_out            => Phy_tx_clk_axi_d,
    scndry_aclk           => Clk,
    scndry_resetn         => '1',
    prmry_vect_in         => (OTHERS => '0'),
    scndry_vect_out       => open
     );
   ----------------------------------------------------------------------------
   -- INT_tx_clk_sync_PROCESS
   ----------------------------------------------------------------------------
   -- This process syncronizes the tx Clk and generates an enable pulse
   ----------------------------------------------------------------------------
   INT_TX_CLK_SYNC_PROCESS : process (Clk)
   begin  --   
      if (Clk'event and Clk = '1') then
         if (Rst = RESET_ACTIVE) then
            tx_clk_reg_d1 <= '0';
            tx_clk_reg_d2 <= '0';
            tx_clk_reg_d3 <= '0';
         else            
            tx_clk_reg_d1 <= Phy_tx_clk_axi_d;
            tx_clk_reg_d2 <= tx_clk_reg_d1;
            tx_clk_reg_d3 <= tx_clk_reg_d2;
         end if;
      end if;
   end process INT_TX_CLK_SYNC_PROCESS;  
   
   txClkEn <= '1' when tx_clk_reg_d2 = '1' and tx_clk_reg_d3 = '0' else 
                 '0';

  
   ----------------------------------------------------------------------------
   -- ADJP
   ----------------------------------------------------------------------------
   -- Adjust the packet length is it is less than minimum
   ----------------------------------------------------------------------------
   ADJP : process(mac_tx_frame_length)
   begin
      if mac_tx_frame_length > MinimumPacketLength then
         nibbleLength <= mac_tx_frame_length(5 to 15) & '0';
         en_pad       <= '0';
      else
         nibbleLength <= MinimumPacketLength(5 to 15) & '0';
         en_pad       <= '1';
      end if;
   end process ADJP;    
   
   nibbleLength_orig <= mac_tx_frame_length(5 to 15) & '0';

   mac_tx_frame_length <= Tx_packet_length;
   ----------------------------------------------------------------------------
  
end imp;






-------------------------------------------------------------------------------
-- xemac.vhd - entity/architecture pair
-------------------------------------------------------------------------------
--  ***************************************************************************
--  ** DISCLAIMER OF LIABILITY                                               **
--  **                                                                       **
--  **  This file contains proprietary and confidential information of       **
--  **  Xilinx, Inc. ("Xilinx"), that is distributed under a license         **
--  **  from Xilinx, and may be used, copied and/or disclosed only           **
--  **  pursuant to the terms of a valid license agreement with Xilinx.      **
--  **                                                                       **
--  **  XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION                **
--  **  ("MATERIALS") "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER           **
--  **  EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING WITHOUT                  **
--  **  LIMITATION, ANY WARRANTY WITH RESPECT TO NONINFRINGEMENT,            **
--  **  MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE. Xilinx        **
--  **  does not warrant that functions included in the Materials will       **
--  **  meet the requirements of Licensee, or that the operation of the      **
--  **  Materials will be uninterrupted or error-free, or that defects       **
--  **  in the Materials will be corrected. Furthermore, Xilinx does         **
--  **  not warrant or make any representations regarding use, or the        **
--  **  results of the use, of the Materials in terms of correctness,        **
--  **  accuracy, reliability or otherwise.                                  **
--  **                                                                       **
--  **  Xilinx products are not designed or intended to be fail-safe,        **
--  **  or for use in any application requiring fail-safe performance,       **
--  **  such as life-support or safety devices or systems, Class III         **
--  **  medical devices, nuclear facilities, applications related to         **
--  **  the deployment of airbags, or any other applications that could      **
--  **  lead to death, personal injury or severe property or                 **
--  **  environmental damage (individually and collectively, "critical       **
--  **  applications"). Customer assumes the sole risk and liability         **
--  **  of any use of Xilinx products in critical applications,              **
--  **  subject only to applicable laws and regulations governing            **
--  **  limitations on product liability.                                    **
--  **                                                                       **
--  **  Copyright 2010 Xilinx, Inc.                                          **
--  **  All rights reserved.                                                 **
--  **                                                                       **
--  **  This disclaimer and copyright notice must be retained as part        **
--  **  of this file at all times.                                           **
--  ***************************************************************************
--
-------------------------------------------------------------------------------
-- Filename     : xemac.vhd
-- Version      : v2.0
-- Description  : Design file for the Ethernet Lite MAC with
--                IPIF elements included.
-- VHDL-Standard: VHDL'93
-------------------------------------------------------------------------------
-- Structure:   
--
--  axi_ethernetlite.vhd
--      \
--      \-- axi_interface.vhd
--      \-- xemac.vhd
--           \
--           \-- mdio_if.vhd
--           \-- emac_dpram.vhd                     
--           \    \                     
--           \    \-- RAMB16_S4_S36
--           \                          
--           \    
--           \-- emac.vhd                     
--                \                     
--                \-- MacAddrRAM                   
--                \-- receive.vhd
--                \      rx_statemachine.vhd
--                \      rx_intrfce.vhd
--                \         async_fifo_fg.vhd
--                \      crcgenrx.vhd
--                \                     
--                \-- transmit.vhd
--                       crcgentx.vhd
--                          crcnibshiftreg
--                       tx_intrfce.vhd
--                          async_fifo_fg.vhd
--                       tx_statemachine.vhd
--                       deferral.vhd
--                          cntr5bit.vhd
--                          defer_state.vhd
--                       bocntr.vhd
--                          lfsr16.vhd
--                       msh_cnt.vhd
--                       ld_arith_reg.vhd
--
-------------------------------------------------------------------------------
-- Author:    PVK
-- History:    
-- PVK        06/07/2010     First Version
-- ^^^^^^
-- First version.  
-- ~~~~~~
-- PVK        07/21/2010     
-- ^^^^^^
-- Updated local register decoding logic to fix the issue related with read.
-- ~~~~~~
-------------------------------------------------------------------------------
-- Naming Conventions:
--      active low signals:                     "*_n"
--      clock signals:                          "clk", "clk_div#", "clk_#x"
--      reset signals:                          "rst", "rst_n"
--      generics:                               "C_*"
--      user defined types:                     "*_TYPE"
--      state machine next state:               "*_ns"
--      state machine current state:            "*_cs"
--      combinatorial signals:                  "*_com"
--      pipelined or register delay signals:    "*_d#"
--      counter signals:                        "*cnt*"
--      clock enable signals:                   "*_ce"
--      internal version of output port         "*_i"
--      device pins:                            "*_pin"
--      ports:                                  - Names begin with Uppercase
--      processes:                              "*_PROCESS"
--      component instantiations:               "<ENTITY_>I_<#|FUNC>
-------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.numeric_std.all;

-------------------------------------------------------------------------------
-- axi_ethernetlite_v3_0_18 library is used for axi_ethernetlite_v3_0_18
-- component declarations
-------------------------------------------------------------------------------
library axi_ethernetlite_v3_0_18;
use axi_ethernetlite_v3_0_18.all;

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Vcomponents from unisim library is used for FIFO instatiation
-- function declarations
-------------------------------------------------------------------------------
library unisim;
use unisim.Vcomponents.all;

-------------------------------------------------------------------------------
-- Definition of Generics:
-------------------------------------------------------------------------------
-- C_FAMILY               -- Target device family (spartan3e, spartan3a,
--                           spartan3an, spartan3af, virtex4 or virtex6)
-- C_S_AXI_ADDR_WIDTH     -- AXI address bus width - allowed value - 32 only
-- C_S_AXI_DATA_WIDTH     -- AXI data bus width - allowed value - 32 only
-- C_S_AXI_ACLK_PERIOD_PS -- The period of the AXI clock in ps
-- C_DUPLEX               -- 1 = full duplex, 0 = half duplex
-- C_TX_PING_PONG         -- 1 = ping-pong memory used for transmit buffer
-- C_RX_PING_PONG         -- 1 = ping-pong memory used for receive buffer
-- C_INCLUDE_MDIO         -- 1 = Include MDIO Innterface, 0 = No MDIO Interface
-- NODE_MAC               --   = Default MAC address of the core
-------------------------------------------------------------------------------
-- Definition of Ports:
--
--   System signals
--     Clk           -- System clock
--     Rst           -- System Reset
--     IP2INTC_Irpt  -- System Interrupt
--   IPIC signals   
--     IP2Bus_Data   -- IP  to Bus data 
--     IP2Bus_Error  -- IP  to Bus error
--     Bus2IP_Addr   -- Bus to IP address
--     Bus2IP_Data   -- Bus to IP data
--     Bus2IP_BE     -- Bus to IP byte enables
--     Bus2IP_RdCE   -- Bus to IP read chip enable
--     Bus2IP_WrCE   -- Bus to IP write chip enable
--     Bus2IP_Burst  -- Bus to IP burst
--  Ethernet
--     PHY_tx_clk    -- Ethernet tranmit clock
--     PHY_rx_clk    -- Ethernet receive clock
--     PHY_crs       -- Ethernet carrier sense
--     PHY_dv        -- Ethernet receive data valid
--     PHY_rx_data   -- Ethernet receive data
--     PHY_col       -- Ethernet collision indicator
--     PHY_rx_er     -- Ethernet receive error
--     PHY_rst_n     -- Ethernet PHY Reset
--     PHY_tx_en     -- Ethernet transmit enable
--     PHY_tx_data   -- Ethernet transmit data
--     Loopback      -- Internal Loopback enable
--     PHY_MDIO_I    -- Ethernet PHY MDIO data input 
--     PHY_MDIO_O    -- Ethernet PHY MDIO data output 
--     PHY_MDIO_T    -- Ethernet PHY MDIO data 3-state control
--     PHY_MDC       -- Ethernet PHY management clock
-------------------------------------------------------------------------------
-- ENTITY
-------------------------------------------------------------------------------
entity xemac is
  generic (
    C_FAMILY                : string := "virtex6";
    C_SELECT_XPM            : integer := 1;
    C_S_AXI_ADDR_WIDTH      : integer := 32;
    C_S_AXI_DATA_WIDTH      : integer := 32;
    C_S_AXI_ACLK_PERIOD_PS  : integer := 10000;
    C_DUPLEX                : integer := 1; -- 1 = full duplex, 0 = half duplex
    C_RX_PING_PONG          : integer := 0; -- 1 = ping-pong memory used for
                                            --     receive buffer
    C_TX_PING_PONG          : integer := 0; -- 1 = ping-pong memory used for
                                            --     transmit buffer
    C_INCLUDE_MDIO          : integer := 1; -- 1 = Include MDIO interface
                                            -- 0 = No MDIO interface
    NODE_MAC                : bit_vector := x"00005e00FACE"
                                            --  power up defaul MAC address
    );
  port (
    Clk           : in  std_logic;
    Rst           : in  std_logic;
    IP2INTC_Irpt  : out std_logic;


    -- Controls to the IP/IPIF modules
    IP2Bus_Data   : out std_logic_vector((C_S_AXI_DATA_WIDTH-1) downto 0 );
    IP2Bus_Error  : out std_logic;
    Bus2IP_Addr   : in  std_logic_vector(12 downto 0);
    Bus2IP_Data   : in  std_logic_vector((C_S_AXI_DATA_WIDTH-1) downto 0);
    Bus2IP_BE     : in  std_logic_vector(((C_S_AXI_DATA_WIDTH/8)-1)downto 0);
    Bus2IP_RdCE   : in  std_logic;
    Bus2IP_WrCE   : in  std_logic;
    Bus2IP_Burst  : in  std_logic;


    -- Ethernet Interface
    PHY_tx_clk    : in std_logic;
    PHY_rx_clk    : in std_logic;
    PHY_crs       : in std_logic;
    PHY_dv        : in std_logic;
    PHY_rx_data   : in std_logic_vector (3 downto 0);
    PHY_col       : in std_logic;
    PHY_rx_er     : in std_logic;
    PHY_tx_en     : out std_logic;
    PHY_tx_data   : out std_logic_vector (3 downto 0);
    Loopback      : out std_logic;

    -- MDIO Interface
    PHY_MDIO_I    : in  std_logic;
    PHY_MDIO_O    : out std_logic;
    PHY_MDIO_T    : out std_logic;
    PHY_MDC       : out std_logic
   );

end xemac;


architecture imp of xemac is

attribute DowngradeIPIdentifiedWarnings: string;

attribute DowngradeIPIdentifiedWarnings of imp : architecture is "yes";

-------------------------------------------------------------------------------
--  Constant Declarations
-------------------------------------------------------------------------------
constant MDIO_CNT         : integer := ((200000/C_S_AXI_ACLK_PERIOD_PS)+1);
constant IP2BUS_DATA_ZERO : std_logic_vector(0 to 31) := X"00000000";


-------------------------------------------------------------------------------
--  Signal and Type Declarations
-------------------------------------------------------------------------------
signal phy_rx_data_i    : std_logic_vector (3 downto 0);
signal phy_tx_data_i    : std_logic_vector (3 downto 0);
signal tx_DPM_ce        : std_logic;
signal tx_DPM_ce_i      : std_logic; -- added 03-03-05 MSH
signal tx_DPM_adr       : std_logic_vector (11 downto 0);
signal tx_DPM_wr_data   : std_logic_vector (3 downto 0);
signal tx_DPM_rd_data   : std_logic_vector (3 downto 0);
signal tx_ping_rd_data  : std_logic_vector (3 downto 0);
signal tx_pong_rd_data  : std_logic_vector (3 downto 0) := (others => '0');
signal tx_DPM_wr_rd_n   : std_logic;
signal rx_DPM_ce        : std_logic;
signal rx_DPM_ce_i      : std_logic; -- added 03-03-05 MSH
signal rx_DPM_adr       : std_logic_vector (11 downto 0);
signal rx_DPM_wr_data   : std_logic_vector (3 downto 0);
signal rx_DPM_rd_data   : std_logic_vector (3 downto 0);
signal rx_ping_rd_data  : std_logic_vector (3 downto 0);
signal rx_pong_rd_data  : std_logic_vector (3 downto 0) := (others => '0');
signal rx_DPM_wr_rd_n   : std_logic;
signal IPIF_tx_Ping_CE  : std_logic;
signal IPIF_tx_Pong_CE  : std_logic := '0';
signal IPIF_rx_Ping_CE  : std_logic;
signal IPIF_rx_Pong_CE  : std_logic := '0';
signal tx_ping_data_out : std_logic_vector (31 downto 0);
signal tx_pong_data_out : std_logic_vector (31 downto 0) := (others => '0');
signal rx_ping_data_out : std_logic_vector (31 downto 0);
signal rx_pong_data_out : std_logic_vector (31 downto 0) := (others => '0');
signal dpm_wr_ack       : std_logic;
signal dpm_rd_ack       : std_logic;
signal rx_done          : std_logic;
signal rx_done_d1       : std_logic := '0';
signal tx_done          : std_logic;
signal tx_done_d1       : std_logic := '0';
signal tx_done_d2       : std_logic := '0';
signal tx_ping_ce       : std_logic;
signal tx_pong_ping_l   : std_logic := '0';
signal tx_idle          : std_logic;
signal rx_idle          : std_logic;
signal rx_ping_ce       : std_logic;
signal rx_pong_ping_l   : std_logic := '0';
signal reg_access             : std_logic;
signal reg_en                 : std_logic;
signal tx_ping_reg_en         : std_logic;
signal tx_pong_reg_en         : std_logic;
signal rx_ping_reg_en         : std_logic;
signal rx_pong_reg_en         : std_logic;
signal tx_ping_ctrl_reg_en    : std_logic;
signal tx_ping_length_reg_en  : std_logic;
signal tx_pong_ctrl_reg_en    : std_logic;
signal tx_pong_length_reg_en  : std_logic;
signal rx_ping_ctrl_reg_en    : std_logic;
signal rx_pong_ctrl_reg_en    : std_logic;
signal loopback_en            : std_logic;
signal tx_intr_en             : std_logic;
signal ping_mac_program       : std_logic;
signal pong_mac_program       : std_logic;
signal ping_tx_status         : std_logic;
signal pong_tx_status         : std_logic;
signal ping_pkt_lenth         : std_logic_vector(15 downto 0);
signal pong_pkt_lenth         : std_logic_vector(15 downto 0);
signal rx_intr_en             : std_logic;
signal ping_rx_status         : std_logic;
signal pong_rx_status         : std_logic;
signal ping_tx_done           : std_logic;
signal mdio_data_out          : std_logic_vector(31 downto 0);
signal reg_data_out           : std_logic_vector(31 downto 0);
signal mdio_reg_en            : std_logic;
signal gie_reg                : std_logic;
signal gie_reg_en             : std_logic;
signal gie_enable             : std_logic;
signal tx_packet_length       : std_logic_vector(15 downto 0);
signal stat_reg_en            : std_logic;
signal status_reg             : std_logic_vector(5 downto 0);
signal ping_mac_prog_done     : std_logic;
signal transmit_start         : std_logic;
signal mac_program_start      : std_logic;
signal rx_buffer_ready        : std_logic;
signal dpm_addr_ack           : std_logic;
signal control_reg            : std_logic;
signal length_reg             : std_logic;
signal word_access            : std_logic;
signal reg_access_i           : std_logic;
signal ip2intc_irpt_i         : std_logic;
signal reg_access_d1          : std_logic;
signal ping_soft_status       : std_logic;
signal pong_soft_status       : std_logic;
signal rx_pong_ce_en          : std_logic;
signal tx_pong_ce_en          : std_logic;


-------------------------------------------------------------------------------
-- New ipif_ssp1 signal declaration                                          --
-------------------------------------------------------------------------------

signal bus2ip_ce       : std_logic;
signal tx_ping_ce_en   : std_logic;
signal rx_ping_ce_en   : std_logic;

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Component Declarations
-------------------------------------------------------------------------------
component SRL16E
    generic (
      INIT : bit_vector := X"0000"
      );

  port  (
    Q   : out std_logic; --[out]
    A0  : in  std_logic; --[in]
    A1  : in  std_logic; --[in]
    A2  : in  std_logic; --[in]
    A3  : in  std_logic; --[in]
    CE  : in  std_logic; --[in]
    CLK : in  std_logic; --[in]
    D   : in  std_logic  --[in]
  );
end component;


component FDR
  port (
    Q : out std_logic;
    C : in std_logic;
    D : in std_logic;
    R : in std_logic
  );
end component;

component FDRE
  port (
    Q  : out std_logic;
    C  : in std_logic;
    CE : in std_logic;
    D  : in std_logic;
    R  : in std_logic
  );
end component;

component LUT4
   generic(INIT : bit_vector);
   port (
     O  : out std_logic;
     I0 : in std_logic;
     I1 : in std_logic;
     I2 : in std_logic;
     I3 : in std_logic
   );
end component;


begin


   IP2Bus_Error <= '0';

   -- IP2INTC_Irpt generation if global interrupt is enable
   ip2intc_irpt_i <= gie_enable and ((rx_done and rx_intr_en) or
                                     (tx_done and tx_intr_en));
   ----------------------------------------------------------------------------
   -- IP2INTC_IRPT register
   ----------------------------------------------------------------------------
   IP2INTC_IRPT_REG_I: FDR
     port map (
       Q => IP2INTC_Irpt ,  --[out]
       C => Clk ,           --[in]
       D => ip2intc_irpt_i, --[in]
       R => Rst             --[in]
     );

--   ----------------------------------------------------------------------------
--   -- IPIF interface
--   ----------------------------------------------------------------------------


   -- PHY_tx_data conversion
   PHY_tx_data(0) <= phy_tx_data_i(0);
   PHY_tx_data(1) <= phy_tx_data_i(1);
   PHY_tx_data(2) <= phy_tx_data_i(2);
   PHY_tx_data(3) <= phy_tx_data_i(3);

   -- PHY_rx_data conversion
   phy_rx_data_i(0) <= PHY_rx_data(0);
   phy_rx_data_i(1) <= PHY_rx_data(1);
   phy_rx_data_i(2) <= PHY_rx_data(2);
   phy_rx_data_i(3) <= PHY_rx_data(3);

   ----------------------------------------------------------------------------
   -- EMAC
   ----------------------------------------------------------------------------
   EMAC_I: entity axi_ethernetlite_v3_0_18.emac
     generic map (
       C_DUPLEX            => C_DUPLEX,
       NODE_MAC            => NODE_MAC,
       C_FAMILY            => C_FAMILY
       )
     port map (
       Clk                 => Clk,
       Rst                 => Rst,
       Phy_tx_clk          => PHY_tx_clk,
       Phy_rx_clk          => PHY_rx_clk,
       Phy_crs             => phy_crs,
       Phy_dv              => Phy_dv,
       Phy_rx_data         => Phy_rx_data_i,
       Phy_col             => Phy_col,
       Phy_rx_er           => Phy_rx_er,
       Phy_tx_en           => Phy_tx_en,
       Phy_tx_data         => Phy_tx_data_i,
       Tx_DPM_ce           => tx_DPM_ce_i, 
       Tx_DPM_adr          => tx_DPM_adr,
       Tx_DPM_wr_data      => tx_DPM_wr_data,
       Tx_DPM_rd_data      => tx_DPM_rd_data,
       Tx_DPM_wr_rd_n      => tx_DPM_wr_rd_n,
       Tx_done             => tx_done,
       Tx_pong_ping_l      => tx_pong_ping_l,
       Tx_idle             => tx_idle,
       Rx_idle             => rx_idle,
       Rx_DPM_ce           => rx_DPM_ce_i, 
       Rx_DPM_adr          => rx_DPM_adr,
       Rx_DPM_wr_data      => rx_DPM_wr_data,
       Rx_DPM_rd_data      => rx_DPM_rd_data,
       Rx_DPM_wr_rd_n      => rx_DPM_wr_rd_n ,
       Rx_done             => rx_done,
       Rx_pong_ping_l      => rx_pong_ping_l,
       Tx_packet_length    => tx_packet_length,
       Transmit_start      => transmit_start,
       Mac_program_start   => mac_program_start,
       Rx_buffer_ready     => rx_buffer_ready
       );
   ----------------------------------------------------------------------------

   -- This core only supports word access
   word_access <= '1' when bus2ip_be="1111" else '0';

   -- DPRAM buffer chip enable generation
   bus2ip_ce     <= (Bus2IP_RdCE or (Bus2IP_WrCE and word_access));
   tx_ping_ce_en <= not Bus2IP_Addr(12) and not Bus2IP_Addr(11);
   rx_ping_ce_en <= Bus2IP_Addr(12) and not Bus2IP_Addr(11);

   IPIF_tx_Ping_CE <= bus2ip_ce and tx_ping_ce_en;
   IPIF_rx_Ping_CE <= bus2ip_ce and rx_ping_ce_en;

   -- IP2Bus_Data generation
   IP2BUS_DATA_GENERATE: for i in 31 downto 0 generate
      IP2Bus_Data(i) <= ((
                          (tx_ping_data_out(i) and tx_ping_ce_en)  or
                          (tx_pong_data_out(i) and tx_pong_ce_en)  or
                          (rx_ping_data_out(i) and rx_ping_ce_en)  or
                          (rx_pong_data_out(i) and rx_pong_ce_en)
                         ) and not reg_access)
                        or
                        ((
                          (reg_data_out(i)  and not mdio_reg_en) or
                          (mdio_data_out(i) and     mdio_reg_en)
                         ) and reg_access) ;

   end generate IP2BUS_DATA_GENERATE;


   ----------------------------------------------------------------------------
   -- DPM_TX_RD_DATA_GENERATE
   ----------------------------------------------------------------------------
   -- This logic generates tx_DPM_rd_data for transmit section from
   -- tx_ping_buffer and tx_pong_buffer.
   ----------------------------------------------------------------------------
   DPM_TX_RD_DATA_GENERATE: for i in 0 to 3 generate
      tx_DPM_rd_data(i) <= (tx_ping_rd_data(i) and not tx_pong_ping_l
                                               and (not tx_idle)) or
                            (tx_pong_rd_data(i) and     tx_pong_ping_l
                                                and (not tx_idle));
   end generate DPM_TX_RD_DATA_GENERATE;

   ----------------------------------------------------------------------------
   -- DPM_RX_RD_DATA_GENERATE
   ----------------------------------------------------------------------------
   -- This logic generates rx_DPM_rd_data for receive section from
   -- rx_ping_buffer and rx_pong_buffer.
   ----------------------------------------------------------------------------
   DPM_RX_RD_DATA_GENERATE: for i in 0 to 3 generate
      rx_DPM_rd_data(i) <= (rx_ping_rd_data(i) and not rx_pong_ping_l) or
                           (rx_pong_rd_data(i) and     rx_pong_ping_l);
   end generate DPM_RX_RD_DATA_GENERATE;

   -- Chip enable generation
   tx_ping_ce <= tx_DPM_ce and not tx_pong_ping_l;
   tx_DPM_ce  <= tx_DPM_ce_i;
   rx_DPM_ce  <= rx_DPM_ce_i;
   rx_ping_ce <= rx_DPM_ce and not rx_pong_ping_l;

   ----------------------------------------------------------------------------
   -- TX_PING Buffer
   ----------------------------------------------------------------------------
   TX_PING: entity axi_ethernetlite_v3_0_18.emac_dpram
     generic map (
       C_FAMILY             => C_FAMILY,
       C_SELECT_XPM         => C_SELECT_XPM
       )
     port map (
       Clk                  => Clk                        ,
       Rst                  => Rst                        ,
       Ce_a                 => tx_ping_ce                 ,
       Wr_rd_n_a            => tx_DPM_wr_rd_n             ,
       Adr_a                => tx_DPM_adr                 ,
       Data_in_a            => tx_DPM_wr_data             ,
       Data_out_a           => tx_ping_rd_data            ,
       Ce_b                 => IPIF_tx_Ping_CE            ,
       Wr_rd_n_b            => Bus2IP_WrCE                ,
       Adr_b                => bus2ip_addr(10 downto 2)   ,
       Data_in_b            => Bus2IP_Data                ,
       Data_out_b           => tx_ping_data_out
       );


   ----------------------------------------------------------------------------
   -- RX_PING Buffer
   ----------------------------------------------------------------------------
   RX_PING: entity axi_ethernetlite_v3_0_18.emac_dpram
     generic map (
       C_FAMILY             => C_FAMILY,
       C_SELECT_XPM         => C_SELECT_XPM
       )
     port map (
       Clk                  => Clk                        ,
       Rst                  => Rst                        ,
       Ce_a                 => rx_ping_ce                 ,
       Wr_rd_n_a            => rx_DPM_wr_rd_n             ,
       Adr_a                => rx_DPM_adr                 ,
       Data_in_a            => rx_DPM_wr_data             ,
       Data_out_a           => rx_ping_rd_data            ,
       Ce_b                 => IPIF_rx_Ping_CE            ,
       Wr_rd_n_b            => Bus2IP_WrCE                ,
       Adr_b                => bus2ip_addr(10 downto 2)   ,
       Data_in_b            => Bus2IP_Data                ,
       Data_out_b           => rx_ping_data_out
       );


   ----------------------------------------------------------------------------
   -- TX Done register
   ----------------------------------------------------------------------------
   TX_DONE_D1_I: FDR
     port map (
       Q => tx_done_d1 , --[out]
       C => Clk ,        --[in]
       D => tx_done    , --[in]
       R => Rst          --[in]
     );

   TX_DONE_D2_I: FDR
     port map (
       Q => tx_done_d2 , --[out]
       C => Clk ,        --[in]
       D => tx_done_d1 , --[in]
       R => Rst          --[in]
     );

   ----------------------------------------------------------------------------
   -- Transmit Pong memory generate
   ----------------------------------------------------------------------------
   TX_PONG_GEN: if C_TX_PING_PONG = 1 generate

      signal tx_pong_ce        : std_logic;
      signal pp_tog_ce         : std_logic;

      attribute INIT                  : string;

      -- attribute INIT of PP_TOG_LUT_I: label is "1111";

      Begin

         TX_PONG_I: entity axi_ethernetlite_v3_0_18.emac_dpram
           generic map (
             C_FAMILY             => C_FAMILY,
             C_SELECT_XPM         => C_SELECT_XPM
             )
           port map (
             Clk                  => Clk                        ,
             Rst                  => Rst                        ,
             Ce_a                 => tx_pong_ce                 ,
             Wr_rd_n_a            => tx_DPM_wr_rd_n             ,
             Adr_a                => tx_DPM_adr                 ,
             Data_in_a            => tx_DPM_wr_data             ,
             Data_out_a           => tx_pong_rd_data            ,
             Ce_b                 => IPIF_tx_Pong_CE            ,
             Wr_rd_n_b            => Bus2IP_WrCE                ,
             Adr_b                => bus2ip_addr(10 downto 2)   ,
             Data_in_b            => Bus2IP_Data                ,
             Data_out_b           => tx_pong_data_out
             );

      -- TX Pong Buffer Chip enable
      tx_pong_ce <= tx_DPM_ce and tx_pong_ping_l;

      --IPIF_tx_Pong_CE <= bus2ip_ce and not Bus2IP_Addr(12) Bus2IP_Addr(11);
      IPIF_tx_Pong_CE <= bus2ip_ce and tx_pong_ce_en;
      tx_pong_ce_en <=  not Bus2IP_Addr(12) and Bus2IP_Addr(11);
      -------------------------------------------------------------------------
      -- TX_PONG_PING_L_PROCESS
      -------------------------------------------------------------------------
      -- This process generate tx_pong_ping_l for TX PING/PONG buffer access
      -------------------------------------------------------------------------
      TX_PONG_PING_L_PROCESS:process (Clk)
      begin   -- process
          if (Clk'event and Clk = '1') then
             if (Rst = '1') then
                tx_pong_ping_l <= '0';
             elsif (tx_done_d1 = '1' ) then
                   tx_pong_ping_l <= not tx_pong_ping_l;
             elsif (pong_tx_status = '1' and ping_tx_status = '0' ) then
                   tx_pong_ping_l <= '1';
             elsif (pong_tx_status = '0' and ping_tx_status = '1' ) then
                   tx_pong_ping_l <= '0';
             else
                tx_pong_ping_l <= tx_pong_ping_l;
             end if;
          end if;
      end process;

   end generate TX_PONG_GEN;



   ----------------------------------------------------------------------------
   -- RX Done register
   ----------------------------------------------------------------------------
   RX_DONE_D1_I: FDR
     port map (
       Q => rx_done_d1 , --[out]
       C => Clk        , --[in]
       D => rx_done    , --[in]
       R => Rst          --[in]
     );

   ----------------------------------------------------------------------------
   -- Receive Pong memory generate
   ----------------------------------------------------------------------------
   RX_PONG_GEN: if C_RX_PING_PONG = 1 generate

      signal rx_pong_ce        : std_logic;

      Begin

        RX_PONG_I: entity axi_ethernetlite_v3_0_18.emac_dpram
          generic map (
            C_FAMILY             => C_FAMILY,
            C_SELECT_XPM         => C_SELECT_XPM
            )
          port map (
            Clk                  => Clk                        ,
            Rst                  => Rst                        ,
            Ce_a                 => rx_pong_ce                 ,
            Wr_rd_n_a            => rx_DPM_wr_rd_n             ,
            Adr_a                => rx_DPM_adr                 ,
            Data_in_a            => rx_DPM_wr_data             ,
            Data_out_a           => rx_pong_rd_data            ,
            Ce_b                 => IPIF_rx_Pong_CE            ,
            Wr_rd_n_b            => Bus2IP_WrCE                ,
            Adr_b                => bus2ip_addr(10 downto 2)   ,
            Data_in_b            => Bus2IP_Data                ,
            Data_out_b           => rx_pong_data_out
            );

      -- RX Pong Buffer enable
      rx_pong_ce <= rx_DPM_ce and rx_pong_ping_l;

      --IPIF_rx_Pong_CE <= bus2ip_ce  and Bus2IP_Addr(12) and Bus2IP_Addr(11);
      IPIF_rx_Pong_CE <= bus2ip_ce and rx_pong_ce_en;
      rx_pong_ce_en   <= Bus2IP_Addr(12) and Bus2IP_Addr(11);
      -------------------------------------------------------------------------
      -- RX_PONG_PING_L_PROCESS
      -------------------------------------------------------------------------
      -- This process generate rx_pong_ping_l for RX PING/PONG buffer access
      -------------------------------------------------------------------------
        RX_PONG_PING_L_PROCESS:process (Clk)
         begin   -- process
            if (Clk'event and Clk = '1') then
               if (Rst = '1') then
                  rx_pong_ping_l <= '0';
               elsif (rx_done_d1 = '1') then
                  if rx_pong_ping_l = '0' then
                     rx_pong_ping_l <= '1';
                  else
                     rx_pong_ping_l <= '0';
                 end if;
               else
                  rx_pong_ping_l <= rx_pong_ping_l;
               end if;
            end if;
         end process;

   end generate RX_PONG_GEN;

   ----------------------------------------------------------------------------
   -- Regiter Address Decoding
   ----------------------------------------------------------------------------
   -- This process loads data from the AXI when there is a write request and
   -- the control register is enabled.
   -- Register Address Space
   -----------------------------------------
   -- **** MDIO Registers offset ****
   --       Address Register    => 0x07E4
   --       Write Data Register => 0x07E8
   --       Read Data Register  => 0x07Ec
   --       Control Register    => 0x07F0
   -----------------------------------------
   -- **** Transmit Registers offset ****
   --       Ping Length Register  => 0x07F4
   --       Ping Control Register => 0x07FC
   --       Pong Length  Register => 0x0FF4
   --       Pong Control Register => 0x0FFC
   -----------------------------------------
   -- **** Receive Registers offset ****
   --       Ping Control Register => 0x17FC
   --       Pong Control Register => 0x1FFC
   ------------------------------------------
   -- bus2ip_addr(12 downto 0)= axi_addr (12 downto 0)
   ----------------------------------------------------------------------------
   reg_access_i <= '1' when bus2ip_addr(10 downto 5) = "111111"
                 else '0';


   -- Register access enable
   reg_en <= reg_access_i and (not Bus2IP_Burst);

   -- TX/RX PING/PONG address decode
   tx_ping_reg_en <= reg_en and (not bus2ip_addr(12)) and (not bus2ip_addr(11));
   rx_ping_reg_en <= reg_en and (    bus2ip_addr(12)) and (not bus2ip_addr(11));

   -- Status/Control/Length address decode
   stat_reg_en <= not (bus2ip_addr(4) and bus2ip_addr(3) and bus2ip_addr(2));
   control_reg <= bus2ip_addr(4) and      bus2ip_addr(3)  and bus2ip_addr(2);
   length_reg  <= bus2ip_addr(4) and (not bus2ip_addr(3)) and bus2ip_addr(2);
   gie_reg     <= bus2ip_addr(4) and      bus2ip_addr(3) and (not bus2ip_addr(2));




   ---- TX/RX Ping/Pong Control/Length reg enable
   tx_ping_ctrl_reg_en   <= tx_ping_reg_en and control_reg;
   tx_ping_length_reg_en <= tx_ping_reg_en and length_reg;
   rx_ping_ctrl_reg_en   <= rx_ping_reg_en and control_reg;
   gie_reg_en            <= tx_ping_reg_en and gie_reg;


   ----------------------------------------------------------------------------
   -- REG_ACCESS_PROCESS
   ----------------------------------------------------------------------------
   -- Registering the reg_access to break long timing path
   ----------------------------------------------------------------------------
   REG_ACCESS_PROCESS : process (Clk)
   begin  -- process
      if (Clk'event and Clk = '1') then
         if (Rst = '1') then
            reg_access     <= '0';
            reg_access_d1  <= '0';
         elsif Bus2IP_RdCE='1' then
            -- TX/RX Ping/Pong Control/Length reg enable
            reg_access     <= reg_access_i;
            reg_access_d1  <= reg_access;
         end if;
      end if;
   end process REG_ACCESS_PROCESS;

   ----------------------------------------------------------------------------
   -- TX_PONG_REG_GEN : Receive Pong Register generate
   ----------------------------------------------------------------------------
   -- This Logic is included only if both the buffers are enabled.
   ----------------------------------------------------------------------------
   TX_PONG_REG_GEN: if C_TX_PING_PONG = 1 generate

      tx_pong_reg_en      <= reg_en and (not bus2ip_addr(12))
                                    and (bus2ip_addr(11));
      tx_pong_ctrl_reg_en <= '1' when (tx_pong_reg_en='1') and
                                      (control_reg='1')    else
                             '0';

      tx_pong_length_reg_en <= '1' when (tx_pong_reg_en='1') and
                                        (length_reg='1') else
                               '0';

      -------------------------------------------------------------------------
      -- TX_PONG_CTRL_REG_PROCESS
      -------------------------------------------------------------------------
      -- This process loads data from the AXI when there is a write request and
      -- the control register is enabled.
      -------------------------------------------------------------------------
      TX_PONG_CTRL_REG_PROCESS : process (Clk)
      begin  -- process
         if (Clk'event and Clk = '1') then
            if (Rst = '1') then
               pong_mac_program   <= '0';
               pong_tx_status     <= '0';
               pong_soft_status   <= '0';
            elsif (Bus2IP_WrCE = '1' and tx_pong_ctrl_reg_en = '1') then
               --  Load Pong Control Register with AXI
               --  data if there is a write request
               --  and the control register is enabled
               pong_soft_status   <= Bus2IP_Data(31);
               pong_mac_program   <= Bus2IP_Data(1);
               pong_tx_status     <= Bus2IP_Data(0);
            -- Clear the status bit when trnasmit complete
            elsif (tx_done_d1 = '1' and tx_pong_ping_l = '1') then
               pong_tx_status     <= '0';
               pong_mac_program   <= '0';
            end if;
         end if;
      end process TX_PONG_CTRL_REG_PROCESS;

      -------------------------------------------------------------------------
      -- TX_PONG_LENGTH_REG_PROCESS
      -------------------------------------------------------------------------
      -- This process loads data from the AXI when there is a write request and
      -- the length register is enabled.
      -------------------------------------------------------------------------
      TX_PONG_LENGTH_REG_PROCESS : process (Clk)
      begin  -- process
         if (Clk'event and Clk = '1') then
            if (Rst = '1') then
               pong_pkt_lenth <= (others=>'0');
            elsif (Bus2IP_WrCE = '1' and tx_pong_length_reg_en = '1') then
               --  Load Packet length Register with AXI
               --  data if there is a write request
               --  and the length register is enabled
               pong_pkt_lenth <= Bus2IP_Data(15 downto 0);
            end if;
         end if;
      end process TX_PONG_LENGTH_REG_PROCESS;

   end generate TX_PONG_REG_GEN;

   ----------------------------------------------------------------------------
   -- NO_TX_PING_SIG :No Pong registers
   ----------------------------------------------------------------------------
   NO_TX_PING_SIG: if C_TX_PING_PONG = 0 generate

      tx_pong_ping_l        <= '0';
      tx_pong_length_reg_en <= '0';
      tx_pong_ctrl_reg_en   <= '0';
      pong_pkt_lenth        <= (others=>'0');
      pong_mac_program      <= '0';
      pong_tx_status        <= '0';
      IPIF_tx_Pong_CE       <= '0';
      tx_pong_data_out      <= (others=>'0');
      tx_pong_rd_data       <= (others=>'0');


   end generate NO_TX_PING_SIG;


   ----------------------------------------------------------------------------
   -- RX_PONG_REG_GEN: Receive Pong Register generate
   ----------------------------------------------------------------------------
   -- This Logic is included only if both the buffers are enabled.
   ----------------------------------------------------------------------------
   RX_PONG_REG_GEN: if C_RX_PING_PONG = 1 generate

      rx_pong_reg_en      <= reg_en and (bus2ip_addr(12)) and (bus2ip_addr(11));
      rx_pong_ctrl_reg_en <= '1' when (rx_pong_reg_en='1') and
                                      (control_reg='1')    else
                             '0';

      -- Receive frame indicator
      rx_buffer_ready     <= not (ping_rx_status and pong_rx_status);

      -------------------------------------------------------------------------
      -- RX_PONG_CTRL_REG_PROCESS
      -------------------------------------------------------------------------
      -- This process loads data from the AXI when there is a write request and
      -- the Pong control register is enabled.
      -------------------------------------------------------------------------
      RX_PONG_CTRL_REG_PROCESS : process (Clk)
      begin  -- process
         if (Clk'event and Clk = '1') then
            if (Rst = '1') then
                pong_rx_status   <= '0';
            elsif (Bus2IP_WrCE = '1' and rx_pong_ctrl_reg_en = '1') then
               --  Load Control Register with AXI
               --  data if there is a write request
               --  and the control register is enabled
               pong_rx_status   <= Bus2IP_Data(0);
            -- Clear the status bit when trnasmit complete
            --elsif (rx_done_d1 = '1' and rx_pong_ping_l = '1') then
            elsif (rx_done = '1' and rx_pong_ping_l = '1') then
               pong_rx_status   <= '1';
            end if;
         end if;
      end process RX_PONG_CTRL_REG_PROCESS;

   end generate RX_PONG_REG_GEN;

   ----------------------------------------------------------------------------
   -- No Pong registers
   ----------------------------------------------------------------------------
   NO_RX_PING_SIG: if C_RX_PING_PONG = 0 generate

      rx_pong_ping_l      <= '0';
      rx_pong_reg_en      <= '0';
      rx_pong_ctrl_reg_en <= '0';
      pong_rx_status      <= '0';
      IPIF_rx_Pong_CE     <= '0';
      rx_pong_rd_data     <= (others=>'0');
      rx_pong_data_out    <= (others=>'0');

      -- Receive frame indicator
      rx_buffer_ready     <= not ping_rx_status ;

   end generate NO_RX_PING_SIG;

   ----------------------------------------------------------------------------
   -- TX_PING_CTRL_REG_PROCESS
   ----------------------------------------------------------------------------
   -- This process loads data from the AXI when there is a write request and
   -- the control register is enabled.
   ----------------------------------------------------------------------------
   TX_PING_CTRL_REG_PROCESS : process (Clk)
   begin  -- process
      if (Clk'event and Clk = '1') then
         if (Rst = '1') then
            tx_intr_en       <= '0';
            ping_mac_program <= '0';
            ping_tx_status   <= '0';
            ping_soft_status <= '0';
         elsif (Bus2IP_WrCE = '1' and tx_ping_ctrl_reg_en = '1') then
            --  Load Control Register with AXI
            --  data if there is a write request
            --  and the control register is enabled
            ping_soft_status <= Bus2IP_Data(31);
            tx_intr_en       <= Bus2IP_Data(3);
            ping_mac_program <= Bus2IP_Data(1);
            ping_tx_status   <= Bus2IP_Data(0);
         -- Clear the status bit when trnasmit complete
         elsif (tx_done_d1 = '1' and tx_pong_ping_l = '0') then
            ping_tx_status   <= '0';
            ping_mac_program <= '0';
         end if;
      end if;
   end process TX_PING_CTRL_REG_PROCESS;

   ----------------------------------------------------------------------------
   -- TX_LOOPBACK_REG_PROCESS
   ----------------------------------------------------------------------------
   -- This process loads data from the AXI when there is a write request and
   -- the control register is enabled.
   ----------------------------------------------------------------------------
   TX_LOOPBACK_REG_PROCESS : process (Clk)
   begin  -- process
      if (Clk'event and Clk = '1') then
         if (Rst = '1') then
            loopback_en      <= '0';
         elsif (Bus2IP_WrCE = '1' and tx_ping_ctrl_reg_en = '1'
                                  and tx_idle='1'               ) then
            --  Load loopback Register with AXI
            --  data if there is a write request
            --  and the Loopback register is enabled
            loopback_en      <= Bus2IP_Data(4);
         -- Clear the status bit when trnasmit complete
         end if;
      end if;
   end process TX_LOOPBACK_REG_PROCESS;

   ----------------------------------------------------------------------------
   -- CDC module for syncing tx_en_i in fifo_empty domain
   ----------------------------------------------------------------------------
--  CDC_LOOPBACK: entity  proc_common_v4_0.cdc_sync
--  generic map (
--    C_CDC_TYPE           => 1,
--    C_RESET_STATE        => 0,
--    C_SINGLE_BIT         => 1,
--    C_FLOP_INPUT         => 0,
--    C_VECTOR_WIDTH       => 1,
--    C_MTBF_STAGES        => 4
--            )
--  port map(
--    prmry_aclk            => '1',
--    prmry_resetn          => '1',
--    prmry_in              => loopback_en,
--    prmry_ack             => open,
--    scndry_out            => Loopback,
--    scndry_aclk           => PHY_rx_clk,
--    scndry_resetn         => '1',
--    prmry_vect_in         => (OTHERS => '0'),
--    scndry_vect_out       => open
--     );


  Loopback <= loopback_en; --added the cdc block to drive the output directly

   ----------------------------------------------------------------------------
   -- TX_PING_LENGTH_REG_PROCESS
   ----------------------------------------------------------------------------
   -- This process loads data from the AXI when there is a write request and
   -- the Length register is enabled.
   ----------------------------------------------------------------------------
   TX_PING_LENGTH_REG_PROCESS : process (Clk)
   begin  -- process
      if (Clk'event and Clk = '1') then
         if (Rst = '1') then
            ping_pkt_lenth <= (others=>'0');
         elsif (Bus2IP_WrCE = '1' and tx_ping_length_reg_en = '1') then
            --  Load Packet length Register with AXI
            --  data if there is a write request
            --  and the length register is enabled
            ping_pkt_lenth <= Bus2IP_Data(15 downto 0);
         end if;
      end if;
   end process TX_PING_LENGTH_REG_PROCESS;

   ----------------------------------------------------------------------------
   -- GIE_EN_REG_PROCESS
   ----------------------------------------------------------------------------
   -- This process loads data from the AXI when there is a write request and
   -- the GIE register is enabled.
   ----------------------------------------------------------------------------
   GIE_EN_REG_PROCESS : process (Clk)
   begin  -- process
      if (Clk'event and Clk = '1') then
         if (Rst = '1') then
            gie_enable <= '0';
         elsif (Bus2IP_WrCE = '1' and gie_reg_en = '1') then
            --  Load Global Interrupt Enable Register with AXI
            --  data if there is a write request
            --  and the length register is enabled
            gie_enable <= Bus2IP_Data(31);
         end if;
      end if;
   end process GIE_EN_REG_PROCESS;

   ----------------------------------------------------------------------------
   -- RX_PING_CTRL_REG_PROCESS
   ----------------------------------------------------------------------------
   -- This process loads data from the AXI when there is a write request and
   -- the Ping control register is enabled.
   ----------------------------------------------------------------------------
   RX_PING_CTRL_REG_PROCESS : process (Clk)
   begin  -- process
      if (Clk'event and Clk = '1') then
         if (Rst = '1') then
             rx_intr_en       <= '0';
             ping_rx_status   <= '0';
         elsif (Bus2IP_WrCE = '1' and rx_ping_ctrl_reg_en = '1') then
            --  Load Control Register with AXI
            --  data if there is a write request
            --  and the control register is enabled
            rx_intr_en       <= Bus2IP_Data(3);
            ping_rx_status   <= Bus2IP_Data(0);
         -- Clear the status bit when trnasmit complete
         elsif (rx_done = '1' and rx_pong_ping_l = '0') then
            ping_rx_status   <= '1';
         end if;
      end if;
   end process RX_PING_CTRL_REG_PROCESS;

   ----------------------------------------------------------------------------
   -- REGISTER_READ_PROCESS
   ----------------------------------------------------------------------------
   -- This process loads data from the AXI when there is a write request and
   -- the control register is enabled.
   ----------------------------------------------------------------------------
   REGISTER_READ_PROCESS : process (Clk)
   begin  -- process
      if (Clk'event and Clk = '1') then
         if (Rst = '1') then
            reg_data_out   <= (others=>'0');
         elsif (Bus2IP_RdCE = '1' and tx_ping_ctrl_reg_en = '1') then
         -- TX PING Control Register Read through AXI
            reg_data_out(0)   <= ping_tx_status;
            reg_data_out(1)   <= ping_mac_program;
            reg_data_out(2)   <= '0';
            reg_data_out(3)   <= tx_intr_en;
            reg_data_out(4)   <= loopback_en;
            reg_data_out(31)    <= ping_soft_status;
            reg_data_out(30 downto 5)   <= (others=>'0');
         elsif (Bus2IP_RdCE = '1' and tx_pong_ctrl_reg_en = '1') then
         -- TX PONG Control Register Read through AXI
            reg_data_out(0)   <= pong_tx_status;
            reg_data_out(1)   <= pong_mac_program;
            reg_data_out(30 downto 2)   <= (others=>'0');
            reg_data_out(31)    <= pong_soft_status;
         elsif (Bus2IP_RdCE = '1' and tx_ping_length_reg_en = '1') then
         -- TX PING Length Register Read through AXI
            reg_data_out(31 downto 16)   <= (others=>'0');
            reg_data_out(15 downto 0)  <= ping_pkt_lenth;
         elsif (Bus2IP_RdCE = '1' and tx_pong_length_reg_en = '1') then
         -- TX PONG Length Register Read through AXI
            reg_data_out(31 downto 16)   <= (others=>'0');
            reg_data_out(15 downto 0)  <= pong_pkt_lenth;
         elsif (Bus2IP_RdCE = '1' and rx_ping_ctrl_reg_en = '1') then
         -- RX PING Control Register Read through AXI
            reg_data_out(0)   <= ping_rx_status;
            reg_data_out(1)   <= '0';
            reg_data_out(2)   <= '0';
            reg_data_out(3)   <= rx_intr_en;
            reg_data_out(31 downto 4)   <= (others=>'0');
         elsif (Bus2IP_RdCE = '1' and rx_pong_ctrl_reg_en = '1') then
         -- RX PONG Control Register Read through AXI
            reg_data_out(0)   <= pong_rx_status;
            reg_data_out(31 downto 1)   <= (others=>'0');
         elsif (Bus2IP_RdCE = '1' and gie_reg_en = '1') then
         -- GIE Register Read through AXI
            reg_data_out(31)   <= gie_enable;
            reg_data_out(30 downto 0)   <= (others=>'0');
         elsif (Bus2IP_RdCE = '1' and stat_reg_en = '1') then
         -- Common Status Register Read through AXI
            reg_data_out(0)   <= status_reg(0);
            reg_data_out(1)   <= status_reg(1);
            reg_data_out(2)   <= status_reg(2);
            reg_data_out(3)   <= status_reg(3);
            reg_data_out(4)   <= status_reg(4);
            reg_data_out(5)   <= status_reg(5);
            reg_data_out(31 downto 6)   <= (others=>'0');
         --else
         --   reg_data_out <= (others=>'0');
         end if;
      end if;
   end process REGISTER_READ_PROCESS;

   ----------------------------------------------------------------------------
   -- COMMON_STATUS_REG_PROCESS
   ----------------------------------------------------------------------------
   -- This process loads data from the AXI when there is a write request and
   -- the control register is enabled.
   -- status_reg        : std_logic_vector(0 to 5);
   -- status reg address = 0x07E0
   -- status_reg(5) : Ping TX complete
   -- status_reg(4) : Pong TX complete
   -- status_reg(3) : Ping RX complete
   -- status_reg(2) : Pong RX complete
   -- status_reg(1) : Ping MAC program complete
   -- status_reg(0) : Pong MAC program complete
   -- All Status bit will be cleared after reading this register
   ----------------------------------------------------------------------------
   COMMON_STATUS_REG_PROCESS : process (Clk)
   begin  -- process
      if (Clk'event and Clk = '1') then
         if (Rst = '1') then
             status_reg   <= (others=>'0');
         elsif (tx_done = '1') then
            if (tx_pong_ping_l = '0' and ping_mac_program='0' ) then
               status_reg    <= (others=>'0');
               status_reg(5) <= '1';
            elsif (tx_pong_ping_l = '0' and ping_mac_program='1' ) then
               status_reg    <= (others=>'0');
               status_reg(1) <= '1';
            elsif (tx_pong_ping_l = '1' and pong_mac_program='0' ) then
               status_reg    <= (others=>'0');
               status_reg(4) <= '1';
            elsif (tx_pong_ping_l = '1' and pong_mac_program='1' ) then
               status_reg    <= (others=>'0');
               status_reg(0) <= '1';
            end if;
         elsif (rx_done_d1 = '1') then
            if (rx_pong_ping_l = '0') then
               status_reg    <= (others=>'0');
               status_reg(3) <= '1';
            else
               status_reg    <= (others=>'0');
               status_reg(2) <= '1';
            end if;
         end if;
      end if;
   end process COMMON_STATUS_REG_PROCESS;

   ----------------------------------------------------------------------------
   -- TX_LENGTH_MUX_PROCESS
   ----------------------------------------------------------------------------
   -- This process loads data from the AXI when there is a write request and
   -- the control register is enabled.
   ----------------------------------------------------------------------------
   TX_LENGTH_MUX_PROCESS : process (Clk)
   begin  -- process
      if (Clk'event and Clk = '1') then
         if (Rst = '1') then
             tx_packet_length   <= (others=>'0');
         elsif (tx_pong_ping_l = '1') then
            --  Load Control Register with AXI
            tx_packet_length <= pong_pkt_lenth;
         -- Clear the status bit when trnasmit complete
         else
            tx_packet_length <= ping_pkt_lenth;
         end if;
      end if;
   end process TX_LENGTH_MUX_PROCESS;

   -- Tx Start indicator
   transmit_start <= ((ping_tx_status and not ping_mac_program) or
                      (pong_tx_status and not pong_mac_program)) and
                       not tx_done_d2;

   -- MAC program start indicator
   mac_program_start <= (ping_tx_status and ping_mac_program) or
                        (pong_tx_status and pong_mac_program);

   ----------------------------------------------------------------------------

   ----------------------------------------------------------------------------
   -- MDIO_GEN :- Include MDIO interface if the parameter C_INCLUDE_MDIO = 1
   ----------------------------------------------------------------------------
   MDIO_GEN: if C_INCLUDE_MDIO = 1 generate

      signal mdio_addr_en       : std_logic;
      signal mdio_wr_data_en    : std_logic;
      signal mdio_rd_data_en    : std_logic;
      signal mdio_ctrl_en       : std_logic;
      signal mdio_op_i          : std_logic;
      signal mdio_en_i          : std_logic;
      signal mdio_req_i         : std_logic;
      signal mdio_done_i        : std_logic;
      signal mdio_wr_data_reg   : std_logic_vector(15 downto 0);
      signal mdio_rd_data_reg   : std_logic_vector(15 downto 0);
      signal mdio_phy_addr      : std_logic_vector(4 downto 0);
      signal mdio_reg_addr      : std_logic_vector(4 downto 0);
      signal mdio_clk_i         : std_logic;
     -- signal mdio_ctrl_en_reg   : std_logic;
      signal clk_cnt            : integer range 0 to 63;
   begin

      -- MDIO reg enable
      mdio_reg_en     <= --not stat_reg_en_reg and
                         (mdio_addr_en    or
                          mdio_wr_data_en or
                          mdio_rd_data_en or
                          mdio_ctrl_en ) and (not Bus2IP_Burst);
                          --mdio_ctrl_en or mdio_ctrl_en_reg      ) and (not Bus2IP_Burst);

      -- MDIO address reg enable
      mdio_addr_en    <= reg_en and (not bus2ip_addr(4))
                                and (not bus2ip_addr(3))
                                and (    bus2ip_addr(2));

      -- MDIO write data reg enable
      mdio_wr_data_en <= reg_en and (not bus2ip_addr(4))
                                and (    bus2ip_addr(3))
                                and (not bus2ip_addr(2));

      -- MDIO read data reg enable
      mdio_rd_data_en <= reg_en and (not bus2ip_addr(4))
                                and (    bus2ip_addr(3))
                                and (    bus2ip_addr(2));

      -- MDIO controlreg enable
      mdio_ctrl_en    <= reg_en and (    bus2ip_addr(4))
                                and (not bus2ip_addr(3))
                                and (not bus2ip_addr(2));

      -------------------------------------------------------------------------
      -- MDIO_CTRL_REG_WR_PROCESS
      -------------------------------------------------------------------------
      -- This process loads data from the AXI when there is a write request and
      -- the MDIO control register is enabled.
      -------------------------------------------------------------------------
      MDIO_CTRL_REG_WR_PROCESS : process (Clk)
      begin  -- process
         if (Clk'event and Clk = '1') then
            if (Rst = '1') then
               mdio_en_i    <= '0';
               mdio_req_i   <= '0';
            elsif (Bus2IP_WrCE = '1' and mdio_ctrl_en= '1') then
               --  Load MDIO Control Register with AXI
               --  data if there is a write request
               --  and the control register is enabled
               mdio_en_i    <= Bus2IP_Data(3);
               mdio_req_i   <= Bus2IP_Data(0);
            -- Clear the status bit when trnasmit complete
            elsif mdio_done_i = '1' then
               mdio_req_i   <=  '0';
            end if;
         end if;
      end process MDIO_CTRL_REG_WR_PROCESS;

      -------------------------------------------------------------------------
      -- MDIO_ADDR_REG_WR_PROCESS
      -------------------------------------------------------------------------
      -- This process loads data from the AXI when there is a write request and
      -- the MDIO Address register is enabled.
      -------------------------------------------------------------------------
      MDIO_ADDR_REG_WR_PROCESS : process (Clk)
      begin  -- process
         if (Clk'event and Clk = '1') then
            if (Rst = '1') then
               mdio_phy_addr <= (others =>'0');
               mdio_reg_addr <= (others =>'0');
               mdio_op_i     <= '0';
            elsif (Bus2IP_WrCE = '1' and mdio_addr_en= '1') then
               --  Load MDIO ADDR Register with AXI
               --  data if there is a write request
               --  and the Address register is enabled
               mdio_phy_addr <= Bus2IP_Data(9 downto 5);
               mdio_reg_addr <= Bus2IP_Data(4 downto 0);
               mdio_op_i     <= Bus2IP_Data(10);

            end if;
         end if;
      end process MDIO_ADDR_REG_WR_PROCESS;

      -------------------------------------------------------------------------
      -- MDIO_WRITE_REG_WR_PROCESS
      -------------------------------------------------------------------------
      -- This process loads data from the AXI when there is a write request
      -- and the MDIO Write register is enabled.
      -------------------------------------------------------------------------
      MDIO_WRITE_REG_WR_PROCESS : process (Clk)
      begin  -- process
         if (Clk'event and Clk = '1') then
            if (Rst = '1') then
               mdio_wr_data_reg <= (others =>'0');
            elsif (Bus2IP_WrCE = '1' and mdio_wr_data_en= '1') then
               --  Load MDIO Write Data Register with AXI
               --  data if there is a write request
               --  and the Write Data register is enabled
               mdio_wr_data_reg <= Bus2IP_Data(15 downto 0);

            end if;
         end if;
      end process MDIO_WRITE_REG_WR_PROCESS;

      -------------------------------------------------------------------------
      -- MDIO_REG_RD_PROCESS
      -------------------------------------------------------------------------
      -- This process allows MDIO register read from the AXI when there is a
      -- read request and the MDIO registers are enabled.
      -------------------------------------------------------------------------
      MDIO_REG_RD_PROCESS : process (Clk)
      begin  -- process
         if (Clk'event and Clk = '1') then
            if (Rst = '1') then
               mdio_data_out <= (others =>'0');
            elsif (Bus2IP_RdCE = '1' and mdio_addr_en= '1') then
               --  MDIO Address Register Read through AXI
               mdio_data_out(4 downto 0) <= mdio_reg_addr;
               mdio_data_out(9 downto 5) <= mdio_phy_addr;
               mdio_data_out(10)       <= mdio_op_i;
               mdio_data_out(31  downto 11) <= (others=>'0');
            elsif (Bus2IP_RdCE = '1' and mdio_wr_data_en= '1') then
               --  MDIO Write Data Register Read through AXI
               mdio_data_out(15 downto 0) <= mdio_wr_data_reg;
               mdio_data_out(31  downto 16) <= (others=>'0');
            elsif (Bus2IP_RdCE = '1' and mdio_rd_data_en= '1') then
               --  MDIO Read Data Register Read through AXI
               mdio_data_out(15 downto 0) <= mdio_rd_data_reg;
               mdio_data_out(31 downto 16) <= (others=>'0');
            elsif (Bus2IP_RdCE = '1' and mdio_ctrl_en= '1') then
               --  MDIO Control Register Read through AXI
               mdio_data_out(0) <= mdio_req_i;
               mdio_data_out(1) <= '0';
               mdio_data_out(2) <= '0';
               mdio_data_out(3) <= mdio_en_i;
               mdio_data_out(31  downto 4) <= (others=>'0');
            --else
            --   mdio_data_out <= (others =>'0');
            end if;
         end if;
      end process MDIO_REG_RD_PROCESS;

      -------------------------------------------------------------------------
      -- PROCESS : MDIO_CLK_COUNTER
      -------------------------------------------------------------------------
      -- Generating MDIO clock. The minimum period for MDC clock is 400 ns.
      -------------------------------------------------------------------------
      MDIO_CLK_COUNTER : process(Clk)
      begin
         if (Clk'event and Clk = '1') then
            if (Rst = '1' ) then
               clk_cnt  <= MDIO_CNT;
               mdio_clk_i <= '0';
            elsif (clk_cnt = 0) then
               clk_cnt    <= MDIO_CNT;
               mdio_clk_i <= not mdio_clk_i;
            else
               clk_cnt <= clk_cnt - 1;
            end if;
         end if;
      end process;

      -------------------------------------------------------------------------
      -- MDIO master interface module
      -------------------------------------------------------------------------
      MDIO_IF_I: entity axi_ethernetlite_v3_0_18.mdio_if
         port map (
            Clk            => Clk       ,
            Rst            => Rst     ,
            MDIO_CLK       => mdio_clk_i       ,
            MDIO_en        => mdio_en_i        ,
            MDIO_OP        => mdio_op_i        ,
            MDIO_Req       => mdio_req_i       ,
            MDIO_PHY_AD    => mdio_phy_addr    ,
            MDIO_REG_AD    => mdio_reg_addr    ,
            MDIO_WR_DATA   => mdio_wr_data_reg ,
            MDIO_RD_DATA   => mdio_rd_data_reg ,
            PHY_MDIO_I     => PHY_MDIO_I       ,
            PHY_MDIO_O     => PHY_MDIO_O       ,
            PHY_MDIO_T     => PHY_MDIO_T       ,
            PHY_MDC        => PHY_MDC          ,
            MDIO_done      => mdio_done_i
            );
   end generate MDIO_GEN;

   ----------------------------------------------------------------------------
   -- NO_MDIO_GEN :- Include MDIO interface if the parameter C_INCLUDE_MDIO = 0
   ----------------------------------------------------------------------------
   NO_MDIO_GEN: if C_INCLUDE_MDIO = 0 generate
   begin

      mdio_data_out <= (others=>'0');
      mdio_reg_en   <= '0';
      PHY_MDIO_O    <= '0';
      PHY_MDIO_T    <= '1';

   end generate NO_MDIO_GEN;

end imp;






-------------------------------------------------------------------------------
-- axi_interface - entity / architecture pair
-------------------------------------------------------------------------------
--  ***************************************************************************
--  ** DISCLAIMER OF LIABILITY                                               **
--  **                                                                       **
--  **  This file contains proprietary and confidential information of       **
--  **  Xilinx, Inc. ("Xilinx"), that is distributed under a license         **
--  **  from Xilinx, and may be used, copied and/or disclosed only           **
--  **  pursuant to the terms of a valid license agreement with Xilinx.      **
--  **                                                                       **
--  **  XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION                **
--  **  ("MATERIALS") "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER           **
--  **  EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING WITHOUT                  **
--  **  LIMITATION, ANY WARRANTY WITH RESPECT TO NONINFRINGEMENT,            **
--  **  MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE. Xilinx        **
--  **  does not warrant that functions included in the Materials will       **
--  **  meet the requirements of Licensee, or that the operation of the      **
--  **  Materials will be uninterrupted or error-free, or that defects       **
--  **  in the Materials will be corrected. Furthermore, Xilinx does         **
--  **  not warrant or make any representations regarding use, or the        **
--  **  results of the use, of the Materials in terms of correctness,        **
--  **  accuracy, reliability or otherwise.                                  **
--  **                                                                       **
--  **  Xilinx products are not designed or intended to be fail-safe,        **
--  **  or for use in any application requiring fail-safe performance,       **
--  **  such as life-support or safety devices or systems, Class III         **
--  **  medical devices, nuclear facilities, applications related to         **
--  **  the deployment of airbags, or any other applications that could      **
--  **  lead to death, personal injury or severe property or                 **
--  **  environmental damage (individually and collectively, "critical       **
--  **  applications"). Customer assumes the sole risk and liability         **
--  **  of any use of Xilinx products in critical applications,              **
--  **  subject only to applicable laws and regulations governing            **
--  **  limitations on product liability.                                    **
--  **                                                                       **
--  **  Copyright 2010 Xilinx, Inc.                                          **
--  **  All rights reserved.                                                 **
--  **                                                                       **
--  **  This disclaimer and copyright notice must be retained as part        **
--  **  of this file at all times.                                           **
--  ***************************************************************************
-------------------------------------------------------------------------------
-- Filename:        axi_interface.vhd
-- Version:         v1.00a
-- Description:     This module takes care of AXI protocol interface for AXI4 
--                  AXI4-Lite interfaces. This supports word access and INCR 
--                  burst only.
--
-- VHDL-Standard:   VHDL'93
-------------------------------------------------------------------------------
-- Structure:   
--
--  axi_ethernetlite.vhd
--      \
--      \-- axi_interface.vhd
--      \-- xemac.vhd
--           \
--           \-- mdio_if.vhd
--           \-- emac_dpram.vhd                     
--           \    \                     
--           \    \-- RAMB16_S4_S36
--           \                          
--           \    
--           \-- emac.vhd                     
--                \                     
--                \-- MacAddrRAM                   
--                \-- receive.vhd
--                \      rx_statemachine.vhd
--                \      rx_intrfce.vhd
--                \         async_fifo_fg.vhd
--                \      crcgenrx.vhd
--                \                     
--                \-- transmit.vhd
--                       crcgentx.vhd
--                          crcnibshiftreg
--                       tx_intrfce.vhd
--                          async_fifo_fg.vhd
--                       tx_statemachine.vhd
--                       deferral.vhd
--                          cntr5bit.vhd
--                          defer_state.vhd
--                       bocntr.vhd
--                          lfsr16.vhd
--                       msh_cnt.vhd
--                       ld_arith_reg.vhd
--
-------------------------------------------------------------------------------
-- Author:    PVK
-- History:    
-- PVK        06/07/2010     First Version
-- ^^^^^^
-- First version.  
-- ~~~~~~
-------------------------------------------------------------------------------
-- Naming Conventions:
--      active low signals:                     "*_n"
--      clock signals:                          "clk", "clk_div#", "clk_#x" 
--      reset signals:                          "rst", "rst_n" 
--      generics:                               "C_*" 
--      user defined types:                     "*_TYPE" 
--      state machine next state:               "*_ns" 
--      state machine current state:            "*_cs" 
--      combinatorial signals:                  "*_cmb" 
--      pipelined or register delay signals:    "*_d#" 
--      counter signals:                        "*cnt*"
--      clock enable signals:                   "*_ce" 
--      internal version of output port         "*_i"
--      device pins:                            "*_pin" 
--      ports:                                  - Names begin with Uppercase 
--      processes:                              "*_PROCESS" 
--      component instantiations:               "<ENTITY_>I_<#|FUNC>
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use IEEE.std_logic_arith.all;
use IEEE.std_logic_unsigned.all;
use ieee.std_logic_misc.all;

-------------------------------------------------------------------------------
-- Vcomponents from unisim library is used for FIFO instatiation
-- function declarations
-------------------------------------------------------------------------------
library unisim;
use unisim.Vcomponents.all;

-------------------------------------------------------------------------------
-- Definition of Generics :
-------------------------------------------------------------------------------
-- System generics
--    C_FAMILY                   --  Xilinx FPGA Family
--                          
-- AXI generics             
--    C_S_AXI_PROTOCOL           --  AXI protocol type
--    C_S_AXI_CTRL_BASEADDR      --  Base address of the core
--    C_S_AXI_HIGHADDR           --  Permits alias of address space
--                                   by making greater than xFFF
--    C_S_AXI_CTRL_ADDR_WIDTH    --  Width of AXI Address Bus (in bits)
--    C_S_AXI_CTRL_DATA_WIDTH    --  Width of the AXI Data Bus (in bits)
--
-------------------------------------------------------------------------------
-- Definition of Ports :
-------------------------------------------------------------------------------
-- System signals
--    S_AXI_ACLK          --  AXI Clock
--    S_AXI_ARESET        --  AXI Reset
--    IP2INTC_Irpt        --  Device interrupt output to microprocessor 
--                            interrupt input or system interrupt controller.
-- AXI signals                
--    S_AXI_AWADDR        --  AXI Write address
--    S_AXI_AWVALID       --  Write address valid
--    S_AXI_AWREADY       --  Write address ready
--    S_AXI_WDATA         --  Write data
--    S_AXI_WSTRB         --  Write strobes
--    S_AXI_WVALID        --  Write valid
--    S_AXI_WREADY        --  Write ready
--    S_AXI_BRESP         --  Write response
--    S_AXI_BVALID        --  Write response valid
--    S_AXI_BREADY        --  Response ready
--    S_AXI_ARADDR        --  Read address
--    S_AXI_ARVALID       --  Read address valid
--    S_AXI_ARREADY       --  Read address ready
--    S_AXI_RDATA         --  Read data
--    S_AXI_RRESP         --  Read response
--    S_AXI_RVALID        --  Read valid
--    S_AXI_RREADY        --  Read ready
--
-- IPIC signals   
--    IP2Bus_Data        -- IP  to Bus data 
--    IP2Bus_Error       -- IP  to Bus error
--    Bus2IP_Addr        -- Bus to IP address
--    Bus2IP_Data        -- Bus to IP data
--    Bus2IP_RdCE        -- Bus to IP read chip enable
--    Bus2IP_WrCE        -- Bus to IP write chip enable
--    Bus2IP_BE          -- Bus to IP byte enables
--    Bus2IP_Burst       -- Bus to IP burst indicator
-------------------------------------------------------------------------------


-------------------------------------------------------------------------------
-- Entity section
-------------------------------------------------------------------------------

entity axi_interface is

  generic (

--  -- System Parameter
    C_FAMILY              : string                    := "virtex6";
--  -- AXI Parameters
    C_S_AXI_PROTOCOL      : string                    := "AXI4";
    C_S_AXI_ID_WIDTH      : integer range 1 to 32     := 4; 
    C_S_AXI_ADDR_WIDTH    : integer                   := 13;
    C_S_AXI_DATA_WIDTH    : integer range 32 to 128   := 32
   );
 
  port (
--   -- AXI Global System Signals
       S_AXI_ACLK    : in  std_logic;
       S_AXI_ARESETN : in  std_logic;
--   -- AXI Write Address Channel Signals
       S_AXI_AWID    : in  std_logic_vector((C_S_AXI_ID_WIDTH-1) downto 0);
       S_AXI_AWADDR  : in  std_logic_vector((C_S_AXI_ADDR_WIDTH-1) downto 0);
       S_AXI_AWLEN   : in  std_logic_vector(7 downto 0);
       S_AXI_AWSIZE  : in  std_logic_vector(2 downto 0);
       S_AXI_AWBURST : in  std_logic_vector(1 downto 0);
       S_AXI_AWCACHE : in  std_logic_vector(3 downto 0);
       S_AXI_AWVALID : in  std_logic;
       S_AXI_AWREADY : out std_logic;
--   -- AXI Write Channel Signals
       S_AXI_WDATA   : in  std_logic_vector((C_S_AXI_DATA_WIDTH-1) downto 0);
       S_AXI_WSTRB   : in  std_logic_vector
                               (((C_S_AXI_DATA_WIDTH/8)-1) downto 0);
       S_AXI_WLAST   : in  std_logic;
       S_AXI_WVALID  : in  std_logic;
       S_AXI_WREADY  : out std_logic;

--   -- AXI Write Response Channel Signals
       S_AXI_BID     : out std_logic_vector((C_S_AXI_ID_WIDTH-1) downto 0);
       S_AXI_BRESP   : out std_logic_vector(1 downto 0);
       S_AXI_BVALID  : out std_logic;
       S_AXI_BREADY  : in  std_logic;
--   -- AXI Read Address Channel Signals
       S_AXI_ARID    : in  std_logic_vector((C_S_AXI_ID_WIDTH-1) downto 0);
       S_AXI_ARADDR  : in  std_logic_vector((C_S_AXI_ADDR_WIDTH-1) downto 0);
       S_AXI_ARLEN   : in  std_logic_vector(7 downto 0);
       S_AXI_ARSIZE  : in  std_logic_vector(2 downto 0);
       S_AXI_ARBURST : in  std_logic_vector(1 downto 0);
       S_AXI_ARCACHE : in  std_logic_vector(3 downto 0);
       S_AXI_ARVALID : in  std_logic;
       S_AXI_ARREADY : out std_logic;
--   -- AXI Read Data Channel Signals
       S_AXI_RID     : out std_logic_vector((C_S_AXI_ID_WIDTH-1) downto 0);
       S_AXI_RDATA   : out std_logic_vector((C_S_AXI_DATA_WIDTH-1) downto 0);
       S_AXI_RRESP   : out std_logic_vector(1 downto 0);
       S_AXI_RLAST   : out std_logic;
       S_AXI_RVALID  : out std_logic;
       S_AXI_RREADY  : in  std_logic;

        
      -- Controls to the IP/IPIF modules
       IP2Bus_Data   : in  std_logic_vector((C_S_AXI_DATA_WIDTH-1) downto 0 );
      -- IP2Bus_Error  : in  std_logic;

       Bus2IP_Addr   : out std_logic_vector((C_S_AXI_ADDR_WIDTH-1) downto 0);
       Bus2IP_Data   : out std_logic_vector((C_S_AXI_DATA_WIDTH-1) downto 0);
       Bus2IP_BE     : out std_logic_vector(((C_S_AXI_DATA_WIDTH/8)-1)downto 0);
       Bus2IP_Burst  : out std_logic;
       Bus2IP_RdCE   : out std_logic;
       Bus2IP_WrCE   : out std_logic
);

end entity axi_interface;

-------------------------------------------------------------------------------
-- Architecture section
-------------------------------------------------------------------------------
architecture rtl of axi_interface is



  -----------------------------------------------------------------------------
    -- Constant Declarations
  -----------------------------------------------------------------------------
  constant  ZEROES     : std_logic_vector := X"00000000";
  constant RST_ACTIVE  : std_logic := '0';
  -----------------------------------------------------------------------------
    -- Signal and Type Declarations
  -----------------------------------------------------------------------------
  signal bus2ip_addr_i   : std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
  signal wid             : std_logic_vector(C_S_AXI_ID_WIDTH-1 downto 0); 
  signal rid             : std_logic_vector(C_S_AXI_ID_WIDTH-1 downto 0); 
  signal read_burst_cntr : std_logic_vector(7 downto 0); 
  signal bvalid          : std_logic;
  signal rvalid          : std_logic;
  signal read_req           : std_logic;
  signal write_req          : std_logic;
  signal awready_i          : std_logic;
  signal arready_i          : std_logic;
  signal arready_i1          : std_logic;
  signal arready_i2          : std_logic;
  signal s_axi_rlast_i      : std_logic;
  signal read_burst_length  : std_logic_vector(7 downto 0);
  signal rd_burst           : std_logic;
  signal rd_last            : std_logic;
  signal read_req_d1        : std_logic;
  signal read_req_re        : std_logic;
  signal bus2ip_rdce_i      : std_logic;
  signal bus2ip_rdce_i_d1   : std_logic;

  signal IP2Bus_Data_sampled: std_logic_vector((C_S_AXI_DATA_WIDTH-1) downto 0 );

  signal read_in_prog, write_in_prog : std_logic;
  signal read_complete, write_complete : std_logic;


  -----------------------------------------------------------------------------
    -- Begin Architecture
  -----------------------------------------------------------------------------
    
  begin

  -- AXI signal assignment
  S_AXI_BRESP   <= "00";
  S_AXI_BVALID  <= bvalid;
--  S_AXI_RDATA   <= IP2Bus_Data;
  --S_AXI_RVALID  <= rvalid;
  S_AXI_RRESP   <= "00";
  
  -- IPIC signal assignment          
  Bus2IP_Addr   <= bus2ip_addr_i;
  Bus2IP_Data   <= S_AXI_WDATA;
  Bus2IP_RdCE   <= bus2ip_rdce_i;
  Bus2IP_BE     <= S_AXI_WSTRB;
  Bus2IP_Burst  <= '0';
  
  AXI4_RDATA_GEN : if (C_S_AXI_PROTOCOL = "AXI4") generate
      AXI_READ_OUTPUT_P: process (S_AXI_ACLK) is
      begin
          if (S_AXI_ACLK'event and S_AXI_ACLK = '1') then
              if (S_AXI_ARESETN=RST_ACTIVE) then
                  S_AXI_RDATA  <= (others =>'0');
              elsif S_AXI_RREADY = '1' then
                  S_AXI_RDATA   <= IP2Bus_Data;
              end if;
          end if;
      end process AXI_READ_OUTPUT_P;

  
  end generate AXI4_RDATA_GEN;

  AXI4LITE_RDATA_GEN : if (C_S_AXI_PROTOCOL = "AXI4LITE") generate
     S_AXI_RDATA   <= IP2Bus_Data_sampled;
  
  end generate AXI4LITE_RDATA_GEN;




  -- AWREADY is enabled only if valid write request and no read request 
  --awready_i <= (not write_req) and not (S_AXI_ARVALID or read_req or rvalid);
  
  -- ARREADY is enabled only if valid read request and no current write request 
  --arready_i <= (not read_req) and not (write_req);
  




  -----------------------------------------------------------------------------
  --  Generate AXI4-MM interface if (C_S_AXI_PROTOCOL="AXI4")
  -----------------------------------------------------------------------------
  AXI4_MM_IF_GEN : if (C_S_AXI_PROTOCOL = "AXI4") generate

  S_AXI_AWREADY <= awready_i;
  S_AXI_WREADY  <= write_req;
  S_AXI_ARREADY <= arready_i;
  Bus2IP_WrCE   <= S_AXI_WVALID and write_req;

  --  -----------------------------------------------------------------------
  --  Process AXI_AWREADY_P to generate Write request on the IPIC
  --  -----------------------------------------------------------------------
      AXI_AWREADY_P: process (S_AXI_ACLK) is
      begin
          if (S_AXI_ACLK'event and S_AXI_ACLK = '1') then
              if (S_AXI_ARESETN=RST_ACTIVE) then
                  awready_i <='0';
              elsif (S_AXI_AWVALID = '1' and awready_i = '1') then
                  awready_i  <= '0';
              else
                  awready_i  <= (not write_req) and not (S_AXI_ARVALID or read_req or rvalid);
              end if;
          end if;
      end process AXI_AWREADY_P;

  --  -----------------------------------------------------------------------
  --  Process AXI_ARREADY_P to generate Write request on the IPIC
  --  -----------------------------------------------------------------------
      AXI_ARREADY_P: process (S_AXI_ACLK) is
      begin
          if (S_AXI_ACLK'event and S_AXI_ACLK = '1') then
              if (S_AXI_ARESETN=RST_ACTIVE) then
                  arready_i <='0';
              elsif (S_AXI_ARVALID = '1' and arready_i = '1') then
                  arready_i  <= '0';
              else
                  arready_i <= (not read_req) and not (S_AXI_AWVALID or write_req);
              end if;
          end if;
      end process AXI_ARREADY_P;

  --  -----------------------------------------------------------------------
  --  Process AXI_READ_OUTPUT_P to generate Write request on the IPIC
  --  -----------------------------------------------------------------------
      --AXI_READ_OUTPUT_P: process (S_AXI_ACLK) is
      --begin
      --    if (S_AXI_ACLK'event and S_AXI_ACLK = '1') then
      --        if (S_AXI_ARESETN=RST_ACTIVE) then
      --            S_AXI_RDATA  <= (others =>'0');
      --            S_AXI_RVALID <='0';
      --        elsif S_AXI_RREADY = '1' then
      --            S_AXI_RDATA   <= IP2Bus_Data;
      --            S_AXI_RVALID  <= rvalid;
      --        end if;
      --    end if;
      --end process AXI_READ_OUTPUT_P;


      AXI_READ_VALID_P: process (S_AXI_ACLK) is
      begin
          if (S_AXI_ACLK'event and S_AXI_ACLK = '1') then
              if (S_AXI_ARESETN=RST_ACTIVE) then
                  S_AXI_RVALID <='0';
              elsif S_AXI_RREADY = '1' then
                  S_AXI_RVALID  <= rvalid;
              end if;
          end if;
      end process AXI_READ_VALID_P;

      AXI_READ_CE_DELAY_P: process (S_AXI_ACLK) is
      begin
          if (S_AXI_ACLK'event and S_AXI_ACLK = '1') then
              if (S_AXI_ARESETN=RST_ACTIVE) then
                  bus2ip_rdce_i_d1  <= '0';
              else 
                  bus2ip_rdce_i_d1  <= bus2ip_rdce_i;
              end if;
          end if;
      end process AXI_READ_CE_DELAY_P;

      AXI_READ_OUTPUT_P: process (S_AXI_ACLK) is
      begin
          if (S_AXI_ACLK'event and S_AXI_ACLK = '1') then
              if (S_AXI_ARESETN=RST_ACTIVE) then
                  IP2Bus_Data_sampled  <= (others =>'0');
              elsif bus2ip_rdce_i_d1 = '1' then
                  IP2Bus_Data_sampled  <= IP2Bus_Data;
              end if;
          end if;
      end process AXI_READ_OUTPUT_P;

     -- AXI4 outputs
     S_AXI_BID     <= wid; 
     S_AXI_RID     <= rid;
     --S_AXI_RLAST   <= s_axi_rlast_i and rvalid; 
     
     -- Read burst
     rd_burst <=  or_reduce(read_burst_length);
     rd_last <= (s_axi_rlast_i and rd_burst) or (s_axi_rlast_i and S_AXI_RREADY);
     s_axi_rlast_i <= '1' when read_burst_cntr = "00000000" else '0';  
     
     -- Read request on IPIC
     bus2ip_rdce_i <= read_req_re  or (read_req and S_AXI_RREADY);

     -- AXI/IPIC Read request signal generation
     read_req_re <= read_req and not read_req_d1;



  --  -----------------------------------------------------------------------
  --  Process AXI_READ_OUTPUT_P to generate Write request on the IPIC
  --  -----------------------------------------------------------------------
      AXI_READ_LAST_OUTPUT_P: process (S_AXI_ACLK) is
      begin
          if (S_AXI_ACLK'event and S_AXI_ACLK = '1') then
              if (S_AXI_ARESETN=RST_ACTIVE) then
                  S_AXI_RLAST  <= '0';
              elsif S_AXI_RREADY = '1' then
                  S_AXI_RLAST   <= s_axi_rlast_i and rvalid;
              end if;
          end if;
      end process AXI_READ_LAST_OUTPUT_P;



  --  -----------------------------------------------------------------------
  --  Process WRITE_REQUEST_P to generate Write request on the IPIC
  --  -----------------------------------------------------------------------
      WRITE_REQUEST_P: process (S_AXI_ACLK) is
      begin
          if (S_AXI_ACLK'event and S_AXI_ACLK = '1') then
              if (S_AXI_ARESETN=RST_ACTIVE) then
                  write_req <= '0';
              elsif (S_AXI_AWVALID = '1' and awready_i = '1') then   
                  write_req <= '1';
              elsif (S_AXI_WVALID = '1' and S_AXI_WLAST = '1') then
                  write_req <= '0';
              else
                  write_req <= write_req;
              end if;
          end if;
      end process WRITE_REQUEST_P;


  --  -----------------------------------------------------------------------
  --  Process READ_REQUEST_P to generate read request
  --  -----------------------------------------------------------------------
      READ_REQUEST_P: process (S_AXI_ACLK) is
      begin
          if (S_AXI_ACLK'event and S_AXI_ACLK = '1') then
              if (S_AXI_ARESETN=RST_ACTIVE) then
                  read_req <= '0';
              elsif (S_AXI_ARVALID = '1' and arready_i = '1') then   
                  read_req <= '1';
              elsif (s_axi_rlast_i = '1') then 
                  read_req <= '0';
              end if;
          end if;
      end process READ_REQUEST_P;

  --  -----------------------------------------------------------------------
  --  Process ADDR_GEN_P to generate bus2ip_addr for read/write
  --  -----------------------------------------------------------------------
      ADDR_GEN_P: process (S_AXI_ACLK) is
      begin
          if (S_AXI_ACLK'event and S_AXI_ACLK = '1') then
              if (S_AXI_ARESETN=RST_ACTIVE) then
                  bus2ip_addr_i <= (others=>'0');
              elsif (S_AXI_ARVALID = '1' and arready_i = '1') then   
                  bus2ip_addr_i <= S_AXI_ARADDR;
              elsif (bus2ip_rdce_i='1' and rd_burst='1') then
                  bus2ip_addr_i <= bus2ip_addr_i + 4;
              elsif (S_AXI_AWVALID = '1' and awready_i = '1') then   
                  bus2ip_addr_i <= S_AXI_AWADDR;
              elsif (S_AXI_WVALID = '1' and write_req='1') then
                  bus2ip_addr_i <= bus2ip_addr_i + 4;
              end if;
          end if;
      end process ADDR_GEN_P;
  
  --  -----------------------------------------------------------------------
  --  Process WRITE_ID_P to generate Write response ID on AXI
  --  -----------------------------------------------------------------------
      WRITE_ID_P: process (S_AXI_ACLK) is
      begin
          if (S_AXI_ACLK'event and S_AXI_ACLK = '1') then
              if (S_AXI_ARESETN=RST_ACTIVE) then
                  wid <= (others=>'0');
              elsif (S_AXI_AWVALID = '1' and awready_i = '1') then   
                  wid <= S_AXI_AWID;
              end if;
          end if;
      end process WRITE_ID_P;

  --  -----------------------------------------------------------------------
  --  Process WRITE_BVALID_P to generate Write Response valid
  --  -----------------------------------------------------------------------
      WRITE_BVALID_P: process (S_AXI_ACLK) is
      begin
          if (S_AXI_ACLK'event and S_AXI_ACLK = '1') then
              if (S_AXI_ARESETN=RST_ACTIVE) then
                  bvalid <= '0';
              elsif (S_AXI_WLAST = '1' and write_req = '1' and S_AXI_WVALID = '1') then   
                  bvalid <= '1';
              elsif (S_AXI_BREADY = '1') then
                  bvalid <= '0';
              end if;
          end if;
      end process WRITE_BVALID_P;


  --  -----------------------------------------------------------------------
  --  Process READ_ID_P to generate read ID
  --  -----------------------------------------------------------------------
      READ_ID_P: process (S_AXI_ACLK) is
      begin
          if (S_AXI_ACLK'event and S_AXI_ACLK = '1') then
              if (S_AXI_ARESETN=RST_ACTIVE) then
                  rid <= (others=>'0');
              elsif (S_AXI_ARVALID = '1' and arready_i = '1') then   
                  rid <= S_AXI_ARID;
              end if;
          end if;
      end process READ_ID_P;

  --  -----------------------------------------------------------------------
  --  Process READ_BURST_CNTR_P to generate rdlast signal after completion
  --  of burst
  --  -----------------------------------------------------------------------
      READ_BURST_CNTR_P: process (S_AXI_ACLK) is
      begin
          if (S_AXI_ACLK'event and S_AXI_ACLK = '1') then
              if (S_AXI_ARESETN=RST_ACTIVE) then
                  read_burst_cntr <= (others=>'0');
              elsif (S_AXI_ARVALID = '1' and arready_i = '1') then   
                  read_burst_cntr <= S_AXI_ARLEN;
              elsif (rvalid = '1' and S_AXI_RREADY='1') then
                  read_burst_cntr <= read_burst_cntr-'1';
              end if;
          end if;
      end process READ_BURST_CNTR_P;

  --  -----------------------------------------------------------------------
  --  Process READ_BURST_LENGTH_P to latch the burst length for read xfer
  --  -----------------------------------------------------------------------
      READ_BURST_LENGTH_P: process (S_AXI_ACLK) is
      begin
          if (S_AXI_ACLK'event and S_AXI_ACLK = '1') then
              if (S_AXI_ARESETN=RST_ACTIVE) then
                  read_burst_length <= (others=>'0');
              elsif (S_AXI_ARVALID = '1' and arready_i = '1') then   
                  read_burst_length <= S_AXI_ARLEN;
              end if;
          end if;
      end process READ_BURST_LENGTH_P;


  --  -----------------------------------------------------------------------
  --  Process READ_RVALID_P to generate Read valid
  --  -----------------------------------------------------------------------
      READ_RVALID_P: process (S_AXI_ACLK) is
      begin
          if (S_AXI_ACLK'event and S_AXI_ACLK = '1') then
              if (S_AXI_ARESETN=RST_ACTIVE) then
                  rvalid <= '0';
              elsif (s_axi_rlast_i = '1' and S_AXI_RREADY='1' and rd_burst='1') then
                  rvalid <= '0';
              elsif (read_req = '1') then   
                  rvalid <= '1';
              elsif (s_axi_rlast_i = '1' and S_AXI_RREADY='1') then
                  rvalid <= '0';
              end if;
          end if;
      end process READ_RVALID_P;


  --  -----------------------------------------------------------------------
  --  Process READ_REQUEST_REG_P to generate Read request on IPIC
  --  -----------------------------------------------------------------------
      READ_REQUEST_REG_P: process (S_AXI_ACLK) is
      begin
          if (S_AXI_ACLK'event and S_AXI_ACLK = '1') then
              if (S_AXI_ARESETN=RST_ACTIVE) then
                  read_req_d1 <= '0';
              else
                  read_req_d1 <= read_req;
              end if;
          end if;
      end process READ_REQUEST_REG_P;

  

  --------------------------
  end generate AXI4_MM_IF_GEN;
  ---------------------------




  -----------------------------------------------------------------------------
  --  Generate AXI4-Lite interface if (C_S_AXI_PROTOCOL="AXI4LITE")
  -----------------------------------------------------------------------------
  AXI4_LITE_IF_GEN : if (C_S_AXI_PROTOCOL = "AXI4LITE") generate

  S_AXI_AWREADY <= awready_i;
  S_AXI_WREADY  <= awready_i;
  S_AXI_ARREADY <= arready_i;
  Bus2IP_WrCE   <= S_AXI_WVALID and write_in_prog; --and write_req;

  --  -----------------------------------------------------------------------
  --  Process AXI_AWREADY_P to generate Write request on the IPIC
  --  -----------------------------------------------------------------------
      AXI_AWREADY_P1: process (S_AXI_ACLK) is
      begin
          if (S_AXI_ACLK'event and S_AXI_ACLK = '1') then
              if (S_AXI_ARESETN=RST_ACTIVE) then
                  write_in_prog <='0';
                  read_in_prog <='0';
              elsif ((rvalid = '1' and S_AXI_RREADY = '1') or (bvalid = '1' and S_AXI_BREADY = '1')) then -- and write_complete = '1') then
          --    elsif (read_complete = '1' or (bvalid = '1' and S_AXI_BREADY = '1')) then -- and write_complete = '1') then
                  write_in_prog <='0';
                  read_in_prog <='0';
              elsif (S_AXI_ARVALID = '1' and write_in_prog = '0') then
                  read_in_prog <='1';
              elsif ((S_AXI_AWVALID = '1' and S_AXI_WVALID = '1') and read_in_prog = '0') then
                  write_in_prog <='1';
              end if;
          end if;
      end process AXI_AWREADY_P1;
  --  -----------------------------------------------------------------------
  --  Process AXI_AWREADY_P to generate Write request on the IPIC
  --  -----------------------------------------------------------------------
      AXI_AWREADY_P2: process (S_AXI_ACLK) is
      begin
          if (S_AXI_ACLK'event and S_AXI_ACLK = '1') then
              if (S_AXI_ARESETN=RST_ACTIVE) then
                  awready_i <='0';
              elsif (S_AXI_WVALID = '1' and write_in_prog = '1' and awready_i = '0') then
                  awready_i  <= '1';
              else
                  awready_i  <= '0'; --(not write_req) and not (S_AXI_ARVALID or read_req or rvalid);
              end if;
          end if;
      end process AXI_AWREADY_P2;


  --  -----------------------------------------------------------------------
  --  Process WRITE_BVALID_P to generate Write Response valid
  --  -----------------------------------------------------------------------
      WRITE_BVALID_P: process (S_AXI_ACLK) is
      begin
          if (S_AXI_ACLK'event and S_AXI_ACLK = '1') then
              if (S_AXI_ARESETN=RST_ACTIVE) then
                  bvalid <= '0';
              elsif (awready_i = '1') then   
                  bvalid <= '1';
              elsif (S_AXI_BREADY = '1') then
                  bvalid <= '0';
              end if;
          end if;
      end process WRITE_BVALID_P;
      
      WRITE_BVALID_P2: process (S_AXI_ACLK) is
      begin
          if (S_AXI_ACLK'event and S_AXI_ACLK = '1') then
              if (S_AXI_ARESETN=RST_ACTIVE) then
                  write_complete <= '0';
              elsif (bvalid = '1' and S_AXI_BREADY = '1' and write_complete = '0') then   
                  write_complete <= '1';
              else
                  write_complete <= '0';
              end if;
          end if;
      end process WRITE_BVALID_P2;


  --  -----------------------------------------------------------------------
  --  Process AXI_ARREADY_P to generate Write request on the IPIC
  --  -----------------------------------------------------------------------
      AXI_ARREADY_P1: process (S_AXI_ACLK) is
      begin
          if (S_AXI_ACLK'event and S_AXI_ACLK = '1') then
              if (S_AXI_ARESETN=RST_ACTIVE) then
                  arready_i1 <='0';
              elsif (read_in_prog = '1') then -- and rvalid = '1') then --S_AXI_ARVALID = '1' and read_complete = '1' and arready_i = '0') then
                  arready_i1  <= '1';
              else
                  arready_i1 <= '0';
              end if;
          end if;
      end process AXI_ARREADY_P1;

      AXI_ARREADY_P2: process (S_AXI_ACLK) is
      begin
          if (S_AXI_ACLK'event and S_AXI_ACLK = '1') then
              if (S_AXI_ARESETN=RST_ACTIVE) then
                  arready_i2 <='0';
              else
                  arready_i2 <= arready_i1;
              end if;
          end if;
      end process AXI_ARREADY_P2;

           arready_i <= arready_i1 and (not arready_i2);


  --    AXI_READ_VALID_P1: process (S_AXI_ACLK) is
  --    begin
  --        if (S_AXI_ACLK'event and S_AXI_ACLK = '1') then
  --            if (S_AXI_ARESETN=RST_ACTIVE) then
  --                S_AXI_RVALID <='0';
  --            elsif S_AXI_RREADY = '1' then
                  S_AXI_RVALID  <= rvalid;
  --            end if;
  --        end if;
  --    end process AXI_READ_VALID_P1;

      AXI_READ_CE_DELAY_P1: process (S_AXI_ACLK) is
      begin
          if (S_AXI_ACLK'event and S_AXI_ACLK = '1') then
              if (S_AXI_ARESETN=RST_ACTIVE) then
                  bus2ip_rdce_i_d1  <= '0';
              else 
                  bus2ip_rdce_i_d1  <= bus2ip_rdce_i;
              end if;
          end if;
      end process AXI_READ_CE_DELAY_P1;

      AXI_READ_OUTPUT_P1: process (S_AXI_ACLK) is
      begin
          if (S_AXI_ACLK'event and S_AXI_ACLK = '1') then
              if (S_AXI_ARESETN=RST_ACTIVE) then
                  IP2Bus_Data_sampled  <= (others =>'0');
              elsif bus2ip_rdce_i_d1 = '1' then
                  IP2Bus_Data_sampled  <= IP2Bus_Data;
              end if;
          end if;
      end process AXI_READ_OUTPUT_P1;

  
     -- AXI4 outputs
     --S_AXI_BID   <= (others => '0');
     --S_AXI_RID   <= (others => '0');
     S_AXI_RLAST <= rvalid; 
     S_AXI_BID     <= wid; 
     S_AXI_RID     <= rid;
     wid   <= (others => '0');
     rid   <= (others => '0');


  --  -----------------------------------------------------------------------
  --  Process WRITE_REQUEST_P to generate Write request on the IPIC
  --  -----------------------------------------------------------------------
--      WRITE_REQUEST_P: process (S_AXI_ACLK) is
--      begin
--          if (S_AXI_ACLK'event and S_AXI_ACLK = '1') then
--              if (S_AXI_ARESETN=RST_ACTIVE) then
--                  write_req <= '0';
--              elsif (S_AXI_AWVALID = '1' and awready_i = '1') then   
--                  write_req <= '1';
--              elsif (write_req = '1' and S_AXI_WVALID = '1') then
--                  write_req <= '0';
--	      else
--	          write_req <= write_req;
--              end if;
--          end if;
--      end process WRITE_REQUEST_P;


  --  -----------------------------------------------------------------------
  --  Process READ_REQUEST_P to generate read request
  --  -----------------------------------------------------------------------
      READ_REQUEST_P: process (S_AXI_ACLK) is
      begin
          if (S_AXI_ACLK'event and S_AXI_ACLK = '1') then
              if (S_AXI_ARESETN=RST_ACTIVE) then -- or read_in_prog = '0') then
                  read_req <= '0';
              elsif (S_AXI_ARVALID = '1' and bus2ip_rdce_i_d1 = '0') then   
                  read_req <= '1';
              elsif (S_AXI_RREADY = '1') then 
                  read_req <= '0';
              end if;
          end if;
      end process READ_REQUEST_P;

  --  -----------------------------------------------------------------------
  --  Process ADDR_GEN_P to generate bus2ip_addr for read/write
  --  -----------------------------------------------------------------------
      ADDR_GEN_P: process (S_AXI_ACLK) is
      begin
          if (S_AXI_ACLK'event and S_AXI_ACLK = '1') then
              if (S_AXI_ARESETN=RST_ACTIVE) then
                  bus2ip_addr_i <= (others=>'0');
              elsif (S_AXI_ARVALID = '1' and write_in_prog = '0') then --read_in_prog = '1') then   
                  bus2ip_addr_i <= S_AXI_ARADDR;
              elsif (S_AXI_AWVALID = '1' and read_in_prog = '0') then --write_in_prog = '1') then   
                  bus2ip_addr_i <= S_AXI_AWADDR;
              end if;
          end if;
      end process ADDR_GEN_P;
  

  --  -----------------------------------------------------------------------
  --  Process READ_RVALID_P to generate Read valid
  --  -----------------------------------------------------------------------
      READ_RVALID_P: process (S_AXI_ACLK) is
      begin
          if (S_AXI_ACLK'event and S_AXI_ACLK = '1') then
              if (S_AXI_ARESETN=RST_ACTIVE) then
                  rvalid <= '0';
              elsif (S_AXI_RREADY='1' and rvalid = '1') then
                  rvalid <= '0';
              elsif (read_req = '1' and bus2ip_rdce_i_d1 = '1') then   
                  rvalid <= '1';
              end if;
          end if;
      end process READ_RVALID_P;

      READ_RVALID_P1: process (S_AXI_ACLK) is
      begin
          if (S_AXI_ACLK'event and S_AXI_ACLK = '1') then
              if (S_AXI_ARESETN=RST_ACTIVE) then
                  read_complete <= '0';
              elsif ((rvalid = '1' and S_AXI_RREADY = '1')) then --(arready_i = '1') then
                  read_complete <= '1';
              else
                  read_complete <= '0';
                  
              end if;
          end if;
      end process READ_RVALID_P1;


 -- --  -----------------------------------------------------------------------
 -- --  Process WRITE_ID_P to generate Write response ID on AXI
 -- --  -----------------------------------------------------------------------
 --     WRITE_ID_P: process (S_AXI_ACLK) is
 --     begin
 --         if (S_AXI_ACLK'event and S_AXI_ACLK = '1') then
 --             if (S_AXI_ARESETN=RST_ACTIVE) then
 --                 wid <= (others=>'0');
 --             elsif (S_AXI_AWVALID = '1' and awready_i = '1') then   
 --                 wid <= S_AXI_AWID;
 --             end if;
 --         end if;
 --     end process WRITE_ID_P;
 --
 -- --  Process READ_ID_P to generate read ID
 -- --  -----------------------------------------------------------------------
 --     READ_ID_P: process (S_AXI_ACLK) is
 --     begin
 --         if (S_AXI_ACLK'event and S_AXI_ACLK = '1') then
 --             if (S_AXI_ARESETN=RST_ACTIVE) then
 --                 rid <= (others=>'0');
 --             elsif (S_AXI_ARVALID = '1' and arready_i = '1') then   
 --                 rid <= S_AXI_ARID;
 --             end if;
 --         end if;
 --     end process READ_ID_P;
      
      
      -- Read request on IPIC
      bus2ip_rdce_i <= read_in_prog; --read_req;
      

  --------------------------
  end generate AXI4_LITE_IF_GEN;
  ---------------------------

 

end architecture rtl;
--


-------------------------------------------------------------------------------
-- axi_ethernetlite - entity/architecture pair
-------------------------------------------------------------------------------
--  ***************************************************************************
--  ** DISCLAIMER OF LIABILITY                                               **
--  **                                                                       **
--  **  This file contains proprietary and confidential information of       **
--  **  Xilinx, Inc. ("Xilinx"), that is distributed under a license         **
--  **  from Xilinx, and may be used, copied and/or disclosed only           **
--  **  pursuant to the terms of a valid license agreement with Xilinx.      **
--  **                                                                       **
--  **  XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION                **
--  **  ("MATERIALS") "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER           **
--  **  EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING WITHOUT                  **
--  **  LIMITATION, ANY WARRANTY WITH RESPECT TO NONINFRINGEMENT,            **
--  **  MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE. Xilinx        **
--  **  does not warrant that functions included in the Materials will       **
--  **  meet the requirements of Licensee, or that the operation of the      **
--  **  Materials will be uninterrupted or error-free, or that defects       **
--  **  in the Materials will be corrected. Furthermore, Xilinx does         **
--  **  not warrant or make any representations regarding use, or the        **
--  **  results of the use, of the Materials in terms of correctness,        **
--  **  accuracy, reliability or otherwise.                                  **
--  **                                                                       **
--  **  Xilinx products are not designed or intended to be fail-safe,        **
--  **  or for use in any application requiring fail-safe performance,       **
--  **  such as life-support or safety devices or systems, Class III         **
--  **  medical devices, nuclear facilities, applications related to         **
--  **  the deployment of airbags, or any other applications that could      **
--  **  lead to death, personal injury or severe property or                 **
--  **  environmental damage (individually and collectively, "critical       **
--  **  applications"). Customer assumes the sole risk and liability         **
--  **  of any use of Xilinx products in critical applications,              **
--  **  subject only to applicable laws and regulations governing            **
--  **  limitations on product liability.                                    **
--  **                                                                       **
--  **  Copyright 2010 Xilinx, Inc.                                          **
--  **  All rights reserved.                                                 **
--  **                                                                       **
--  **  This disclaimer and copyright notice must be retained as part        **
--  **  of this file at all times.                                           **
--  ***************************************************************************
-------------------------------------------------------------------------------
-- Filename     : axi_ethernetlite.vhd
-- Version      : v2.0
-- Description  : This is the top level wrapper file for the Ethernet
--                Lite function It provides a 10 or 100 Mbs full or half 
--                duplex Ethernet bus with an interface to an AXI Interface.               
-- VHDL-Standard: VHDL'93
-------------------------------------------------------------------------------
-- Structure:   
--
--  axi_ethernetlite.vhd
--      \
--      \-- axi_interface.vhd
--      \-- xemac.vhd
--           \
--           \-- mdio_if.vhd
--           \-- emac_dpram.vhd                     
--           \    \                     
--           \    \-- RAMB16_S4_S36
--           \                          
--           \    
--           \-- emac.vhd                     
--                \                     
--                \-- MacAddrRAM                   
--                \-- receive.vhd
--                \      rx_statemachine.vhd
--                \      rx_intrfce.vhd
--                \         async_fifo_fg.vhd
--                \      crcgenrx.vhd
--                \                     
--                \-- transmit.vhd
--                       crcgentx.vhd
--                          crcnibshiftreg
--                       tx_intrfce.vhd
--                          async_fifo_fg.vhd
--                       tx_statemachine.vhd
--                       deferral.vhd
--                          cntr5bit.vhd
--                          defer_state.vhd
--                       bocntr.vhd
--                          lfsr16.vhd
--                       msh_cnt.vhd
--                       ld_arith_reg.vhd
--
-------------------------------------------------------------------------------
-- Author:    PVK
-- History:    
-- PVK        06/07/2010     First Version
-- ^^^^^^
-- First version.  
-- ~~~~~~
-- PVK        07/29/2010     First Version
-- ^^^^^^
-- Removed ARLOCK and AWLOCK, AWPROT, ARPROT signals from the list.
-- ~~~~~~
-------------------------------------------------------------------------------
-- Naming Conventions:
--      active low signals:                     "*_n"
--      clock signals:                          "clk", "clk_div#", "clk_#x" 
--      reset signals:                          "rst", "rst_n" 
--      generics:                                "C_*" 
--      user defined types:                     "*_TYPE" 
--      state machine next state:               "*_ns" 
--      state machine current state:            "*_cs" 
--      combinatorial signals:                  "*_com" 
--      pipelined or register delay signals:    "*_d#" 
--      counter signals:                        "*cnt*"
--      clock enable signals:                   "*_ce" 
--      internal version of output port         "*_i"
--      device pins:                            "*_pin" 
--      ports:                                  - Names begin with Uppercase 
--      processes:                              "*_PROCESS" 
--      component instantiations:               "<ENTITY_>I_<#|FUNC>
------------------------------------------------------------------------------- 
-------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;

-------------------------------------------------------------------------------
-- axi_ethernetlite_v3_0_18 library is used for axi_ethernetlite_v3_0_18 
-- component declarations
-------------------------------------------------------------------------------
library axi_ethernetlite_v3_0_18;
use axi_ethernetlite_v3_0_18.mac_pkg.all;
use axi_ethernetlite_v3_0_18.axi_interface;
use axi_ethernetlite_v3_0_18.all;

-------------------------------------------------------------------------------
library lib_cdc_v1_0_2;
use lib_cdc_v1_0_2.all;

-------------------------------------------------------------------------------
-- Vcomponents from unisim library is used for FIFO instatiation
-- function declarations
-------------------------------------------------------------------------------
library unisim;
use unisim.Vcomponents.all;

-------------------------------------------------------------------------------
-- Definition of Generics:
-------------------------------------------------------------------------------
-- 
-- C_FAMILY                    -- Target device family 
-- C_SELECT_XPM                -- Selects XPM if set to 1 else selects BMG
-- C_S_AXI_ACLK_PERIOD_PS      -- The period of the AXI clock in ps
-- C_S_AXI_ADDR_WIDTH          -- AXI address bus width - allowed value - 32 only
-- C_S_AXI_DATA_WIDTH          -- AXI data bus width - allowed value - 32 or 64 only
-- C_S_AXI_ID_WIDTH            -- AXI Identification TAG width - 1 to 32
-- C_S_AXI_PROTOCOL            -- AXI protocol type
--              
-- C_DUPLEX                    -- 1 = Full duplex, 0 = Half duplex
-- C_TX_PING_PONG              -- 1 = Ping-pong memory used for transmit buffer
--                                0 = Pong memory not used for transmit buffer 
-- C_RX_PING_PONG              -- 1 = Ping-pong memory used for receive buffer
--                                0 = Pong memory not used for receive buffer 
-- C_INCLUDE_MDIO              -- 1 = Include MDIO Innterface, 
--                                0 = No MDIO Interface
-- C_INCLUDE_INTERNAL_LOOPBACK -- 1 = Include Internal Loopback logic, 
--                                0 = Internal Loopback logic disabled
-- C_INCLUDE_GLOBAL_BUFFERS    -- 1 = Include global buffers for PHY clocks
--                                0 = Use normal input buffers for PHY clocks
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-- Definition of Ports:
--
-- s_axi_aclk            -- AXI Clock
-- s_axi_aresetn          -- AXI Reset - active low
-- -- interrupts           
-- ip2intc_irpt       -- Interrupt to processor
--==================================
-- axi write address Channel Signals
--==================================
-- s_axi_awid            -- AXI Write Address ID
-- s_axi_awaddr          -- AXI Write address - 32 bit
-- s_axi_awlen           -- AXI Write Data Length
-- s_axi_awsize          -- AXI Burst Size - allowed values
--                       -- 000 - byte burst
--                       -- 001 - half word
--                       -- 010 - word
--                       -- 011 - double word
--                       -- NA for all remaining values
-- s_axi_awburst         -- AXI Burst Type
--                       -- 00  - Fixed
--                       -- 01  - Incr
--                       -- 10  - Wrap
--                       -- 11  - Reserved
-- s_axi_awcache         -- AXI Cache Type
-- s_axi_awvalid         -- Write address valid
-- s_axi_awready         -- Write address ready
--===============================
-- axi write data channel Signals
--===============================
-- s_axi_wdata           -- AXI Write data width
-- s_axi_wstrb           -- AXI Write strobes
-- s_axi_wlast           -- AXI Last write indicator signal
-- s_axi_wvalid          -- AXI Write valid
-- s_axi_wready          -- AXI Write ready
--================================
-- axi write data response Signals
--================================
-- s_axi_bid             -- AXI Write Response channel number
-- s_axi_bresp           -- AXI Write response
--                       -- 00  - Okay
--                       -- 01  - ExOkay
--                       -- 10  - Slave Error
--                       -- 11  - Decode Error
-- s_axi_bvalid          -- AXI Write response valid
-- s_axi_bready          -- AXI Response ready
--=================================
-- axi read address Channel Signals
--=================================
-- s_axi_arid            -- AXI Read ID
-- s_axi_araddr          -- AXI Read address
-- s_axi_arlen           -- AXI Read Data length
-- s_axi_arsize          -- AXI Read Size
-- s_axi_arburst         -- AXI Read Burst length
-- s_axi_arcache         -- AXI Read Cache
-- s_axi_arprot          -- AXI Read Protection
-- s_axi_rvalid          -- AXI Read valid
-- s_axi_rready          -- AXI Read ready
--==============================
-- axi read data channel Signals
--==============================
-- s_axi_rid             -- AXI Read Channel ID
-- s_axi_rdata           -- AXI Read data
-- s_axi_rresp           -- AXI Read response
-- s_axi_rlast           -- AXI Read Data Last signal
-- s_axi_rvalid          -- AXI Read address valid
-- s_axi_rready          -- AXI Read address ready

--                 
-- -- ethernet
-- phy_tx_clk       -- Ethernet tranmit clock
-- phy_rx_clk       -- Ethernet receive clock
-- phy_crs          -- Ethernet carrier sense
-- phy_dv           -- Ethernet receive data valid
-- phy_rx_data      -- Ethernet receive data
-- phy_col          -- Ethernet collision indicator
-- phy_rx_er        -- Ethernet receive error
-- phy_rst_n        -- Ethernet PHY Reset
-- phy_tx_en        -- Ethernet transmit enable
-- phy_tx_data      -- Ethernet transmit data
-- phy_mdio_i       -- Ethernet PHY MDIO data input 
-- phy_mdio_o       -- Ethernet PHY MDIO data output 
-- phy_mdio_t       -- Ethernet PHY MDIO data 3-state control
-- phy_mdc          -- Ethernet PHY management clock
-------------------------------------------------------------------------------
-- ENTITY
-------------------------------------------------------------------------------
entity axi_ethernetlite is
  generic 
   (
    C_FAMILY                        : string := "virtex6";
    C_SELECT_XPM                    : integer := 1;
    C_INSTANCE                      : string := "axi_ethernetlite_inst";
    C_S_AXI_ACLK_PERIOD_PS          : integer := 10000;
    C_S_AXI_ADDR_WIDTH              : integer := 13;
    C_S_AXI_DATA_WIDTH              : integer range 32 to 32 := 32;
    C_S_AXI_ID_WIDTH                : integer := 4;
    C_S_AXI_PROTOCOL                : string  := "AXI4";

    C_INCLUDE_MDIO                  : integer := 1; 
    C_INCLUDE_INTERNAL_LOOPBACK     : integer := 0; 
    C_INCLUDE_GLOBAL_BUFFERS        : integer := 1; 
    C_DUPLEX                        : integer range 0 to 1:= 1; 
    C_TX_PING_PONG                  : integer range 0 to 1:= 0;
    C_RX_PING_PONG                  : integer range 0 to 1:= 0
    );
  port 
    (

--  -- AXI Slave signals ------------------------------------------------------
--   -- AXI Global System Signals
       s_axi_aclk       : in  std_logic;
       s_axi_aresetn    : in  std_logic;
       ip2intc_irpt     : out std_logic;
       

--   -- axi slave burst Interface
--   -- axi write address Channel Signals
       s_axi_awid    : in  std_logic_vector(C_S_AXI_ID_WIDTH-1 downto 0);
       s_axi_awaddr  : in  std_logic_vector(12 downto 0); -- (C_S_AXI_ADDR_WIDTH-1 downto 0);
       s_axi_awlen   : in  std_logic_vector(7 downto 0);
       s_axi_awsize  : in  std_logic_vector(2 downto 0);
       s_axi_awburst : in  std_logic_vector(1 downto 0);
       s_axi_awcache : in  std_logic_vector(3 downto 0);
       s_axi_awvalid : in  std_logic;
       s_axi_awready : out std_logic;

--   -- axi write data Channel Signals
       s_axi_wdata   : in  std_logic_vector(31 downto 0); -- (C_S_AXI_DATA_WIDTH-1 downto 0);
       s_axi_wstrb   : in  std_logic_vector(3 downto 0);
                               --(((C_S_AXI_DATA_WIDTH/8)-1) downto 0);
       s_axi_wlast   : in  std_logic;
       s_axi_wvalid  : in  std_logic;
       s_axi_wready  : out std_logic;

--   -- axi write response Channel Signals
       s_axi_bid     : out std_logic_vector(C_S_AXI_ID_WIDTH-1 downto 0);
       s_axi_bresp   : out std_logic_vector(1 downto 0);
       s_axi_bvalid  : out std_logic;
       s_axi_bready  : in  std_logic;
--   -- axi read address Channel Signals
       s_axi_arid    : in  std_logic_vector(C_S_AXI_ID_WIDTH-1 downto 0);
       s_axi_araddr  : in  std_logic_vector(12 downto 0); -- (C_S_AXI_ADDR_WIDTH-1 downto 0);
       s_axi_arlen   : in  std_logic_vector(7 downto 0);
       s_axi_arsize  : in  std_logic_vector(2 downto 0);
       s_axi_arburst : in  std_logic_vector(1 downto 0);
       s_axi_arcache : in  std_logic_vector(3 downto 0);
       s_axi_arvalid : in  std_logic;
       s_axi_arready : out std_logic;
       
--   -- axi read data Channel Signals
       s_axi_rid     : out std_logic_vector(C_S_AXI_ID_WIDTH-1 downto 0);
       s_axi_rdata   : out std_logic_vector(31 downto 0); -- (C_S_AXI_DATA_WIDTH-1 downto 0);
       s_axi_rresp   : out std_logic_vector(1 downto 0);
       s_axi_rlast   : out std_logic;
       s_axi_rvalid  : out std_logic;
       s_axi_rready  : in  std_logic;     


--   -- Ethernet Interface
       phy_tx_clk        : in std_logic;
       phy_rx_clk        : in std_logic;
       phy_crs           : in std_logic;
       phy_dv            : in std_logic;
       phy_rx_data       : in std_logic_vector (3 downto 0);
       phy_col           : in std_logic;
       phy_rx_er         : in std_logic;
       phy_rst_n         : out std_logic; 
       phy_tx_en         : out std_logic;
       phy_tx_data       : out std_logic_vector (3 downto 0);
       phy_mdio_i        : in  std_logic;
       phy_mdio_o        : out std_logic;
       phy_mdio_t        : out std_logic;
       phy_mdc           : out std_logic   
    
    );
    
-- XST attributes    

-- Fan-out attributes for XST
-- attribute MAX_FANOUT                             : string;
-- attribute MAX_FANOUT of s_axi_aclk               : signal is "10000";
-- attribute MAX_FANOUT of s_axi_aresetn            : signal is "10000";


--Psfutil attributes
attribute ASSIGNMENT       :   string;
attribute ADDRESS          :   string; 
attribute PAIR             :   string; 

end axi_ethernetlite;

-------------------------------------------------------------------------------
-- Architecture
-------------------------------------------------------------------------------  

architecture imp of axi_ethernetlite is

attribute DowngradeIPIdentifiedWarnings: string;

attribute DowngradeIPIdentifiedWarnings of imp : architecture is "yes";

 --Parameters captured for webtalk
   -- C_FAMILY
   -- C_S_AXI_ACLK_PERIOD_PS
   -- C_S_AXI_DATA_WIDTH           
   -- C_S_AXI_PROTOCOL             
   -- C_INCLUDE_MDIO               
   -- C_INCLUDE_INTERNAL_LOOPBACK  
   -- C_INCLUDE_GLOBAL_BUFFERS     
   -- C_DUPLEX                      
   -- C_TX_PING_PONG               
   -- C_RX_PING_PONG               
 
 --    constant C_CORE_GENERATION_INFO : string := C_INSTANCE & ",axi_ethernetlite,{"
 --     & "c_family="                       & C_FAMILY
 --     & ",C_INSTANCE = "                  & C_INSTANCE
 --     & ",c_s_axi_protocol="              & C_S_AXI_PROTOCOL
 --     & ",c_s_axi_aclk_period_ps="        & integer'image(C_S_AXI_ACLK_PERIOD_PS) 
 --     & ",c_s_axi_data_width="            & integer'image(C_S_AXI_DATA_WIDTH)
 --     & ",c_include_mdio="                & integer'image(C_INCLUDE_MDIO)
 --     & ",c_include_internal_loopback="   & integer'image(C_INCLUDE_INTERNAL_LOOPBACK)
 --     & ",c_include_global_buffers="      & integer'image(C_INCLUDE_GLOBAL_BUFFERS)
 --     & ",c_duplex="                      & integer'image(C_DUPLEX)
 --     & ",c_tx_ping_pong="                & integer'image(C_TX_PING_PONG)
 --     & ",c_rx_ping_pong="                & integer'image(C_RX_PING_PONG)
 --     & "}";
 -- 
 --     attribute CORE_GENERATION_INFO : string;
 --     attribute CORE_GENERATION_INFO of imp : architecture is C_CORE_GENERATION_INFO;

-------------------------------------------------------------------------------
--  Constant Declarations
-------------------------------------------------------------------------------
constant NODE_MAC : bit_vector := x"00005e00FACE";


-------------------------------------------------------------------------------
--   Signal declaration Section 
-------------------------------------------------------------------------------
signal phy_rx_clk_i    : std_logic;
signal phy_tx_clk_i    : std_logic;
signal phy_rx_clk_ib   : std_logic;
signal phy_tx_clk_ib   : std_logic;
signal phy_rx_data_i   : std_logic_vector(3 downto 0); 
signal phy_tx_data_i   : std_logic_vector(3 downto 0);
signal phy_tx_data_i_cdc   : std_logic_vector(3 downto 0);
signal phy_dv_i        : std_logic;
signal phy_rx_er_i     : std_logic;
signal phy_tx_en_i     : std_logic;
signal phy_tx_en_i_cdc : std_logic;
signal Loopback        : std_logic;
signal phy_rx_data_in  : std_logic_vector (3 downto 0);
signal phy_rx_data_in_cdc : std_logic_vector (3 downto 0);
signal phy_dv_in       : std_logic;
signal phy_dv_in_cdc   : std_logic;
signal phy_rx_data_reg : std_logic_vector(3 downto 0);
signal phy_rx_er_reg   : std_logic;
signal phy_dv_reg      : std_logic;

signal phy_tx_clk_core    : std_logic;
signal phy_rx_clk_core    : std_logic;

-- IPIC Signals
signal temp_Bus2IP_Addr: std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
signal bus2ip_addr     : std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
signal Bus2IP_Data     : std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
signal bus2ip_rdce     : std_logic;
signal bus2ip_wrce     : std_logic;
signal ip2bus_data     : std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
signal bus2ip_burst    : std_logic;
signal bus2ip_be       : std_logic_vector((C_S_AXI_DATA_WIDTH/8)-1 downto 0);
signal bus_rst_tx_sync_core         : std_logic;
--signal bus_rst_rx_sync         : std_logic;
signal bus_rst_rx_sync_core         : std_logic;
signal bus_rst         : std_logic;
signal ip2bus_errack   : std_logic;


component FDRE
  port 
   (
    Q  : out std_logic;
    C  : in std_logic;
    CE : in std_logic;
    D  : in std_logic;
    R  : in std_logic
   );
end component;
  
component BUFG
  port (
     O  : out std_ulogic;
     I : in std_ulogic := '0'
  );
end component;

component BUFGMUX
  port (
     O  : out std_ulogic;
     I0 : in std_ulogic := '0';
     I1 : in std_ulogic := '0';
     S  : in std_ulogic
  );
end component;

component BUF 
  port(
    O : out std_ulogic;

    I : in std_ulogic
    );
end component;

COMPONENT IBUF
    PORT(i : IN std_logic;
	 o : OUT std_logic);
END COMPONENT;

--  attribute IOB         : string;  

begin -- this is the begin between declarations and architecture body


   -- PHY Reset
   PHY_rst_n   <= S_AXI_ARESETN ;

   -- Bus Reset
   bus_rst     <= not S_AXI_ARESETN ;
 

 BUS_RST_RX_SYNC_CORE_I: entity  lib_cdc_v1_0_2.cdc_sync
  generic map (
    C_CDC_TYPE           => 1,
    C_RESET_STATE        => 0,
    C_SINGLE_BIT         => 1,
    C_FLOP_INPUT         => 0,
    C_VECTOR_WIDTH       => 1,
    C_MTBF_STAGES        => 4
            )
  port map(
    prmry_aclk            => '1',
    prmry_resetn          => '1',
    prmry_in              => bus_rst,
    prmry_ack             => open,
    scndry_out            => bus_rst_rx_sync_core,
    scndry_aclk           => phy_rx_clk_core,
    scndry_resetn         => '1',
    prmry_vect_in         => (OTHERS => '0'),
    scndry_vect_out       => open
     );


 BUS_RST_TX_SYNC_CORE_I: entity  lib_cdc_v1_0_2.cdc_sync
  generic map (
    C_CDC_TYPE           => 1,
    C_RESET_STATE        => 0,
    C_SINGLE_BIT         => 1,
    C_FLOP_INPUT         => 0,
    C_VECTOR_WIDTH       => 1,
    C_MTBF_STAGES        => 4
            )
  port map(
    prmry_aclk            => '1',
    prmry_resetn          => '1',
    prmry_in              => bus_rst,
    prmry_ack             => open,
    scndry_out            => bus_rst_tx_sync_core,
    scndry_aclk           => phy_tx_clk_core,
    scndry_resetn         => '1',
    prmry_vect_in         => (OTHERS => '0'),
    scndry_vect_out       => open
     );



   ----------------------------------------------------------------------------
   -- LOOPBACK_GEN :- Include MDIO interface if the parameter 
   -- C_INCLUDE_INTERNAL_LOOPBACK = 1
   ----------------------------------------------------------------------------
   LOOPBACK_GEN: if C_INCLUDE_INTERNAL_LOOPBACK = 1 generate
   begin

      -------------------------------------------------------------------------
      -- INCLUDE_BUFG_GEN :- Include Global Buffer for PHY clocks 
      -- C_INCLUDE_GLOBAL_BUFFERS = 1
      -------------------------------------------------------------------------
      INCLUDE_BUFG_GEN: if C_INCLUDE_GLOBAL_BUFFERS = 1 generate
      begin
         -------------------------------------------------------------------------
         -- IBUF for TX/RX clocks
         -------------------------------------------------------------------------
         TX_IBUF_INST: IBUF
           port map (
             O  => phy_tx_clk_ib,
             I  => PHY_tx_clk
           );

         RX_IBUF_INST: IBUF
           port map (
             O  => phy_rx_clk_ib,
             I  => PHY_rx_clk
           );
         -------------------------------------------------------------------------
         -- BUFG for TX clock
         -------------------------------------------------------------------------
         CLOCK_BUFG_TX: BUFG
           port map (
             O  => phy_tx_clk_core,  --[out]
             I  => PHY_tx_clk_ib     --[in]
           );

      -------------------------------------------------------------------------
      -- BUFGMUX for clock muxing in Loopback mode
      -------------------------------------------------------------------------
      CLOCK_MUX: BUFGMUX
        port map (
          O  => phy_rx_clk_core, --[out]
          I0 => PHY_rx_clk_ib,   --[in]
          I1 => phy_tx_clk_ib, --[in]
          S  => Loopback         --[in]
        );           

      end generate INCLUDE_BUFG_GEN; 

      -------------------------------------------------------------------------
      -- NO_BUFG_GEN :- Dont include Global Buffer for PHY clocks 
      -- C_INCLUDE_GLOBAL_BUFFERS = 0
      -------------------------------------------------------------------------
      NO_BUFG_GEN: if C_INCLUDE_GLOBAL_BUFFERS = 0 generate
      begin

         phy_tx_clk_core  <= PHY_tx_clk;

      -------------------------------------------------------------------------
      -- BUFGMUX for clock muxing in Loopback mode
      -------------------------------------------------------------------------
      CLOCK_MUX: BUFGMUX
        port map (
          O  => phy_rx_clk_core, --[out]
          I0 => PHY_rx_clk,      --[in]
          I1 => phy_tx_clk_core, --[in]
          S  => Loopback         --[in]
        );
      
      end generate NO_BUFG_GEN; 


      -------------------------------------------------------------------------
      -- Internal Loopback generation logic
      -------------------------------------------------------------------------
      phy_rx_data_in <=  phy_tx_data_i when Loopback = '1' else
                         phy_rx_data_reg;
      
      phy_dv_in      <=  phy_tx_en_i   when Loopback = '1' else
                         phy_dv_reg;
      
      -- No receive error is generated in internal loopback
      phy_rx_er_i    <= '0' when Loopback = '1' else
                         phy_rx_er_reg;
      
      
      -- Transmit and Receive clocks         
      phy_tx_clk_i <= phy_tx_clk_core;--not(phy_tx_clk_core);
      phy_rx_clk_i <= phy_rx_clk_core;--not(phy_rx_clk_core);

   ----------------------------------------------------------------------------
   -- CDC module for syncing phy_dv_in in rx_clk domain
   ----------------------------------------------------------------------------
  CDC_PHY_DV_IN: entity  lib_cdc_v1_0_2.cdc_sync
  generic map (
    C_CDC_TYPE           => 1,
    C_RESET_STATE        => 0,
    C_SINGLE_BIT         => 1,
    C_FLOP_INPUT         => 0,
    C_VECTOR_WIDTH       => 1,
    C_MTBF_STAGES        => 2
            )
  port map(
    prmry_aclk            => '1',
    prmry_resetn          => '1',
    prmry_in              => phy_dv_in,
    prmry_ack             => open,
    scndry_out            => phy_dv_in_cdc,
    scndry_aclk           => phy_rx_clk_i,
    scndry_resetn         => '1',
    prmry_vect_in         => (OTHERS => '0'),
    scndry_vect_out       => open
     );


 --BUS_RST_RX_SYNC_I: entity  lib_cdc_v1_0_2.cdc_sync
 -- generic map (
 --   C_CDC_TYPE           => 1,
 --   C_RESET_STATE        => 0,
 --   C_SINGLE_BIT         => 1,
 --   C_FLOP_INPUT         => 0,
 --   C_VECTOR_WIDTH       => 1,
 --   C_MTBF_STAGES        => 4
 --           )
 -- port map(
 --   prmry_aclk            => '1',
 --   prmry_resetn          => '1',
 --   prmry_in              => bus_rst,
 --   prmry_ack             => open,
 --   scndry_out            => bus_rst_rx_sync,
 --   scndry_aclk           => phy_rx_clk_i,
 --   scndry_resetn         => '1',
 --   prmry_vect_in         => (OTHERS => '0'),
 --   scndry_vect_out       => open
 --    );





      -------------------------------------------------------------------------
      -- Registering RX signal 
      -------------------------------------------------------------------------
      DV_FF: FDR
        port map (
          Q  => phy_dv_i,             --[out]
          C  => phy_rx_clk_i,         --[in]
          D  => phy_dv_in_cdc,        --[in]
          R  => bus_rst_rx_sync_core);             --[in]
     
   ----------------------------------------------------------------------------
   -- CDC module for syncing phy_rx_data_in in rx_clk domain
   ----------------------------------------------------------------------------
  CDC_PHY_RX_DATA_IN: entity  lib_cdc_v1_0_2.cdc_sync
  generic map (
    C_CDC_TYPE           => 1,
    C_RESET_STATE        => 0,
    C_SINGLE_BIT         => 0,
    C_FLOP_INPUT         => 0,
    C_VECTOR_WIDTH       => 4,
    C_MTBF_STAGES        => 2
            )
  port map(
    prmry_aclk            => '1',
    prmry_resetn          => '1',
    prmry_in              => '1',
    prmry_ack             => open,
    scndry_out            => open,
    scndry_aclk           => phy_rx_clk_i,
    scndry_resetn         => '1',
    prmry_vect_in         => phy_rx_data_in,
    scndry_vect_out       => phy_rx_data_in_cdc
     );
      -------------------------------------------------------------------------
      -- Registering RX data input with clock mux output
      -------------------------------------------------------------------------
      RX_REG_GEN: for i in 3 downto 0 generate
      begin
         RX_FF_LOOP: FDRE
           port map (
             Q  => phy_rx_data_i(i),   --[out]
             C  => phy_rx_clk_i,       --[in]
             CE => '1',                --[in]
             D  => phy_rx_data_in_cdc(i),  --[in]
             R  => bus_rst_rx_sync_core);  --[in]
      
      end generate RX_REG_GEN;

   end generate LOOPBACK_GEN; 

   ----------------------------------------------------------------------------
   -- NO_LOOPBACK_GEN :- Include MDIO interface if the parameter 
   -- C_INCLUDE_INTERNAL_LOOPBACK = 0
   ----------------------------------------------------------------------------
   NO_LOOPBACK_GEN: if C_INCLUDE_INTERNAL_LOOPBACK = 0 generate
   begin

      -------------------------------------------------------------------------
      -- INCLUDE_BUFG_GEN :- Include Global Buffer for PHY clocks 
      -- C_INCLUDE_GLOBAL_BUFFERS = 1
      -------------------------------------------------------------------------
      INCLUDE_BUFG_GEN: if C_INCLUDE_GLOBAL_BUFFERS = 1 generate
      begin
         ------------------------------------------------------------------------- 
         -- IBUF for TX/RX clocks
         -------------------------------------------------------------------------
         TX_IBUF_INST: IBUF
           port map (
             O  => phy_tx_clk_ib,
             I  => PHY_tx_clk
           );

         RX_IBUF_INST: IBUF
           port map (
             O  => phy_rx_clk_ib,
             I  => PHY_rx_clk
           );
         -------------------------------------------------------------------------
         -- BUFG for clock muxing 
         -------------------------------------------------------------------------
         CLOCK_BUFG_TX: BUFG
           port map (
             O  => phy_tx_clk_core,  --[out]
             I  => PHY_tx_clk_ib     --[in]
           );

         -------------------------------------------------------------------------
         -- BUFG for clock muxing 
         -------------------------------------------------------------------------
         CLOCK_BUFG_RX: BUFG
           port map (
             O  => phy_rx_clk_core,  --[out]
             I  => PHY_rx_clk_ib     --[in]
           );
      

      end generate INCLUDE_BUFG_GEN; 

      -------------------------------------------------------------------------
      -- NO_BUFG_GEN :- Dont include Global Buffer for PHY clocks 
      -- C_INCLUDE_GLOBAL_BUFFERS = 0
      -------------------------------------------------------------------------
      NO_BUFG_GEN: if C_INCLUDE_GLOBAL_BUFFERS = 0 generate
      begin

         phy_tx_clk_core  <= PHY_tx_clk;
         phy_rx_clk_core  <= PHY_rx_clk;
      
      end generate NO_BUFG_GEN; 


      -- Transmit and Receive clocks for core         
      phy_tx_clk_i  <= phy_tx_clk_core;--not(phy_tx_clk_core);
      phy_rx_clk_i  <= phy_rx_clk_core;--not(phy_rx_clk_core);
       
      -- TX/RX internal signals
      phy_rx_data_i <= phy_rx_data_reg;
      phy_rx_er_i   <= phy_rx_er_reg;
      phy_dv_i      <= phy_dv_reg;
      
      
   end generate NO_LOOPBACK_GEN; 

   ----------------------------------------------------------------------------
   -- CDC module for syncing phy_tx_en in tx_clk domain
   ----------------------------------------------------------------------------
  CDC_PHY_TX_EN_O: entity  lib_cdc_v1_0_2.cdc_sync
  generic map (
    C_CDC_TYPE           => 1,
    C_RESET_STATE        => 0,
    C_SINGLE_BIT         => 1,
    C_FLOP_INPUT         => 0,
    C_VECTOR_WIDTH       => 1,
    C_MTBF_STAGES        => 2
            )
  port map(
    prmry_aclk            => '1',
    prmry_resetn          => '1',
    prmry_in              => PHY_tx_en_i,
    prmry_ack             => open,
    scndry_out            => PHY_tx_en_i_cdc,
    scndry_aclk           => phy_tx_clk_core,
    scndry_resetn         => '1',
    prmry_vect_in         => (OTHERS => '0'),
    scndry_vect_out       => open
     );

   ----------------------------------------------------------------------------
   -- CDC module for syncing phy_tx_data_out in tx_clk domain
   ----------------------------------------------------------------------------
  CDC_PHY_TX_DATA_OUT: entity  lib_cdc_v1_0_2.cdc_sync
  generic map (
    C_CDC_TYPE           => 1,
    C_RESET_STATE        => 0,
    C_SINGLE_BIT         => 0,
    C_FLOP_INPUT         => 0,
    C_VECTOR_WIDTH       => 4,
    C_MTBF_STAGES        => 2
            )
  port map(
    prmry_aclk            => '1',
    prmry_resetn          => '1',
    prmry_in              => '1',
    prmry_ack             => open,
    scndry_out            => open,
    scndry_aclk           => phy_tx_clk_core,
    scndry_resetn         => '1',
    prmry_vect_in         => phy_tx_data_i,
    scndry_vect_out       => phy_tx_data_i_cdc
     );    
           
   ----------------------------------------------------------------------------
   -- Registering the Ethernet data signals
   ----------------------------------------------------------------------------   
   IOFFS_GEN: for i in 3 downto 0 generate
--   attribute IOB of RX_FF_I : label is "true";
--   attribute IOB of TX_FF_I : label is "true";
   begin
      RX_FF_I: FDRE
         port map (
            Q  => phy_rx_data_reg(i), --[out]
            C  => phy_rx_clk_core,    --[in]
            CE => '1',                --[in]
            D  => PHY_rx_data(i),     --[in]
            R  => bus_rst_rx_sync_core);           --[in]
            
      TX_FF_I: FDRE
         port map (
            Q  => PHY_tx_data(i),     --[out]
            C  => phy_tx_clk_core,    --[in]
            CE => '1',                --[in]
            D  => phy_tx_data_i_cdc(i),   --[in]
            R  => bus_rst_tx_sync_core);           --[in]
          
    end generate IOFFS_GEN;


   ----------------------------------------------------------------------------
   -- Registering the Ethernet control signals
   ----------------------------------------------------------------------------   
   IOFFS_GEN2: if(true) generate 
--      attribute IOB of DVD_FF : label is "true";
--      attribute IOB of RER_FF : label is "true";
--      attribute IOB of TEN_FF : label is "true";
      begin
         DVD_FF: FDRE
           port map (
             Q  => phy_dv_reg,      --[out]
             C  => phy_rx_clk_core, --[in]
             CE => '1',             --[in]
             D  => PHY_dv,          --[in]
             R  => bus_rst_rx_sync_core);        --[in]
               
         RER_FF: FDRE
           port map (
             Q  => phy_rx_er_reg,   --[out]
             C  => phy_rx_clk_core, --[in]
             CE => '1',             --[in]
             D  => PHY_rx_er,       --[in]
             R  => bus_rst_rx_sync_core);        --[in]
               
         TEN_FF: FDRE
           port map (
             Q  => PHY_tx_en,       --[out]
             C  => phy_tx_clk_core, --[in]
             CE => '1',             --[in]
             D  => PHY_tx_en_i_cdc, --[in]
             R  => bus_rst_tx_sync_core);        --[in]    
               
   end generate IOFFS_GEN2;
      
   ----------------------------------------------------------------------------
   -- XEMAC Module
   ----------------------------------------------------------------------------   
   XEMAC_I : entity axi_ethernetlite_v3_0_18.xemac
     generic map 
        (
        C_FAMILY                 => C_FAMILY,
        C_SELECT_XPM             => C_SELECT_XPM,
        C_S_AXI_ADDR_WIDTH       => C_S_AXI_ADDR_WIDTH,  
        C_S_AXI_DATA_WIDTH       => C_S_AXI_DATA_WIDTH,                     
        C_S_AXI_ACLK_PERIOD_PS   => C_S_AXI_ACLK_PERIOD_PS,
        C_DUPLEX                 => C_DUPLEX,
        C_RX_PING_PONG           => C_RX_PING_PONG,
        C_TX_PING_PONG           => C_TX_PING_PONG,
        C_INCLUDE_MDIO           => C_INCLUDE_MDIO,
        NODE_MAC                 => NODE_MAC

        )
     
     port map 
        (   
        Clk       => S_AXI_ACLK,
        Rst       => bus_rst,
        IP2INTC_Irpt   => IP2INTC_Irpt,


 
     -- Bus2IP Signals
        Bus2IP_Addr          => bus2ip_addr,
        Bus2IP_Data          => bus2ip_data,
        Bus2IP_BE            => bus2ip_be,
        Bus2IP_Burst         => bus2ip_burst,
        Bus2IP_RdCE          => bus2ip_rdce,
        Bus2IP_WrCE          => bus2ip_wrce,

     -- IP2Bus Signals
        IP2Bus_Data          => ip2bus_data,
        IP2Bus_Error         => ip2bus_errack,

     -- EMAC Signals
        PHY_tx_clk     => phy_tx_clk_i,
        PHY_rx_clk     => phy_rx_clk_i,
        PHY_crs        => PHY_crs,
        PHY_dv         => phy_dv_i,
        PHY_rx_data    => phy_rx_data_i,
        PHY_col        => PHY_col,
        PHY_rx_er      => phy_rx_er_i,
        PHY_tx_en      => PHY_tx_en_i,
        PHY_tx_data    => PHY_tx_data_i,
        PHY_MDIO_I     => phy_mdio_i,
        PHY_MDIO_O     => phy_mdio_o,
        PHY_MDIO_T     => phy_mdio_t,
        PHY_MDC        => phy_mdc,
        Loopback       => Loopback 
        );
        
I_AXI_NATIVE_IPIF: entity axi_ethernetlite_v3_0_18.axi_interface
  generic map (
  
        C_S_AXI_ADDR_WIDTH          => C_S_AXI_ADDR_WIDTH,  
        C_S_AXI_DATA_WIDTH          => C_S_AXI_DATA_WIDTH,                     
        C_S_AXI_ID_WIDTH            => C_S_AXI_ID_WIDTH,
        C_S_AXI_PROTOCOL            => C_S_AXI_PROTOCOL,
        C_FAMILY                    => C_FAMILY

        )
  port map (  
            
          
        S_AXI_ACLK      =>  s_axi_aclk,
        S_AXI_ARESETN   =>  s_axi_aresetn,
        S_AXI_AWADDR    =>  s_axi_awaddr,
        S_AXI_AWID      =>  s_axi_awid,
        S_AXI_AWLEN     =>  s_axi_awlen,
        S_AXI_AWSIZE    =>  s_axi_awsize,
        S_AXI_AWBURST   =>  s_axi_awburst,
        S_AXI_AWCACHE   =>  s_axi_awcache,
        S_AXI_AWVALID   =>  s_axi_awvalid,
        S_AXI_AWREADY   =>  s_axi_awready,
        S_AXI_WDATA     =>  s_axi_wdata,
        S_AXI_WSTRB     =>  s_axi_wstrb,
        S_AXI_WLAST     =>  s_axi_wlast,
        S_AXI_WVALID    =>  s_axi_wvalid,
        S_AXI_WREADY    =>  s_axi_wready,
        S_AXI_BID       =>  s_axi_bid,
        S_AXI_BRESP     =>  s_axi_bresp,
        S_AXI_BVALID    =>  s_axi_bvalid,
        S_AXI_BREADY    =>  s_axi_bready,
        S_AXI_ARID      =>  s_axi_arid,
        S_AXI_ARADDR    =>  s_axi_araddr,                                       
        S_AXI_ARLEN     =>  s_axi_arlen,                                        
        S_AXI_ARSIZE    =>  s_axi_arsize,                                       
        S_AXI_ARBURST   =>  s_axi_arburst,                                      
        S_AXI_ARCACHE   =>  s_axi_arcache,                                      
        S_AXI_ARVALID   =>  s_axi_arvalid,
        S_AXI_ARREADY   =>  s_axi_arready,                                              
        S_AXI_RID       =>  s_axi_rid,                                      
        S_AXI_RDATA     =>  s_axi_rdata,                                        
        S_AXI_RRESP     =>  s_axi_rresp,                                        
        S_AXI_RLAST     =>  s_axi_rlast,                                        
        S_AXI_RVALID    =>  s_axi_rvalid,                                       
        S_AXI_RREADY    =>  s_axi_rready,                                       
                                                                
            
-- IP Interconnect (IPIC) port signals ------------------------------------                                                     
      -- Controls to the IP/IPIF modules
            -- IP Interconnect (IPIC) port signals
        IP2Bus_Data     =>  ip2bus_data,
     --   IP2Bus_Error    =>  ip2bus_errack,
			    
        Bus2IP_Addr     =>  bus2ip_addr,
        Bus2IP_Data     =>  bus2ip_data,
        Bus2IP_BE       =>  bus2ip_be,
        Bus2IP_Burst    =>  bus2ip_burst,
        Bus2IP_RdCE     =>  bus2ip_rdce,
        Bus2IP_WrCE     =>  bus2ip_wrce
        ); 
        

------------------------------------------------------------------------------------------

end imp;


