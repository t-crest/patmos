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
import io.Device

import scala.tools.nsc.interpreter.IMain
import scala.tools.nsc.Settings
import java.io.DataInputStream
import ch.epfl.lamp.fjbg.{ FJBGContext, JClass }
import java.io.File

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
  case class ICacheConfig(size: Int, assoc: Int, repl: String)
  val ICache: ICacheConfig
  case class DCacheConfig(size: Int, assoc: Int, repl: String, writeThrough: Boolean)
  val DCache: DCacheConfig
  case class SCacheConfig(size: Int)
  val SCache: SCacheConfig

  case class SPMConfig(size: Int)
  val ISPM: SPMConfig
  val DSPM: SPMConfig
  val BootSPM: SPMConfig

  case class ExtMemConfig(size: Long, ram: DeviceConfig)
  val ExtMem: ExtMemConfig

  case class DeviceConfig(name : String, params : Map[String, String], offset : Int, intrs : List[Int])
  val Devs: List[Config#DeviceConfig]

  override def toString =
    description + " at " + (frequency/1000000).toString() + " MHz"
}

object Config {

  private def parseSize(text: String): Int = {
    val regex = """(\d+)([KMG]?)""".r
    val suffixMult = Map("" -> (1 << 0),
                         "K" -> (1 << 10),
                         "M" -> (1 << 20),
                         "G" -> (1 << 30))
    val regex(num, suffix) = text.toUpperCase
    num.toInt * suffixMult.getOrElse(suffix, 0)
  }

  private def parseSizeLong(text: String): Long = {
    val regex = """(\d+)([KMG]?)""".r
    val suffixMult = Map("" -> (1 << 0),
               "K" -> (1 << 10),
               "M" -> (1 << 20),
               "G" -> (1 << 30))
    val regex(num, suffix) = text.toUpperCase
    num.toLong * suffixMult.getOrElse(suffix, 0)
  }

  private def getAttr(node: scala.xml.Node, elem: String, attr: String,
                      optional: Boolean) = {
    val seq = node \ elem
    if (seq.isEmpty) {
      if (!optional) {
        sys.error("Element "+elem+" not found in node "+node)
      } else {
        None
      }
    } else {
      val value = seq(0) \ attr
      if (value.isEmpty) {
        if (!optional) {
          sys.error("Attribute "+attr+" not found in element "+elem+" in node "+node)
        } else {
          None
        }
      } else {
        Option(value)
      }
    }
  }

  private def getIntAttr(node: scala.xml.Node, elem: String, attr: String,
                         optional: Boolean, default: Int) = {

    val value = getAttr(node, elem, attr, optional)
    if (value == None) default else value.get.text.toInt
  }

  private def getBooleanAttr(node: scala.xml.Node, elem: String, attr: String,
                             optional: Boolean, default: Boolean) = {

    val value = getAttr(node, elem, attr, optional)
    if (value == None) default else value.get.text.toBoolean
  }

  private def getSizeAttr(node: scala.xml.Node, elem: String, attr: String,
                          optional: Boolean, default: Int) = {

    val value = getAttr(node, elem, attr, optional)
    if (value == None) default else parseSize(value.get.text)
  }

  private def getTextAttr(node: scala.xml.Node, elem: String, attr: String,
                          optional: Boolean, default: String) = {

    val value = getAttr(node, elem, attr, optional)
    if (value == None) default else value.get.text
  }

  private def find(node: scala.xml.Node, item: String): scala.xml.Node = {
    val seq = node \ item
    if (seq.isEmpty) {
      sys.error("Item "+item+" not found in node "+node)
    }
    seq(0)
  }

  private def resolveReference(base: File, name: String) = {
    if (new File(name).isAbsolute) {
      name
    } else {
      new File(base.getCanonicalFile.getParent, name).toString
    }
  }

  private def fromXML(node: scala.xml.Node, file: File): Config = {

    val parent = (node \ "@default")
    val hasParent = !parent.isEmpty
    val parentFile = resolveReference(file, parent.text)

    val defaultConf = if (hasParent) load(parentFile) else nullConfig

    new Config {
      val description = (node \ "description").text

      val frequency = getIntAttr(node, "frequency", "@Hz",
                                 hasParent, defaultConf.frequency)

      val dual = getBooleanAttr(node, "pipeline", "@dual",
                                hasParent, defaultConf.pipeCount > 1)
      val pipeCount = if (dual) 2 else 1

      val burstLength  = getIntAttr(node, "bus", "@burstLength",
                                    hasParent, defaultConf.burstLength)
      val writeCombine = getBooleanAttr(node, "bus", "@writeCombine",
                                        hasParent, defaultConf.writeCombine)

      val MCache =
        new MCacheConfig(getSizeAttr(node, "MCache", "@size",
                                     hasParent, defaultConf.MCache.size),
                         getIntAttr(node,  "MCache", "@blocks",
                                    hasParent, defaultConf.MCache.blocks),
                         getTextAttr(node, "MCache", "@repl",
                                     hasParent, defaultConf.MCache.repl))

      val ICache =
        new ICacheConfig(getSizeAttr(node, "ICache", "@size",
                                     hasParent, defaultConf.ICache.size),
                         getIntAttr(node,  "ICache", "@assoc",
                                    hasParent, defaultConf.ICache.assoc),
                         getTextAttr(node, "ICache", "@repl",
                                     hasParent, defaultConf.ICache.repl))

      val DCache =
        new DCacheConfig(getSizeAttr(node, "DCache", "@size",
                                     hasParent, defaultConf.DCache.size),
                         getIntAttr(node,  "DCache", "@assoc",
                                    hasParent, defaultConf.DCache.assoc),
                         getTextAttr(node, "DCache", "@repl",
                                     hasParent, defaultConf.DCache.repl),
                         getBooleanAttr(node, "DCache", "@writeThrough",
                                        hasParent, defaultConf.DCache.writeThrough))

      val SCache =
        new SCacheConfig(getSizeAttr(node, "SCache", "@size",
                                     hasParent, defaultConf.SCache.size))

      val ISPM =
        new SPMConfig(getSizeAttr(node, "ISPM", "@size",
                                  hasParent, defaultConf.ISPM.size))
      val DSPM =
        new SPMConfig(getSizeAttr(node, "DSPM", "@size",
                                  hasParent, defaultConf.DSPM.size))
      val BootSPM =
        new SPMConfig(getSizeAttr(node, "BootSPM", "@size",
                                  hasParent, defaultConf.BootSPM.size))

      val DevList = ((node \ "Devs") \ "Dev")

      val ExtMemNode = find(node, "ExtMem")
      var ExtMemDev = new DeviceConfig("", Map(), -1, List[Int]())
      if (!(ExtMemNode \ "@DevTypeRef").isEmpty){
        ExtMemDev = devFromXML(ExtMemNode,DevList,false)
      }
      val ExtMem = new ExtMemConfig(parseSizeLong(find(ExtMemNode, "@size").text),
                                    ExtMemDev)

      val DevNodes = ((node \ "IOs") \ "IO")
      val Devs : List[Config#DeviceConfig] =
        DevNodes.map(devFromXML(_, DevList)).toList ++ defaultConf.Devs

      // Make sure static state of devices is initialized
      for (d <- Devs) { initDevice(d) }
      if(ExtMem.ram.name != ""){
        initDevice(ExtMem.ram)
      }

      // Emit defines for emulator
      val emuConfig = Driver.createOutputFile("emulator_config.h")
      if (ICache.size > 0) { emuConfig.write("#define ICACHE\n") }
      if (MCache.size > 0) { emuConfig.write("#define MCACHE\n") }
      for (d <- Devs) { emuConfig.write("#define IO_"+d.name.toUpperCase+"\n") }
      emuConfig.write("#define EXTMEM_"+ExtMem.ram.name.toUpperCase+"\n")
      emuConfig.close();

      private def devFromXML(node: scala.xml.Node, devs: scala.xml.NodeSeq,
                             needOffset: Boolean = true): DeviceConfig = {
        val key = find(node, "@DevTypeRef").text
        val devList = (devs.filter(d => (d \ "@DevType").text == key))
        if (devList.isEmpty) {
          sys.error("No Dev specification found for "+node)
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
        val offset = if (needOffset) {
          find(node, "@offset").text.toInt
        } else {
          -1
        }
        val intrsList = (node \ "@intrs").text
        val intrs = if (intrsList.isEmpty) {
          List[Int]()
        } else {
          intrsList.split(",").toList.map(_.trim.toInt)
        }
        ChiselError.info("IO device "+key+": entity "+name+
                         ", offset "+offset+", params "+params+
                         (if (!intrs.isEmpty) ", interrupts: "+intrs else ""))
        new DeviceConfig(name, params, offset, intrs)
      }
    }
  }


  // Baseline config with invalid values
  val nullConfig = new Config {
    val description = "dummy"
    val frequency = 0
    val pipeCount = 0
    val burstLength = 0
    val writeCombine = false
    val minPcWidth = 0
    val MCache = new MCacheConfig(0, 0, "")
    val ICache = new ICacheConfig(0, 0, "")
    val DCache = new DCacheConfig(0, 0, "", true)
    val SCache = new SCacheConfig(0)
    val ISPM = new SPMConfig(0)
    val DSPM = new SPMConfig(0)
    val BootSPM = new SPMConfig(0)
    val ExtMem = new ExtMemConfig(0,new DeviceConfig("", Map(), -1, List[Int]()))
    val Devs = List[DeviceConfig]()
  }

  private var conf: Config = nullConfig
  def getConfig = conf

  private def load(file: String) = {
    val f = new File(file)
    val node = xml.XML.loadFile(f)
    fromXML(node, f)
  }
  def loadConfig(file: String) = {
    conf = load(file)
  }

  // This should not be a public variable
  var minPcWidth = 0

  def initDevice(dev : Config#DeviceConfig) = {
    // get class for device
    val clazz = Class.forName("io."+dev.name)
    // create device instance
    val meth = clazz.getMethods.find(_.getName == "init").get
    meth.invoke(null, dev.params)
  }

  def createDevice(dev : Config#DeviceConfig) : Device = {
    // get class for device
    val clazz = Class.forName("io."+dev.name)
    // create device instance
    val meth = clazz.getMethods.find(_.getName == "create").get
    val rawDev = meth.invoke(null, dev.params)
    rawDev.asInstanceOf[Device]
  }

  def connectIOPins(name : String, outer : Node, inner : Node) = {
      // get class for pin trait
      val clazz = Class.forName("io."+name+"$Pins")
      if (clazz.isInstance(outer)) {
        // get method to retrieve pin bundle
        val methName = name(0).toLower + name.substring(1, name.length) + "Pins"
        for (m <- clazz.getMethods) {
          if (m.getName != methName && !m.getName.endsWith("_$eq")) {

            val isInherited = clazz.getInterfaces().foldLeft(false)(
              _ || _.getMethods.map(_.getName).contains(m.getName))

            if (!isInherited) {
              val fileName = name+"$Pins.class"
              val classStream = new DataInputStream(clazz.getResourceAsStream(fileName))
              val jClass = new FJBGContext().JClass(classStream)

              ChiselError.error("Pins trait for IO device "+name+
                                " cannot declare non-inherited member "+m.getName+
                                ", only member "+methName+" allowed"+
                                " (file "+jClass.getSourceFileName+")")
            }
          }
        }
        val meth = clazz.getMethods.find(_.getName == methName)
        if (meth == None) {
          ChiselError.info("No pins for IO device "+name)
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
      for (m <- clazz.getMethods) {
        if (m.getName != methName && !m.getName.endsWith("_$eq")) {

          val isInherited = clazz.getInterfaces().foldLeft(false)(
            _ || _.getMethods.map(_.getName).contains(m.getName))

          if (!isInherited) {
            val fileName = name+"$Intrs.class"
            val classStream = new DataInputStream(clazz.getResourceAsStream(fileName))
            val jClass = new FJBGContext().JClass(classStream)

            ChiselError.error("Intrs trait for IO device "+name+
                              " cannot declare non-inherited member "+m.getName+
                              ", only member "+methName+" allowed"+
                              " (file "+jClass.getSourceFileName+")", null)
          }
        }
      }
      val meth = clazz.getMethods.find(_.getName == methName)
      if (meth == None) {
        ChiselError.error("Interrupt pins not found for device "+name)
      } else {
        val intrPins = meth.get.invoke(clazz.cast(inner)).asInstanceOf[Vec[Bool]]
        if (intrPins.length != dev.intrs.length) {
          ChiselError.error("Inconsistent interrupt counts for IO device "+name)
        } else {
          for (i <- 0 until dev.intrs.length) {
            outer.intrs(dev.intrs(i)) := intrPins(i)
          }
        }
      }
    }
  }

  private def genTraitedClass[T](base : String, list : List[String]) : T = {
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
    genTraitedClass[PatmosIO]("PatmosIO", conf.ExtMem.ram.name :: conf.Devs.map(_.name))
  }
}
