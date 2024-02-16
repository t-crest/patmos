/*
 * A memory management unit for Patmos.
 *
 * Author: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package patmos

import chisel3._

class NoMemoryManagement extends MemoryManagementType {
  // just connect virtual and physical end
  io.phys.M <> io.virt.M
  io.virt.S <> io.phys.S
  io.ctrl.S.Resp := DontCare
  io.ctrl.S.Data := DontCare
}
