package datacache

import Chisel._
import ocp._
import patmos.Constants._
import patmos.WriteCombinePerf

class WriteBufferType() extends Module {
  val io = IO(new Bundle {
    val readMaster = new OcpBurstSlavePort(ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH)
    val writeMaster = new OcpCacheSlavePort(ADDR_WIDTH, DATA_WIDTH)
    val slave = new OcpBurstMasterPort(ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH)
    val perf = new WriteCombinePerf()
  })
}
