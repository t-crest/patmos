package ni

import chisel3._
import chisel3.util._
import ocp._
//import ocp.Ocp_io_m
import argo._
import java.lang.management.MemoryNotificationInfo
import chisel3.experimental.BundleLiterals._

class Config_bus(argoConf: ArgoConfig) extends Module {
  val io = IO(new Bundle {
    val ocp_config_m = Input(new Ocp_io_m())
    val ocp_config_s = Output(new Ocp_io_s())
    val supervisor = Input(Bool())
    val config_unit = Input(new ConfIFMaster(argoConf.HEADER_FIELD_WIDTH, argoConf.HEADER_CTRL_WIDTH))
    val config = Output(new ConfIFMaster(argoConf.HEADER_FIELD_WIDTH, argoConf.HEADER_CTRL_WIDTH))
    val TDM_ctrl = Input(new ConfIFSlave())
    val TDM_ctrl_sel = Output(Bool())
    val sched_tbl = Input(new ConfIFSlave())
    val sched_tbl_sel = Output(Bool())
    val DMA_tbl = Input(new ConfIFSlave())
    val DMA_tbl_sel = Output(Bool())
    val MC_ctrl = Input(new ConfIFSlave())
    val MC_ctrl_sel = Output(Bool())
    val irq_unit_fifo = Input(new ConfIFSlave())
    val irq_unit_fifo_sel = Output(Bool())
  })

  // FIX!!!!
  

  def HEADER_FIELD_WIDTH: Int = argoConf.HEADER_FIELD_WIDTH
  def HEADER_CTRL_WIDTH: Int = argoConf.HEADER_CTRL_WIDTH

  val next_ocp_resp = Wire(UInt(OCP_RESP_WIDTH.W))
  val bank_id  = Wire(UInt(Ocp.CPKT_BANK_WIDTH.W))

  // Arbitrates access to the configuration ports on the memories.
  // Priority is given to the config unit, if no request from the config unit
  // the OCP port gets access.
  // Starvation cannot happen since the config unit can only write twice in
  // three clock cycles
  
  //Default values of signals
  //The config unit is served as default
  io.ocp_config_s.SResp := ocp_resp_reg
  io.ocp_config_s.SData := io.DMA_tbl.Data(31, 0)
  io.ocp_config_s.SCmdAccept := 0.U

  bank_id := io.config_unit.Addr(HEADER_FIELD_WIDTH-HEADER_CTRL_WIDTH-1,
                          HEADER_FIELD_WIDTH-HEADER_CTRL_WIDTH-Ocp.CPKT_BANK_WIDTH)
  io.config.Addr := io.config_unit.Addr
  io.config.En := io.config_unit.En
  io.config.Wr := io.config_unit.Wr
  io.config.Data := io.config_unit.Data

  // keep the value of the response register
  // next_ocp_resp <= ocp_resp_reg;

  
  // If no request from the config unit and a request from the OCP port
  when (io.config_unit.En === 0.U && io.ocp_config_m.MCmd =/= OCP_CMD_IDLE) {
    // OCP transaction is completed in the next clock cycle
    next_ocp_resp := Ocp.OCP_RESP_DVA
    io.ocp_config_s.SCmdAccept := 1.U
    io.config.Data(31, 0) := io.ocp_config_m.MData

    bank_id := unsigned(ocp_config_m.MAddr(HEADER_FIELD_WIDTH-
                                  HEADER_CTRL_WIDTH+2-1, HEADER_FIELD_WIDTH-
                                            HEADER_CTRL_WIDTH-Ocp.CPKT_BANK_WIDTH+2));
    io.config.Addr := io.ocp_config_m.MAddr(HEADER_FIELD_WIDTH-
                                                HEADER_CTRL_WIDTH+2-1, 2)
    io.config.En := 1.U
    io.config.Wr := 0.U
    // If OCP request is a write and the processor is in supervisor mode
    when (io.ocp_config_m.MCmd =/= OCP_CMD_IDLE && supervisor === 1.U) {
      when (io.ocp_config_m.MCmd === OCP_CMD_WR) {
        io.config.Wr := 1.U
      }
    .otherwise
      //PUT THIS BACK next_ocp_resp <= OCP_RESP_ERR;
      next_ocp_resp := Ocp.OCP_RESP_DVA
    }
  }

  // Hold the OCP response signal valid until the response accept from the
  // master
  // TODO: Hold the read data as well
  when ((ocp_resp_reg =/= Ocp.OCP_RESP_NULL) && (io.ocp_config_m.MRespAccept === 1.U)) {
    next_ocp_resp := Ocp.OCP_RESP_NULL
  }

  when ((io.TDM_ctrl.Error || io.sched_tbl.Error || io.DMA_tbl.Error || io.irq_unit_fifo.Error) === 1.U) {
    //PUT THIS BACK ocp_config_s.SResp <= OCP_RESP_ERR;
  }

  //Default select no bank
  io.TDM_ctrl_sel := 0.U
  io.MC_ctrl_sel := 0.U
  sched_tbl_sel := 0.U 
  io.DMA_tbl_sel := 0.U 
  io.irq_unit_fifo_sel := 0.U
  // Based on the current bank_id we select which bank to activate
  switch ( bank_id ) {  
    is (DMA_BANK) {
      DMA_tbl_sel := 1.U 
    } is (SCHED_BANK) {
      sched_tbl_sel := 1.U 
    } is (TDM_BANK) {
      TDM_ctrl_sel := 1.U 
    } is(MC_BANK) {
      MC_ctrl_sel := 1.U 
    } is (IRQ_BANK) {
      irq_unit_fifo_sel := 1.U
    } is (PERF_BANK) {
    }
  }


  switch ( prev_bank_id ) {
    is (DMA_BANK) {
      ocp_config_s.SData := DMA_tbl.Data(31, 0)
    } is (SCHED_BANK) {
      ocp_config_s.SData := sched_tbl.Data(31, 0)
    } is (TDM_BANK) {
      ocp_config_s.SData := TDM_ctrl.Data(31, 0)
    } is (MC_BANK) {
      ocp_config_s.SData := MC_ctrl.Data(31, 0)
    } is (IRQ_BANK) {
      ocp_config_s.SData := irq_unit_fifo.Data(31, 0)
    } is (PERF_BANK) {
    }
  }

  val ocp_resp_reg = RegInit(Ocp.OCP_RESP_NULL)
  val prev_bank_id = RegInit(0.U)

  ocp_resp_reg := next_ocp_resp
  prev_bank_id := bank_id

  // config_arbiter_reg : process(clk)
  // begin
  //   if rising_edge(clk) then
  //     if reset = '1' then
  //       ocp_resp_reg <= OCP_RESP_NULL;
  //       prev_bank_id <= (others=>'0');
  //     else
  //       ocp_resp_reg <= next_ocp_resp;
  //       prev_bank_id <= bank_id;
  //     end if ;
  //   end if;
  // end process;

}
