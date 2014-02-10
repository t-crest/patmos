package tools

object TestAsm {

    def readBin(fileName: String): Array[Int] = {

    println("Reading " + fileName)
    // an encoding to read a binary file? Strange new world.
    val source = scala.io.Source.fromFile(fileName)(scala.io.Codec.ISO8859)
    val byteArray = source.map(_.toByte).toArray
    source.close()

    // use an array to convert input
    val arr = new Array[Int](math.max(1, byteArray.length / 4))

    if (byteArray.length == 0) {
      arr(0) = 0
    }

    for (i <- 0 until byteArray.length / 4) {
      var word = 0
      for (j <- 0 until 4) {
        word <<= 8
        word += byteArray(i * 4 + j).toInt & 0xff
      }
      // printf("%08x\n", Bits(word))
      arr(i) = word
    }

    arr
  }

  def main(args: Array[String]): Unit = {
    val arr = readBin("/Users/martin/t-crest/patmos/tmp/abc.bin")
    for (i <- 0 to arr.length-1) {
      println(arr(i))
    }
  }
}