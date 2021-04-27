package ni

import chisel3._
import chisel3.util._
import ocp._
import argo._
import java.lang.management.MemoryNotificationInfo
import chisel3.experimental.BundleLiterals._
import chisel3.experimental.ChiselEnum

// Rx_unit FSM states
object Rx_unit {
  object State extends ChiselEnum {
    val sIdle, sDataWLow, sDataWHigh, sConfigWLow, sConfigWHigh, sIrqW = Value
  }
}

// Rx_unit module
class Rx_unit(argoConf: ArgoConfig) extends Module {
  import Rx_unit.State
  import Rx_unit.State._

  val io = IO(new Bundle {
    // Memory bus
    val spm = Output(new MemIFMaster(argoConf.HEADER_FIELD_WIDTH, argoConf.HEADER_CTRL_WIDTH))
    // Config bus (internal MEM) - NB: MemIfMaster is identical to ConfIfMaster
    val config = Output(new MemIFMaster(argoConf.HEADER_FIELD_WIDTH, argoConf.HEADER_CTRL_WIDTH))
    val irq_fifo_data = Output(UInt(argoConf.HEADER_WIDTH.W))
    val irq_fifo_data_valid = Output(Bool())
    val irq_fifo_irq_valid = Output(Bool())

    val pkt_in = Input(UInt(argoConf.LINK_WIDTH.W))
  })

  def H_FIELD_ROUTE_W: Int  = argoConf.HEADER_FIELD_WIDTH + argoConf.HEADER_ROUTE_WIDTH

  val new_pkt, new_data_pkt, new_config_pkt, new_irq_pkt = Wire(Bool())
  val wdata_high_en, wdata_low_en, addr_load, lst_data_pkt = Wire(Bool())
  val addr, next_addr, int_addr, next_int_addr = Wire(UInt(argoConf.HEADER_WIDTH.W))

  new_pkt := io.pkt_in(argoConf.LINK_WIDTH-1) && io.pkt_in(argoConf.LINK_WIDTH-2) && !(io.pkt_in(argoConf.LINK_WIDTH-3))

  new_data_pkt := new_pkt && (! io.pkt_in(argoConf.HEADER_FIELD_WIDTH + argoConf.HEADER_ROUTE_WIDTH - 2))
  new_config_pkt := new_pkt && !(io.pkt_in(argoConf.HEADER_FIELD_WIDTH + argoConf.HEADER_ROUTE_WIDTH - 1)) && io.pkt_in(argoConf.HEADER_FIELD_WIDTH + argoConf.HEADER_ROUTE_WIDTH - 2)

  new_irq_pkt := new_pkt && io.pkt_in(argoConf.HEADER_FIELD_WIDTH + argoConf.HEADER_ROUTE_WIDTH - 1) && io.pkt_in(argoConf.HEADER_FIELD_WIDTH + argoConf.HEADER_ROUTE_WIDTH - 2);

  //SPM and config output assignments
  io.spm.Data(31, 0) := io.pkt_in(31,0)
 	
  io.config.Data(31,0) := io.pkt_in(31, 0)
 	
  io.spm.Addr := addr;
  io.config.Addr := int_addr;

  //Signal irq_fifo_data_valid assignment, the IRQ FIFO push is delayed in order to happen with the last spm wr/en
  io.irq_fifo_data_valid := lst_data_pkt && io.pkt_in(argoConf.LINK_WIDTH - 3);

  // FSM register
  val state = RegInit(sIdle)

  // FSM combinatorial
  switch (state) {
    is (sIdle) {
      addr_load := 1.U
      next_int_addr := io.pkt_in(argoConf.HEADER_ROUTE_WIDTH + argoConf.HEADER_FIELD_WIDTH - argoConf.HEADER_CTRL_WIDTH - 1, argoConf.HEADER_ROUTE_WIDTH)
      when (new_data_pkt === 1.U) {
	state := sDataWHigh
      } .elsewhen (new_config_pkt === 1.U) {
	state := sConfigWHigh
      } .elsewhen (new_irq_pkt === 1.U) {
	state := sIrqW
      }
    }
    is (sDataWHigh) {
      next_int_addr := addr + 1.U
      when (io.pkt_in(argoConf.LINK_WIDTH - 3) === 0.U) {
	wdata_high_en := 1.U
        state := sDataWLow
      } .otherwise {
	io.spm.En(0) := 1.U;
        io.spm.Wr := 1.U
        io.spm.Data(63, 32) := io.pkt_in(31, 0)
        state := sIdle
      }
    }
    is (sDataWLow) {
      io.spm.En := "b11".U 
      io.spm.Wr := 1.U
      io.irq_fifo_data := int_addr
      next_addr := next_int_addr
      when (io.pkt_in(argoConf.LINK_WIDTH - 3) === 0.U) {
	state <= sDataWHigh
      } . otherwise {
	state := sIdle
      }
    }
    is (sConfigWHigh) {
      io.config.En := 1.U
      io.config.Wr := 1.U
      when (io.pkt_in(argoConf.LINK_WIDTH - 3) === 0.U) {
	state := sConfigWLow
      } .otherwise {
	state := sIdle;
      }
    }
    is (sConfigWLow) {
      io.config.En := 1.U
      io.config.Wr := 1.U
      when (io.pkt_in(argoConf.LINK_WIDTH - 3) === 0.U) {
	state := sConfigWLow
    } .otherwise {
	state := sIdle
      }
    }
    is (sIrqW) {
      io.spm.En(0) := 1.U
      io.spm.Wr := 1.U
      io.irq_fifo_irq_valid := 1.U
      io.spm.Data(63, 32) := io.pkt_in(31, 0)
      state := sIdle;
    }
  }

  val up_cnt_reg = Reg(UInt())
  up_cnt_reg := Mux(addr_load === 1.U, io.pkt_in(argoConf.HEADER_ROUTE_WIDTH + argoConf.HEADER_FIELD_WIDTH - argoConf.HEADER_CTRL_WIDTH - 1, argoConf.HEADER_ROUTE_WIDTH), next_addr)

  val en_reg = RegEnable(io.pkt_in(31,0), wdata_high_en)

  val gp_reg = RegInit(0.U(UInt()))
  gp_reg := next_int_addr
  int_addr := gp_reg

  val lst_pkt_dff = RegInit(0.U(UInt()))
  when (((!(new_data_pkt && io.pkt_in(argoConf.HEADER_FIELD_WIDTH + argoConf.HEADER_ROUTE_WIDTH - 1))) && io.pkt_in(argoConf.LINK_WIDTH - 3)) === 1.U) {
    lst_pkt_dff := 0.U
  } .elsewhen ((new_data_pkt && io.pkt_in(argoConf.HEADER_FIELD_WIDTH + argoConf.HEADER_ROUTE_WIDTH - 1)) === 1.U) {
    lst_pkt_dff := 1.U
  }
}
