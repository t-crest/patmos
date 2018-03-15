/*
 * Copyright: 2017, Technical University of Denmark, DTU Compute
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * License: Simplified BSD License
 * 
 * Schedules for the S4NOC, as described in:
 * 
 * Florian Brandner and Martin Schoeberl,
 * Static Routing in Symmetric Real-Time Network-on-Chips,
 * In Proceedings of the 20th International Conference on Real-Time
 * and Network Systems (RTNS 2012), 2012, 61-70
 * 
 * Available at:
 * https://github.com/t-crest/s4noc/tree/master/noc/vhdl/generated
 */

package s4noc

import scala.io.Source

object ScheduleTable {

  val FourNodes =
    "nel|" +
    "  nl|" +
    "   el|"

  val NineNodes =
    "nel|" +
    " nwl|" +
    "  esl|" +
    "   wsl|" +
    "     nl|" +
    "      el|" +
    "       sl|" +
    "        wl|"

  val SixTeenNodes =
    "nneel|" +
    " esl|" +
    "   neel|" +
    "    nnel|" +
    "     wnnl|" +
    "       eesl|" +
    "        nl|" +
    "         nel|" +
    "          nwl|" +
    "           nnl|" +
    "            eel|" +
    "             swl|" +
    "               el|" +
    "                sl|" +
    "                 wl|"

  def main(args: Array[String]): Unit = {
    var cnt = Source.fromFile(args(0)).getLines.length
    val lines = Source.fromFile(args(0)).getLines
    for (l <- lines) {
      val end = if (cnt > 1) " +" else ""
      println("    \"" + l + "l|\"" + end)
      cnt -= 1
    }
  }

}
