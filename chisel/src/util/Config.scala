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

import scala.tools.nsc.interpreter._
import scala.tools.nsc._

import Chisel._
import Node._

import patmos._

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

  case class ExtMemConfig(size: Int)
  val ExtMem: ExtMemConfig

  override def toString =
    description + " at " + (frequency/1000000).toString() + " MHz"
}

object Config {
  
  def parseSize(text: String): Int = {
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

      val ExtMem = new ExtMemConfig(parseSize(((node \ "ExtMem")(0) \ "@size").text))
    }

  // TODO: this list should come from the configuration file
  val traitList = List("UartPins", "LedPins")

  def connectIOPins(outer : Node, inner : Node) {
    for (name <- traitList) {
      // get class for pin trait
      val clazz = Class.forName("patmos."+name)
      if (clazz.isInstance(outer)) {
        // get method to retrieve pin bundle
        val methName = name(0).toLower + name.substring(1, name.length)
        val meth = clazz.getMethods.find(_.getName == methName).get
        // retrieve pin bundles
        val outerPins = meth.invoke(clazz.cast(outer))
        val innerPins = meth.invoke(clazz.cast(inner))
        // actually connect pins
        outerPins.asInstanceOf[Bundle] <> innerPins.asInstanceOf[Bundle]
      }
    }
  }

  def genTraitedClass[T](pack : String, base : String, list : List[String]) : T = {
    // build class definition
    val traitClass = list.foldLeft("Trait"+base)(_ + "_%s".format(_))
    val traitClassDef = "class "+traitClass+" extends "+pack+"."+base
    val classDef = list.foldLeft(traitClassDef)(_ + " with %s.%s".format(pack, _))

    // fire up a new Scala interpreter/compiler
    val settings = new Settings()
    settings.embeddedDefaults(this.getClass.getClassLoader())
    val interpreter = new IMain(settings)
    // define the new class
    interpreter.compileString(classDef)
    // load the new class
    val clazz = interpreter.classLoader.loadClass(traitClass)
    // get an instance with the right type
    clazz.newInstance().asInstanceOf[T] 
  }

  def getInOutIO() : InOutIO = {
    genTraitedClass[InOutIO]("patmos", "InOutIO", traitList)
  }

  def getPatmosCoreIO() : PatmosCoreIO = {
    genTraitedClass[PatmosCoreIO]("patmos", "PatmosCoreIO", traitList)
  }

  def getPatmosIO() : PatmosIO = {
    genTraitedClass[PatmosIO]("patmos", "PatmosIO", traitList)
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
    val ExtMem = new ExtMemConfig(0)
  }
  
  def load(file: String): Config = {
    val node = xml.XML.loadFile(file)
    fromXML(node)
  }
}
