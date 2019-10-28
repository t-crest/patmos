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
-- Wrapper for a 2x2 bi-torus NoC, intended use with Chisel wrapper
--
-- Author: Eleftherios Kyriakakis
--------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;
use work.config_types.all;
use work.config.all;
use work.ocp.all;
use work.noc_interface.all;
use work.argo_types.all;


entity noc_wrapper_2x2 is
  port (
    clk	: in std_logic;
    reset	: in std_logic;
    supervisor	: in std_logic_vector(3 downto 0);
    irq	: out std_logic_vector(7 downto 0);
  -- Core 0
    io_ocpPorts_0_M_Cmd        : in std_logic_vector(OCP_CMD_WIDTH-1 downto 0);
    io_ocpPorts_0_M_Addr       : in std_logic_vector(OCP_ADDR_WIDTH-1 downto 0);
    io_ocpPorts_0_M_Data       : in std_logic_vector(OCP_DATA_WIDTH-1 downto 0);
    io_ocpPorts_0_M_ByteEn     : in std_logic_vector(OCP_BYTE_WIDTH-1 downto 0);
    io_ocpPorts_0_M_RespAccept : in std_logic;
    io_ocpPorts_0_S_Resp       : out std_logic_vector(OCP_RESP_WIDTH-1 downto 0);
    io_ocpPorts_0_S_Data       : out std_logic_vector(OCP_DATA_WIDTH-1 downto 0);
    io_ocpPorts_0_S_CmdAccept  : out std_logic;
  -- Core 1
    io_ocpPorts_1_M_Cmd        : in std_logic_vector(OCP_CMD_WIDTH-1 downto 0);
    io_ocpPorts_1_M_Addr       : in std_logic_vector(OCP_ADDR_WIDTH-1 downto 0);
    io_ocpPorts_1_M_Data       : in std_logic_vector(OCP_DATA_WIDTH-1 downto 0);
    io_ocpPorts_1_M_ByteEn     : in std_logic_vector(OCP_BYTE_WIDTH-1 downto 0);
    io_ocpPorts_1_M_RespAccept : in std_logic;
    io_ocpPorts_1_S_Resp       : out std_logic_vector(OCP_RESP_WIDTH-1 downto 0);
    io_ocpPorts_1_S_Data       : out std_logic_vector(OCP_DATA_WIDTH-1 downto 0);
    io_ocpPorts_1_S_CmdAccept  : out std_logic;
  -- Core 2
    io_ocpPorts_2_M_Cmd        : in std_logic_vector(OCP_CMD_WIDTH-1 downto 0);
    io_ocpPorts_2_M_Addr       : in std_logic_vector(OCP_ADDR_WIDTH-1 downto 0);
    io_ocpPorts_2_M_Data       : in std_logic_vector(OCP_DATA_WIDTH-1 downto 0);
    io_ocpPorts_2_M_ByteEn     : in std_logic_vector(OCP_BYTE_WIDTH-1 downto 0);
    io_ocpPorts_2_M_RespAccept : in std_logic;
    io_ocpPorts_2_S_Resp       : out std_logic_vector(OCP_RESP_WIDTH-1 downto 0);
    io_ocpPorts_2_S_Data       : out std_logic_vector(OCP_DATA_WIDTH-1 downto 0);
    io_ocpPorts_2_S_CmdAccept  : out std_logic;
  -- Core 3
    io_ocpPorts_3_M_Cmd        : in std_logic_vector(OCP_CMD_WIDTH-1 downto 0);
    io_ocpPorts_3_M_Addr       : in std_logic_vector(OCP_ADDR_WIDTH-1 downto 0);
    io_ocpPorts_3_M_Data       : in std_logic_vector(OCP_DATA_WIDTH-1 downto 0);
    io_ocpPorts_3_M_ByteEn     : in std_logic_vector(OCP_BYTE_WIDTH-1 downto 0);
    io_ocpPorts_3_M_RespAccept : in std_logic;
    io_ocpPorts_3_S_Resp       : out std_logic_vector(OCP_RESP_WIDTH-1 downto 0);
    io_ocpPorts_3_S_Data       : out std_logic_vector(OCP_DATA_WIDTH-1 downto 0);
    io_ocpPorts_3_S_CmdAccept  : out std_logic;
	-- SPM 0
    io_spmPorts_0_M_Addr	: out unsigned(HEADER_FIELD_WIDTH-HEADER_CTRL_WIDTH-1 downto 0);
    io_spmPorts_0_M_En      : out std_logic_vector(1 downto 0);
    io_spmPorts_0_M_Wr      : out std_logic;
    io_spmPorts_0_M_Data    : out unsigned((2*WORD_WIDTH)-1 downto 0);
    io_spmPorts_0_S_Data	: in unsigned((2*WORD_WIDTH)-1 downto 0);
    io_spmPorts_0_S_Error   : in std_logic;
  -- SPM 1
    io_spmPorts_1_M_Addr	: out unsigned(HEADER_FIELD_WIDTH-HEADER_CTRL_WIDTH-1 downto 0);
    io_spmPorts_1_M_En      : out std_logic_vector(1 downto 0);
    io_spmPorts_1_M_Wr      : out std_logic;
    io_spmPorts_1_M_Data    : out unsigned((2*WORD_WIDTH)-1 downto 0);
    io_spmPorts_1_S_Data	: in unsigned((2*WORD_WIDTH)-1 downto 0);
    io_spmPorts_1_S_Error   : in std_logic;
  -- SPM 2
    io_spmPorts_2_M_Addr	: out unsigned(HEADER_FIELD_WIDTH-HEADER_CTRL_WIDTH-1 downto 0);
    io_spmPorts_2_M_En      : out std_logic_vector(1 downto 0);
    io_spmPorts_2_M_Wr      : out std_logic;
    io_spmPorts_2_M_Data    : out unsigned((2*WORD_WIDTH)-1 downto 0);
    io_spmPorts_2_S_Data	: in unsigned((2*WORD_WIDTH)-1 downto 0);
    io_spmPorts_2_S_Error   : in std_logic;
  -- SPM 3
    io_spmPorts_3_M_Addr	: out unsigned(HEADER_FIELD_WIDTH-HEADER_CTRL_WIDTH-1 downto 0);
    io_spmPorts_3_M_En      : out std_logic_vector(1 downto 0);
    io_spmPorts_3_M_Wr      : out std_logic;
    io_spmPorts_3_M_Data    : out unsigned((2*WORD_WIDTH)-1 downto 0);
    io_spmPorts_3_S_Data	: in unsigned((2*WORD_WIDTH)-1 downto 0);
    io_spmPorts_3_S_Error   : in std_logic
  );

end noc_wrapper_2x2;

architecture struct of noc_wrapper_2x2 is

------------------------------signal declarations----------------------------

component noc is
	port(
		clk	: in std_logic;
		reset	: in std_logic;
		ocp_io_ms	: in ocp_io_m_a;
		supervisor	: in std_logic_vector(3 downto 0);
		ocp_io_ss	: out ocp_io_s_a;
		spm_ports_m	: out mem_if_masters;
		spm_ports_s	: in mem_if_slaves;
		irq	: out std_logic_vector(7 downto 0)
	);
end component;

begin

noc_2x2_inst: noc
port map(
    clk => clk,
    reset => reset,
    supervisor => supervisor,
    irq => irq,
    --OCP
    --M
    ocp_io_ms(0).MCmd => io_ocpPorts_0_M_Cmd,
    ocp_io_ms(1).MCmd => io_ocpPorts_1_M_Cmd,
    ocp_io_ms(2).MCmd => io_ocpPorts_2_M_Cmd,
    ocp_io_ms(3).MCmd => io_ocpPorts_3_M_Cmd,

    ocp_io_ms(0).MAddr => io_ocpPorts_0_M_Addr,
    ocp_io_ms(1).MAddr => io_ocpPorts_1_M_Addr,
    ocp_io_ms(2).MAddr => io_ocpPorts_2_M_Addr,
    ocp_io_ms(3).MAddr => io_ocpPorts_3_M_Addr,

    ocp_io_ms(0).MData => io_ocpPorts_0_M_Data,
    ocp_io_ms(1).MData => io_ocpPorts_1_M_Data,
    ocp_io_ms(2).MData => io_ocpPorts_2_M_Data,
    ocp_io_ms(3).MData => io_ocpPorts_3_M_Data,

    ocp_io_ms(0).MByteEn => io_ocpPorts_0_M_ByteEn,
    ocp_io_ms(1).MByteEn => io_ocpPorts_1_M_ByteEn,
    ocp_io_ms(2).MByteEn => io_ocpPorts_2_M_ByteEn,
    ocp_io_ms(3).MByteEn => io_ocpPorts_3_M_ByteEn,

    ocp_io_ms(0).MRespAccept => io_ocpPorts_0_M_RespAccept,
    ocp_io_ms(1).MRespAccept => io_ocpPorts_1_M_RespAccept,
    ocp_io_ms(2).MRespAccept => io_ocpPorts_2_M_RespAccept,
    ocp_io_ms(3).MRespAccept => io_ocpPorts_3_M_RespAccept,
    --S
    ocp_io_ss(0).SResp => io_ocpPorts_0_S_Resp,
    ocp_io_ss(1).SResp => io_ocpPorts_1_S_Resp,
    ocp_io_ss(2).SResp => io_ocpPorts_2_S_Resp,
    ocp_io_ss(3).SResp => io_ocpPorts_3_S_Resp,

    ocp_io_ss(0).SData => io_ocpPorts_0_S_Data,
    ocp_io_ss(1).SData => io_ocpPorts_1_S_Data,
    ocp_io_ss(2).SData => io_ocpPorts_2_S_Data,
    ocp_io_ss(3).SData => io_ocpPorts_3_S_Data,

    ocp_io_ss(0).SCmdAccept => io_ocpPorts_0_S_CmdAccept,
    ocp_io_ss(1).SCmdAccept => io_ocpPorts_1_S_CmdAccept,
    ocp_io_ss(2).SCmdAccept => io_ocpPorts_2_S_CmdAccept,
    ocp_io_ss(3).SCmdAccept => io_ocpPorts_3_S_CmdAccept,

  --SPM
  --M
    spm_ports_m(0).addr => io_spmPorts_0_M_Addr,
    spm_ports_m(1).addr => io_spmPorts_1_M_Addr,
    spm_ports_m(2).addr => io_spmPorts_2_M_Addr,
    spm_ports_m(3).addr => io_spmPorts_3_M_Addr,

    spm_ports_m(0).en => io_spmPorts_0_M_En,
    spm_ports_m(1).en => io_spmPorts_1_M_En,
    spm_ports_m(2).en => io_spmPorts_2_M_En,
    spm_ports_m(3).en => io_spmPorts_3_M_En,

    spm_ports_m(0).wr => io_spmPorts_0_M_Wr,
    spm_ports_m(1).wr => io_spmPorts_1_M_Wr,
    spm_ports_m(2).wr => io_spmPorts_2_M_Wr,
    spm_ports_m(3).wr => io_spmPorts_3_M_Wr,

    spm_ports_m(0).wdata => io_spmPorts_0_M_Data,
    spm_ports_m(1).wdata => io_spmPorts_1_M_Data,
    spm_ports_m(2).wdata => io_spmPorts_2_M_Data,
    spm_ports_m(3).wdata => io_spmPorts_3_M_Data,

    spm_ports_s(0).rdata => io_spmPorts_0_S_Data,
    spm_ports_s(1).rdata => io_spmPorts_1_S_Data,
    spm_ports_s(2).rdata => io_spmPorts_2_S_Data,
    spm_ports_s(3).rdata => io_spmPorts_3_S_Data,

    spm_ports_s(0).error => io_spmPorts_0_S_Error,
    spm_ports_s(1).error => io_spmPorts_1_S_Error,
    spm_ports_s(2).error => io_spmPorts_2_S_Error,
    spm_ports_s(3).error => io_spmPorts_3_S_Error
);

end struct;
