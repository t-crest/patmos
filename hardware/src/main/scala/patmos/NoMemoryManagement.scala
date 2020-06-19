/*
 * A memory management unit for Patmos.
 *
 * Author: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package patmos

import Chisel._

class NoMemoryManagement extends Module {
  val io = IO(new MMUIO())

  // just connect virtual and physical end
  io.phys.M <> io.virt.M
  io.virt.S <> io.phys.S
}
