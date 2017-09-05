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
 * A cross bar for a synchronous Argo router
 *
 * Authors: Rasmus Bo Soerensen (rasmus@rbscloud.dk)
 *
 */

package argo

import Chisel._
import Node._

class XBar(linkWidth : Int = 32) extends Module() {
  val io = new Bundle() {
    val northOut = new ArgoLink(linkWidth).asOutput
    val northIn = new SelLink(linkWidth).asInput
    val southOut = new ArgoLink(linkWidth).asOutput
    val southIn = new SelLink(linkWidth).asInput
    val eastOut = new ArgoLink(linkWidth).asOutput
    val eastIn = new SelLink(linkWidth).asInput
    val westOut = new ArgoLink(linkWidth).asOutput
    val westIn = new SelLink(linkWidth).asInput
    val localOut = new ArgoLink(linkWidth).asOutput
    val localIn = new SelLink(linkWidth).asInput
  }

// Func format:
//  source port:     4    3    2    1    0
//  dest port:    3210 4210 3410 3240 3214

  io.southOut := Mux(io.northIn.sel(0),io.northIn,
                  Mux(io.eastIn.sel(0),io.eastIn,
                  Mux(io.westIn.sel(0),io.westIn,
                  Mux(io.localIn.sel(0),io.localIn,Bits(0)))))

//  io.southOut := (io.northIn & Fill(io.northIn.sel(0))) |
//                 (io.eastIn  & Fill(io.eastIn.sel(0)))  |
//                 (io.westIn  & Fill(io.westIn.sel(0)))  |
//                 (io.localIn & Fill(io.localIn.sel(0)))

  io.westOut  := Mux(io.southIn.sel(1), io.southIn,
                 Mux(io.eastIn.sel(1), io.eastIn,
                 Mux(io.northIn.sel(1), io.northIn,
                 Mux(io.localIn.sel(1), io.localIn,Bits(0)))))

  io.northOut := Mux(io.southIn.sel(2) ,io.southIn,
                 Mux(io.eastIn.sel(2)  ,io.eastIn,
                 Mux(io.westIn.sel(2)  ,io.westIn,
                 Mux(io.localIn.sel(2) ,io.localIn,Bits(0)))))

  io.eastOut  := Mux(io.southIn.sel(3) ,io.southIn,
                 Mux(io.northIn.sel(3) ,io.northIn,
                 Mux(io.westIn.sel(3)  ,io.westIn,
                 Mux(io.localIn.sel(3) ,io.localIn,Bits(0)))))

  io.localOut := Mux(io.southIn.sel(0) ,io.southIn,
                 Mux(io.westIn.sel(1)  ,io.westIn,
                 Mux(io.northIn.sel(2) ,io.northIn,
                 Mux(io.eastIn.sel(3)  ,io.eastIn,Bits(0)))))

}
