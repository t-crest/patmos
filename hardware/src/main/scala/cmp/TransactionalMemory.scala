/*
 * Transactional Memory
 * 
 * Reads and writes are done within transactions.
 * Commits are marked as failed in the case of a read/write conflict
 *
 * Author: Torur Biskopsto Strom (torur.strom@gmail.com)
 *
 */
package cmp

import Chisel._
import chisel3.VecInit

import patmos._
import patmos.Constants._
import ocp._

class TransactionalMemory(corecnt: Int, memsize: Int = 128, bufsize: Int = 16, pipeline: Boolean = true) extends CmpDevice(corecnt) {
  
  val datawidth = DATA_WIDTH
  val memaddrwidth = log2Up(memsize)
  val corecur = Counter(corecnt)
  
  val sharedmem = Mem(UInt(datawidth.W), memsize)
  val sharedmemwr = Bool()
  sharedmemwr := false.B
  val sharedmemwrfin = Bool()
  sharedmemwrfin := false.B
  val sharedmemrdaddrReg = Reg(UInt(memaddrwidth.W))
  val sharedmemwraddr = UInt(memaddrwidth.W)
  sharedmemwraddr := 0.U
  val sharedmemrddata = sharedmem(sharedmemrdaddrReg)
  val sharedmemwrdata = UInt(datawidth.W)
  sharedmemwrdata := 0.U
  
  val _bufwr = Bool()
  _bufwr := false.B
  
  when(sharedmemwr) {
    when(_bufwr) {
      sharedmem(sharedmemwraddr) := sharedmemwrdata
    }
    when(sharedmemwrfin) {
      // Finished transferring
      corecur.inc
    }
  }.otherwise {
    corecur.inc
  }

  val sIdle::sRead::sPreSharedRead::sSharedRead::sPreCommit::sCommit::Nil = Enum(UInt(),6)
  
  for(i <- 0 until corecnt)  
  {
    val ioM = if(pipeline) RegNext(io.cores(i).M) else io.cores(i).M
    val ioS = io.cores(i).S
    
    val bufaddrwidth = log2Up(bufsize)
    
    val memrdaddrReg = Reg(UInt(memaddrwidth.W))
    
    val bufaddr = ioM.Addr(memaddrwidth+2,2)
    val bufaddrs = Reg(Vec(bufsize, UInt(memaddrwidth.W)))
    val bufrds = RegInit(VecInit(Seq.fill(bufsize)(false.B)))
    val bufwrs = RegInit(VecInit(Seq.fill(bufsize)(false.B)))
    val bufnxt = Counter(bufsize+1)
    
    val bufmem = Mem(UInt(datawidth.W), bufsize)
    val bufmemwr = Bool()
    val bufmemrdaddrReg = RegInit(0.U(bufaddrwidth.W))
    bufmemrdaddrReg := 0.U
    val bufmemwraddr = UInt(bufaddrwidth.W)
    bufmemwraddr := 0.U
    val bufmemrddata = bufmem(bufmemrdaddrReg)
    val bufmemwrdata = UInt(datawidth.W)
    bufmemwrdata := 0.U
    
    when(bufmemwr) {
      bufmem(bufmemwraddr) := bufmemwrdata
    }
    
    val bufmatches = UInt(bufsize.W)
    bufmatches := 0.U
    for(j <- 0 until bufsize) {
      bufmatches(j) := (bufaddrs(j) === bufaddr) && (bufrds(j) || bufwrs(j))
    }
    val bufmatch = OHToUInt(bufmatches);
    val bufmatched = bufmatches.orR
    
    val bufconflict = RegInit(false.B)
    
    when(sharedmemwr && _bufwr && (corecur.value =/= i.U)) {
      for(j <- 0 until bufsize) {
        when(bufrds(j) && (sharedmemwraddr === bufaddrs(j))) {
          bufconflict := true.B
        }
      }
    }
    
    
    val sReg = RegInit(sIdle)
    
    when(corecur.value === i.U) {
      sharedmemwr := (sReg === sCommit) && !bufconflict
      sharedmemwrfin := bufmemrdaddrReg === bufnxt.value
      sharedmemwraddr := bufaddrs(bufmemrdaddrReg)
      sharedmemwrdata := bufmemrddata
      _bufwr := bufwrs(bufmemrdaddrReg)
      sharedmemrdaddrReg := memrdaddrReg
    }
   
    val slaveReg = Reg(ioS)
    
    ioS.Data := slaveReg.Data
    ioS.Resp := slaveReg.Resp
    
    slaveReg.Data := sharedmemrddata
    slaveReg.Resp := OcpResp.NULL

    bufmemwr := false.B
    
    val overflowReg = RegInit(false.B)
    
    switch(sReg) {
      is(sIdle) {
        when(ioM.Cmd === OcpCmd.WR) {
          slaveReg.Resp := OcpResp.DVA
          when(bufmatched) {
            bufmemwr := true.B
            bufmemwraddr := bufmatch
            bufmemwrdata := ioM.Data
            bufwrs(bufmatch) := true.B
          }.elsewhen(bufnxt.value === bufsize.U) {
            overflowReg := true.B
          }.otherwise {
            bufaddrs(bufnxt.value) := bufaddr
            bufmemwr := true.B
            bufmemwraddr := bufnxt.value
            bufmemwrdata := ioM.Data
            bufwrs(bufnxt.value) := true.B
            bufnxt.inc
          }
        }.elsewhen(ioM.Cmd === OcpCmd.RD) {
          when(ioM.Addr(15,2) === 0x3FFF.U) {
            when(bufnxt.value === 0.U || bufconflict || overflowReg) {
              // rd/wr conflict or nothing to commit, return failure
              slaveReg.Resp := OcpResp.DVA
              slaveReg.Data := -1.S
              
              overflowReg := false.B
              bufnxt.value := 0.U
              bufconflict := false.B
              for(i <- 0 until bufsize) {
                bufrds(i) := false.B
                bufwrs(i) := false.B
              }
            }.otherwise {
              sReg := sCommit
            }
          }.otherwise {
            when(bufmatched) {
              bufmemrdaddrReg := bufmatch
              sReg := sRead
            }.elsewhen(bufnxt.value === bufsize.U) {
              overflowReg := true.B
            }.otherwise {
              memrdaddrReg := bufaddr
              sReg := sPreSharedRead
            }
          }
        }
      }
      is(sRead) {
        slaveReg.Resp := OcpResp.DVA
        slaveReg.Data := bufmemrddata
        sReg := sIdle
      }
      is(sPreSharedRead) {
        // Wait until my turn. Data is not ready until next cycle
        when(corecur.value === i.U) {
          sReg := sSharedRead
        }
      }
      is(sSharedRead) {
        slaveReg.Resp := OcpResp.DVA
        
        bufaddrs(bufnxt.value) := memrdaddrReg
        bufmemwr := true.B
        bufmemwraddr := bufnxt.value
        bufmemwrdata := sharedmemrddata
        bufrds(bufnxt.value) := true.B
        bufnxt.inc
        
        sReg := sIdle
      }
      is(sCommit) {
        when((corecur.value === i.U) || bufconflict) {
          bufmemrdaddrReg := bufmemrdaddrReg+1.U
          when((bufmemrdaddrReg === bufnxt.value) || bufconflict) {
            // Finish here
            slaveReg.Resp := OcpResp.DVA
            slaveReg.Data := Mux(bufconflict, -1.S, 0.S)
            
            overflowReg := false.B
            bufnxt.value := 0.U
            bufconflict := false.B
            for(i <- 0 until bufsize) {
              bufrds(i) := false.B
              bufwrs(i) := false.B
            }
            
            sReg := sIdle
          }
        }
      }
    }
  }
}

