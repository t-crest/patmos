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

import Chisel._
import Node._

import patmos._
import io.CoreDevice

import scala.tools.nsc.interpreter._
import scala.tools.nsc._

/**
 * The configuration tool for Patmos.
 * 
 * Authors: Martin Schoeberl (martin@jopdesign.com)
 *          Wolfgang Puffitsch (wpuffitsch@gmail.com)
 * 
 */
abstract class Config {
  val description: String
  val frequency: Int
  val pipeCount: Int
  val burstLength: Int
  val writeCombine: Boolean

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

  case class DeviceConfig(name : String, params : Map[String, String], offset : Int, intrs : List[Int])
  val Devs: List[DeviceConfig]

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
      val frequency = find(find(node, "frequency"), "@Hz").text.toInt
      val dual = find(find(node, "pipeline"), "@dual").text.toBoolean
      val pipeCount = if (dual) 2 else 1
      val burstLength = find(find(node, "bus"), "@burstLength").text.toInt
      val writeCombine = find(find(node, "bus"), "@writeCombine").text.toBoolean

      val MCacheNode = find(node, "MCache")
      val MCache =
        new MCacheConfig(parseSize(find(MCacheNode, "@size").text),
                         find(MCacheNode, "@blocks").text.toInt,
                         find(MCacheNode, "@repl").text)
      val DCacheNode = find(node, "DCache")
      val DCache =
        new DCacheConfig(parseSize(find(DCacheNode, "@size").text),
                         find(DCacheNode, "@assoc").text.toInt,
                         find(DCacheNode, "@repl").text)
      val SCacheNode = find(node, "SCache")
      val SCache =
        new SCacheConfig(parseSize(find(SCacheNode, "@size").text))

      val ISPM = new SPMConfig(parseSize(find(find(node, "ISPM"), "@size").text))
      val DSPM = new SPMConfig(parseSize(find(find(node, "DSPM"), "@size").text))
      val BootSPM = new SPMConfig(parseSize(find(find(node, "BootSPM"), "@size").text))

      val ExtMem = new ExtMemConfig(parseSize(find(find(node, "ExtMem"), "@size").text))

      val DevList = ((node \ "IODevs") \ "IODev")
      val DevNodes = ((node \ "IOs") \ "IO")
      val Devs = DevNodes.map(devFromXML(_, DevList)).toList

      // Make sure static state of devices is initialized
      for (d <- Devs) { initDevice(d) }

      def devFromXML(node: scala.xml.Node, devs: scala.xml.NodeSeq): DeviceConfig = {
        val key = find(node, "@IODevTypeRef").text
        val devList = (devs.filter(d => (d \ "@IODevType").text == key))
        if (devList.isEmpty) {
          sys.error("No IODev specification found for "+node)
        }
        val dev = devList(0)
        val name = find(dev, "@entity").text
        val paramsNode = (dev \ "params")
        val params = if (paramsNode.isEmpty) {
          Map[String,String]()
        } else {
          Map((paramsNode \ "param").map(p => find(p, "@name").text -> 
                                              find(p, "@value").text) : _*)
        }
        val offset = find(node, "@offset").text.toInt
        val intrsList = (node \ "@intrs").text
        val intrs = if (intrsList.isEmpty) {
          List[Int]()
        } else {
          intrsList.split(",").toList.map(_.trim.toInt)
        }

        print("IO device "+key+": entity "+name+", offset "+offset+", params "+params)
        if (!intrs.isEmpty) { print(", interrupts: "+intrs) }
        println()

        new DeviceConfig(name, params, offset, intrs)
      }

	  def find(node: scala.xml.Node, item: String): scala.xml.Node = {
		val seq = node \ item
		if (seq.isEmpty) {
		  sys.error("Item "+item+" not found in node "+node)
		}
		seq(0)
	  }
    }

  def initDevice(dev : Config#DeviceConfig) = {
    // get class for device
    val clazz = Class.forName("io."+dev.name)
    // create device instance
    val meth = clazz.getMethods.find(_.getName == "init").get
    meth.invoke(null, dev.params)
  }

  def createDevice(dev : Config#DeviceConfig) : CoreDevice = {
    // get class for device
    val clazz = Class.forName("io."+dev.name)
    // create device instance
    val meth = clazz.getMethods.find(_.getName == "create").get
    val rawDev = meth.invoke(null, dev.params)
    rawDev.asInstanceOf[CoreDevice]
  }

  def connectIOPins(name : String, outer : Node, inner : Node) = {
      // get class for pin trait
      val clazz = Class.forName("io."+name+"$Pins")
      if (clazz.isInstance(outer)) {
        // get method to retrieve pin bundle
        val methName = name(0).toLower + name.substring(1, name.length) + "Pins"
        val meth = clazz.getMethods.find(_.getName == methName)
        if (meth == None) {
          println("No pins for device: "+clazz+"."+methName)
        } else {
          // retrieve pin bundles
          val outerPins = meth.get.invoke(clazz.cast(outer))
          val innerPins = meth.get.invoke(clazz.cast(inner))
          // actually connect pins
          outerPins.asInstanceOf[Bundle] <> innerPins.asInstanceOf[Bundle]
        }
      }
  }

  def connectAllIOPins(outer : Node, inner : Node) {
    for (name <- conf.Devs.map(_.name)) {
	  connectIOPins(name, outer, inner)
    }
  }

  def connectIntrPins(dev : Config#DeviceConfig, outer : InOutIO, inner : Node) {
    if (!dev.intrs.isEmpty) {
      val name = dev.name
      // get class for interrupt trait
      val clazz = Class.forName("io."+name+"$Intrs")
      // get method to retrieve interrupt bits
      val methName = name(0).toLower + name.substring(1, name.length) + "Intrs"
      val meth = clazz.getMethods.find(_.getName == methName)
      if (meth == None) {
        sys.error("Interrupt pins not found for device: "+clazz+"."+methName)
      } else {
        val intrPins = meth.get.invoke(clazz.cast(inner)).asInstanceOf[Vec[Bool]]
        if (intrPins.length != dev.intrs.length) {
          sys.error("Inconsistent interrupt counts for device: "+clazz+"."+methName)
        } else {
          for (i <- 0 until dev.intrs.length) {
            outer.intrs(dev.intrs(i)) := intrPins(i)
          }
        }
      }
    }
  }

  def genTraitedClass[T](base : String, list : List[String]) : T = {
    // build class definition
    val traitClass = list.foldLeft("Trait"+base)(_+"_"+ _)
    val traitClassDef = "class "+traitClass+" extends "+"patmos."+base
    val classDef = list.foldLeft(traitClassDef)(_+" with io."+_+".Pins")

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
    genTraitedClass[InOutIO]("InOutIO", conf.Devs.map(_.name))
  }

  def getPatmosCoreIO() : PatmosCoreIO = {
    genTraitedClass[PatmosCoreIO]("PatmosCoreIO", conf.Devs.map(_.name))
  }

  def getPatmosIO() : PatmosIO = {
    genTraitedClass[PatmosIO]("PatmosIO", conf.Devs.map(_.name))
  }
  
  // This is probably not the best way to have the singleton
  // for the configuration available and around.
  // We also do not want this to be a var.
  var conf: Config = new Config {
    val description = "dummy"
    val frequency = 0
    val pipeCount = 0
    val burstLength = 0
    val writeCombine = false
    val minPcWidth = 0
    val MCache = new MCacheConfig(0, 0, "")
    val DCache = new DCacheConfig(0, 0, "")
    val SCache = new SCacheConfig(0)
    val ISPM = new SPMConfig(0)
    val DSPM = new SPMConfig(0)
    val BootSPM = new SPMConfig(0)
    val ExtMem = new ExtMemConfig(0)
    val Devs = List()
  }
  
  var minPcWidth = 0

  def load(file: String): Config = {
    val node = xml.XML.loadFile(file)
    fromXML(node)
  }
}
