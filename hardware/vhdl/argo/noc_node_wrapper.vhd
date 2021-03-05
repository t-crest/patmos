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
-- NoC node wrapper intended for use within the Argo Chisel Wrapper Solution.
--
-- Author: Eleftherios Kyriakakis
--------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;
use work.ocp.all;
use work.argo_types.all;
--use work.noc_defs.all;
--use work.noc_interface.all;

entity noc_node_wrapper is
	generic (
		MASTER : Integer := 0
	);
	port (
		clk					: in std_logic;
		reset				: in std_logic;
		irq					: out std_logic_vector(1 downto 0);
		run 				: in std_logic;
		supervisor			: in std_logic;
		masterRun			: out std_logic;

		proc_M_Cmd        	: in std_logic_vector(OCP_CMD_WIDTH-1 downto 0);
		proc_M_Addr       	: in std_logic_vector(OCP_ADDR_WIDTH-1 downto 0);
		proc_M_Data       	: in std_logic_vector(OCP_DATA_WIDTH-1 downto 0);
		proc_M_ByteEn     	: in std_logic_vector(OCP_BYTE_WIDTH-1 downto 0);
		proc_M_RespAccept 	: in std_logic;
		proc_S_Resp       	: out std_logic_vector(OCP_RESP_WIDTH-1 downto 0);
		proc_S_Data       	: out std_logic_vector(OCP_DATA_WIDTH-1 downto 0);
		proc_S_CmdAccept  	: out std_logic;

		spm_M_Addr			: out unsigned(HEADER_FIELD_WIDTH-HEADER_CTRL_WIDTH-1 downto 0);
		spm_M_En			: out std_logic_vector(1 downto 0);
		spm_M_Wr			: out std_logic;
		spm_M_Data			: out std_logic_vector((2*WORD_WIDTH)-1 downto 0);
		spm_S_Data			: in std_logic_vector((2*WORD_WIDTH)-1 downto 0);
		spm_S_Error			: in std_logic;
		
		-- router ports
		north_in_f_req		: in std_logic;
		north_in_f_data		: in std_logic_vector(LINK_WIDTH-1 downto 0);
		north_in_b_ack		: out std_logic;
		east_in_f_req     	: in std_logic;
		east_in_f_data		: in std_logic_vector(LINK_WIDTH-1 downto 0);
		east_in_b_ack		: out std_logic;
		south_in_f_req		: in std_logic;
		south_in_f_data		: in std_logic_vector(LINK_WIDTH-1 downto 0);
		south_in_b_ack		: out std_logic;
		west_in_f_req		: in std_logic;
		west_in_f_data		: in std_logic_vector(LINK_WIDTH-1 downto 0);
		west_in_b_ack		: out std_logic;

		-- Output ports
		north_out_f_req		: out std_logic;
		north_out_f_data	: out std_logic_vector(LINK_WIDTH-1 downto 0);
		north_out_b_ack		: in std_logic;
		east_out_f_req		: out std_logic;
		east_out_f_data		: out std_logic_vector(LINK_WIDTH-1 downto 0);
		east_out_b_ack		: in std_logic;
		south_out_f_req		: out std_logic;
		south_out_f_data  	: out std_logic_vector(LINK_WIDTH-1 downto 0);
		south_out_b_ack		: in std_logic;
		west_out_f_req		: out std_logic;
		west_out_f_data   	: out std_logic_vector(LINK_WIDTH-1 downto 0);
		west_out_b_ack    	: in std_logic
  );
end noc_node_wrapper;

architecture struct of noc_node_wrapper is
	component noc_node is
	generic (
		MASTER : boolean := false
	);
	port (
		clk			: in std_logic;
		reset		: in std_logic;
		supervisor	: in std_logic;
		run 		: in std_logic;
		master_run	: out std_logic;

		proc_m		: in ocp_io_m;
		proc_s      : out ocp_io_s;

		spm_m		: out mem_if_master;
		spm_s		: in mem_if_slave;
		irq			: out std_logic_vector(1 downto 0);

		-- router ports
		north_in_f       : in channel_forward;
		north_in_b       : out channel_backward;
		east_in_f        : in channel_forward;
		east_in_b        : out channel_backward;
		south_in_f       : in channel_forward;
		south_in_b       : out channel_backward;
		west_in_f        : in channel_forward;
		west_in_b        : out channel_backward;

		-- Output ports
		north_out_f      : out channel_forward;
		north_out_b      : in channel_backward;
		east_out_f       : out channel_forward;
		east_out_b       : in channel_backward;
		south_out_f      : out channel_forward;
		south_out_b      : in channel_backward;
		west_out_f       : out channel_forward;
		west_out_b       : in channel_backward
	);
	end component;

	signal spm_s_rdata : unsigned((2*WORD_WIDTH)-1 downto 0);
	signal spm_m_wdata : unsigned((2*WORD_WIDTH)-1 downto 0);
	
begin

	spm_s_rdata <= unsigned(spm_S_Data);
	spm_M_Data <= std_logic_vector(spm_m_wdata);

gen_master: if(MASTER=1) generate
	noc_node_inst: noc_node
	generic map(
		MASTER => true
	)
	port map(
		clk			=> clk,
		reset		=> reset,
		supervisor 	=> supervisor,
		run 		=> run,
		master_run 	=> masterRun,
		irq 		=> irq,

		proc_m.MCMd 		=> proc_M_Cmd,
		proc_m.MAddr 		=> proc_M_Addr,
		proc_m.MData 		=> proc_M_Data,
		proc_m.MByteEn 		=> proc_M_ByteEn,
		proc_m.MRespAccept 	=> proc_M_RespAccept,
		proc_s.SResp 		=> proc_S_Resp,
		proc_s.SData 		=> proc_S_Data,
		proc_s.SCmdAccept 	=> proc_S_CmdAccept,

		spm_m.addr 	=> spm_M_Addr,
		spm_m.en 	=> spm_M_En,
		spm_m.wr 	=> spm_M_Wr,
		spm_m.wdata => spm_m_wdata,
		spm_s.rdata => spm_s_rdata,
		spm_s.error => spm_S_Error,
		
	  -- router ports
	  	north_in_f.req 	=> north_in_f_req,
		north_in_f.data => north_in_f_data,
		north_in_b.ack 	=> north_in_b_ack,
		east_in_f.req 	=> east_in_f_req,
		east_in_f.data 	=> east_in_f_data,
		east_in_b.ack 	=> east_in_b_ack,
		south_in_f.req 	=> south_in_f_req,
		south_in_f.data => south_in_f_data,
		south_in_b.ack 	=> south_in_b_ack,
		west_in_f.req 	=> west_in_f_req,
		west_in_f.data 	=> west_in_f_data,
		west_in_b.ack 	=> west_in_b_ack,

		-- Output ports
		north_out_f.req  => north_out_f_req,
		north_out_f.data => north_out_f_data,
		north_out_b.ack  => north_out_b_ack,
		east_out_f.req 	 => east_out_f_req,
		east_out_f.data  => east_out_f_data,
		east_out_b.ack 	 => east_out_b_ack,
		south_out_f.req	 => south_out_f_req,
		south_out_f.data => south_out_f_data,
		south_out_b.ack	 => south_out_b_ack,
		west_out_f.req 	 => west_out_f_req,
		west_out_f.data  => west_out_f_data,
		west_out_b.ack 	 => west_out_b_ack
	);
end generate;

gen_slave: if(MASTER=0) generate
	noc_node_inst: noc_node
	generic map(
		MASTER => false
	)
	port map(
		clk			=> clk,
		reset		=> reset,
		supervisor 	=> supervisor,
		run 		=> run,
		master_run 	=> masterRun,
		irq 		=> irq,

		proc_m.MCMd 		=> proc_M_Cmd,
		proc_m.MAddr 		=> proc_M_Addr,
		proc_m.MData 		=> proc_M_Data,
		proc_m.MByteEn 		=> proc_M_ByteEn,
		proc_m.MRespAccept  => proc_M_RespAccept,
		proc_s.SResp 		=> proc_S_Resp,
		proc_s.SData 		=> proc_S_Data,
		proc_s.SCmdAccept 	=> proc_S_CmdAccept,

		spm_m.addr 	=> spm_M_Addr,
		spm_m.en 	=> spm_M_En,
		spm_m.wr 	=> spm_M_Wr,
		spm_m.wdata => spm_m_wdata,
        spm_s.rdata => spm_s_rdata,
		spm_s.error => spm_S_Error,
		
	  -- router ports
	  	north_in_f.req 	=> north_in_f_req,
		north_in_f.data => north_in_f_data,
		north_in_b.ack 	=> north_in_b_ack,
		east_in_f.req 	=> east_in_f_req,
		east_in_f.data 	=> east_in_f_data,
		east_in_b.ack 	=> east_in_b_ack,
		south_in_f.req 	=> south_in_f_req,
		south_in_f.data => south_in_f_data,
		south_in_b.ack 	=> south_in_b_ack,
		west_in_f.req 	=> west_in_f_req,
		west_in_f.data 	=> west_in_f_data,
		west_in_b.ack 	=> west_in_b_ack,

		-- Output ports
		north_out_f.req 	=> north_out_f_req,
		north_out_f.data 	=> north_out_f_data,
		north_out_b.ack 	=> north_out_b_ack,
		east_out_f.req 		=> east_out_f_req,
		east_out_f.data 	=> east_out_f_data,
		east_out_b.ack 		=> east_out_b_ack,
		south_out_f.req		=> south_out_f_req,
		south_out_f.data 	=> south_out_f_data,
		south_out_b.ack		=> south_out_b_ack,
		west_out_f.req 		=> west_out_f_req,
		west_out_f.data 	=> west_out_f_data,
		west_out_b.ack 		=> west_out_b_ack
	);
end generate;

end struct;


