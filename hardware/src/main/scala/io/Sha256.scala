/*
 * SHA256 module
 *
 * Authors: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package io

import Chisel._

import patmos.Constants._

import chisel3.dontTouch

import ocp._

object Sha256 extends DeviceObject {
  def init(params: Map[String, String]) = {
  }
  def create(params: Map[String, String]) : Sha256 = {
    Module(new Sha256())
  }
}

class Sha256() extends CoreDevice() {

  override val io = IO(new CoreDeviceIO())
  
  // Register for requests from OCP master
  val masterReg = Reg(next = io.ocp.M)

  // Constants for SHA256
  val DATA_WIDTH = 32
  val ROUND_COUNT = 64
  val HASH_WORD_COUNT = 8
  val MSG_WORD_COUNT = 16

  val hashDefaults = Vec(Seq(
    "h6a09e667".U(DATA_WIDTH.W), "hbb67ae85".U,
    "h3c6ef372".U, "ha54ff53a".U,
    "h510e527f".U, "h9b05688c".U,
    "h1f83d9ab".U, "h5be0cd19".U
  ))

  val roundConsts = Vec(Seq(
    "h428a2f98".U(DATA_WIDTH.W), "h71374491".U,
    "hb5c0fbcf".U, "he9b5dba5".U,
    "h3956c25b".U, "h59f111f1".U,
    "h923f82a4".U, "hab1c5ed5".U,
    "hd807aa98".U, "h12835b01".U,
    "h243185be".U, "h550c7dc3".U,
    "h72be5d74".U, "h80deb1fe".U,
    "h9bdc06a7".U, "hc19bf174".U,
    "he49b69c1".U, "hefbe4786".U,
    "h0fc19dc6".U, "h240ca1cc".U,
    "h2de92c6f".U, "h4a7484aa".U,
    "h5cb0a9dc".U, "h76f988da".U,
    "h983e5152".U, "ha831c66d".U,
    "hb00327c8".U, "hbf597fc7".U,
    "hc6e00bf3".U, "hd5a79147".U,
    "h06ca6351".U, "h14292967".U,
    "h27b70a85".U, "h2e1b2138".U,
    "h4d2c6dfc".U, "h53380d13".U,
    "h650a7354".U, "h766a0abb".U,
    "h81c2c92e".U, "h92722c85".U,
    "ha2bfe8a1".U, "ha81a664b".U,
    "hc24b8b70".U, "hc76c51a3".U,
    "hd192e819".U, "hd6990624".U,
    "hf40e3585".U, "h106aa070".U,
    "h19a4c116".U, "h1e376c08".U,
    "h2748774c".U, "h34b0bcb5".U,
    "h391c0cb3".U, "h4ed8aa4a".U,
    "h5b9cca4f".U, "h682e6ff3".U,
    "h748f82ee".U, "h78a5636f".U,
    "h84c87814".U, "h8cc70208".U,
    "h90befffa".U, "ha4506ceb".U,
    "hbef9a3f7".U, "hc67178f2".U
  ))

  // The hash value
  val hash = Reg(Vec(HASH_WORD_COUNT, UInt(DATA_WIDTH.W)))

  // Temporary registers
  val a = Reg(UInt(DATA_WIDTH.W))
  val b = Reg(UInt(DATA_WIDTH.W))
  val c = Reg(UInt(DATA_WIDTH.W))
  val d = Reg(UInt(DATA_WIDTH.W))
  val e = Reg(UInt(DATA_WIDTH.W))
  val f = Reg(UInt(DATA_WIDTH.W))
  val g = Reg(UInt(DATA_WIDTH.W))
  val h = Reg(UInt(DATA_WIDTH.W))

  // Index
  val idxReg = Reg(init = 0.U((log2Up(ROUND_COUNT)+1).W))
  idxReg := io.ocp.M.Addr(log2Up(MSG_WORD_COUNT)+1, 2)

  // Message memory
  val msg = Mem(UInt(DATA_WIDTH.W), MSG_WORD_COUNT)

  // Read data from message memory
  val msgRdData = msg(idxReg(log2Up(MSG_WORD_COUNT)-1, 0))

  // Helper signal for byte enables
  val comb = Wire(Vec(masterReg.ByteEn.getWidth, UInt(8.W)))
  for (i <- 0 until masterReg.ByteEn.getWidth) {
    comb(i) := 0.U
  }

  // States
  val idle :: restart :: start :: compress :: update :: Nil = Enum(UInt(), 5)
  val stateReg = Reg(init = restart)

  // Transformation functions
  def rotateRight(data : UInt, amt : Int) = {
    data(amt-1, 0) ## data(DATA_WIDTH-1, amt)
  }
  def s0(data : UInt) = {
    rotateRight(data, 7) ^ rotateRight(data, 18) ^ (data.asUInt >> 3.U)
  }
  def s1(data : UInt) = {
    rotateRight(data, 17) ^ rotateRight(data, 19) ^ (data.asUInt >> 10.U)
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
  val w0 = Wire(UInt(DATA_WIDTH.W))
  val wt = Reg(Vec(13, UInt(DATA_WIDTH.W)))
  val wx = Reg(UInt(DATA_WIDTH.W))
  val wy = Reg(UInt(DATA_WIDTH.W))
  val wz = Reg(UInt(DATA_WIDTH.W))
  for (i <- 1 until 13) {
    wt(i) := wt(i-1)
  }
  wx := wt(12) + s0(wt(11))
  wy := wx + wt(4)
  wz := wy + s1(wt(0))
  w0 := Mux(idxReg < MSG_WORD_COUNT.U, msgRdData, wz)
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
    idxReg := idxReg + 1.U

    stateReg := compress
  }

  // Compression loop
  when (stateReg === compress) {
    idxReg := idxReg + 1.U
    when (idxReg < ROUND_COUNT.U) {
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
  io.ocp.S.Data := 0.U(DATA_WIDTH.W)

  // Handle OCP reads
  when (masterReg.Cmd === OcpCmd.RD) {
    io.ocp.S.Resp := OcpResp.DVA
    // Read state
    when (masterReg.Addr(7, 2) === "b000000".U) {
      io.ocp.S.Data := Cat(0.U((DATA_WIDTH-1).W), stateReg =/= idle)
    }
    // Read hash value
    when (masterReg.Addr(7, 5) === "b010".U) {
      io.ocp.S.Data := Mux(stateReg === idle,
                           hash(masterReg.Addr(log2Up(HASH_WORD_COUNT)+1, 2)),
                           0.U)
    }
  }

  // Handle OCP writes
  when (masterReg.Cmd === OcpCmd.WR) {
    io.ocp.S.Resp := OcpResp.DVA
    when (stateReg === idle) {      
      when (masterReg.Addr(7, 2) === "b000000".U) {
        when(masterReg.Data(0) === 0.U){ // Reset seed
          stateReg := restart
        }.elsewhen(masterReg.Data(0) === 1.U){// Start computation
          idxReg := 0.U
          stateReg := start
        }
      }
      // Write seed value
      when (masterReg.Addr(7, 5) === "b010".U) {
        hash(masterReg.Addr(log2Up(HASH_WORD_COUNT)+1, 2)) := masterReg.Data
      }
      // Write data
      when (masterReg.Addr(7, 6) === "b10".U) {
        // Fill in data according to byte enables
        for (i <- 0 until masterReg.ByteEn.getWidth) {
          comb(i) := Mux(masterReg.ByteEn(i) === 1.U,
                         masterReg.Data(8*i+7, 8*i),
                         msgRdData(8*i+7, 8*i))
        }
        msg(masterReg.Addr(log2Up(MSG_WORD_COUNT)+1, 2)) := comb.reduceLeft((x,y) => y##x)
      }
    }
  }

}
