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

class TransactionalMemory(corecnt: Int, memsize: Int = 128, bufsize: Int = 16) extends Module {
  
  override val io = Vec.fill(corecnt){new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH)} 
  
  val datawidth = DATA_WIDTH
  
  val corecur = Counter(corecnt)
  val memaddrwidth = log2Up(memsize)
  
  val mem = Mem(UInt(width = datawidth), memsize, seqRead = true)
  val _memrdaddrReg = Reg(UInt(width = memaddrwidth))
  val _memrddata = mem(_memrdaddrReg)
  val _memwraddr = UInt(width = memaddrwidth)
  _memwraddr := 0.U
  val _memwrdata = UInt(width = datawidth)
  _memwrdata := 0.U
  val _memwr = Bool()
  _memwr := false.B
  
  val _bufcur = Counter(bufsize)
  val _bufnxt = UInt(width = log2Up(bufsize))
  _bufnxt := 0.U
  val _bufwr = Bool()
  _bufwr := false.B
  val _bufconflict = false.B
  
  when(_memwr) {
    when(_bufconflict) {
      corecur.inc
    }.otherwise {
      when(_bufwr) {
        mem(_memwraddr) := _memwrdata  
      }
      when(_bufcur.value === _bufnxt) {
        // Finished transferring
        _bufcur.value := 0.U
        corecur.inc
      }.otherwise {
        _bufcur.inc
      }
    }
  }.otherwise {
    corecur.inc
  }
  
  val sIdle::sPreRead::sRead::sPreCommit::sCommit::Nil = Enum(UInt(),5)
  
  for(i <- 0 until corecnt)  
  {
    val memwrReg = RegInit(false.B)
    val memaddrReg = Reg(UInt(width = memaddrwidth))
    
    val bufaddr = io(i).M.Addr(memaddrwidth+2,2)
    val bufaddrs = Vec(bufsize, Reg(UInt(width = memaddrwidth)))
    val bufnxt = Counter(bufsize)
    val bufdatas = Vec(bufsize, Reg(UInt(width = datawidth)))
    val bufrds = Vec(bufsize, Reg(init = false.B))
    val bufwrs = Vec(bufsize, Reg(init = false.B))
    val bufmatches = UInt(width = bufsize)
    bufmatches := 0.U
    for(j <- 0 until bufsize) {
      bufmatches(j) := (bufaddrs(j) === bufaddr) && (bufrds(j) || bufwrs(j))
    }
    val bufmatch = OHToUInt(bufmatches);
    val bufmatched = bufmatches.orR
    
    val bufconflict = RegInit(false.B)
    
    when(_memwr && _bufwr && (corecur.value =/= i.U)) {
      for(j <- 0 until bufsize) {
        when(bufrds(j) && (_memwraddr === bufaddrs(j))) {
          bufconflict := true.B
        }
      }
    }
    
    when(corecur.value === i.U) {
      _bufnxt := bufnxt.value
      _memwraddr := bufaddrs(_bufcur.value)
      _memwrdata := bufdatas(_bufcur.value)
      _bufwr := bufwrs(_bufcur.value)
      _bufconflict := bufconflict
      _memwr := memwrReg
      _memrdaddrReg := memaddrReg
    }
   
    val slaveReg = Reg(io(i).S)
    io(i).S := slaveReg
    slaveReg.Data := _memrddata
    slaveReg.Resp := OcpResp.NULL
    
    val sReg = RegInit(sIdle)
    
    memwrReg := false.B
    
    switch(sReg) {
      is(sIdle) {
        when(io(i).M.Cmd === OcpCmd.WR) {
          slaveReg.Resp := OcpResp.DVA
          when(bufmatched) {
            bufdatas(bufmatch) := io(i).M.Data
            bufwrs(bufmatch) := true.B
          }.otherwise {
            bufaddrs(bufnxt.value) := bufaddr
            bufdatas(bufnxt.value) := io(i).M.Data
            bufwrs(bufnxt.value) := true.B
            bufnxt.inc
          }
        }.elsewhen(io(i).M.Cmd === OcpCmd.RD) {
          when(io(i).M.Addr(15,2) === 0x3FFF.U) {
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
              slaveReg.Resp := OcpResp.DVA
              slaveReg.Data := bufdatas(bufmatch)
            }.otherwise {
              memaddrReg := bufaddr
              sReg := sPreRead
            }
          }
        }
      }
      is(sPreRead) {
        // Wait until my turn. Data is not ready until next cycle
        when(corecur.value === i.U) {
          sReg := sRead
        }
      }
      is(sRead) {
        bufaddrs(bufnxt.value) := memaddrReg
        bufdatas(bufnxt.value) := _memrddata
        bufrds(bufnxt.value) := true.B
        bufnxt.inc
        
        slaveReg.Resp := OcpResp.DVA
        
        sReg := sIdle
      }
      is(sCommit) {
        memwrReg := true.B
        when(_bufcur.value === bufnxt.value || bufconflict) {
          // Finish here
          
          memwrReg := false.B
          
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

