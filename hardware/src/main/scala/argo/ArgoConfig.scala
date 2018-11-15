package argo

import java.io.{File, PrintWriter}

import Chisel._
import Node._
import patmos.Constants._
import util.Config
import util.Utility
import ocp._

abstract class ArgoConfig{
	// NoC Dimensions
	var N: Int
	var M: Int
	def CORES : Int = N*M
  var PRD_LENGTH: Int
	// Topology
	var topoType : String
  // General header packet constants and types
  var HEADER_ROUTE_WIDTH : Int
  var HEADER_FIELD_WIDTH : Int
  var HEADER_CTRL_WIDTH : Int
	def HEADER_WIDTH : Int = HEADER_FIELD_WIDTH - HEADER_CTRL_WIDTH
  var ACTIVE_BIT : Int
  // SPM
	var SPM_ADDR_WIDTH : Int
  var SPM_IDX_SIZE: Int
	def SPM_BYTES : Int = Math.pow(2, SPM_IDX_SIZE).toInt
  // Link constants
  var LINK_DATA_WIDTH : Int = DATA_WIDTH
  var LINK_CTRL : Int = 3
  def LINK_WIDTH : Int = LINK_DATA_WIDTH + LINK_CTRL
}

object ArgoConfig {
	// Default Configuration
	val defaultConf = new ArgoConfig {
		var N = 2
		var M = 2
    var topoType = "bitorus"
    var PRD_LENGTH = 8
		var HEADER_ROUTE_WIDTH = 16
  	var HEADER_FIELD_WIDTH = 16
  	var HEADER_CTRL_WIDTH  = 2
  	var ACTIVE_BIT = 1
		var SPM_ADDR_WIDTH = 16
  	var SPM_IDX_SIZE = 12
	}
	// Variables
	private val conf: ArgoConfig = defaultConf
	
	// Getters
  def getConfig = conf
  
  // Setters
  def setCores(cores: Int) : Unit = {
  	conf.N = Math.sqrt(cores).toInt
  	conf.M = Math.sqrt(cores).toInt
  }

	def setSPMSize(bytes: Int) : Unit = {
		conf.SPM_IDX_SIZE = log2Down(bytes)
	}

	// Generators
  def genConfigVHD(argoConf: ArgoConfig, filepath: String): Unit ={
		new PrintWriter(new File(filepath),"UTF8"){
      print("----------------------------\n" +
        "--DO NOT MODIFY! This is an auto-generated file by " + this.getClass.getName + "\n"+
        "----------------------------\n" +
        "library ieee;\n" +
        "use ieee.std_logic_1164.all;\n" +
        "use work.config_types.all;\n\n" +
        "package config is\n\n" +
				"\tconstant SPM_ADDR_WIDTH : integer := " + argoConf.SPM_ADDR_WIDTH + ";\n" +
				"\tconstant TARGET_ARCHITECTURE : ARCHITECTURES := FPGA;\n" +
				"\tconstant TARGET_IMPLEMENTATION : IMPLEMENTATIONS := SYNC;\n" +
				"\tconstant GATING_ENABLED : integer := 1;\n"+
        "\tconstant N : integer := " + argoConf.N + "; -- Horizontal width\n" +
        "\tconstant M : integer := " + argoConf.M + "; -- Vertical Height\n" +
        "\tconstant NODES : integer := " + argoConf.CORES + ";\n" +
        "\tconstant PRD_LENGTH : integer := "+ argoConf.PRD_LENGTH + "; -- The number of timeslots in one TDM period\n\n" +
				"\tconstant PDELAY : time := 500 ps;\n" +
				"\tconstant NA_HPERIOD : time := 5 ns;\n" +
				"\tconstant P_HPERIOD : time := 5 ns;\n" +
				"\tconstant SKEW : time := 0 ns;\n" +
				"\tconstant delay : time := 0.3 ns;\n"+
        "\nend package ; -- aegean_def")
      close()}
  }
}
