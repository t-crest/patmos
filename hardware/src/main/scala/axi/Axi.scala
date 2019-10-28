package axi

import Chisel._

object AXIResp {
  val OKAY = UInt("b000")
  val EXOKAY = UInt("b001")
  val SLVERR = UInt("b010")
  val DECERR = UInt("b011")
}

class AxiLiteWriteAddressChannel(addrWidth: Int) extends Bundle {
  val addr = Bits(width = Math.max(addrWidth, 32)) // M->S
  val prot = Bits(width = 2) // M->S Ignored in our case)
  if (addrWidth % 32 != 0) throw new IllegalArgumentException("Address width has to be multiple of 32 bits")

  // This does not really clone, but Data.clone doesn't either
  override def clone() = {
    val res = new AxiLiteWriteAddressChannel(addrWidth)
    res.asInstanceOf[this.type]
  }
}

class AxiLiteWriteDataChannel(dataWidth: Int) extends Bundle {
  val data = Bits(width = Math.max(dataWidth, 32)) // M->S
  val strb = Bits(width = dataWidth / 8) // M->S
  if (dataWidth % 32 != 0) throw new IllegalArgumentException("Data width has to be multiple of 32 bits")

  // This does not really clone, but Data.clone doesn't either
  override def clone() = {
    val res = new AxiLiteWriteDataChannel(dataWidth)
    res.asInstanceOf[this.type]
  }
}

class AxiLiteWriteRespChannel() extends Bundle {
  val resp = Bits(width = 2) // S->M

  // This does not really clone, but Data.clone doesn't either
  override def clone() = {
    val res = new AxiLiteWriteRespChannel()
    res.asInstanceOf[this.type]
  }
}

class AxiLiteReadAddressChannel(addrWidth: Int) extends Bundle {
  val addr = Bits(width = Math.max(addrWidth, 32)) // M->S
  val prot = Bits(width = 2) // M->S Ignored in our case)
  if (addrWidth % 32 != 0) throw new IllegalArgumentException("Address width has to be multiple of 32 bits")

  // This does not really clone, but Data.clone doesn't either
  override def clone() = {
    val res = new AxiLiteReadAddressChannel(addrWidth)
    res.asInstanceOf[this.type]
  }
}

class AxiLiteReadRespChannel(dataWidth: Int) extends Bundle {
  val data = Bits(width = Math.max(dataWidth, 32)) // S->M
  val resp = Bits(width = 2) // S->M
  if (dataWidth % 32 != 0) throw new IllegalArgumentException("Data width has to be multiple of 32 bits")

  // This does not really clone, but Data.clone doesn't either
  override def clone() = {
    val res = new AxiLiteReadRespChannel(dataWidth)
    res.asInstanceOf[this.type]
  }
}

class AxiLiteMasterPort(addrWidth: Int, dataWidth: Int) extends Bundle {
  val aw = Decoupled(new AxiLiteWriteAddressChannel(addrWidth))
  val w = Decoupled(new AxiLiteWriteDataChannel(dataWidth))
  val b = Decoupled(new AxiLiteWriteRespChannel()).flip()
  val ar = Decoupled(new AxiLiteReadAddressChannel(addrWidth))
  val r = Decoupled(new AxiLiteReadRespChannel(dataWidth)).flip()
}

class AxiLiteSlavePort(addrWidth: Int, dataWidth: Int) extends Bundle {
  val aw = Decoupled(new AxiLiteWriteAddressChannel(addrWidth)).flip()
  val w = Decoupled(new AxiLiteWriteDataChannel(dataWidth)).flip()
  val b = Decoupled(new AxiLiteWriteRespChannel())
  val ar = Decoupled(new AxiLiteReadAddressChannel(addrWidth)).flip()
  val r = Decoupled(new AxiLiteReadRespChannel(dataWidth))
}