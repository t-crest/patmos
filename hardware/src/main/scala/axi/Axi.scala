package axi

import Chisel._

class AxiLiteWriteAddressChannel(addrWidth: Int) extends Bundle {
  val addr = Bits(width = Math.min(addrWidth, 32)) // M->S
  val prot = Bits(width = 2) // M->S Ignored in our case)
  if (addrWidth % 32 != 0) throw new IllegalArgumentException("Address width has to be multiple of 32 bits")
}

class AxiLiteWriteDataChannel(dataWidth: Int) extends Bundle {
  val data = Bits(width = dataWidth) // M->S
  val strb = Bits(width = dataWidth / 8) // M->S
  if (dataWidth % 32 != 0) throw new IllegalArgumentException("Data width has to be multiple of 32 bits")
}

class AxiLiteWriteRespChannel() extends Bundle {
  val resp = Bits(width = 2) // S->M
}

class AxiLiteReadAddressChannel(addrWidth: Int) extends Bundle {
  val addr = Bits(width = Math.min(addrWidth, 32)) // M->S
  val prot = Bits(width = 2) // M->S Ignored in our case)
  if (addrWidth % 32 != 0) throw new IllegalArgumentException("Address width has to be multiple of 32 bits")
}

class AxiLiteReadRespChannel(dataWidth: Int) extends Bundle {
  val data = Bits(dataWidth) // S->M
  val resp = Bits(width = 2) // S->M
  if (dataWidth % 32 != 0) throw new IllegalArgumentException("Data width has to be multiple of 32 bits")
}

class AxiLiteMasterPort(addrWidth: Int, dataWidth: Int) extends Bundle {
  val aw = Decoupled(new AxiLiteReadAddressChannel(addrWidth))
  val w = Decoupled(new AxiLiteWriteDataChannel(dataWidth))
  val b = Decoupled(new AxiLiteWriteRespChannel()).flip()
  val ar = Decoupled(new AxiLiteReadAddressChannel(addrWidth))
  val r = Decoupled(new AxiLiteReadRespChannel(dataWidth)).flip()
}

class AxiLiteSlavePort(addrWidth: Int, dataWidth: Int) extends Bundle {
  val aw = Decoupled(new AxiLiteReadAddressChannel(addrWidth)).flip()
  val w = Decoupled(new AxiLiteWriteDataChannel(dataWidth)).flip()
  val b = Decoupled(new AxiLiteWriteRespChannel())
  val ar = Decoupled(new AxiLiteReadAddressChannel(addrWidth)).flip()
  val r = Decoupled(new AxiLiteReadRespChannel(dataWidth))
}