/*
 * Asynchronous arbiter that can be used to connect various synchronous 
 * or asynchronous resources to cores and arbitrate access to it.
 *
 * Author: Torur Biskopsto Strom (torur.strom@gmail.com)
 *
 */
package cmp

import Chisel._

class AsyncArbiter2IO extends Bundle
{
  val ack = Bool(INPUT)
  val req = Bool(OUTPUT)

  override def clone = new AsyncArbiter2IO().asInstanceOf[this.type]
}

class AsyncArbiterIO(cnt: Int) extends AsyncArbiter2IO
{
  val cores = Vec(cnt, new AsyncArbiter2IO().flip())

  override def clone = new AsyncArbiterIO(cnt).asInstanceOf[this.type]
}

class AsyncArbiter2BB() extends BlackBox {
  val io = new AsyncArbiter2IO()
  {
    val req1 = Bool(INPUT)
    val req2 = Bool(INPUT)
    val ack1 = Bool(OUTPUT)
    val ack2 = Bool(OUTPUT)
  }

  // rename component
  setModuleName("AsyncArbiter2")

  //renameClock(clock, "clk")
  //renameReset("rst")

  io.req.setName("req")
  io.req1.setName("req1")
  io.req2.setName("req2")
  io.ack.setName("ack")
  io.ack1.setName("ack1")
  io.ack2.setName("ack2")
}

class AsyncArbiter(corecnt : Int) extends Module {
  override val io = new AsyncArbiterIO(corecnt)

  val leafmutexes = (0 until math.ceil(corecnt/2).toInt).map(i =>
  {
    val mutex = Module(new AsyncArbiter2BB())
    val idx = i*2
    mutex.io.req1 := io.cores(idx).req
    io.cores(idx).ack := mutex.io.ack1
    if(idx < corecnt-1)
    {
      mutex.io.req2 := io.cores(idx+1).req
      io.cores(idx+1).ack := mutex.io.ack2
    }
    mutex
  })



  val genmutex = new ((IndexedSeq[AsyncArbiter2BB]) => AsyncArbiter2BB){
    def apply(children:IndexedSeq[AsyncArbiter2BB]):AsyncArbiter2BB =
    {
      val len = children.count(e => true)
      if(len < 2)
        children(0)
      val _children =
        if(len > 2)
        {
          val childs = children.splitAt(len/2)
          (apply(childs._1), apply(childs._2))
        }
        else
          (children(0), children(1))

      val child1 = _children._1
      val child2 = _children._2

      val parent = Module(new AsyncArbiter2BB())

      parent.io.req1 := child1.io.req
      child1.io.ack := parent.io.ack1
      parent.io.req2 := child2.io.req
      child2.io.ack := parent.io.ack2
      parent
    }
  }

  val par = genmutex(leafmutexes)

  par.io.ack := io.ack
  io.req := par.io.req
}
