package io

import scala.math._

import Chisel._
import Node._
import ocp._
import patmos.Constants._

object MemBridge extends DeviceObject {
    var ocpAddrWidth = 32
    var extAddrWidth = 32
    var dataWidth = 32

    def init(params : Map[String, String]) = {
        // Er...no params parsing, do nothing
    }

    def create(params: Map[String, String]) : MemBridge = {
        Module(new MemBridge(ocpAddrWidth=ocpAddrWidth, extAddrWidth=extAddrWidth, dataWidth=dataWidth))
    }

    trait Pins {
        // Factor out the 4 later...
        val memBridgePins = new OcpBurstMasterPort(extAddrWidth, dataWidth, 4)
    }
}

class MemBridge(ocpAddrWidth : Int = 32,
                extAddrWidth : Int = 32,
                dataWidth : Int = 32) extends BurstDevice(ocpAddrWidth) {
    override val io = new BurstDeviceIO(ocpAddrWidth) with MemBridge.Pins

    // Ok, this might not work
    // Wire straight through and hope for the best
    io.ocp <> io.memBridgePins
}
