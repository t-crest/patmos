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
 * Definition of Argo network types
 *
 * Authors: Eleftherios Kyriakakis (elky@dtu.dk)
 *
 */

package argo

import Chisel._
import Node._
import patmos.Constants._
import util._
import ocp._

class MemIFMaster(HEADER_FIELD_WIDTH: Int, HEADER_CTRL_WIDTH: Int) extends Bundle() {
  val Addr = UInt(width = HEADER_FIELD_WIDTH - HEADER_CTRL_WIDTH)
  val En = Bits(width = 2)
  val Wr = Bool()
  val Data = Bits(width = 64)
}

class MemIFSlave extends Bundle {
  val Data = Bits(width = 64)
  val Error = Bool()
}

class SPMMasterPort(headerFieldWdith: Int, headerCtrlWidth: Int) extends Bundle {
  val M = new MemIFMaster(headerFieldWdith, headerCtrlWidth).asOutput()
  val S = new MemIFSlave().asInput()
}

class SPMSlavePort(headerFieldWdith: Int, headerCtrlWidth: Int) extends Bundle {
  val M = new MemIFMaster(headerFieldWdith, headerCtrlWidth).asInput()
  val S = new MemIFSlave().asOutput()
}

class ChannelForward(argoConf : ArgoConfig) extends Bundle {
  val req = Bool()
  val data = Bits(width = argoConf.LINK_WIDTH)
}

class ChannelBackward(argoConf: ArgoConfig) extends Bundle {
  val ack = Bool()
}

class RouterPort(argoConf: ArgoConfig) extends Bundle {
  val f = new ChannelForward(argoConf).asInput()
  val b = new ChannelBackward(argoConf).asOutput()
}

class OutputPort(argoConf: ArgoConfig) extends Bundle {
  val f = new ChannelForward(argoConf).asOutput()
  val b = new ChannelBackward(argoConf).asInput()
}

class NodeInterconnection(argoConf: ArgoConfig) extends Bundle {
  val north_wire_in = Wire(init = UInt(0, width = argoConf.LINK_WIDTH))
  val east_wire_in = Wire(init = UInt(0, width = argoConf.LINK_WIDTH))
  val south_wire_in = Wire(init = UInt(0, width = argoConf.LINK_WIDTH))
  val west_wire_in = Wire(init = UInt(0, width = argoConf.LINK_WIDTH))
  val north_wire_out = Wire(init = UInt(0, width = argoConf.LINK_WIDTH))
  val east_wire_out = Wire(init = UInt(0, width = argoConf.LINK_WIDTH))
  val south_wire_out = Wire(init = UInt(0, width = argoConf.LINK_WIDTH))
  val west_wire_out = Wire(init = UInt(0, width = argoConf.LINK_WIDTH))
}
