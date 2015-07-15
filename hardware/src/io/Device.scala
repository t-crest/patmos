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

/*
 * Common definitions for I/O devices
 *
 * Authors: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package io

import Chisel._
import Node._

import ocp._

import patmos.Constants._

abstract class DeviceObject() {
  // every device object must have methods "create" and "init", and a trait "Pins"
  def init(params: Map[String, String])
  def create(params: Map[String, String]) : Device
  trait Pins
  trait Intrs

  // helper functions for parameter parsing

  def getParam(params: Map[String, String], key: String) : String = {
    val param = params.get(key)
    if (param == None) {
      throw new IllegalArgumentException("Parameter " + key + " not found")
    }
    param.get
  }

  def getIntParam(params: Map[String, String], key: String) : Int = {
    val param = getParam(params, key)
    try { param.toInt
    } catch { case exc : Exception =>
      throw new IllegalArgumentException("Parameter " + key + " must be an integer")
    }
  }

  def getPosIntParam(params: Map[String, String], key: String) : Int = {
    val param = getIntParam(params, key)
    if (param <= 0) {
      throw new IllegalArgumentException("Parameter " + key + " must be a positive integer")
    }
    param
  }

  def getBoolParam(params: Map[String, String], key: String) : Boolean = {
    val param = getParam(params, key)
    if (param == "true") {
      true
    }
    else if(param == "false"){
      false
    } else {
      throw new IllegalArgumentException("Parameter " + key + " must be either \"true\" or \"false\"")
    }
  }
}

abstract class Device() extends Module() {
  val io = new InternalIO()
}

class InternalIO() extends Bundle() {
  val internalPort = new Bundle()
}

class CoreDevice() extends Device() {
  override val io = new CoreDeviceIO()
}

class CoreDeviceIO() extends InternalIO() {
  val ocp = new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH)
}

class IODevice() extends Device() {
  override val io = new IODeviceIO()
}

class IODeviceIO() extends InternalIO() {
  val ocp = new OcpIOSlavePort(ADDR_WIDTH, DATA_WIDTH)
}

class BurstDevice(addrBits: Int) extends Device() {
  override val io = new BurstDeviceIO(addrBits)
}

class BurstDeviceIO(addrBits: Int) extends InternalIO() {
  val ocp = new OcpBurstSlavePort(addrBits, DATA_WIDTH, BURST_LENGTH)
}
