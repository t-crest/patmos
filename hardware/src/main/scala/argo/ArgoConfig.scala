package argo

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
	// Topology
	var topoType : String
  // General header packet constants and types
  var HEADER_ROUTE_WIDTH : Int
  var HEADER_FIELD_WIDTH : Int
  var HEADER_CTRL_WIDTH : Int
  var ACTIVE_BIT : Int
  // SPM
  var SPM_IDX_SIZE: Int
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
		var HEADER_ROUTE_WIDTH = 16
  	var HEADER_FIELD_WIDTH = 16
  	var HEADER_CTRL_WIDTH  = 2
  	var ACTIVE_BIT = 1
  	var SPM_IDX_SIZE = 12
	}
	// Variables
	private val conf: ArgoConfig = defaultConf
	
	// Getters
  def getConfig = conf
  def getSize = conf.N*conf.M
  
  // Setters
  def setSize(size: Int) : Unit = {
  	conf.N = log2Down(size)
  	conf.M = log2Down(size)
  }
}
