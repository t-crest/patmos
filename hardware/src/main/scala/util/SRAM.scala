package util

import chisel3._
import chisel3.util.{HasBlackBoxPath, UIntToOH}
import SRAM.{SeqToVec, extractBytes}


class SRAM(size: Int, width: Int) extends Module {

  val addrWidth = log2Up(size)

  val io = IO(new Bundle {
    val rdAddr = Input(UInt(addrWidth.W))
    val rdData = Output(UInt(width.W))
    val wrAddr = Input(UInt(addrWidth.W))
    val wrEna  = Input(Bool())
    val wrData = Input(UInt(width.W))
  })

  // number of banks needed to achieve the requested depth
  val numberOfBanks = scala.math.ceil(size / 1024.0).toInt

  io.rdData := extractBytes(io.wrData) // split write data into bytes
    .map { wrByte => // for each byte column, write and return read wire

    if(addrWidth <= 10) { // only one bank is needed to achieve depth

      val bank = SRAMBlackBox(this.clock)
      bank.write(io.wrEna)(io.wrAddr, wrByte)
      bank.read(io.rdAddr) // return read wire

    } else { // multiple banks are needed to achieve depth

      val banks = Seq.fill(numberOfBanks)(SRAMBlackBox(this.clock))

      val bankSelectors = // create one hot bank selection signal
        UIntToOH(io.rdAddr(addrWidth - 1, 10))
          .asBools
          .take(numberOfBanks)

      // reduction function to select the correct read data from the banks
      def select(left: (Bool, UInt), right: (Bool, UInt)): (Bool, UInt) =
        Mux(left._1, left._1, right._1) -> Mux(left._1, left._2, right._2)

      // read access
      val (_, rdByte) = banks
        .zip(bankSelectors)
        .map { case (bank, sel) => sel -> bank.read(io.rdAddr(10, 0)) } // create selector and read value pairs
        .reduce(select) // select the ONE read value with an asserted selector

      banks
        .zip(bankSelectors)
        .foreach { case (bank, sel) => bank.write(io.wrEna && sel)(io.wrAddr(10, 0), wrByte) } // write to bank when wrEna and the bank selector are asserted

      rdByte // return read wire
    }

  }.toVec.reduceTree(_ ## _) // concatenate all byte read values

}

object SRAM {

  // allow for method call conversion to a chisel vec
  implicit class SeqToVec[T <: Data](seq: Seq[T]) { def toVec: Vec[T] = VecInit(seq) }

  // create an array of bytes from an arbitrary UInt
  def extractBytes(num: UInt): Seq[UInt] = {
    val numBytes = scala.math.ceil(num.getWidth / 8.0).toInt
    num(num.getWidth-1, (numBytes-1) * 8).asTypeOf(UInt(8.W)) +: Seq.tabulate(numBytes-1)(_ * 8).map(i => num(i+7,i))
  }
}

object SRAMBlackBox {
  def apply(clock: Clock): sky130_sram_1kbyte_1rw1r_8x1024_8 = {
    val sram = Module(new sky130_sram_1kbyte_1rw1r_8x1024_8)
    sram.io.clk0 := clock
    sram.io.clk1 := clock
    sram.io.csb0 := 0.B // always selected
    sram.io.csb1 := 0.B // always selected
    sram.io.wmask0 := 1.U // the 1 byte write value is always enabled
    sram
  }
}

class sky130_sram_1kbyte_1rw1r_8x1024_8 extends BlackBox with HasBlackBoxPath {

  val io = IO(new Bundle {
    val clk0 = Input(Clock()) // rw port clock
    val csb0 = Input(Bool()) // rw port active low chip select
    val web0 = Input(Bool()) // wr port active low write control
    val wmask0 = Input(UInt(1.W)) // write mask
    val addr0 = Input(UInt(10.W)) // rw port address
    val din0 = Input(UInt(8.W)) // rw port write data
    val dout0 = Output(UInt(8.W)) // rw port read data
    val clk1 = Input(Clock()) // r port clock
    val csb1 = Input(Bool()) // r port active low chip select
    val addr1 = Input(UInt(10.W)) // r port address
    val dout1 = Output(UInt(8.W)) // r port read data
  })
  addPath("verilog/sky130_sram_1kbyte_1rw1r_8x1024_8.v")

  // connect signals for a read
  def read(index: UInt): UInt = {
    io.addr1 := index
    io.dout0
  }

  // connect signals for a write
  def write(en: Bool)(index: UInt, data: UInt): Unit = {
    io.web0 := !en
    io.addr0 := index
    io.din0 := data
  }

}