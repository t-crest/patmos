package cache

import Chisel._
import Node._
import scala.math._

object CacheCons {

  val DIRECT_MAP = 0
  val ADDRESS_WIDTH = 32
  val NR_CACHE_LINES = 4  				//Number of cache lines
  val NUMBER_OF_INSTRUCTIONS = 4   			//Number of instruction in one cache line
  val CACHE_LINE_SIZE = (NUMBER_OF_INSTRUCTIONS * 32) 	//Cache line size
  val BYTE_OFFSET = 2					//For 32bit instructions
  val BLOCK_OFFSET = log2Up(NUMBER_OF_INSTRUCTIONS)	//Bits for selecting instruction within a cache line
//  if (DIRECT_MAP = 1) {
//  	val INDEX_CACHE = log2Up(NR_CACHE_LINES)	//Number of bits from addresses for cache indexing
//  } else {
	val INDEX_CACHE = 0
//  }
  val TAG_CACHE = (ADDRESS_WIDTH - INDEX_CACHE - BLOCK_OFFSET - BYTE_OFFSET)	//Tag bits size
  val CACHE_LINE_ADDR_WIDTH = (ADDRESS_WIDTH - BLOCK_OFFSET - BYTE_OFFSET)	//Address bits for cache line 
  val INSTRUCTION_OFFSET = ADDRESS_WIDTH - CACHE_LINE_ADDR_WIDTH
}
