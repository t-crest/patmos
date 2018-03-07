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

import Const._
import scala.util.Random


object Schedule {

  def getSchedule(n: Int) = {

    val s = n match {
      case 2 => ScheduleTable.FourNodes
      case 3 => ScheduleTable.NineNodes
      case 4 => ScheduleTable.SixTeenNodes
      case _ => throw new Error("Currently only 2x2, 3x3, and 4x4 NoCs supported, you requested: "+n+"x"+n)
    }

    def port(c: Char) = {
      c match {
        case 'n' => NORTH
        case 'e' => EAST
        case 's' => SOUTH
        case 'w' => WEST
        case 'l' => LOCAL
        case ' ' => 0
      }
    }

    def nextFrom(c: Char) = {
      c match {
        case 'n' => 's'
        case 'e' => 'w'
        case 's' => 'n'
        case 'w' => 'e'
        case 'l' => 'x' // no next for the last
        case ' ' => 'l' // stick to l on empty/waiting slots
      }
    }

    val split = s.split('|')
    val len = split.reduceLeft((a, b) => if (a.length > b.length) a else b).length
    val schedule = new Array[Array[Int]](len)
    val valid = new Array[Boolean](len)
    for (i <- 0 until len) {
      schedule(i) = new Array[Int](NR_OF_PORTS)
    }
    for (i <- 0 until split.length) {
      var from = 'l'
      for (j <- 0 until split(i).length) {
        val to = split(i)(j)
        if (to != ' ') {
          schedule(j)(port(to)) = port(from)
          from = nextFrom(to)
        }
      }
    }
    var line = 0
    for (i <- 0 until len - 1) {
      valid(i) = split(line)(i) != ' '
      if (valid(i)) line += 1
    }
    println("Schedule is " + schedule.length + " clock cycles")
    (schedule, valid)
  }

  /* A 2x2 schedule is as follows:
ne
  n
   e
 */

  def gen2x2Schedule() = {
    Array(Array(LOCAL, 0, 0, 0, 0), // P1: enter from local and exit to north register
      Array(0, SOUTH, 0, 0, 0), // P1: enter from south and exit to east register
      Array(LOCAL, 0, 0, 0, WEST), // P2: local to north, P1: from west to local
      Array(0, LOCAL, 0, 0, SOUTH), // P3: local to east, P2: south to local
      Array(0, 0, 0, 0, WEST)) // P3: from west to local
    // The last drain from west to local increases the schedule length by 1,
    // but could be overlapped.
    // Which means having it in the first slot, as there is no exit in the first slot.
  }

  def genRandomSchedule(slen: Int) = {
    val schedule = new Array[Array[Int]](slen)
    for (i <- 0 until slen) {
      val oneSlot = new Array[Int](NR_OF_PORTS)
      for (j <- 0 until NR_OF_PORTS) {
        oneSlot(j) = Random.nextInt(5)
      }
      schedule(i) = oneSlot
    }
    schedule
  }

  def main(args: Array[String]): Unit = {
    print(getSchedule(2))
  }

}
