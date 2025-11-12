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

import java.io._

import patmos._
import io.CoreDevice
import io.Device
import cop.Coprocessor

import java.io.DataInputStream
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
  val coreCount: Int
  val cmpDevices: Set[String]
  val burstLength: Int
  val writeCombine: Boolean
  val mmu: Boolean
  val roundRobinArbiter: Boolean

  case class ICacheConfig(typ: String, size: Int, assoc: Int, repl: String)
  val ICache: ICacheConfig
  case class DCacheConfig(size: Int, assoc: Int, repl: String, writeThrough: Boolean)
  val DCache: DCacheConfig
  case class SCacheConfig(size: Int)
  val SCache: SCacheConfig

  case class L2CacheConfig(include: Boolean, size: Int, ways: Int, bytesPerBlock: Int, repl: String)
  val L2Cache: L2CacheConfig

  case class SPMConfig(size: Int)
  val ISPM: SPMConfig
  val DSPM: SPMConfig

  case class ExtMemConfig(size: Long, ram: DeviceConfig)
  val ExtMem: ExtMemConfig

  case class DeviceConfig(ref : String, name : String, params : Map[String, String], offset : Int, intrs : List[Int], core : Int = 0, allcores : Boolean = false)
  val Devs: List[Config#DeviceConfig]

  case class CoprocessorConfig(name : String, params : Map[String, String], CoprocessorID : Int, requiresMemoryAccess : Boolean, isBlackBox : Boolean, externalPath : String)
  val Coprocessors: List[Config#CoprocessorConfig]
  val coprocessorCount: Int

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

      val coreCount = getIntAttr(node, "cores", "@count",
                                 hasParent, defaultConf.coreCount)

      val cmpDevices = ((node \ "CmpDevs") \ "CmpDev").map(e => (e \ "@name").text).toSet ++ 
        defaultConf.cmpDevices

      val burstLength  = getIntAttr(node, "bus", "@burstLength",
                                    hasParent, defaultConf.burstLength)
      val writeCombine = getBooleanAttr(node, "bus", "@writeCombine",
                                        hasParent, defaultConf.writeCombine)
      val mmu = getBooleanAttr(node, "bus", "@mmu",
                               hasParent, defaultConf.mmu)
      val roundRobinArbiter = getBooleanAttr(node, "bus", "@roundRobinArbiter",
                               hasParent, defaultConf.roundRobinArbiter)

      val ICache =
        new ICacheConfig(getTextAttr(node, "ICache", "@type",
                                     hasParent, defaultConf.ICache.typ),
                         getSizeAttr(node, "ICache", "@size",
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

      val L2Cache = if ((node \ "L2Cache").isEmpty) {
        L2CacheConfig(include = false, 0, 0, 0, "")
      } else {
        L2CacheConfig(include = true,
          getSizeAttr(node, "L2Cache", "@size",
            hasParent, defaultConf.L2Cache.size),
          getIntAttr(node, "L2Cache", "@ways",
            hasParent, defaultConf.L2Cache.ways),
          getIntAttr(node, "L2Cache", "@bytesPerBlock",
            hasParent, defaultConf.L2Cache.bytesPerBlock),
          getTextAttr(node, "L2Cache", "@repl",
            hasParent, defaultConf.L2Cache.repl))
      }

      val ISPM =
        new SPMConfig(getSizeAttr(node, "ISPM", "@size",
                                  hasParent, defaultConf.ISPM.size))
      val DSPM =
        new SPMConfig(getSizeAttr(node, "DSPM", "@size",
                                  hasParent, defaultConf.DSPM.size))

      val DevList = ((node \ "Devs") \ "Dev")

      val ExtMemNode = find(node, "ExtMem")
      var ExtMemDev = new DeviceConfig("","", Map(), -1, List[Int]())
      var ExtMemAddrWidth = "" 
      if (!(ExtMemNode \ "@DevTypeRef").isEmpty){
                ExtMemDev = devFromXML(ExtMemNode,DevList,false)
        
        for ((key, v) <- ExtMemDev.params){
          if (key.substring(key.length() - 9) == "AddrWidth"){
            ExtMemAddrWidth = v
          }
        } 
      }
      
      val UartDev = DevList.filter(d => (d \ "@DevType").text == "Uart")
      val UartParams = (UartDev \ "params")
      val UartParamsMap = if (UartParams.isEmpty) {
          Map[String,String]()
        } else {
          Map((UartParams \ "param").map(p => find(p, "@name").text ->
                                              find(p, "@value").text) : _*)
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
      val buildDir = scala.util.Properties.envOrElse("HWBUILDDIR", "build" ) + "/"
      new java.io.File(buildDir).mkdirs // build dir is created
      val file_config = new File(buildDir + "emulator_config.h") 
      val emuConfig = new PrintWriter(file_config)
      emuConfig.write("#define CORE_COUNT "+coreCount+"\n")
      emuConfig.write("#define ICACHE_"+ICache.typ.toUpperCase+"\n")
      emuConfig.write("#define IO_UART\n")
      for (d <- Devs) { emuConfig.write("#define IO_"+d.name.toUpperCase+"\n") }
      emuConfig.write("#define EXTMEM_"+ExtMem.ram.name.toUpperCase+"\n")
      emuConfig.write("#define EXTMEM_ADDR_BITS "+ ExtMemAddrWidth +"\n")
      emuConfig.write("#define BAUDRATE " + ConstantsForConf.UART_BAUD.toString + "\n") //TODO take baud from configuration .XML
      emuConfig.write("#define FREQ "+ frequency +"\n")
      emuConfig.close();
      
      
      val CopList = ((node \ "Coprocessors") \ "Coprocessor")
      val Coprocessors : List[Config#CoprocessorConfig] =  CopList.map(copFromXML(_, CopList)).toList ++ defaultConf.Coprocessors
      val coprocessorCount = Coprocessors.size
      
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
        val coreNode = (node \ "@core")
        val core = if(coreNode.isEmpty) 0 else coreNode.text.toInt
        
        val allcoresNode = (node \ "@allcores")
        val allcores = if(allcoresNode.isEmpty) false else allcoresNode.text.toBoolean

        println("IO device "+key+": entity "+name+
                         ", offset "+offset+", params "+params+
                         (if(!intrs.isEmpty) ", interrupts: "+intrs else "")+
                         ", " + (if(allcores) "all cores" else "core "+core))
        new DeviceConfig(key, name, params, offset, intrs, core, allcores)
      }

      private def copFromXML(node: scala.xml.Node, copList: scala.xml.NodeSeq): CoprocessorConfig = {
        
        val key = find(node, "@CoprocessorID").text
        val devListFiltered = (copList.filter(d => (d \ "@CoprocessorID").text == key))
        if (devListFiltered.isEmpty) {
          sys.error("No CoprocessorID specification found for "+node)
        }

        val cop = devListFiltered(0)
        val name = find(cop, "@Name").text
        val id = find(cop, "@CoprocessorID").text.toInt
        
        val memAccessStr = (node \ "@requiresMemoryAccess").text
        val memAccess = if (memAccessStr.isEmpty) {
          false
        } else {
          memAccessStr.toBoolean
        }

        val externalPathAttr = (node \ "@externalPath").text
        val externalPath = if (externalPathAttr.isEmpty) {
          ""
        } else {
          externalPathAttr
        }

        val isBlackBox = if (externalPath.isEmpty) {
          false
        } else {
          true
        }

        val paramsNode = (cop \ "params")
        val params = if (paramsNode.isEmpty) {
          Map[String,String]()
        } else {
          Map((paramsNode \ "param").map(p => find(p, "@name").text ->
            find(p, "@value").text) : _*)
        }
        
        println("Coprocessor:" +name +",ID:"+id+", requires Memory Access:"+memAccess + (if (isBlackBox) ", Coprocessor is BlackBox, External Path "+externalPath else ""))
        new CoprocessorConfig(name, params, id, memAccess, isBlackBox, externalPath)
      }
    }
  }


  // Baseline configuration with invalid values
  // MS: why do we need this?
  val nullConfig = new Config {
    val description = "dummy"
    val frequency = 0
    val pipeCount = 0
    val coreCount = 0
    val cmpDevices = Set[String]()
    val burstLength = 0
    val writeCombine = false
    val roundRobinArbiter = false
    val mmu = false
    val minPcWidth = 0
    val datFile = ""
    val ICache = new ICacheConfig("", 0, 0, "")
    val DCache = new DCacheConfig(0, 0, "", true)
    val SCache = new SCacheConfig(0)
    val L2Cache = new L2CacheConfig(include = false, 0, 0, 0, "")
    val ISPM = new SPMConfig(0)
    val DSPM = new SPMConfig(0)
    val ExtMem = new ExtMemConfig(0,new DeviceConfig("","", Map(), -1, List[Int]()))
    val Devs = List[DeviceConfig]()
    val Coprocessors = List[CoprocessorConfig]()
    val coprocessorCount =0
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

  // These should not be public variables
  var minPcWidth = 0
  var datFile = ""

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

  def initCoprocessor(cop : Config#CoprocessorConfig) = {
    // get class for coprocessor
    val clazz = Class.forName("cop."+cop.name)
    // create device instance
    val meth = clazz.getMethods.find(_.getName == "init").get
    meth.invoke(null, cop.params)
  }

  def createCoprocessor(cop : Config#CoprocessorConfig) : Coprocessor = {
    // get class for device
    val clazz = Class.forName("cop."+cop.name)
    // create device instance
    val meth = clazz.getMethods.find(_.getName == "create").get
    val rawCop = meth.invoke(null, cop.params)
    rawCop.asInstanceOf[Coprocessor]
  }
}
