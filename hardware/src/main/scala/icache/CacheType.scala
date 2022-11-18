/*
 * Method cache without actual functionality
 *
 * Authors: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *        Philipp Degasperi (philipp.degasperi@gmail.com)
 */

package patmos

import chisel3._

abstract class CacheType() extends Module {
  val io = IO(new ICacheIO())
}
