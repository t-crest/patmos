package argo

import java.io.{File, PrintWriter}

import sys.process._
import Chisel._
import patmos.Constants._
import scala.language.postfixOps

abstract class ArgoConfig {
  // NoC Dimensions
  var N: Int
  var M: Int

  def CORES: Int = N * M

  def PRD_LENGTH: Int = CORES * 2

  // Topology
  var topoType: String
  var routerDetph: Int
  var linkDepth: Int
  // General header packet constants and types
  var HEADER_ROUTE_WIDTH: Int
  var HEADER_FIELD_WIDTH: Int
  var HEADER_CTRL_WIDTH: Int

  def HEADER_WIDTH: Int = HEADER_FIELD_WIDTH - HEADER_CTRL_WIDTH

  var ACTIVE_BIT: Int
  // SPM
  var SPM_ADDR_WIDTH: Int
  var SPM_IDX_SIZE: Int

  def SPM_BYTES: Int = Math.pow(2, SPM_IDX_SIZE).toInt

  // Link constants
  var LINK_DATA_WIDTH: Int = DATA_WIDTH
  var LINK_CTRL: Int = 3

  def LINK_WIDTH: Int = LINK_DATA_WIDTH + LINK_CTRL

  def PLATFORM_SPEC =
    <platform width={N.toString} height={M.toString}>
      <topology type={topoType} routerDepth={routerDetph.toString} linkDepth={linkDepth.toString}></topology>
    </platform>

  def COMMUNICATION_SPEC =
      <communication comType="all2all" phits="3"/>

}

object ArgoConfig {
  // Default Configuration
  val defaultConf = new ArgoConfig {
    var N = 2
    var M = 2
    var topoType = "bitorus"
    var routerDetph = 3
    var linkDepth = 0
    var HEADER_ROUTE_WIDTH = 16
    var HEADER_FIELD_WIDTH = 16
    var HEADER_CTRL_WIDTH = 2
    var ACTIVE_BIT = 1
    var SPM_ADDR_WIDTH = 16
    var SPM_IDX_SIZE = 12
  }
  // Variables
  private val conf: ArgoConfig = defaultConf

  // Getters
  def getConfig = conf

  def isSqrt(x: Int): Int = {
    val y = Math.sqrt(x)
    if (y*y == x) y.toInt
    else -1
  }

  // Setters
  def setCores(cores: Int): Unit = {
    if (isSqrt(cores) > 0) {
      conf.N = Math.sqrt(cores).toInt
      conf.M = Math.sqrt(cores).toInt
    } else if (cores % 2 == 0) {
      try {
        val factorOCores = Range(cores - 1, 1, -1).view.filter(n => cores % n == 0).head //find the factor of @param cores
        conf.N = factorOCores
        conf.M = cores / factorOCores
      } catch {
        case _: Throwable => throw new IllegalArgumentException("ArgoConfig: illegal number of cores")
      }
    } else {
      throw new IllegalArgumentException("ArgoConfig: illegal number of cores is not an even number")
    }
  }

  def setSPMSize(bytes: Int): Unit = {
    conf.SPM_IDX_SIZE = log2Down(bytes)
  }

  // Generators
  def genConfigVHD(filepath: String): Unit = {
    new PrintWriter(new File(filepath), "UTF8") {
      print("----------------------------\n" +
        "--DO NOT MODIFY! This is an auto-generated file by " + this.getClass.getName + "\n" +
        "----------------------------\n" +
        "library ieee;\n" +
        "use ieee.std_logic_1164.all;\n" +
        "use work.config_types.all;\n\n" +
        "package config is\n\n" +
        "\tconstant SPM_ADDR_WIDTH : integer := " + conf.SPM_ADDR_WIDTH + ";\n" +
        "\tconstant TARGET_ARCHITECTURE : ARCHITECTURES := FPGA;\n" +
        "\tconstant TARGET_IMPLEMENTATION : IMPLEMENTATIONS := SYNC;\n" +
        "\tconstant GATING_ENABLED : integer := 1;\n" +
        "\tconstant N : integer := " + conf.N + "; -- Horizontal width\n" +
        "\tconstant M : integer := " + conf.M + "; -- Vertical Height\n" +
        "\tconstant NODES : integer := " + conf.CORES + ";\n" +
        "\tconstant PRD_LENGTH : integer := " + conf.PRD_LENGTH + "; -- The number of timeslots in one TDM period\n\n" +
        "\tconstant PDELAY : time := 500 ps;\n" +
        "\tconstant NA_HPERIOD : time := 5 ns;\n" +
        "\tconstant P_HPERIOD : time := 5 ns;\n" +
        "\tconstant SKEW : time := 0 ns;\n" +
        "\tconstant delay : time := 0.3 ns;\n" +
        "\nend package ; -- aegean_def")
      close()
    }
  }

  def genPoseidonSched(poseidonPath: String, generatedPath: String, platformFile: String, communicationFile: String, schedPath: String): Unit = {
    val genDirectory = new File(generatedPath)
    if(!genDirectory.exists()){
      genDirectory.mkdir()
    }

    //Write out platform specification
    new PrintWriter(new File(generatedPath+platformFile), "UTF8") {
      println("<?xml version=\"1.0\" encoding=\"UTF-8\"?>")
      print(conf.PLATFORM_SPEC)
      close()
    }

    //Write out communication specification
    new PrintWriter(generatedPath+communicationFile, "UTF8") {
      println("<?xml version=\"1.0\" encoding=\"UTF-8\"?>")
      print(conf.COMMUNICATION_SPEC)
      close()
    }

    println(poseidonPath + "poseidon" +
      " -p" + (generatedPath+platformFile) +
      " -c" + (generatedPath+communicationFile) +
      " -s" + schedPath +
      " -m" + "GREEDY" +
      " -v" + "2" +
      " -d" !)
  }

  def genNoCInitFile(poseidonPath: String, schedFile: String, destPath: String): Unit = {
    println(
      poseidonPath + "poseidon-conv" +
      " " + schedFile +
      " -o" + destPath !
    )
  }
}
