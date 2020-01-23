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

  def roundRobinArbiter(reqs: UInt, continue: Bool = Bool(false)) = {

    val curReg = Reg(UInt(width = log2Up(reqs.getWidth)))

    val hi = UInt(width = reqs.getWidth)
    val lo = UInt(width = reqs.getWidth)

    lo := UInt(0)
    hi := UInt(0)
    for (i <- 0 until reqs.getWidth) {
      lo(i) := reqs(i) && (curReg >= UInt(i))
      hi(i) := reqs(i) && (curReg < UInt(i))
    }

    when(!reqs(curReg) || continue) {
      when(hi.orR) {
        curReg := PriorityEncoder(hi)
      }.otherwise {
        curReg := PriorityEncoder(lo)
      }
    }

    curReg
  }

  class TDMSPM(corecnt:Int, spmsize:Int) extends Module {

    override val io = new Bundle()
    {
      val sched = UInt(INPUT, corecnt)
      val cores = Vec(corecnt, new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH))
    }

    val spm = Module(new patmos.Spm(spmsize))
    val cur = SPMPool.roundRobinArbiter(io.sched, Bool(true))
    val lst = RegNext(cur)

    spm.io.M <> io.cores(cur).M

    for(i <- 0 until corecnt)
    {
      io.cores(i).S <> spm.io.S
      io.cores(i).S.Resp := Mux(lst === i.U, spm.io.S.Resp, OcpResp.NULL)
    }
  }
}

class SPMPool(corecnt:Int, spmcnt:Int, spmsize:Int, spmcntmax:Int = 15, spmsizemax:Int = 4096) extends Module {

  if(spmcnt > spmcntmax)
    throw new IllegalArgumentException("SPM count is greater than SPM maximum count")

  if(spmsize > spmsizemax)
    throw new IllegalArgumentException("SPM size is greater than SPM maximum size")

  val io = IO(new CmpIO(corecnt))  //Vec(corecnt, new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH))

  val spms = (0 until spmcnt).map(e => Module(new SPMPool.TDMSPM(corecnt, spmsize)))

  // remove empty fields

  val spmios = Wire(Vec(spms.map(e => e.io.cores)))

  val spmscheds = Wire(Vec(spms.map(e => Reg(UInt(width = corecnt)))))

  for(i <- 0 until spms.length)
    spms(i).io.sched := spmscheds(i)


  val spmaddrwidth = log2Up(spmcntmax+1)
  val spmdataaddrwidth = log2Up(spmsizemax)

  val avails = Reverse(Cat(spmscheds.map(e => !e.orR)))
  val nxtavail = PriorityEncoder(avails)
  val anyavail = avails.orR

  val respRegs = Wire(Vec(corecnt, RegInit(OcpResp.NULL)))
  val dataRegs = Wire(Vec(corecnt, Reg(io.cores(0).S.Data)))

  val dumio = Wire(Vec(corecnt, new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH)))

  for(i <- 0 until corecnt)
  {
    val mReg = Reg(io.cores(i).M)



    val m = Mux(io.cores(i).M.Cmd === OcpCmd.IDLE, mReg, io.cores(i).M)

    val spmsel = m.Addr(spmaddrwidth+spmdataaddrwidth-1, spmdataaddrwidth)
    val spmselReg = mReg.Addr(spmaddrwidth+spmdataaddrwidth-1, spmdataaddrwidth)

    for(j <- 0 until spms.length) {
      spmios(j)(i).M := m
      spmios(j)(i).M.Cmd := Mux(spmsel === j.U, m.Cmd, OcpCmd.IDLE)
    }

    dumio(i).M := m
    dumio(i).M.Cmd := Mux(spmsel === spmcntmax.U, m.Cmd, OcpCmd.IDLE)

    val s = Mux(spmselReg === spmcntmax.U, dumio(i).S, spmios(spmselReg)(i.U).S)
    io.cores(i).S := s

    when(io.cores(i).M.Cmd =/= OcpCmd.IDLE || s.Resp =/= OcpResp.NULL) {
      mReg := io.cores(i).M
    }

    respRegs(i) := OcpResp.NULL
    dumio(i).S.Resp := respRegs(i)
    dumio(i).S.Data := dataRegs(i)
  }

  val cur = SPMPool.counter(corecnt)

  val curio = dumio(cur)
  val spmsel = curio.M.Addr(spmdataaddrwidth-1, 2)

  when(curio.M.Cmd =/= OcpCmd.IDLE) {
    respRegs(cur) := OcpResp.DVA

    when(spmsel === spmcntmax.U) {
      when(anyavail) {
        dataRegs(cur) := nxtavail
        spmscheds(nxtavail) := UIntToOH(cur)
      }
    }.otherwise {
      when(curio.M.Cmd === OcpCmd.WR) {
        spmscheds(spmsel) := curio.M.Data
      }.otherwise {
        respRegs(cur) := spmscheds(spmsel)
      }
    }
  }
}
