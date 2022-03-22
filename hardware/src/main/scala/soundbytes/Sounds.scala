package soundbytes

import soundbytes.Sounds._

import java.io.{File, ByteArrayInputStream}
import javax.sound.sampled.{AudioFormat, AudioSystem, AudioInputStream, AudioFileFormat}

object Sounds {

  val buf = new Array[Byte](2)
  val af = new AudioFormat(Constants.SampleFrequency.toFloat, 16, 1, true, true)
  val sdl = AudioSystem.getSourceDataLine(af)
  
  
  def getSamples = {
    val samples = new Array[Short](50000)

    val halfPeriod = Constants.SampleFrequency / 440 / 2
    var cnt = 0
    var high = true
    var attack = true

    var env = 0
    for (i <- 0 until  samples.length) {
      var s = if (high) 20000 else -20000
      if (cnt == halfPeriod) {
        cnt = 0
        high = !high
      } else {
        cnt += 1
      }
      s = s * env / 30000
      if (env > 30000) attack = false
      if (attack) {
        env += 10
      } else if (env >= 0) {
        env -= 1
      }
      samples(i) = s.toShort
    }
    samples
  }

  def getFileSamples(s: String): Array[Short] = {
    var f = new File(s);
    var stream = AudioSystem.getAudioInputStream(f);
    stream = AudioSystem.getAudioInputStream(af, stream)
  
    var sampleCount = stream.getFrameLength().toInt
    var samples = new Array[Short](sampleCount)
    for(i <- 0 until sampleCount) { 
      stream.read(buf, 0, 2)
      samples(i) = ((buf(0) << 8) | (buf(1) & 0xFF)).toShort
    }
    
    return samples
  }

  def startPlayer = {
  	println("starting")
    sdl.open(af, 2048)
    sdl.start()
  	println("started")
  }

  def play(s: Short) = {
    buf(0) = (s >> 8).toByte
	  buf(1) = (s & 0xFF).toByte
    sdl.write(buf, 0, 2)
  }
  
  def playArray(arr: Array[Short]) = {
    var samples = arr.length
    var offset = 0
    var arr_conv = new Array[Byte](2048)

    while(samples > 1024) {
      for(i <- 0 until 1024) {
        arr_conv(2 * i) = (arr(offset + i) >> 8).toByte
        arr_conv(2 * i + 1) = (arr(offset + i) & 0xFF).toByte
	    }
	    sdl.write(arr_conv, 0, 2048)
	    samples -= 1024
	    offset += 1024
  	}

    for(i <- 0 until samples) {
        arr_conv(2 * i) = (arr(offset + i) >> 8).toByte
        arr_conv(2 * i + 1) = (arr(offset + i) & 0xFF).toByte
    }
    sdl.write(arr_conv, 0, samples * 2)
  }
  
  def saveArray(arr : Array[Short], s : String) = {
    var f = new File(s)
    f.createNewFile()
    
    var arr_conv = new Array[Byte](arr.length * 2)
    for(i <- 0 until arr.length) {
        arr_conv(2 * i) = (arr(i) >> 8).toByte
        arr_conv(2 * i + 1) = (arr(i) & 0xFF).toByte
    }
    
    var stream = new AudioInputStream(new ByteArrayInputStream(arr_conv), af, arr.length)
    AudioSystem.write(stream, AudioFileFormat.Type.WAVE, f)
  }
  
  def stopPlayer = {
  	println("draining")
    sdl.drain()
  	println("drained")
  }


}

object PlaySounds extends App {
  val samples = getSamples
  startPlayer

  for (s <- samples) {
    play(s)
  }
  stopPlayer
}
