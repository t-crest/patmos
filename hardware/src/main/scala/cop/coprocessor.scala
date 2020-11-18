
package cop

import Chisel._

import patmos._
import patmos.Constants._
import util._
import ocp._

abstract class CoprocessorObject() {
  // every device object must have methods "create" and "init"
  def init(params: Map[String, String])
  def create(params: Map[String, String]) : Coprocessor
}

abstract class Coprocessor() extends Module() {
  
}

abstract class BaseCoprocessor() extends Coprocessor(){
    override val io = IO(new Bundle() {
      val copIn = new PatmosToCoprocessor().asInput
      val copOut = new CoprocessorToPatmos().asOutput  
    })
}

abstract class Coprocessor_MemoryAccess() extends Coprocessor(){
    override val io = IO(new Bundle() {
        val copIn = new PatmosToCoprocessor().asInput
        val copOut = new CoprocessorToPatmos().asOutput
        val memPort = new OcpBurstMasterPort(EXTMEM_ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH)  
    })
}