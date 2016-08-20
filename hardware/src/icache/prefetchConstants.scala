package patmos

import Chisel._
import Node._
import PIConstants._
import Constants._
import scala.io.Source
import scala.math._


object PrefetchCons {
  
  //Reading the rpt.txt file
  val readRPT = Source.fromFile("src/icache/rpt.txt").getLines().map(_.split(" ")).toArray
  val MAX_INDEX_RPT = (readRPT.length - 1)
   
  var index_array = new Array[Int](MAX_INDEX_RPT)
  var trigger_array = new Array[Int](MAX_INDEX_RPT)
  var type_array = new Array[Int](MAX_INDEX_RPT)
  var destination_array = new Array[Int](MAX_INDEX_RPT)
  var iteration_array = new Array[Int](MAX_INDEX_RPT)
  var next_array = new Array[Int](MAX_INDEX_RPT)
  var count_array = new Array[Int](MAX_INDEX_RPT)
  var depth_array = new Array[Int](MAX_INDEX_RPT)
  var retdes_array = new Array[Int](MAX_INDEX_RPT) 
  
  for(i <- 1 to MAX_INDEX_RPT) {
    index_array(i-1) = (readRPT(i)(0)).toInt  
    trigger_array(i-1) = (readRPT(i)(1)).toInt
    type_array(i-1) = (readRPT(i)(2)).toInt
    destination_array(i-1) = (readRPT(i)(3)).toInt
    iteration_array(i-1) = (readRPT(i)(4)).toInt
    next_array(i-1) = (readRPT(i)(5)).toInt
    count_array(i-1) = (readRPT(i)(6)).toInt 
    depth_array(i-1) = (readRPT(i)(7)).toInt
    retdes_array(i-1) = (readRPT(i)(8)).toInt
  }

  var index_array_c = new Array[UInt](MAX_INDEX_RPT)
  var trigger_array_c = new Array[UInt](MAX_INDEX_RPT)
  var type_array_c = new Array[UInt](MAX_INDEX_RPT)
  var destination_array_c = new Array[UInt](MAX_INDEX_RPT)
  var iteration_array_c = new Array[UInt](MAX_INDEX_RPT)
  var next_array_c = new Array[UInt](MAX_INDEX_RPT)
  var count_array_c = new Array[UInt](MAX_INDEX_RPT)
  var depth_array_c = new Array[UInt](MAX_INDEX_RPT)
  var retdes_array_c = new Array[UInt](MAX_INDEX_RPT) 

  var INDEX_WIDTH = 3
  if(MAX_INDEX_RPT > 8)
    INDEX_WIDTH = log2Up(MAX_INDEX_RPT)
  
  var INDEX_REG_WIDTH = 3
  if((index_array.max) > 8)
    INDEX_REG_WIDTH = log2Up(index_array.max)
  
  var MAX_ITERATION_WIDTH = 5
  if((iteration_array.max) > 32)
    MAX_ITERATION_WIDTH = log2Up(iteration_array.max)
  
  var MAX_COUNT_WIDTH = 5
  if((count_array.max) > 32)
    MAX_COUNT_WIDTH = log2Up(count_array.max)
  
  var MAX_DEPTH = 3
  if((depth_array.max) > 3)
    MAX_DEPTH = depth_array.max
  
  var MAX_DEPTH_WIDTH = 3
  if(MAX_DEPTH > 8)
    MAX_DEPTH_WIDTH = log2Up(MAX_DEPTH)
  
  var MAX_SMALL_LOOP_WIDTH = 3
  if((count_array.max) > 8)
    MAX_SMALL_LOOP_WIDTH  = log2Up(count_array.max)  
  
  var MAX_LOOP_ITER_WIDTH = 3
  if((iteration_array.max) > 8)
    MAX_LOOP_ITER_WIDTH = log2Up(iteration_array.max)  

  //calculating the max number of call entries in RPT table
  var max_call = 1
  for(i <- 0 until MAX_INDEX_RPT)
    if(type_array(i) == 1)
      max_call += 1
  val MAX_CALLS = max_call

  //RPT columns
  def trigger_f():Vec[UInt] = {
    for (i <- 0 until MAX_INDEX_RPT)
      trigger_array_c(i) = UInt(trigger_array(i), width = (TAG_SIZE + INDEX_SIZE)) 
    Vec[UInt](trigger_array_c)
  }
  def type_f():Vec[UInt] = {
    for (i <- 0 until MAX_INDEX_RPT)
      type_array_c(i) = UInt(type_array(i), width = 2) 
    Vec[UInt](type_array_c)
  }
  def destination_f():Vec[UInt] = {
    for (i <- 0 until MAX_INDEX_RPT)
      destination_array_c(i) = UInt(destination_array(i), width = (TAG_SIZE + INDEX_SIZE)) 
    Vec[UInt](destination_array_c)
  }
  def iteration_f():Vec[UInt] = {
    for (i <- 0 until MAX_INDEX_RPT)
      iteration_array_c(i) = UInt(iteration_array(i), width = MAX_ITERATION_WIDTH) 
    Vec[UInt](iteration_array_c)
  }
  def next_f():Vec[UInt] = {
    for (i <- 0 until MAX_INDEX_RPT)
      next_array_c(i) = UInt(next_array(i), width = INDEX_WIDTH) 
    Vec[UInt](next_array_c)
  }
  def count_f():Vec[UInt] = {
    for (i <- 0 until MAX_INDEX_RPT)
      count_array_c(i) = UInt(count_array(i), width = MAX_COUNT_WIDTH) 
    Vec[UInt](count_array_c)
  }
  def depth_f():Vec[UInt] = {
    for (i <- 0 until MAX_INDEX_RPT)
      depth_array_c(i) = UInt(depth_array(i), width = MAX_DEPTH_WIDTH) 
    Vec[UInt](depth_array_c)
  }
  def retdes_f():Vec[UInt] = {
    for (i <- 0 until MAX_INDEX_RPT)
      retdes_array_c(i) = UInt(retdes_array(i), width = (TAG_SIZE + INDEX_SIZE)) 
    Vec[UInt](retdes_array_c)
  }

}    


//object LockCons {
  //Reading lock.txt
//  val lines_l = Source.fromFile("lock.txt").getLines().map(_.split(" ")).toArray
//  val MAX_INDEX_LOCK = (lines_l.length - 1)
//  val INDEX_WIDTH_LOCK = log2Up(MAX_INDEX_LOCK)
  
//  var index_array_l = new Array[Int](MAX_INDEX_LOCK)
//  var lock_addr_l = new Array[Int](MAX_INDEX_LOCK)
//  var unlock_addr_l = new Array[Int](MAX_INDEX_LOCK)

//  for(i <- 1 to MAX_INDEX_LOCK) {
//    index_array_l(i-1) = (lines_l(i)(0)).toInt
//    lock_addr_l(i-1) = (lines_l(i)(1)).toInt
//    unlock_addr_l(i-1) = (lines_l(i)(2)).toInt
//  }
  
//  var index_array_lc = new Array[UInt](MAX_INDEX_LOCK)
//  var lock_addr_lc = new Array[UInt](MAX_INDEX_LOCK)
//  var unlock_addr_lc = new Array[UInt](MAX_INDEX_LOCK)
  
//  def index_lf():Vec[UInt] = {
//    for(i <- 0 until MAX_INDEX_LOCK)
//      index_array_lc(i) = UInt(index_array_l(i), width = INDEX_WIDTH_LOCK)
//    Vec[UInt](index_array_lc)
//  }
//  def lock_addr_lf():Vec[UInt] = {
//    for(i <- 0 until MAX_INDEX_LOCK)
//      lock_addr_lc(i) = UInt(lock_addr_l(i), width = 32)
//    Vec[UInt](lock_addr_lc)
//  }
//  def unlock_addr_lf():Vec[UInt] = {
//    for(i <- 0 until MAX_INDEX_LOCK)
//      unlock_addr_lc(i) = UInt(unlock_addr_l(i), width = 32)
//    Vec[UInt](unlock_addr_lc)
//  }  
//}
