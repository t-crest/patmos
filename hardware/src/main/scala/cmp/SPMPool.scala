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

    val width = log2Up(reqs.getWidth())
    val curReg = Reg(UInt(width = width))
    when(!reqs(curReg) || continue) {
      curReg := PriorityEncoder(Cat(reqs, reqs)(curReg+width.U,curReg))
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
    io.dataout := mem(Reg(next =io.addr))
  }

  class TDMSPM(corecnt:Int, spmsize:Int, spmwidth:Int, slotwidth:Int = 1) extends Module {

    val spm = Module(new SPM(spmsize,spmwidth))
    override val io = new IOTDMSPM(corecnt,spm.io.addr.getWidth,spm.io.datain.getWidth)

    val widthcnt = Reg(init = UInt(0, log2Up(slotwidth)))
    when(widthcnt === UInt(slotwidth-1)) {
      widthcnt := UInt(0)
    }.otherwise {
      widthcnt := widthcnt + UInt(1)
    }

    val cur = SPMPool.roundRobinArbiter(io.sched, widthcnt === UInt(slotwidth-1))

    spm.io.addr := io.cores(cur).addr
    spm.io.datain := io.cores(cur).datain
    spm.io.wr := io.cores(cur).wr
    io.dataout := spm.io.dataout
    for(i <- 0 until corecnt)
    {
      io.cores(i).rdy := Reg(next = cur === UInt(i))
    }
  }
}

class SPMPool(corecnt:Int, spmcnt:Int, spmsize:Int, spmwidth:Int, tdmslotwidth:Int = 1) extends Module {

  val spms = (0 until spmcnt).map(e => Module(new SPMPool.TDMSPM(corecnt, spmsize, spmwidth, tdmslotwidth)))

  val spmios = Vec(spms.map(e => e.io))
  val spmscheds = Vec(spms.map(e => {
    val sched = Reg(UInt(width = corecnt))
    e.io.sched := sched
    sched
  }))
  val spmaddrwidth = log2Up(spmcnt+1)
  val addrwidth = spms(0).spm.io.addr.getWidth+spmaddrwidth

  override val io = new SPMPool.IOSPMPool(corecnt,addrwidth,spms(0).spm.io.datain.getWidth)


  for(i <- 0 until corecnt)
  {
    io(i).dataout := SInt(-1)
    io(i).rdy := UInt(0)

    for(j <- 0 until spmcnt)
    {
      spms(j).io.cores(i).addr := io(i).addr
      spms(j).io.cores(i).datain := io(i).datain

      val sel = io(i).addr(addrwidth-1, addrwidth-spmaddrwidth) === UInt(j+1)
      val selReg = Reg(next = sel)

      spms(j).io.cores(i).wr := Mux(sel, io(i).wr, Bool(false))

      // We delay the address used for the output
      when(selReg) {
        io(i).dataout := spms(j).io.dataout
        io(i).rdy := spms(j).io.cores(i).rdy
      }
    }
  }

  val avails = Reverse(Cat(spmscheds.map(e => !orR(e))))
  val nxtavail = PriorityEncoder(avails)
  val anyavail = orR(avails)

  val cur = SPMPool.counter(corecnt)
  val curio = io(cur)


  val cursel = curio.addr(addrwidth-1, addrwidth-spmaddrwidth) === UInt(0)
  val curselReg = Reg(next = cursel)
  val curaddrlo = curio.addr(addrwidth-spmaddrwidth-1, 0)

  val dataoutReg = Reg(curio.dataout)
  val rdyReg = Reg(curio.rdy)

  when(cursel) {
    rdyReg := Bool(true)
    when(curio.rd && curaddrlo === UInt(0)) {
      when(anyavail) {
        dataoutReg := nxtavail
        spmscheds(nxtavail) := UIntToOH(cur)
      }
    }.otherwise {
      when(curio.wr) {
        spmscheds(curaddrlo >> 1) := curio.datain
      }.otherwise {
        dataoutReg := spmscheds(curaddrlo >> 1)
      }
    }
  }

  val lst = Reg(next = cur)
  val lstio = io(lst)

  when(curselReg) {
    lstio.rdy := rdyReg
    lstio.dataout := dataoutReg
  }
}

class SPMPoolOCPWrapper(corecnt:Int, spmcnt:Int, spmsize:Int, spmwidth:Int, tdmslotwidth:Int = 1) extends Module {

  val pool = Module(new SPMPool(corecnt,spmcnt,spmsize,spmwidth,tdmslotwidth))

  override val io = Vec(corecnt, new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH))

  for (i <- 0 until corecnt) {

    pool.io(i).rd := io(i).M.Cmd === OcpCmd.RD
    pool.io(i).wr := io(i).M.Cmd === OcpCmd.WR
    pool.io(i).addr := io(i).M.Addr
    pool.io(i).datain := io(i).M.Data
    io(i).S.Data := pool.io(i).dataout

    val req = Reg(init = Bool(false))

    when(io(i).M.Cmd === OcpCmd.RD || io(i).M.Cmd === OcpCmd.WR) {
      req := Bool(true)
    }.elsewhen(req && pool.io(i).rdy) {
      req := Bool(false)
    }

    io(i).S.Resp := OcpResp.NULL
    when(req && pool.io(i).rdy) {
      io(i).S.Resp := OcpResp.DVA
    }

  }
}
