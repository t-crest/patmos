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
  
  override def toString = description + " at " + (frequency/1000000).toString() + " MHz"
  
  def toXML =
    <processor>
      <description>{description}</description>
      <frequency>{frequency}</frequency>  	
    </processor>

}

object Config {
  
  def fromXML(node: scala.xml.Node): Config =
    new Config {
      val description = (node \ "description").text
      val frequency = (node \ "frequency").text.toInt
  }
  
  def load(file: String) = {
    val node = xml.XML.loadFile(file)
    fromXML(node)
  }
  
  /** Helper main to avoid writing an XML file by hand */ 
  def main(args: Array[String]) = {
    
    val processor = new Config {
      val description = "default configuration"
      val frequency = 80000000
    }
    
    println(processor.toString())
    println(processor.toXML.toString)
    scala.xml.XML.save("default.xml", processor.toXML)
    
    val config = load("default2.xml")
    
    println(config)
  }
}