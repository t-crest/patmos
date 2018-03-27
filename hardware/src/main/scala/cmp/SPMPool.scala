package cmp

import Chisel._

import patmos.Constants._
import ocp._





object SPMPool {
  def counter(max: Int) = {

    val x = RegInit(UInt(0, width = log2Up(max)))
    x := Mux(x === max.U, 0.U, x + 1.U)
    x
  }

  def roundRobinArbiter(reqs: Bits, continue: Bool = Bool(false)) = {

    val curReg = Reg(UInt(width = log2Up(reqs.getWidth())))

    val hi = Bits(width = reqs.getWidth())
    val lo = Bits(width = reqs.getWidth())

    lo := UInt(0)
    hi := UInt(0)
    for (i <- 0 until reqs.getWidth()) {
      lo(i) := reqs(i) && (curReg >= UInt(i))
      hi(i) := reqs(i) && (curReg < UInt(i))
    }

    when(!reqs(curReg) || continue) {
      when(orR(hi)) {
        curReg := PriorityEncoder(hi)
      }.otherwise {
        curReg := PriorityEncoder(lo)
      }
    }

    curReg
  }

  def roundRobinArbiterO(reqs: Bits, continue: Bool = Bool(false)) = {

    val reqscat = Cat(reqs, reqs)
    val reqwindows = Vec((0 until reqs.getWidth()).map(i => reqscat(i+reqs.getWidth(),1+i)))

    val curReg = Reg(UInt(width = log2Up(reqs.getWidth())))
    val nxt = PriorityEncoder(reqwindows(curReg))-UInt(3)+curReg
    when(!reqs(curReg) || continue) {
      curReg := nxt
    }

    curReg
  }

  trait IReady {
    val rdy = Bool(OUTPUT)
  }

  trait IDataOut {
    val dataout = UInt(OUTPUT)
  }

  class IOSPMBase(addrwidth:Int, datawidth:Int) extends Bundle {
    val addr = UInt(INPUT, addrwidth)
    val datain = UInt(INPUT, datawidth)
    val wr = Bool(INPUT)

    override def clone = new IOSPMBase(addrwidth,datawidth).asInstanceOf[this.type]
  }

  class IOSPM(addrwidth:Int, datawidth:Int) extends IOSPMBase(addrwidth,datawidth) with IDataOut {
    override val dataout = UInt(OUTPUT, datawidth)

    override def clone = new IOSPM(addrwidth,datawidth).asInstanceOf[this.type]
  }

  class IOTDMSPMElement(addrwidth:Int, datawidth:Int) extends IOSPMBase(addrwidth,datawidth) with IReady {
    val rd = Bool(INPUT)
    override def clone = new IOTDMSPMElement(addrwidth,datawidth).asInstanceOf[this.type]
  }

  class IOTDMSPM(slots:Int, addrwidth:Int, datawidth:Int) extends Bundle with IDataOut {
    val cores = Vec(slots,  new IOTDMSPMElement(addrwidth,datawidth))
    val sched = UInt(INPUT, slots)
    override val dataout = UInt(OUTPUT, datawidth)

    override def clone = new IOTDMSPM(slots,addrwidth,datawidth).asInstanceOf[this.type]
  }

  class IOPoolElement(addrwidth:Int, datawidth:Int) extends IOSPM(addrwidth,datawidth) with IReady {

    val rd = Bool(INPUT)

    override def clone = new IOPoolElement(addrwidth,datawidth).asInstanceOf[this.type]
  }

  class IOSPMPool(slots:Int, addrwidth:Int, datawidth:Int) extends Vec(null,(0 until slots)
    .map(e => new IOPoolElement(addrwidth,datawidth))) {
    override def clone = new IOSPMPool(slots,addrwidth,datawidth).asInstanceOf[this.type]
  }

  class SPM(size:Int, width:Int) extends Module {
    override val io = new IOSPM(log2Up(size), width)
    val mem = Mem(UInt(width = width), size, seqRead = true)

    when(io.wr) {
      mem(io.addr) := io.datain
    }
    io.dataout := mem(Reg(next = io.addr))
  }

  class TDMSPM(corecnt:Int, spmsize:Int, spmwidth:Int) extends Module {

    val spm = Module(new SPM(spmsize,spmwidth))
    override val io = new IOTDMSPM(corecnt,spm.io.addr.getWidth,spm.io.datain.getWidth)

    val cur = SPMPool.roundRobinArbiterO(io.sched, Bool(true))

    spm.io.addr := io.cores(cur).addr
    spm.io.datain := io.cores(cur).datain
    spm.io.wr := io.cores(cur).wr
    io.dataout := spm.io.dataout
    for(i <- 0 until corecnt)
    {
      io.cores(i).rdy := Reg(next = (cur === i.U) && (io.cores(i).rd || io.cores(i).wr))
    }
  }
}

class SPMPool(corecnt:Int, spmcnt:Int, spmsize:Int, spmwidth:Int) extends Module {

  val spms = (0 until spmcnt).map(e => Module(new SPMPool.TDMSPM(corecnt, spmsize, spmwidth)))

  val spmios = Vec(spms.map(e => e.io))
  val spmscheds = Vec(spms.map(e => {
    val sched = Reg(UInt(width = corecnt))
    e.io.sched := sched
    sched
  }))
  val spmaddrwidth = log2Up(spmcnt+1)
  val addrwidth = spms(0).spm.io.addr.getWidth+spmaddrwidth

  override val io = new SPMPool.IOSPMPool(corecnt,addrwidth,spms(0).spm.io.datain.getWidth)

  val avails = Reverse(Cat(spmscheds.map(e => !orR(e))))
  val nxtavail = PriorityEncoder(avails)
  val anyavail = orR(avails)

  val cur = SPMPool.counter(corecnt)

  for(i <- 0 until corecnt)
  {
    val curaddrhi = io(i).addr(addrwidth-1, addrwidth-spmaddrwidth)
    val curaddrlo = io(i).addr(addrwidth-spmaddrwidth-1, 2)

    io(i).rdy := Bool(false)
    io(i).dataout := -1.S

    when(curaddrhi === ((1 << curaddrhi.getWidth())-1).U) {
      when(cur === i.U) {
        io(i).rdy := io(i).rd || io(i).wr
        when(io(i).rd && curaddrlo === ((1 << curaddrlo.getWidth())-1).U) {
          when(anyavail) {
            io(i).dataout := nxtavail
            spmscheds(nxtavail) := UIntToOH(cur)
          }
        }.otherwise {
          when(io(i).wr) {
            spmscheds(curaddrlo) := io(i).datain
          }.otherwise {
            io(i).dataout := spmscheds(curaddrlo)
          }
        }
      }
    }.otherwise {
      io(i).dataout := spmios(curaddrhi).dataout
      io(i).rdy := spmios(curaddrhi).cores(i).rdy
    }

    for(j <- 0 until spmcnt)
    {
      spms(j).io.cores(i).addr := io(i).addr(addrwidth-spmaddrwidth-1, 0)
      spms(j).io.cores(i).datain := io(i).datain
      spms(j).io.cores(i).wr := Mux(curaddrhi === j.U && !spms(j).io.cores(i).rdy, io(i).wr, Bool(false))
      spms(j).io.cores(i).rd := Mux(curaddrhi === j.U && !spms(j).io.cores(i).rdy, io(i).rd, Bool(false))
    }
  }
}

class SPMPoolOCPWrapper(corecnt:Int, spmcnt:Int, spmsize:Int, spmwidth:Int) extends Module {

  val pool = Module(new SPMPool(corecnt,spmcnt,spmsize,spmwidth))

  override val io = Vec(corecnt, new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH))

  for (i <- 0 until corecnt) {

    val cmdReg = Reg(io(i).M.Cmd)
    val addrReg = Reg(io(i).M.Addr)
    val dataReg = Reg(io(i).M.Data)

    pool.io(i).rd := cmdReg === OcpCmd.RD
    pool.io(i).wr := cmdReg === OcpCmd.WR
    pool.io(i).addr := addrReg
    pool.io(i).datain := dataReg
    io(i).S.Data := pool.io(i).dataout


    io(i).S.Resp := OcpResp.NULL

    when(cmdReg === OcpCmd.IDLE) {
      cmdReg := io(i).M.Cmd
      addrReg := io(i).M.Addr
      dataReg := io(i).M.Data
    }.elsewhen(pool.io(i).rdy) {
      io(i).S.Resp := OcpResp.DVA
      cmdReg := io(i).M.Cmd
      addrReg := io(i).M.Addr
      dataReg := io(i).M.Data
    }
  }
}
