package ptp1588assist

/**
  * Mockup Ethernet frame
  * Nibbles in the frame are in LSB-first per byte order
  */
abstract class EthernetFrame{

  //Constants
  val IP_UDP_PROTOCOL = 0x11
  val PTP_FOLLOW_UP_TYPE = 0x8

  //Vars
  val preamble : Array[Byte]
  val dstMac : Array[Byte]
  val srcMac : Array[Byte]
  val ethType : Array[Byte]
  val ipHeader : Array[Byte]
  val udpHeader : Array[Byte]
  val ptpHeader : Array[Byte]
  val ptpBody : Array[Byte]
  val ptpSuffix : Array[Byte]
  val fcs : Array[Byte]
  val igp : Array[Byte]

  //Getters for nibbles
  def preambleNibbles : Array[Int] = dataBytesToNibbles(preamble, msbFirst = false)
  def dstMacNibbles : Array[Int] = dataBytesToNibbles(dstMac, msbFirst = false)
  def srcMacNibbles : Array[Int] = dataBytesToNibbles(srcMac, msbFirst = false)
  def ethTypeNibbles : Array[Int] = dataBytesToNibbles(ethType, msbFirst = false)
  def ipHeaderNibbles : Array[Int] = dataBytesToNibbles(ipHeader, msbFirst = false)
  def udpHeaderNibbles : Array[Int] = dataBytesToNibbles(udpHeader, msbFirst = false)
  def ptpHeaderNibbles : Array[Int] = dataBytesToNibbles(ptpHeader, msbFirst = false)
  def ptpBodyNibbles : Array[Int] = dataBytesToNibbles(ptpBody, msbFirst = false)
  def ptpSuffixNibbles : Array[Int] = dataBytesToNibbles(ptpSuffix, msbFirst = false)
  def fcsNibbles : Array[Int] = dataBytesToNibbles(fcs, msbFirst = false)
  def igpNibbles : Array[Int] = dataBytesToNibbles(igp, msbFirst = false)

  /**
    * Converts a byte to an array of nibbles
    * @param byte gets broken down to nibbles
    * @param msbFirst controls the order of the nibbles
    * @return an array of two nibbles
    */
  def byteToNibble(byte: Int, msbFirst: Boolean): Array[Int] ={
    val nibbles = new Array[Int](2)
    nibbles(0) = byte & 0x0F
    nibbles(1) = (byte & 0xF0) >> 4
    if(msbFirst){
      nibbles.reverse
    } else {
      nibbles
    }
  }

  /**
    * Converts an array of bytes to an array of nibbles
    * @param bytes the data to be broken down to nibbles
    * @param msbFirst controls the order of the nibbles per byte
    * @return an array of nibbles size bytes.size*2
    */
  def dataBytesToNibbles(bytes: Array[Byte], msbFirst: Boolean): Array[Int] = {
    val nibbles = new Array[Int](bytes.length*2)
    var i = 0
    for (byte <- bytes){
      val tempByteNibbles = byteToNibble(byte, msbFirst)
      nibbles(i) = tempByteNibbles(0)
      nibbles(i+1) = tempByteNibbles(1)
      i=i+2
    }
    nibbles
  }

  /**
    * Lazy way for converting Integer values in array to bytes
    * @param xs Integer Array values
    * @return Array of bytes
    */
  def toBytes(xs: Int*) = xs.map(_.toByte).toArray

}

object EthernetTesting{

  val mockupPTPEthFrameDHCP = new EthernetFrame {
    override val preamble: Array[Byte] = toBytes(0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0xD5)
    override val dstMac : Array[Byte] = toBytes(0xff, 0xff, 0xff, 0xff, 0xff, 0xff)
    override val srcMac : Array[Byte] = toBytes(0x80, 0xce, 0x62, 0xd8, 0xc7, 0x39)
    override val ethType : Array[Byte] = toBytes(0x08, 0x00)
    override val ipHeader : Array[Byte] = toBytes(0x45, 0x10, 0x01, 0x48, 0x00, 0x00, 0x00, 0x00,
                                                  0x80, 0x11, 0x39, 0x96, 0x00, 0x00, 0x00, 0x00,
                                                  0xff, 0xff, 0xff, 0xff)
    override val udpHeader: Array[Byte] = toBytes(0x00, 0x44, 0x00, 0x43, 0x01, 0x34, 0x5c, 0xec)
    //Should be ignored
    override val ptpHeader: Array[Byte] = toBytes(PTP_FOLLOW_UP_TYPE, 0x02, 0x00, 0x2C, 0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00, 0xE4, 0xAF, 0xA1, 0x30,
                                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x55, 0x55,
                                                  0x00, 0x00)
    override val ptpBody: Array[Byte] = toBytes(0x54, 0x32, 0x11, 0x11, 0x55, 0x55, 0xAA, 0xAA)
    override val ptpSuffix: Array[Byte] = toBytes(0x00)
    override val fcs: Array[Byte] = toBytes(0x00)
    override val igp: Array[Byte] = toBytes(0x00)
  }

  val mockupPTPVLANFrameOverIpUDP = new EthernetFrame {
    override val preamble: Array[Byte] = toBytes(0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0xD5)
    override val dstMac : Array[Byte] = toBytes(0x01, 0x00, 0x5E, 0x00, 0x01, 0x81)
    override val srcMac : Array[Byte] = toBytes(0x01, 0x00, 0x5E, 0x00, 0x01, 0x81)
    override val ethType : Array[Byte] = toBytes(0x81, 0x00, 0x01, 0x11, 0x08, 0x00)
    override val ipHeader : Array[Byte] = toBytes(0x45, 0x00, 0x00, 0x48, 0x00, 0x5D, 0x40, 0x00,
                                                  0x01, 0x11, 0x29, 0x68, 0xC0, 0xA8, 0x01, 0x01,
                                                  0xE0, 0x00, 0x01, 0x81)
    override val udpHeader: Array[Byte] = toBytes(0x01, 0x3F, 0x01, 0x3F, 0x00, 0x34, 0x00, 0x00)
    override val ptpHeader: Array[Byte] = toBytes(PTP_FOLLOW_UP_TYPE, 0x02, 0x00, 0x2C, 0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00, 0xE4, 0xAF, 0xA1, 0x30,
                                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x55, 0x55,
                                                  0x00, 0x00)
    override val ptpBody: Array[Byte] = toBytes(0x54, 0x32, 0x11, 0x11, 0x55, 0x55, 0xAA, 0xAA)
    override val ptpSuffix: Array[Byte] = toBytes(0x00)
    override val fcs: Array[Byte] = toBytes(0x00)
    override val igp: Array[Byte] = toBytes(0x00)
  }

  val mockupPTPEthFrameOverIpUDP = new EthernetFrame {
    override val preamble: Array[Byte] = toBytes(0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0xD5)
    override val dstMac : Array[Byte] = toBytes(0x01, 0x00, 0x5E, 0x00, 0x01, 0x81)
    override val srcMac : Array[Byte] = toBytes(0x01, 0x00, 0x5E, 0x00, 0x01, 0x81)
    override val ethType : Array[Byte] = toBytes(0x08, 0x00)
    override val ipHeader : Array[Byte] = toBytes(0x45, 0x00, 0x00, 0x48, 0x00, 0x5D, 0x40, 0x00,
                                                  0x01, 0x11, 0x29, 0x68, 0xC0, 0xA8, 0x01, 0x01,
                                                  0xE0, 0x00, 0x01, 0x81)
    override val udpHeader: Array[Byte] = toBytes(0x01, 0x3F, 0x01, 0x3F, 0x00, 0x34, 0x00, 0x00)
    override val ptpHeader: Array[Byte] = toBytes(PTP_FOLLOW_UP_TYPE, 0x02, 0x00, 0x2C, 0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00, 0xE4, 0xAF, 0xA1, 0x30,
                                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x55, 0x55,
                                                  0x00, 0x00)
    override val ptpBody: Array[Byte] = toBytes(0x54, 0x32, 0x11, 0x11, 0x55, 0x55, 0xAA, 0xAA)
    override val ptpSuffix: Array[Byte] = toBytes(0x00)
    override val fcs: Array[Byte] = toBytes(0x00)
    override val igp: Array[Byte] = toBytes(0x00)
  }

}