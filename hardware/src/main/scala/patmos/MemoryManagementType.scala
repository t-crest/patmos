/*
 * A memory management unit for Patmos.
 *
 * Author: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package patmos

import chisel3._

class MemoryManagementType extends Module {
  val io = IO(new MMUIO())
}
