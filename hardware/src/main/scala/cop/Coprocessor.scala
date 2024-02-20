/*
 * Patmos Accelerator Interface base classes.
 *
 * Authors: Christoph Lehr (christoph.lehr@gmx.at)
 *          Alexander Baranyai (alexyai@gmx.at)
 *          Clemens Pircher (clemens.lukas@gmx.at)
 *
 */

package cop

import chisel3._
import chisel3.util._

import patmos._
import patmos.Constants._
import ocp._

abstract class CoprocessorObject() {
  // every device object must have methods "create" and "init"
  def init(params: Map[String, String])
  def create(params: Map[String, String]) : Coprocessor
}

abstract class Coprocessor() extends Module() {
  
}

abstract class BaseCoprocessor() extends Coprocessor(){
    val io = IO(new Bundle() {
      val copIn = Input(new PatmosToCoprocessor())
      val copOut = Output(new CoprocessorToPatmos())
    })
}

abstract class CoprocessorMemoryAccess() extends Coprocessor(){
    val io = IO(new Bundle() {
        val copIn = Input(new PatmosToCoprocessor())
        val copOut = Output(new CoprocessorToPatmos())
        val memPort = new OcpBurstMasterPort(EXTMEM_ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH)  
    })
}
