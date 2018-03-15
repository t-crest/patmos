package Spown

import Chisel._

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

class TDMSPM(slots:Int, spmfact: => SPM, width:Int = 1) extends Module {
  val spm = Module(spmfact)
  override val io = new IOTDMSPM(slots,spm.io.addr.getWidth,spm.io.datain.getWidth)

  val widthcnt = Reg(init = UInt(0, log2Up(width)))
  when(widthcnt === UInt(width-1)) {
    widthcnt := UInt(0)
  }.otherwise {
    widthcnt := widthcnt + UInt(1)
  }

  val cur = Util.roundRobinArbiterO(io.sched, widthcnt === UInt(width-1))

  spm.io.addr := io.cores(cur).addr
  spm.io.datain := io.cores(cur).datain
  spm.io.wr := io.cores(cur).wr
  io.dataout := spm.io.dataout
  for(i <- 0 until slots)
  {
    io.cores(i).rdy := cur === UInt(i)
  }
}

class SPMPool(spmcnt:Int, spmfact: => TDMSPM, width:Int = 1) extends Module {
  val spms = (0 until spmcnt).map(e => Module(spmfact))

  val spmios = Vec(spms.map(e => e.io))
  val corecnt = spms(0).io.cores.length
  val spmscheds = Vec(spms.map(e => {
    val sched = Reg(UInt(width = corecnt))
    e.io.sched := sched
    sched
  }))
  val spmaddrwidth = log2Up(spmcnt+1)
  val addrwidth = spms(0).spm.io.addr.getWidth+spmaddrwidth

  override val io = new IOSPMPool(corecnt,addrwidth,spms(0).spm.io.datain.getWidth)

  val avails = Cat(spmscheds.map(e => !orR(e)))
  val nxtavail = PriorityEncoder(avails)
  val anyavail = orR(avails)

  val cur = Util.counter(corecnt.U)
  val curio = io(cur)

  when(curio.addr(addrwidth-1, addrwidth-1-spmaddrwidth) === UInt(0)) {
    curio.rdy := UInt(1)
    when(curio.addr === UInt(0)) {
      when(anyavail && curio.rd) {
        curio.dataout := nxtavail
        spmscheds(curio.addr) := UIntToOH(cur)
      }
    }.otherwise {
      when(curio.wr) {
        spmscheds(curio.addr) := curio.datain
      }.otherwise {
        curio.dataout := spmscheds(curio.addr)
      }
    }
  }

  for(i <- 0 until corecnt)
  {
    io(i).dataout := SInt(-1)
    io(i).rdy := UInt(0)

    for(j <- 0 until spmcnt)
    {
      spms(j).io.cores(i).addr := io(i).addr
      spms(j).io.cores(i).datain := io(i).datain

      when(io(i).addr(addrwidth-1, addrwidth-1-spmaddrwidth) === UInt(j+1)) {
        spms(j).io.cores(i).wr := io(i).wr
        io(i).dataout := spms(j).io.dataout
        io(i).rdy := spms(j).io.cores(i).rdy
      }.otherwise {
        spms(j).io.cores(i).wr := UInt(0)
      }
    }
  }
}

object Util {
  def counter(max: UInt) = {
    val x = RegInit(0.U(max.getWidth))
    x := Mux(x === max, 0.U, x + 1.U)
    x
  }

  def roundRobinArbiter(reqs: Bits, continue: Bool = Bool(false)) = {

    val curReg = Reg(UInt(width = log2Up(reqs.getWidth())))

    val hi = Bits(width = reqs.getWidth())
    val lo = Bits(width = reqs.getWidth())

    lo := UInt(0)
    hi := UInt(0)
    for (i <- 0 until reqs.getWidth()) {
      lo(i) := reqs(i) && (curReg > UInt(i))
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
}

object Spown {
  def main(args: Array[String]): Unit = {
    chiselMain(args, () => Module(new SPMPool(8,new TDMSPM(8,new SPM(32,32)))))
  }
}
