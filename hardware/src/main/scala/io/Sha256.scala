/*
 * SHA256 module
 *
 * Authors: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package io

import Chisel._

import ocp._

object Sha256 extends DeviceObject {
  def init(params: Map[String, String]) = {
  }
  def create(params: Map[String, String]) : Sha256 = {
    Module(new Sha256())
  }
  trait Pins {
  }
}

class Sha256() extends CoreDevice() {

  override val io = new CoreDeviceIO() with Sha256.Pins
  
  // Register for requests from OCP master
  val masterReg = Reg(next = io.ocp.M)

  // Constants for SHA256
  val DATA_WIDTH = 32
  val ROUND_COUNT = 64
  val HASH_WORD_COUNT = 8
  val MSG_WORD_COUNT = 16

  val hashDefaults = Vec(Seq(
    Bits("h6a09e667", width = DATA_WIDTH), Bits("hbb67ae85"),
    Bits("h3c6ef372"), Bits("ha54ff53a"),
    Bits("h510e527f"), Bits("h9b05688c"),
    Bits("h1f83d9ab"), Bits("h5be0cd19")
  ))

  val roundConsts = Vec(Seq(
    Bits("h428a2f98", width = DATA_WIDTH), Bits("h71374491"),
    Bits("hb5c0fbcf"), Bits("he9b5dba5"),
    Bits("h3956c25b"), Bits("h59f111f1"),
    Bits("h923f82a4"), Bits("hab1c5ed5"),
    Bits("hd807aa98"), Bits("h12835b01"),
    Bits("h243185be"), Bits("h550c7dc3"),
    Bits("h72be5d74"), Bits("h80deb1fe"),
    Bits("h9bdc06a7"), Bits("hc19bf174"),
    Bits("he49b69c1"), Bits("hefbe4786"),
    Bits("h0fc19dc6"), Bits("h240ca1cc"),
    Bits("h2de92c6f"), Bits("h4a7484aa"),
    Bits("h5cb0a9dc"), Bits("h76f988da"),
    Bits("h983e5152"), Bits("ha831c66d"),
    Bits("hb00327c8"), Bits("hbf597fc7"),
    Bits("hc6e00bf3"), Bits("hd5a79147"),
    Bits("h06ca6351"), Bits("h14292967"),
    Bits("h27b70a85"), Bits("h2e1b2138"),
    Bits("h4d2c6dfc"), Bits("h53380d13"),
    Bits("h650a7354"), Bits("h766a0abb"),
    Bits("h81c2c92e"), Bits("h92722c85"),
    Bits("ha2bfe8a1"), Bits("ha81a664b"),
    Bits("hc24b8b70"), Bits("hc76c51a3"),
    Bits("hd192e819"), Bits("hd6990624"),
    Bits("hf40e3585"), Bits("h106aa070"),
    Bits("h19a4c116"), Bits("h1e376c08"),
    Bits("h2748774c"), Bits("h34b0bcb5"),
    Bits("h391c0cb3"), Bits("h4ed8aa4a"),
    Bits("h5b9cca4f"), Bits("h682e6ff3"),
    Bits("h748f82ee"), Bits("h78a5636f"),
    Bits("h84c87814"), Bits("h8cc70208"),
    Bits("h90befffa"), Bits("ha4506ceb"),
    Bits("hbef9a3f7"), Bits("hc67178f2")
  ))

  // The hash value
  val hash = Vec.fill(HASH_WORD_COUNT) { Reg(Bits(width = DATA_WIDTH)) }

  // Temporary registers
  val a = Reg(Bits(width = DATA_WIDTH))
  val b = Reg(Bits(width = DATA_WIDTH))
  val c = Reg(Bits(width = DATA_WIDTH))
  val d = Reg(Bits(width = DATA_WIDTH))
  val e = Reg(Bits(width = DATA_WIDTH))
  val f = Reg(Bits(width = DATA_WIDTH))
  val g = Reg(Bits(width = DATA_WIDTH))
  val h = Reg(Bits(width = DATA_WIDTH))

  // Index
  val idxReg = Reg(init = UInt(0, width = log2Up(ROUND_COUNT)+1))
  idxReg := io.ocp.M.Addr(log2Up(MSG_WORD_COUNT)+1, 2)

  // Message memory
  val msg = Mem(Bits(width = DATA_WIDTH), MSG_WORD_COUNT)

  // Read data from message memory
  val msgRdData = msg(idxReg(log2Up(MSG_WORD_COUNT)-1, 0))

  // Helper signal for byte enables
  val comb = Vec.fill(masterReg.ByteEn.getWidth()) { Bits(width = 8) }
  for (i <- 0 until masterReg.ByteEn.getWidth()) {
    comb(i) := Bits(0)
  }

  // States
  val idle :: restart :: start :: compress :: update :: Nil = Enum(UInt(), 5)
  val stateReg = Reg(init = restart)

  // Transformation functions
  def rotateRight(data : Bits, amt : Int) = {
    data(amt-1, 0) ## data(DATA_WIDTH-1, amt)
  }
  def s0(data : Bits) = {
    rotateRight(data, 7) ^ rotateRight(data, 18) ^ (data.toUInt >> UInt(3))
  }
  def s1(data : Bits) = {
    rotateRight(data, 17) ^ rotateRight(data, 19) ^ (data.toUInt >> UInt(10))
  }
  def e0(data : Bits) = {
    rotateRight(data, 2) ^ rotateRight(data, 13) ^ rotateRight(data, 22)
  }
  def e1(data : Bits) = {
    rotateRight(data, 6) ^ rotateRight(data, 11) ^ rotateRight(data, 25)
  }
  def ch(x : Bits, y : Bits, z : Bits) = {
    (x & y) ^ (~x & z)
  }
  def maj(x : Bits, y : Bits, z : Bits) = {
    (x & y) ^ (x & z) ^ (y & z)
  }

  // Reset hash value
  when (stateReg === restart) {
    hash := hashDefaults
    stateReg := idle
  }

  // On-the-fly expansion of working memory
  // See Chavez et al., "Improving SHA-2 Hardware Implementations", CHES 2006
  val w0 = Bits(width = DATA_WIDTH)
  val wt = Vec.fill(13) { Reg(Bits(width = DATA_WIDTH)) }
  val wx = Reg(Bits(width = DATA_WIDTH))
  val wy = Reg(Bits(width = DATA_WIDTH))
  val wz = Reg(Bits(width = DATA_WIDTH))
  for (i <- 1 until 13) {
    wt(i) := wt(i-1)
  }
  wx := wt(12) + s0(wt(11))
  wy := wx + wt(4)
  wz := wy + s1(wt(0))
  w0 := Mux(idxReg < UInt(MSG_WORD_COUNT), msgRdData, wz)
  wt(0) := w0

  // Compression
  val pipeReg = Reg(next = w0 + roundConsts(idxReg(log2Up(ROUND_COUNT)-1, 0)))
  val temp1 = h + e1(e) + ch(e, f, g) + pipeReg
  val temp2 = e0(a) + maj(a, b, c)

  h := g
  g := f
  f := e
  e := d + temp1
  d := c
  c := b
  b := a
  a := temp1 + temp2

  // Start compression
  when (stateReg === start) {
    a := hash(0)
    b := hash(1)
    c := hash(2)
    d := hash(3)
    e := hash(4)
    f := hash(5)
    g := hash(6)
    h := hash(7)
    idxReg := idxReg + UInt(1)

    stateReg := compress
  }

  // Compression loop
  when (stateReg === compress) {
    idxReg := idxReg + UInt(1)
    when (idxReg < UInt(ROUND_COUNT)) {
      stateReg := compress
    }
    .otherwise {
      stateReg := update
    }
  }

  // Update hash value
  when (stateReg === update) {
    hash(0) := hash(0) + a
    hash(1) := hash(1) + b
    hash(2) := hash(2) + c
    hash(3) := hash(3) + d
    hash(4) := hash(4) + e
    hash(5) := hash(5) + f
    hash(6) := hash(6) + g
    hash(7) := hash(7) + h
    
    stateReg := idle
  }

  // Default OCP response
  io.ocp.S.Resp := OcpResp.NULL
  io.ocp.S.Data := Bits(0, width = DATA_WIDTH)

  // Handle OCP reads
  when (masterReg.Cmd === OcpCmd.RD) {
    io.ocp.S.Resp := OcpResp.DVA
    // Read state
    when (masterReg.Addr(7, 2) === Bits("b000000")) {
      io.ocp.S.Data := Cat(Bits(0, width = DATA_WIDTH-1), stateReg =/= idle)
    }
    // Read hash value
    when (masterReg.Addr(7, 5) === Bits("b010")) {
      io.ocp.S.Data := Mux(stateReg === idle,
                           hash(masterReg.Addr(log2Up(HASH_WORD_COUNT)+1, 2)),
                           Bits(0))
    }
  }

  // Handle OCP writes
  when (masterReg.Cmd === OcpCmd.WR) {
    io.ocp.S.Resp := OcpResp.DVA
    when (stateReg === idle) {      
      when (masterReg.Addr(7, 2) === Bits("b000000")) {
        switch (masterReg.Data(0)) {
          // Reset seed
          is(Bits(0)) {
            stateReg := restart
          }
          // Start computation
          is(Bits(1)) {
            idxReg := UInt(0)
            stateReg := start
          }
        }
      }
      // Write seed value
      when (masterReg.Addr(7, 5) === Bits("b010")) {
        hash(masterReg.Addr(log2Up(HASH_WORD_COUNT)+1, 2)) := masterReg.Data
      }
      // Write data
      when (masterReg.Addr(7, 6) === Bits("b10")) {
        // Fill in data according to byte enables
        for (i <- 0 until masterReg.ByteEn.getWidth()) {
          comb(i) := Mux(masterReg.ByteEn(i) === Bits(1),
                         masterReg.Data(8*i+7, 8*i),
                         msgRdData(8*i+7, 8*i))
        }
        msg(masterReg.Addr(log2Up(MSG_WORD_COUNT)+1, 2)) := comb.reduceLeft((x,y) => y##x)
      }
    }
  }

}
