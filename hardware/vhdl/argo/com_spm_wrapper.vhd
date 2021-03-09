--
-- Copyright Technical University of Denmark. All rights reserved.
-- This file is part of the T-CREST project.
--
-- Redistribution and use in source and binary forms, with or without
-- modification, are permitted provided that the following conditions are met:
--
--    1. Redistributions of source code must retain the above copyright notice,
--       this list of conditions and the following disclaimer.
--
--    2. Redistributions in binary form must reproduce the above copyright
--       notice, this list of conditions and the following disclaimer in the
--       documentation and/or other materials provided with the distribution.
--
-- THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
-- OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
-- OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
-- NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
-- DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
-- (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
-- LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
-- ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
-- (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
-- THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
--
-- The views and conclusions contained in the software and documentation are
-- those of the authors and should not be interpreted as representing official
-- policies, either expressed or implied, of the copyright holder.
--


--------------------------------------------------------------------------------
-- Communication scratch pad memory wrapper for use within Argo Chisel Wrapper Solution
--
-- Author: Eleftherios Kyriakakis (elky@dtu.dk)
--------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.ocp.all;
use work.config_types.all;
use work.argo_types.all;

entity com_spm_wrapper is
  generic(
    -- 2**SPM_IDX_SIZE is the number of bytes in the SPM
    SPM_IDX_SIZE : natural := 12
  );
  port (
    clk 					: in std_logic;
    reset 				: in std_logic;
    ocp_M_Cmd     : in std_logic_vector(OCP_CMD_WIDTH-1 downto 0);
    ocp_M_Addr    : in std_logic_vector(OCP_ADDR_WIDTH-1 downto 0);
    ocp_M_Data    : in std_logic_vector(OCP_DATA_WIDTH-1 downto 0);
    ocp_M_ByteEn  : in std_logic_vector(OCP_BYTE_WIDTH-1 downto 0);
    ocp_S_Resp    : out std_logic_vector(OCP_RESP_WIDTH-1 downto 0);
    ocp_S_Data    : out std_logic_vector(OCP_DATA_WIDTH-1 downto 0);
    spm_M_Addr	 : in unsigned(HEADER_FIELD_WIDTH-HEADER_CTRL_WIDTH-1 downto 0);
    spm_M_En		 : in std_logic_vector(1 downto 0);
    spm_M_Wr		 : in std_logic;
    spm_M_Data	 : in std_logic_vector((2*WORD_WIDTH)-1 downto 0);
    spm_S_Data	 : out std_logic_vector((2*WORD_WIDTH)-1 downto 0);
    spm_S_Error	 : out std_logic
);
end entity ; -- com_spm

architecture arch of com_spm_wrapper is
  component com_spm is
    generic(
      -- 2**SPM_IDX_SIZE is the number of bytes in the SPM
      SPM_IDX_SIZE : natural := 12
      );
    port (
      p_clk : in std_logic;
      n_clk : in std_logic;
      reset : in std_logic;
      ocp_core_m : in ocp_core_m;
      ocp_core_s : out ocp_core_s;
      spm_m    : in mem_if_master;
      spm_s   : out mem_if_slave
    );
  end component;

signal spm_s_rdata : unsigned((2*WORD_WIDTH)-1 downto 0) := (others=>'0');
signal spm_m_wdata : unsigned((2*WORD_WIDTH)-1 downto 0) := (others=>'0');

begin

  spm_m_wdata <= unsigned(spm_M_Data);
  spm_S_Data <= std_logic_vector(spm_s_rdata);

  com_spm_inst: com_spm
  generic map(
    SPM_IDX_SIZE=>SPM_IDX_SIZE
  )
  port map(
    p_clk => clk,
    n_clk => clk,
    reset => reset,
    ocp_core_m.MCmd => ocp_M_Cmd,
    ocp_core_m.MAddr => ocp_M_Addr,
    ocp_core_m.MData => ocp_M_Data,
    ocp_core_m.MByteEn => ocp_M_ByteEn,
    ocp_core_s.SResp => ocp_S_Resp,
    ocp_core_s.Sdata => ocp_S_Data,
    spm_m.addr => spm_M_Addr,
    spm_m.en => spm_M_En,
    spm_m.wr => spm_M_Wr,
    spm_m.wdata => spm_m_wdata,
    spm_s.rdata => spm_s_rdata,
    spm_s.error => spm_S_Error
    
  );
end architecture ; -- arch
