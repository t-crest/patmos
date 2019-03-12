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
  
  val corecur = Counter(corecnt-1)
  val memaddrwidth = log2Up(memsize)
  val _memaddr = UInt(width = memaddrwidth)
  _memaddr := 0.U
  val _memwr = Bool()
  _memwr := false.B
  
  val memdata = UInt(width = datawidth)
  memdata := 0.U
  val mem = Mem(memsize, memdata)
  
  val _bufcur = Counter(bufsize-1)
  val _bufnxt = UInt(width = log2Up(bufsize))
  _bufnxt := 0.U
  val _bufaddr = UInt(width = memaddrwidth)
  _bufaddr := 0.U
  val _bufdata = UInt(width = datawidth)
  _bufdata := 0.U
  val _bufwr = Bool()
  _bufwr := false.B
  val _bufconflict = false.B
  
  when(_memwr) {
    when(_bufconflict) {
      corecur.inc
    }.otherwise {
      when(_bufwr) {
        mem(_bufaddr) := _bufdata  
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
    memdata := mem(_memaddr)
    corecur.inc
  }
  
  val sIdle::sPreRead::sRead::sPreCommit::sCommit::Nil = Enum(UInt(),5)
  
  for(i <- 0 until corecnt)  
  {
    val memwr = RegInit(false.B)
    val memaddr = Reg(_memaddr)
    
    val bufaddr = io(i).M.Addr(memaddrwidth+2,2)
    val bufaddrs = Vec(bufsize, Reg(memaddr))
    val bufnxt = Counter(bufsize-1)
    val bufdatas = Vec(bufsize, Reg(memdata))
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
    
    when(_memwr && _bufwr && (_bufcur.value =/= i.U)) {
      for(j <- 0 until bufsize) {
        when(bufrds(j) && (_bufaddr === bufaddrs(j))) {
          bufconflict := true.B
        }
      }
    }
    
    when(corecur.value === i.U) {
      _bufnxt := bufnxt.value
      _bufaddr := bufaddrs(_bufcur.value)
      _bufdata := bufdatas(_bufcur.value)
      _bufwr := bufwrs(_bufcur.value)
      _bufconflict := bufconflict
      _memwr := memwr
      _memaddr := memaddr
    }
   
    val slaveReg = Reg(io(i).S)
    io(i).S := slaveReg
    //slaveReg.Data := memdata
    slaveReg.Resp := OcpResp.NULL
    
    val sReg = RegInit(sIdle)
    
    memwr := false.B
    
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
          when(io(i).M.Addr(0) === true.B) {
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
              memwr := true.B
              sReg := sCommit
            }
          }.otherwise {
            when(bufmatched) {
              slaveReg.Resp := OcpResp.DVA
              slaveReg.Data := bufdatas(bufmatch)
            }.otherwise {
              memaddr := bufaddr
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
        bufaddrs(bufnxt.value) := memaddr
        bufdatas(bufnxt.value) := memdata
        bufrds(bufnxt.value) := true.B
        bufnxt.inc
        
        slaveReg.Resp := OcpResp.DVA
        slaveReg.Data := memdata
        
        sReg := sIdle
      }
      is(sCommit) {
        memwr := true.B
        when(_bufcur.value === bufnxt.value || bufconflict) {
          // Finish here
          
          memwr := false.B
          
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

