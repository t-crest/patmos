
import chisel3._
import chisel3.iotesters._
import org.scalatest ._
import patmos._


class PatmosTestMain extends FlatSpec with Matchers {
  "myTest" should "pass" in {
      chisel3.iotesters.Driver.execute(
        Array(
          "--backend-name",
          "verilator",
          "generate-vcd-output",
          "--target-dir",
          "build",
          "--top-name",
          "PatmosVerTest"
      ),
        () => new Patmos("/home/anthon/t-crest/patmos/hardware/../hardware/config/altde2-115.xml", "/home/anthon/t-crest/patmos/hardware/../tmp/bootable-cmptest.bin", "/home/anthon/t-crest/patmos/hardware/../tmp/bootable-cmptest.dat")
      ) {  c =>
        new PatmosTester(c)
      } should be(true)
    }
  }


class PatmosTester(dut: Patmos) extends PeekPokeTester(dut){
  step(10000)
}