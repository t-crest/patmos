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
 * An Argo network node containing a network interface and a router
 *
 * Authors: Rasmus Bo Soerensen (rasmus@rbscloud.dk)
 *
 */

package argo

import Chisel._
import Node._
import ocp._

object Node extends DeviceObject {
  var linkWidth = 32

  def init(params : Map[String, String]) = {
    linkWidth = getPosIntParam(params, "linkWidth")
  }

  def create(params: Map[String, String]) : Node = {
    Module(new Node(linkWidth=linkWidth))
  }

  trait Pins {
    val nodePins = new Bundle() {
      val irq = new IRQ()
      val spm = new SPMMasterPort(linkWidth)
      val northPort = new RouterPort(linkWidth)
      val southPort = new RouterPort(linkWidth)
      val eastPort = new RouterPort(linkWidth)
      val westPort = new RouterPort(linkWidth)
    }
  }
}

class Node(linkWidth : Int) extends IODevice() {
  override val io = new IODeviceIO() with Node.Pins

}
