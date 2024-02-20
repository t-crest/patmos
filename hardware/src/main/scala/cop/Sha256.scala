/*
 * SHA256 Coprocessor
 *
 * Authors: Clemens Pircher (clemens.lukas@gmx.at)
 * SHA256 state machine is taken (and adapted) from the SHA256 IO Device by Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package cop

import chisel3._
import chisel3.util._

import patmos.Constants._
import ocp._

object Sha256 extends CoprocessorObject {

  def init(params: Map[String, String]) = {}

  def create(params: Map[String, String]): Sha256 = Module(new Sha256())
}


class Sha256() extends CoprocessorMemoryAccess() {
  /*----------------------------------------------constants for SHA256-----------------------------------------------*/
  val ROUND_COUNT = 64
  val HASH_WORD_COUNT = 8
  val MSG_WORD_COUNT = 16
  
  val hashDefaults = VecInit(Seq(
    "h6a09e667".U(DATA_WIDTH.W), "hbb67ae85".U,
    "h3c6ef372".U, "ha54ff53a".U,
    "h510e527f".U, "h9b05688c".U,
    "h1f83d9ab".U, "h5be0cd19".U
  ))

  val roundConsts = VecInit(Seq(
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
  
  /*--------------------------------------------constants for coprocessor--------------------------------------------*/
  // coprocessor function definitions
  val FUNC_RESET            = "b00000".U(5.W)   // reset hash state (COP_WRITE)
  val FUNC_POLL             = "b00001".U(5.W)   // check whether computation is in progress (COP_READ)
  val FUNC_SET_HASH         = "b00010".U(5.W)   // set the hash state (COP_WRITE srcAddr)
  val FUNC_GET_HASH         = "b00011".U(5.W)   // get the hash state (COP_WRITE destAddr) 
  val FUNC_SINGLE_BLOCK     = "b00100".U(5.W)   // hash a single block (COP_WRITE srcAddr)
  val FUNC_MULTIPLE_BLOCKS  = "b00101".U(5.W)   // hash multiple blocks (COP_WRITE srcAddr blockCountReg)

  // general helper constants
  val BURSTS_PER_MSG = MSG_WORD_COUNT / BURST_LENGTH
  val MSG_WORD_COUNT_WIDTH = log2Ceil(MSG_WORD_COUNT)
  val BURSTS_PER_HASH = HASH_WORD_COUNT / BURST_LENGTH
  val HASH_WORD_COUNT_WIDTH = log2Ceil(HASH_WORD_COUNT)
  val WORD_COUNT_WIDTH = MSG_WORD_COUNT_WIDTH.max(HASH_WORD_COUNT_WIDTH)
  val BURST_ADDR_OFFSET = BURST_LENGTH * DATA_WIDTH / BYTE_WIDTH
  val BURST_OFFSET = log2Ceil(BURST_LENGTH)
  
  /*------------------------------------------------shared variables-------------------------------------------------*/
  val isIdle = Wire(Bool())
  val memBufferReg = RegInit(0.U(1.W))
  val ShaBufferReg = RegInit(0.U(1.W))
  
  /*---------------------------------------------sha256 state variables----------------------------------------------*/
  // the hash value
  val hash = Reg(Vec(HASH_WORD_COUNT, UInt(DATA_WIDTH.W)))

  // temporary registers
  val a = Reg(UInt(DATA_WIDTH.W))
  val b = Reg(UInt(DATA_WIDTH.W))
  val c = Reg(UInt(DATA_WIDTH.W))
  val d = Reg(UInt(DATA_WIDTH.W))
  val e = Reg(UInt(DATA_WIDTH.W))
  val f = Reg(UInt(DATA_WIDTH.W))
  val g = Reg(UInt(DATA_WIDTH.W))
  val h = Reg(UInt(DATA_WIDTH.W))

  // index
  val idxReg = RegInit(init = 0.U((log2Ceil(ROUND_COUNT)+1).W))

  // message memory (Note: has been extended to enable double-buffering)
  val msg = Mem(MSG_WORD_COUNT * 2, UInt(DATA_WIDTH.W))

  // read data from message memory
  val msgRdData = msg(Cat(ShaBufferReg, idxReg(log2Ceil(MSG_WORD_COUNT)-1, 0)))

  // states
  val idle :: restart :: start :: compress :: update :: waiting :: Nil = Enum(6)
  val stateReg = RegInit(init = restart)

  /*------------------------------------------coprocessor state variables--------------------------------------------*/
  // state machine for memory reads/writes
  /*val memIdle ::
    memReadReqM :: memReadM ::     // reading message from memory
    memReadReqH :: memReadH ::     // reading hash (seed) from memory
    memWriteReqH :: memWriteH ::   // writing hash to memory
    Nil = Enum(7)*/
  val memIdle :: memReadReqM :: memReadM :: memReadReqH :: memReadH :: memWriteReqH :: memWriteH :: memWait :: Nil = Enum(8)
  val memState = RegInit(memIdle)
  val blockAddrReg = Reg(UInt(DATA_WIDTH.W))
  val hashAddrReg = Reg(UInt(DATA_WIDTH.W))
  val wordCountReg = RegInit(0.U(WORD_COUNT_WIDTH.W))
  val blockCountReg = Reg(UInt(DATA_WIDTH.W))

  /*----------------------------------------------sha256 state machine-----------------------------------------------*/  
  // transformation functions
  def rotateRight(data : UInt, amt : Int) = {
    data(amt-1, 0) ## data(DATA_WIDTH-1, amt)
  }
  def s0(data : UInt) = {
    rotateRight(data, 7) ^ rotateRight(data, 18) ^ (data.asUInt >> 3.U).asUInt
  }
  def s1(data : UInt) = {
    rotateRight(data, 17) ^ rotateRight(data, 19) ^ (data.asUInt >> 10.U).asUInt
  }
  def e0(data : UInt) = {
    rotateRight(data, 2) ^ rotateRight(data, 13) ^ rotateRight(data, 22)
  }
  def e1(data : UInt) = {
    rotateRight(data, 6) ^ rotateRight(data, 11) ^ rotateRight(data, 25)
  }
  def ch(x : UInt, y : UInt, z : UInt) = {
    (x & y) ^ ((~x).asUInt & z)
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
  w0 := Mux(idxReg < MSG_WORD_COUNT.U, msgRdData, wz)
  wt(0) := w0

  // compression
  val pipeReg = RegNext(next = w0 + roundConsts(idxReg(log2Ceil(ROUND_COUNT)-1, 0)))
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
    idxReg := idxReg + 1.U

    stateReg := compress
  }

  // compression loop
  when (stateReg === compress) {
    idxReg := idxReg + 1.U
    when (idxReg < ROUND_COUNT.U) {
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
    
    ShaBufferReg := !ShaBufferReg
    
    // handle interleaved multi-block transfers
    when(memState === memWait) {
      // move on to the next block and restart memory state machine
      idxReg := 0.U
      stateReg := start
      memState := memReadReqM
    }.elsewhen(memState === memIdle) {
      when(ShaBufferReg === memBufferReg) {
        idxReg := 0.U
        stateReg := start // last pass
      }.otherwise {
        stateReg := idle  // we are done
      }
    }.otherwise {
      stateReg := waiting    // we have to wait
    }
  }
  
  /*--------------------------------------------coprocessor state machine--------------------------------------------*/
  // we can only accept new commands when the computation has finished and *all* memory transfers completed
  isIdle := stateReg === idle && memState === memIdle && blockCountReg === 0.U
  
  // default values
  io.copOut.result := 0.U
  io.copOut.ena_out := false.B
  
  // register for retrying operation
  val retryReg = RegInit(false.B)
  retryReg := ((io.copIn.trigger && io.copIn.ena_in) || retryReg) && !io.copOut.ena_out
  
  // start operation
  when((io.copIn.trigger || retryReg) && io.copIn.ena_in) {
    when(io.copIn.isCustom) {
      // no custom operations
    }.elsewhen(io.copIn.read) {
      switch(io.copIn.funcId) {
        is(FUNC_POLL) {
          io.copOut.result := Cat(0.U((DATA_WIDTH - 1).W), !isIdle)
          io.copOut.ena_out := true.B
        }
      }
    }.otherwise{
      switch(io.copIn.funcId) {
        is(FUNC_RESET) {
          when(isIdle) {
            stateReg := restart
            io.copOut.ena_out := true.B
          }
        }
        is(FUNC_SET_HASH) {
          when(isIdle) {
            hashAddrReg := io.copIn.opData(0)
            memState := memReadReqH
            io.copOut.ena_out := true.B
          }
        }
        is(FUNC_GET_HASH) {
          when(isIdle) {
            hashAddrReg := io.copIn.opData(0)
            memState := memWriteReqH
            io.copOut.ena_out := true.B
          }
        }
        is(FUNC_SINGLE_BLOCK) {
          when(isIdle) {
            blockAddrReg := io.copIn.opData(0)
            memState := memReadReqM
            blockCountReg := 1.U
            io.copOut.ena_out := true.B
          }
        }
        is(FUNC_MULTIPLE_BLOCKS) {
          when(isIdle) {
            blockAddrReg := io.copIn.opData(0)
            memState := memReadReqM
            blockCountReg := io.copIn.opData(1)
            io.copOut.ena_out := true.B
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
  switch(memState) {
    is(memReadReqM) {
      io.memPort.M.Cmd := OcpCmd.RD
      io.memPort.M.Addr := blockAddrReg
      when(io.memPort.S.CmdAccept === 1.U) {
        memState := memReadM
      }
    }
    is(memReadM) {
      msg(Cat(memBufferReg, wordCountReg(MSG_WORD_COUNT_WIDTH - 1, 0))) := io.memPort.S.Data
      when(io.memPort.S.Resp === OcpResp.DVA) {
        when(wordCountReg(BURST_OFFSET - 1, 0) < (BURST_LENGTH - 1).U) {
          wordCountReg := wordCountReg + 1.U
        }.otherwise {
          blockAddrReg := blockAddrReg + BURST_ADDR_OFFSET.U
          when(wordCountReg(MSG_WORD_COUNT_WIDTH - 1, BURST_OFFSET) < (BURSTS_PER_MSG - 1).U) {
            wordCountReg := wordCountReg + 1.U
            memState := memReadReqM
          }.otherwise {
            memBufferReg := !memBufferReg
            wordCountReg := 0.U
            blockCountReg := blockCountReg - 1.U
            
            when(stateReg === idle || stateReg === update || stateReg === waiting)
            {
              memState := memReadReqM
              idxReg := 0.U
              stateReg := start
            }.otherwise {
              memState := memWait
            }
            
            when(blockCountReg === 1.U) {
              memState := memIdle
            }
          }
        }
      }
    }
    is(memReadReqH) {
      io.memPort.M.Cmd := OcpCmd.RD
      io.memPort.M.Addr := hashAddrReg
      when(io.memPort.S.CmdAccept === 1.U) {
        memState := memReadH
      }
    }
    is(memReadH) {
      hash(wordCountReg(HASH_WORD_COUNT_WIDTH - 1, 0)) := io.memPort.S.Data
      when(io.memPort.S.Resp === OcpResp.DVA) {
        when(wordCountReg(BURST_OFFSET - 1, 0) < (BURST_LENGTH - 1).U) {
          wordCountReg := wordCountReg + 1.U
        }.otherwise {
          hashAddrReg := hashAddrReg + BURST_ADDR_OFFSET.U
          when(wordCountReg(HASH_WORD_COUNT_WIDTH - 1, BURST_OFFSET) < (BURSTS_PER_HASH - 1).U) {
            wordCountReg := wordCountReg + 1.U
            memState := memReadReqH
          }.otherwise {
            wordCountReg := 0.U
            memState := memIdle
          }
        }
      }
    }
    is(memWriteReqH) {
      io.memPort.M.Cmd := OcpCmd.WR
      io.memPort.M.Addr := hashAddrReg
      io.memPort.M.Data := hash(wordCountReg(HASH_WORD_COUNT_WIDTH - 1, 0));
      io.memPort.M.DataValid := 1.U
      when(io.memPort.S.CmdAccept === 1.U && io.memPort.S.DataAccept === 1.U) {
        wordCountReg := wordCountReg + 1.U
        memState := memWriteH
      }
    }
    is(memWriteH) {
      io.memPort.M.Data := hash(wordCountReg(HASH_WORD_COUNT_WIDTH - 1, 0));
      io.memPort.M.DataValid := 1.U
      when(io.memPort.S.DataAccept === 1.U) {
        when(wordCountReg(BURST_OFFSET - 1, 0) < (BURST_LENGTH - 1).U) {
          wordCountReg := wordCountReg + 1.U
        }
      }
      when(io.memPort.S.Resp === OcpResp.DVA) {
        hashAddrReg := hashAddrReg + BURST_ADDR_OFFSET.U
        when(wordCountReg(HASH_WORD_COUNT_WIDTH - 1, BURST_OFFSET) < (BURSTS_PER_HASH - 1).U) {
          wordCountReg := wordCountReg + 1.U
          memState := memWriteReqH
        }.otherwise {
          wordCountReg := 0.U
          memState := idle
        }
      }
      // TODO: handle restart in case of failure
    }
  }
}
