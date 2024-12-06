/*
 * A connection to an external IO device
 *
 * Authors: Rasmus Bo Soerensen (rasmus@rbscloud.dk)
 *
 */

package io

import chisel3._
import ocp._

object ExtIODevice extends DeviceObject {
    var extAddrWidth = 32
    var dataWidth = 32

    def init(params : Map[String, String]) = {
        extAddrWidth = getPosIntParam(params, "extAddrWidth")
        dataWidth = getPosIntParam(params, "dataWidth")
    }

    def create(params: Map[String, String]) : ExtIODevice = {
        Module(new ExtIODevice(extAddrWidth=extAddrWidth, dataWidth=dataWidth))
    }
}

class ExtIODevice(extAddrWidth : Int = 32,
                dataWidth : Int = 32) extends CoreDevice() {
    override val io = new CoreDeviceIO() with patmos.HasPins {
        val pins: Bundle { val ocp: OcpIOMasterPort } = new Bundle() {
            val ocp = new OcpIOMasterPort(extAddrWidth, dataWidth)
        }
    }

   val coreBus = Module(new OcpCoreBus(extAddrWidth,dataWidth))
   val ioBus = Module(new OcpIOBus(extAddrWidth,dataWidth))
   io.ocp <> coreBus.io.slave
   ioBus.io.master <> io.pins.ocp

    val bridge = new OcpIOBridge(coreBus.io.master,ioBus.io.slave)
}
