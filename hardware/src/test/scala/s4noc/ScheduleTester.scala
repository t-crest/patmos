/*
 * Copyright: 2017, Technical University of Denmark, DTU Compute
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * License: Simplified BSD License
 * 
 */

package s4noc

object ScheduleTester {

  def main(args: Array[String]): Unit = {
    println("Testing the schedule creation")
    val sref = Schedule.gen2x2Schedule()
    println(ScheduleTable.FourNodes)
    val stest = Schedule.getSchedule(2)._1
    for (i <- 0 until sref.length) {
      val slotref = sref(i)
      val slottest = stest(i)
      for (j <- 0 until slotref.length) {
        assert(slotref(j) == slottest(j))
      }
    }
    println("Schedule test PASSED")
  }

}
