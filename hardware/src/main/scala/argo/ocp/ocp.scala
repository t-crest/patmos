// Constants and definitions for OCP

package ocp

import chisel3._
import chisel3.util._

abstract class Ocp extends Bundle {
  val BURST_ADDR_WIDTH = 21

  val OCP_CMD_WIDTH = 3
  val OCP_ADDR_WIDTH = 32
  val OCP_BURST_ADDR_WIDTH = BURST_ADDR_WIDTH
  val OCP_DATA_WIDTH = 32
  val OCP_BYTE_WIDTH = OCP_DATA_WIDTH / 8
  val OCP_RESP_WIDTH = 2

  val OCP_CMD_IDLE = 0.U(OCP_CMD_WIDTH.W)
  val OCP_CMD_WR = 1.U(OCP_CMD_WIDTH.W)
  val OCP_CMD_RD = 2.U(OCP_CMD_WIDTH.W)

  val OCP_RESP_NULL = 0.U(OCP_RESP_WIDTH.W)
  val OCP_RESP_DVA = 1.U(OCP_RESP_WIDTH.W)
  val OCP_RESP_FAIL = 2.U(OCP_RESP_WIDTH.W)
  val OCP_RESP_ERR = 3.U(OCP_RESP_WIDTH.W)
}

class Ocp_core_m extends Ocp {
  val MCmd = UInt(OCP_CMD_WIDTH.W)
  val MAddr = UInt(OCP_ADDR_WIDTH.W)
  val MData = UInt(OCP_DATA_WIDTH.W)
  val MByteEn = UInt(OCP_BYTE_WIDTH.W)
}

class Ocp_core_s extends Ocp {
  val SResp = UInt(OCP_RESP_WIDTH.W)
  val SData = UInt(OCP_DATA_WIDTH.W)
}

class Ocp_io_m extends Ocp {
  val MCmd = UInt(OCP_CMD_WIDTH.W)
  val MAddr = UInt(OCP_ADDR_WIDTH.W)
  val MData = UInt(OCP_DATA_WIDTH.W)
  val MByteEn = UInt(OCP_BYTE_WIDTH.W)
  val MRespAccept = UInt(1.W)
}

class Ocp_io_s extends Ocp {
  val SResp = UInt(OCP_RESP_WIDTH.W)
  val SData = UInt(OCP_DATA_WIDTH.W)
  val SCmdAccept = UInt(1.W)
}

class Ocp_burst_m extends Ocp {
  val MCmd = UInt(OCP_CMD_WIDTH.W)
  val MAddr = UInt(OCP_BURST_ADDR_WIDTH.W)
  val MData = UInt(OCP_DATA_WIDTH.W)
  val MDataByteEn = UInt(OCP_BYTE_WIDTH.W)
  val MDataValid = UInt(1.W)
}

class Ocp_burst_s extends Ocp {
  val SResp = UInt(OCP_RESP_WIDTH.W)
  val SData = UInt(OCP_DATA_WIDTH.W)
  val SCmdAccept = UInt(1.W)
  val SDataAccept = UInt(1.W)
}

