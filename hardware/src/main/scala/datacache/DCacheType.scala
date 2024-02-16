package datacache

import Chisel._
import ocp._
import patmos.Constants._
import patmos.DataCachePerf

class DCacheType(burstLength: Int) extends Module {
  val io = IO(new Bundle {
    val master = new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH)
    val slave = new OcpBurstMasterPort(ADDR_WIDTH, DATA_WIDTH, burstLength)
    val invalidate = Input(Bool())
    val perf = new DataCachePerf()
  })
}
