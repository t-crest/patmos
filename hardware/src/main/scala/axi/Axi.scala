package axi

import chisel3._
import chisel3.util._

object AXIResp {
  val OKAY = "b000".U(3.W)
  val EXOKAY = "b001".U(3.W)
  val SLVERR = "b010".U(3.W)
  val DECERR = "b011".U(3.W)
}

class AxiLiteWriteAddressChannel(addrWidth: Int) extends Bundle {
  val addr = UInt(addrWidth.W) // M->S
  val prot = UInt(2.W) // M->S Ignored in our case)


}

class AxiLiteWriteDataChannel(dataWidth: Int) extends Bundle {
  val data = UInt(dataWidth.W) // M->S
  val strb = UInt((dataWidth / 8).W) // M->S

}

class AxiLiteWriteRespChannel() extends Bundle {
  val resp = UInt(2.W) // S->M
}

class AxiLiteReadAddressChannel(addrWidth: Int) extends Bundle {
  val addr = UInt(addrWidth.W) // M->S
  val prot = UInt(2.W) // M->S Ignored in our case)

}

class AxiLiteReadRespChannel(dataWidth: Int) extends Bundle {
  val data = UInt(dataWidth.W) // S->M
  val resp = UInt(2.W) // S->M

}

class AxiLiteMasterPort(addrWidth: Int, dataWidth: Int) extends Bundle {
  val aw = Decoupled(new AxiLiteWriteAddressChannel(addrWidth))
  val w = Decoupled(new AxiLiteWriteDataChannel(dataWidth))
  val b = Flipped(Decoupled(new AxiLiteWriteRespChannel()))
  val ar = Decoupled(new AxiLiteReadAddressChannel(addrWidth))
  val r = Flipped(Decoupled(new AxiLiteReadRespChannel(dataWidth)))

}

class AxiLiteSlavePort(addrWidth: Int, dataWidth: Int) extends Bundle {
  val aw = Flipped(Decoupled(new AxiLiteWriteAddressChannel(addrWidth)))
  val w = Flipped(Decoupled(new AxiLiteWriteDataChannel(dataWidth)))
  val b = Decoupled(new AxiLiteWriteRespChannel())
  val ar = Flipped(Decoupled(new AxiLiteReadAddressChannel(addrWidth)))
  val r = Decoupled(new AxiLiteReadRespChannel(dataWidth))

}