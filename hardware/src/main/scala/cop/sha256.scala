/*
 * SHA256 Coprocessor
 *
 * Authors: Clemens Pircher (clemens.lukas@gmx.at)
 * SHA256 state machine is taken (and adapted) from the SHA256 IO Device by Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package cop

import Chisel._

import patmos.Constants._
import util._
import ocp._

object Sha256 extends CoprocessorObject {

  def init(params: Map[String, String]) = {}

  def create(params: Map[String, String]): Sha256 = Module(new Sha256())
}


class Sha256() extends Coprocessor_MemoryAccess() {
  /*----------------------------------------------constants for SHA256-----------------------------------------------*/
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
  
  /*--------------------------------------------constants for coprocessor--------------------------------------------*/
  // coprocessor function definitions
  val FUNC_RESET            = "b00000".U(5.W)   // reset hash state (COP_WRITE)
  val FUNC_POLL             = "b00001".U(5.W)   // check whether computation is in progress (COP_READ)
  val FUNC_SET_HASH         = "b00010".U(5.W)   // set the hash state (COP_WRITE src_addr)
  val FUNC_GET_HASH         = "b00011".U(5.W)   // get the hash state (COP_WRITE dest_addr) 
  val FUNC_SINGLE_BLOCK     = "b00100".U(5.W)   // hash a single block (COP_WRITE src_addr)
  val FUNC_MULTIPLE_BLOCKS  = "b00101".U(5.W)   // hash multiple blocks (COP_WRITE src_addr block_count)

  // general helper constants
  val BURSTS_PER_MSG = MSG_WORD_COUNT / BURST_LENGTH
  val MSG_WORD_COUNT_WIDTH = log2Ceil(MSG_WORD_COUNT)
  val BURSTS_PER_HASH = HASH_WORD_COUNT / BURST_LENGTH
  val HASH_WORD_COUNT_WIDTH = log2Ceil(HASH_WORD_COUNT)
  val WORD_COUNT_WIDTH = MSG_WORD_COUNT_WIDTH.max(HASH_WORD_COUNT_WIDTH)
  val BURST_ADDR_OFFSET = BURST_LENGTH * DATA_WIDTH / BYTE_WIDTH
  val BURST_OFFSET = log2Ceil(BURST_LENGTH)
    
  /*---------------------------------------------sha256 state variables----------------------------------------------*/
  // the hash value
  val hash = Reg(Vec(HASH_WORD_COUNT, UInt(DATA_WIDTH.W)))

  // temporary registers
  val a = Reg(UInt(width = DATA_WIDTH))
  val b = Reg(UInt(width = DATA_WIDTH))
  val c = Reg(UInt(width = DATA_WIDTH))
  val d = Reg(UInt(width = DATA_WIDTH))
  val e = Reg(UInt(width = DATA_WIDTH))
  val f = Reg(UInt(width = DATA_WIDTH))
  val g = Reg(UInt(width = DATA_WIDTH))
  val h = Reg(UInt(width = DATA_WIDTH))

  // index
  val idxReg = Reg(init = UInt(0, width = log2Ceil(ROUND_COUNT)+1))

  // message memory
  val msg = Mem(UInt(width = DATA_WIDTH), MSG_WORD_COUNT)

  // read data from message memory
  val msgRdData = msg(idxReg(log2Ceil(MSG_WORD_COUNT)-1, 0))

  // states
  val idle :: restart :: start :: compress :: update :: Nil = Enum(UInt(), 5)
  val stateReg = Reg(init = restart)

  /*------------------------------------------coprocessor state variables--------------------------------------------*/
  // state machine for memory reads/writes
  /*val mem_idle ::
    mem_read_req_m :: mem_read_m ::     // reading message from memory
    mem_read_req_h :: mem_read_h ::     // reading hash (seed) from memory
    mem_write_req_h :: mem_write_h ::   // writing hash to memory
    Nil = Enum(7)*/
  val mem_idle :: mem_read_req_m :: mem_read_m :: mem_read_req_h :: mem_read_h :: mem_write_req_h :: mem_write_h :: Nil = Enum(7)
  val mem_state = RegInit(mem_idle)
  val block_addr = Reg(UInt(width = DATA_WIDTH))
  val hash_addr = Reg(UInt(width = DATA_WIDTH))
  val word_count = RegInit(0.U(WORD_COUNT_WIDTH.W))
  val block_count = Reg(UInt(width = DATA_WIDTH))
  val is_idle = Wire(Bool())

  /*----------------------------------------------sha256 state machine-----------------------------------------------*/  
  // transformation functions
  def rotateRight(data : UInt, amt : Int) = {
    data(amt-1, 0) ## data(DATA_WIDTH-1, amt)
  }
  def s0(data : UInt) = {
    rotateRight(data, 7) ^ rotateRight(data, 18) ^ (data.asUInt >> UInt(3))
  }
  def s1(data : UInt) = {
    rotateRight(data, 17) ^ rotateRight(data, 19) ^ (data.asUInt >> UInt(10))
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

  // reset hash value
  when (stateReg === restart) {
    hash := hashDefaults
    stateReg := idle
  }

  // on-the-fly expansion of working memory
  // see Chavez et al., "Improving SHA-2 Hardware Implementations", CHES 2006
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
  w0 := Mux(idxReg < UInt(MSG_WORD_COUNT), msgRdData, wz)
  wt(0) := w0

  // compression
  val pipeReg = Reg(next = w0 + roundConsts(idxReg(log2Ceil(ROUND_COUNT)-1, 0)))
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

  // start compression
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

  // compression loop
  when (stateReg === compress) {
    idxReg := idxReg + UInt(1)
    when (idxReg < UInt(ROUND_COUNT)) {
      stateReg := compress
    }
    .otherwise {
      stateReg := update
    }
  }

  // update hash value
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
    
    // modification to handle multi-block transfers
    when(block_count > 0.U) {
      mem_state := mem_read_req_m
    }
  }
  
  /*--------------------------------------------coprocessor state machine--------------------------------------------*/
  // we can only accept new commands when the computation has finished and *all* memory transfers completed
  is_idle := stateReg === idle && mem_state === mem_idle && block_count === 0.U
  
  // default values
  io.copOut.result := 0.U
  io.copOut.ena_out := Bool(false)
  
  
  // start operation
  when(io.copIn.trigger & io.copIn.ena_in) {
    when(io.copIn.isCustom) {
      // no custom operations
    }.elsewhen(io.copIn.read) {
      switch(io.copIn.funcId) {
        is(FUNC_POLL) {
          io.copOut.result := Cat(UInt(0, width = DATA_WIDTH - 1), !is_idle)
          io.copOut.ena_out := Bool(true)
        }
      }
    }.otherwise{
      switch(io.copIn.funcId) {
        is(FUNC_RESET) {
          when(is_idle) {
            stateReg := restart
            io.copOut.ena_out := Bool(true)
          }
        }
        is(FUNC_SET_HASH) {
          when(is_idle) {
            hash_addr := io.copIn.opData(0)
            mem_state := mem_read_req_h
            io.copOut.ena_out := Bool(true)
          }
        }
        is(FUNC_GET_HASH) {
          when(is_idle) {
            hash_addr := io.copIn.opData(0)
            mem_state := mem_write_req_h
            io.copOut.ena_out := Bool(true)
          }
        }
        is(FUNC_SINGLE_BLOCK) {
          when(is_idle) {
            block_addr := io.copIn.opData(0)
            mem_state := mem_read_req_m
            block_count := UInt(1)
            io.copOut.ena_out := Bool(true)
          }
        }
        is(FUNC_MULTIPLE_BLOCKS) {
          when(is_idle) {
            block_addr := io.copIn.opData(0)
            mem_state := mem_read_req_m
            block_count := io.copIn.opData(1)
            io.copOut.ena_out := Bool(true)
          }
        }
      }
    }
  }
  
  // memory logic (note: this assumes that all addresses are burst-aligned!)
  io.memPort.M.Cmd := OcpCmd.IDLE
  io.memPort.M.Addr := 0.U
  io.memPort.M.Data := 0.U
  io.memPort.M.DataValid := 0.U
  io.memPort.M.DataByteEn := "b1111".U
  switch(mem_state) {
    is(mem_read_req_m) {
      io.memPort.M.Cmd := OcpCmd.RD
      io.memPort.M.Addr := block_addr
      when(io.memPort.S.CmdAccept === 1.U) {
        mem_state := mem_read_m
      }
    }
    is(mem_read_m) {
      msg(word_count(MSG_WORD_COUNT_WIDTH - 1, 0)) := io.memPort.S.Data
      when(io.memPort.S.Resp === OcpResp.DVA) {
        when(word_count(BURST_OFFSET - 1, 0) < UInt(BURST_LENGTH - 1)) {
          word_count := word_count + 1.U
        }.otherwise {
          when(word_count(MSG_WORD_COUNT_WIDTH - 1, BURST_OFFSET) < UInt(BURSTS_PER_MSG - 1)) {
            word_count := word_count + 1.U
            block_addr := block_addr + UInt(BURST_ADDR_OFFSET)
            mem_state := mem_read_req_m
          }.otherwise {
            word_count := 0.U
            mem_state := mem_idle
          }
        }
      }
    }
    is(mem_read_req_h) {
      io.memPort.M.Cmd := OcpCmd.RD
      io.memPort.M.Addr := hash_addr
      when(io.memPort.S.CmdAccept === UInt(1)) {
        mem_state := mem_read_h
      }
    }
    is(mem_read_h) {
      hash(word_count(HASH_WORD_COUNT_WIDTH - 1, 0)) := io.memPort.S.Data
      when(io.memPort.S.Resp === OcpResp.DVA) {
        when(word_count(BURST_OFFSET - 1, 0) < UInt(BURST_LENGTH - 1)) {
          word_count := word_count + 1.U
        }.otherwise {
          when(word_count(HASH_WORD_COUNT_WIDTH - 1, BURST_OFFSET) < UInt(BURSTS_PER_MSG - 1)) {
          word_count := word_count + 1.U
            hash_addr := hash_addr + UInt(BURST_ADDR_OFFSET)
            mem_state := mem_read_req_h
          }.otherwise {
            word_count := 0.U
            mem_state := mem_idle
            stateReg := start
            block_count := block_count - 1.U
          }
        }
      }
    }
    is(mem_write_req_h) {
      io.memPort.M.Cmd := OcpCmd.WR
      io.memPort.M.Addr := hash_addr
      io.memPort.M.Data := hash(word_count(HASH_WORD_COUNT_WIDTH - 1, 0));
      io.memPort.M.DataValid := 1.U
      when(io.memPort.S.CmdAccept === 1.U && io.memPort.S.DataAccept === 1.U) {
        word_count := word_count + 1.U
        mem_state := mem_write_h
      }
    }
    is(mem_write_h) {
      io.memPort.M.Data := hash(word_count(HASH_WORD_COUNT_WIDTH - 1, 0));
      io.memPort.M.DataValid := 1.U
      when(io.memPort.S.DataAccept === UInt(1)) {
        when(word_count(BURST_OFFSET - 1, 0) < UInt(BURST_LENGTH - 1)) {
          word_count := word_count + 1.U
        }.otherwise {
          when(word_count(HASH_WORD_COUNT_WIDTH - 1, BURST_OFFSET) < UInt(BURSTS_PER_MSG - 1)) {
            word_count := word_count + 1.U
            hash_addr := hash_addr + UInt(BURST_ADDR_OFFSET)
            mem_state := mem_write_req_h
          }.otherwise {
            word_count := 0.U
            mem_state := idle
          }
        }
      }
    }
  }
}
