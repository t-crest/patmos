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

import patmos._
import patmos.Constants._
import ocp._

class TransactionalMemory(corecnt: Int, memsize: Int = 128, bufsize: Int = 16, pipeline: Boolean = true) extends Module {
  
  override val io = Vec.fill(corecnt){new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH)} 
  
  val datawidth = DATA_WIDTH
  val memaddrwidth = log2Up(memsize)
  val corecur = Counter(corecnt)
  
  val sharedmem = Mem(UInt(width = datawidth), memsize, seqRead = true)
  val sharedmemwr = Bool()
  sharedmemwr := false.B
  val sharedmemrdaddrReg = Reg(UInt(width = memaddrwidth))
  val sharedmemwraddr = UInt(width = memaddrwidth)
  sharedmemwraddr := 0.U
  val sharedmemrddata = UInt(width = datawidth)
  sharedmemrddata := 0.U
  val sharedmemwrdata = UInt(width = datawidth)
  sharedmemwrdata := 0.U
  
  val _bufcur = Counter(bufsize)
  val _bufnxt = UInt(width = log2Up(bufsize))
  _bufnxt := 0.U
  val _bufwr = Bool()
  _bufwr := false.B
  val _bufconflict = false.B
  
  when(sharedmemwr && !_bufconflict && _bufwr) {
    when(_bufwr) {
      sharedmem(sharedmemwraddr) := sharedmemwrdata
    }
    _bufcur.inc
    when(_bufcur.value === _bufnxt) {
      // Finished transferring
      _bufcur.value := 0.U
      corecur.inc
    }
  }.otherwise {
    sharedmemrddata := sharedmem(sharedmemrdaddrReg)
    corecur.inc
  }
  
  val sIdle::sRead::sPreSharedRead::sSharedRead::sPreCommit::sCommit::Nil = Enum(UInt(),6)
  
  for(i <- 0 until corecnt)  
  {
    val ioM = if(pipeline) RegNext(io(i).M) else io(i).M
    val ioS = io(i).S
    
    val bufaddrwidth = log2Up(bufsize)
    
    val memwrReg = RegInit(false.B)
    val memrdaddrReg = Reg(UInt(width = memaddrwidth))
    
    val bufaddr = ioM.Addr(memaddrwidth+2,2)
    val bufaddrs = Vec(bufsize, Reg(UInt(width = memaddrwidth)))
    val bufrds = Vec(bufsize, Reg(init = false.B))
    val bufwrs = Vec(bufsize, Reg(init = false.B))
    val bufnxt = Counter(bufsize)
    
    val bufmem = Mem(UInt(width = datawidth), bufsize, seqRead = true)
    val bufmemwr = Bool()
    val bufmemrdaddrReg = Reg(UInt(width = bufaddrwidth))
    val bufmemwraddr = UInt(width = bufaddrwidth)
    bufmemwraddr := 0.U
    val bufmemrddata = UInt(width = datawidth)
    bufmemrddata := 0.U
    val bufmemwrdata = UInt(width = datawidth)
    bufmemwrdata := 0.U
    
    when(bufmemwr) {
      bufmem(bufmemwraddr) := bufmemwrdata
    }.otherwise {
      bufmemrddata := bufmem(bufmemrdaddrReg)
    }
    
    val bufmatches = UInt(width = bufsize)
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
    
    bufmemrdaddrReg := _bufcur.value
    when(corecur.value === i.U) {
      _bufnxt := bufnxt.value
      sharedmemwraddr := RegNext(bufaddrs(_bufcur.value))
      sharedmemwrdata := bufmemrddata
      _bufwr := RegNext(bufwrs(_bufcur.value))
      _bufconflict := bufconflict
      sharedmemwr := memwrReg
      sharedmemrdaddrReg := memrdaddrReg
    }
   
    val slaveReg = Reg(ioS)
    val sReg = RegInit(sIdle)
    
    ioS.Data := slaveReg.Data
    ioS.Resp := slaveReg.Resp
    
    slaveReg.Data := sharedmemrddata
    slaveReg.Resp := OcpResp.NULL

    memwrReg := false.B
    bufmemwr := false.B
    
    switch(sReg) {
      is(sIdle) {
        when(ioM.Cmd === OcpCmd.WR) {
          slaveReg.Resp := OcpResp.DVA
          when(bufmatched) {
            bufmemwr := true.B
            bufmemwraddr := bufmatch
            bufmemwrdata := ioM.Data
            bufwrs(bufmatch) := true.B
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
            when(bufnxt.value === 0.U || bufconflict) {
              // rd/wr conflict or nothing to commit, return failure
              slaveReg.Resp := OcpResp.DVA
              slaveReg.Data := -1.S
              
              bufnxt.value := 0.U
              bufconflict := false.B
              for(i <- 0 until bufsize) {
                bufrds(i) := false.B
                bufwrs(i) := false.B
              }
            }.otherwise {
              memwrReg := true.B
              sReg := sCommit
            }
          }.otherwise {
            when(bufmatched) {
              bufmemrdaddrReg := bufmatch
              sReg := sRead
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
        bufwrs(bufnxt.value) := true.B
        bufnxt.inc
        
        sReg := sIdle
      }
      is(sCommit) {
        memwrReg := true.B
        when(_bufcur.value === bufnxt.value || bufconflict) {
          memwrReg := false.B
          // Finish here
          
          slaveReg.Resp := OcpResp.DVA
          slaveReg.Data := Mux(bufconflict, -1.S, 0.S)
          
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

