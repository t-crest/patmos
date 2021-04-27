package ni

import chisel3._
import chisel3.util._
import ocp._
import argo._
import java.lang.management.MemoryNotificationInfo
import chisel3.experimental.BundleLiterals._

class Spm_bus(argoConf: ArgoConfig) extends Module {
  val io = IO(new Bundle {
    val spm_slv = Input(new MemIFSlave())
    val spm = Output(new MemIFMaster(argoConf.HEADER_FIELD_WIDTH, argoConf.HEADER_CTRL_WIDTH))
    val tx_spm_slv = Output(new MemIFSlave())
    val tx_spm = Input(new MemIFMaster(argoConf.HEADER_FIELD_WIDTH, argoConf.HEADER_CTRL_WIDTH))
    val rx_spm = Input(new MemIFMaster(argoConf.HEADER_FIELD_WIDTH, argoConf.HEADER_CTRL_WIDTH))
  })

  // Signal select
  // Could probably be rewritten using Switch
  // for improved readability
  when ( io.tx_spm.En(1,0) =/= 0.U ) {
    io.spm := io.spm_slv
  } .otherwise {
    when ( io.rx_spm.En(1,0) =/= 0.U ) {
      io.spm := rx_buffer_reg
    } .otherwise {
      when ( io.rx_spm.En(1,0) =/= 0.U ) {
        io.spm := io.rx_spm
      } .otherwise {
        io.spm := io.tx_spm
      }
    }
  }
  
  // Register file
  //val rx_buffer_reg = Reg(0.U.asTypeOf(
  //  new MemIFMaster(argoConf.HEADER_FIELD_WIDTH, argoConf.HEADER_CTRL_WIDTH)))

  // Check whether this reset method touches other wire values (unintended behaviour)
  val rx_buffer_reg = RegInit((
    new MemIFMaster(argoConf.HEADER_FIELD_WIDTH, argoConf.HEADER_CTRL_WIDTH)).Lit(_.En -> 0.U))

  rx_buffer_reg.En(0) := io.rx_spm.En(0) && (io.tx_spm.En(0) || io.tx_spm.En(1))
  rx_buffer_reg.En(1) := io.rx_spm.En(1) && (io.tx_spm.En(0) || io.tx_spm.En(1))
  rx_buffer_reg.Wr := io.rx_spm.Wr
  rx_buffer_reg.Addr := io.rx_spm.Addr
  rx_buffer_reg.Data := io.rx_spm.Data
}
