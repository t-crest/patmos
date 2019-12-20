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
    UInt("h6a09e667", width = DATA_WIDTH), UInt("hbb67ae85"),
    UInt("h3c6ef372"), UInt("ha54ff53a"),
    UInt("h510e527f"), UInt("h9b05688c"),
    UInt("h1f83d9ab"), UInt("h5be0cd19")
  ))

  val roundConsts = Vec(Seq(
    UInt("h428a2f98", width = DATA_WIDTH), UInt("h71374491"),
    UInt("hb5c0fbcf"), UInt("he9b5dba5"),
    UInt("h3956c25b"), UInt("h59f111f1"),
    UInt("h923f82a4"), UInt("hab1c5ed5"),
    UInt("hd807aa98"), UInt("h12835b01"),
    UInt("h243185be"), UInt("h550c7dc3"),
    UInt("h72be5d74"), UInt("h80deb1fe"),
    UInt("h9bdc06a7"), UInt("hc19bf174"),
    UInt("he49b69c1"), UInt("hefbe4786"),
    UInt("h0fc19dc6"), UInt("h240ca1cc"),
    UInt("h2de92c6f"), UInt("h4a7484aa"),
    UInt("h5cb0a9dc"), UInt("h76f988da"),
    UInt("h983e5152"), UInt("ha831c66d"),
    UInt("hb00327c8"), UInt("hbf597fc7"),
    UInt("hc6e00bf3"), UInt("hd5a79147"),
    UInt("h06ca6351"), UInt("h14292967"),
    UInt("h27b70a85"), UInt("h2e1b2138"),
    UInt("h4d2c6dfc"), UInt("h53380d13"),
    UInt("h650a7354"), UInt("h766a0abb"),
    UInt("h81c2c92e"), UInt("h92722c85"),
    UInt("ha2bfe8a1"), UInt("ha81a664b"),
    UInt("hc24b8b70"), UInt("hc76c51a3"),
    UInt("hd192e819"), UInt("hd6990624"),
    UInt("hf40e3585"), UInt("h106aa070"),
    UInt("h19a4c116"), UInt("h1e376c08"),
    UInt("h2748774c"), UInt("h34b0bcb5"),
    UInt("h391c0cb3"), UInt("h4ed8aa4a"),
    UInt("h5b9cca4f"), UInt("h682e6ff3"),
    UInt("h748f82ee"), UInt("h78a5636f"),
    UInt("h84c87814"), UInt("h8cc70208"),
    UInt("h90befffa"), UInt("ha4506ceb"),
    UInt("hbef9a3f7"), UInt("hc67178f2")
  ))

  // The hash value
  val hash = Vec.fill(HASH_WORD_COUNT) { Reg(UInt(width = DATA_WIDTH)) }

  // Temporary registers
  val a = Reg(UInt(width = DATA_WIDTH))
  val b = Reg(UInt(width = DATA_WIDTH))
  val c = Reg(UInt(width = DATA_WIDTH))
  val d = Reg(UInt(width = DATA_WIDTH))
  val e = Reg(UInt(width = DATA_WIDTH))
  val f = Reg(UInt(width = DATA_WIDTH))
  val g = Reg(UInt(width = DATA_WIDTH))
  val h = Reg(UInt(width = DATA_WIDTH))

  // Index
  val idxReg = Reg(init = UInt(0, width = log2Up(ROUND_COUNT)+1))
  idxReg := io.ocp.M.Addr(log2Up(MSG_WORD_COUNT)+1, 2)

  // Message memory
  val msg = Mem(UInt(width = DATA_WIDTH), MSG_WORD_COUNT)

  // Read data from message memory
  val msgRdData = msg(idxReg(log2Up(MSG_WORD_COUNT)-1, 0))

  // Helper signal for byte enables
  val comb = Vec.fill(masterReg.ByteEn.getWidth) { UInt(width = 8) }
  for (i <- 0 until masterReg.ByteEn.getWidth) {
    comb(i) := UInt(0)
  }

  // States
  val idle :: restart :: start :: compress :: update :: Nil = Enum(UInt(), 5)
  val stateReg = Reg(init = restart)

  // Transformation functions
  def rotateRight(data : UInt, amt : Int) = {
    data(amt-1, 0) ## data(DATA_WIDTH-1, amt)
  }
  def s0(data : UInt) = {
    rotateRight(data, 7) ^ rotateRight(data, 18) ^ (data.toUInt >> UInt(3))
  }
  def s1(data : UInt) = {
    rotateRight(data, 17) ^ rotateRight(data, 19) ^ (data.toUInt >> UInt(10))
  }
  def e0(data : UInt) = {
    rotateRight(data, 2) ^ rotateRight(data, 13) ^ rotateRight(data, 22)
  }
  def e1(data : UInt) = {
    rotateRight(data, 6) ^ rotateRight(data, 11) ^ rotateRight(data, 25)
  }
  def ch(x : UInt, y : UInt, z : UInt) = {
    (x & y) ^ (~x & z)
  }
  def maj(x : UInt, y : UInt, z : UInt) = {
    (x & y) ^ (x & z) ^ (y & z)
  }

  // Reset hash value
  when (stateReg === restart) {
    hash := hashDefaults
    stateReg := idle
  }

  // On-the-fly expansion of working memory
  // See Chavez et al., "Improving SHA-2 Hardware Implementations", CHES 2006
  val w0 = UInt(width = DATA_WIDTH)
  val wt = Vec.fill(13) { Reg(UInt(width = DATA_WIDTH)) }
  val wx = Reg(UInt(width = DATA_WIDTH))
  val wy = Reg(UInt(width = DATA_WIDTH))
  val wz = Reg(UInt(width = DATA_WIDTH))
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
  io.ocp.S.Data := UInt(0, width = DATA_WIDTH)

  // Handle OCP reads
  when (masterReg.Cmd === OcpCmd.RD) {
    io.ocp.S.Resp := OcpResp.DVA
    // Read state
    when (masterReg.Addr(7, 2) === UInt("b000000")) {
      io.ocp.S.Data := Cat(UInt(0, width = DATA_WIDTH-1), stateReg =/= idle)
    }
    // Read hash value
    when (masterReg.Addr(7, 5) === UInt("b010")) {
      io.ocp.S.Data := Mux(stateReg === idle,
                           hash(masterReg.Addr(log2Up(HASH_WORD_COUNT)+1, 2)),
                           UInt(0))
    }
  }

  // Handle OCP writes
  when (masterReg.Cmd === OcpCmd.WR) {
    io.ocp.S.Resp := OcpResp.DVA
    when (stateReg === idle) {      
      when (masterReg.Addr(7, 2) === UInt("b000000")) {
        when(masterReg.Data(0) === UInt(0)){ // Reset seed
          stateReg := restart
        }.elsewhen(masterReg.Data(0) === UInt(1)){// Start computation
          idxReg := UInt(0)
          stateReg := start
        }
      }
      // Write seed value
      when (masterReg.Addr(7, 5) === UInt("b010")) {
        hash(masterReg.Addr(log2Up(HASH_WORD_COUNT)+1, 2)) := masterReg.Data
      }
      // Write data
      when (masterReg.Addr(7, 6) === UInt("b10")) {
        // Fill in data according to byte enables
        for (i <- 0 until masterReg.ByteEn.getWidth) {
          comb(i) := Mux(masterReg.ByteEn(i) === UInt(1),
                         masterReg.Data(8*i+7, 8*i),
                         msgRdData(8*i+7, 8*i))
        }
        msg(masterReg.Addr(log2Up(MSG_WORD_COUNT)+1, 2)) := comb.reduceLeft((x,y) => y##x)
      }
    }
  }

}
