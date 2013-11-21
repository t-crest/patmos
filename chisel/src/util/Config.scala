/*
   Copyright 2013 Technical University of Denmark, DTU Compute. 
   All rights reserved.
   
   This file is part of the time-predictable VLIW processor Patmos.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

      1. Redistributions of source code must retain the above copyright notice,
         this list of conditions and the following disclaimer.

      2. Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
   NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   The views and conclusions contained in the software and documentation are
   those of the authors and should not be interpreted as representing official
   policies, either expressed or implied, of the copyright holder.
 */

package util

/**
 * A tiny configuration tool for Patmos.
 * 
 * Authors: Martin Schoeberl (martin@jopdesign.com)
 */
abstract class Config {
  val description: String
  val frequency: Int
  val pipeCount: Int

  case class MCacheConfig(size: Int, blocks: Int, repl: String)
  val MCache: MCacheConfig
  case class DCacheConfig(size: Int, assoc: Int, repl: String)
  val DCache: DCacheConfig  
  case class SCacheConfig(size: Int)
  val SCache: SCacheConfig

  case class SPMConfig(size: Int)
  val ISPM: SPMConfig
  val DSPM: SPMConfig
  val BootSPM: SPMConfig

  override def toString =
    description + " at " + (frequency/1000000).toString() + " MHz"
}

object Config {
  
  def parseSize(text: String): Integer = {
	  val regex = """(\d+)([KMG]?)""".r	  
	  val suffixMult = Map("" -> (1 << 0),
						   "K" -> (1 << 10),
						   "M" -> (1 << 20),
						   "G" -> (1 << 30))
	  val regex(num, suffix) = text.toUpperCase
	  num.toInt * suffixMult.getOrElse(suffix, 0)
  }

  def fromXML(node: scala.xml.Node): Config =
    new Config {
      val description = (node \ "description").text
      val frequency = ((node \ "frequency")(0) \ "@Hz").text.toInt
      val dual = ((node \ "pipeline")(0) \ "@dual").text.toBoolean
      val pipeCount = if (dual) 2 else 1
      val MCacheNode = (node \ "MCache")(0)
      val MCache =
        new MCacheConfig(parseSize((MCacheNode \ "@size").text),
                         (MCacheNode \ "@blocks").text.toInt,
                         (MCacheNode \ "@repl").text)
      val DCacheNode = (node \ "DCache")(0)
      val DCache =
        new DCacheConfig(parseSize((DCacheNode \ "@size").text),
                         (DCacheNode \ "@assoc").text.toInt,
                         (DCacheNode \ "@repl").text)
      val SCacheNode = (node \ "SCache")(0)
      val SCache =
        new SCacheConfig(parseSize((SCacheNode \ "@size").text))

      val ISPM = new SPMConfig(parseSize(((node \ "ISPM")(0) \ "@size").text))
      val DSPM = new SPMConfig(parseSize(((node \ "DSPM")(0) \ "@size").text))
      val BootSPM = new SPMConfig(parseSize(((node \ "BootSPM")(0) \ "@size").text))
    }
  
  // This is probably not the best way to have the singleton
  // for the configuration available and around.
  // We also do not want this to be a var.
  var conf: Config = new Config {
    val description = "dummy"
    val frequency = 0
    val pipeCount = 0
    val MCache = new MCacheConfig(0, 0, "")
    val DCache = new DCacheConfig(0, 0, "")
    val SCache = new SCacheConfig(0)
    val ISPM = new SPMConfig(0)
    val DSPM = new SPMConfig(0)
    val BootSPM = new SPMConfig(0)
  }
  
  def load(file: String): Config = {
    val node = xml.XML.loadFile(file)
    fromXML(node)
  }
}
