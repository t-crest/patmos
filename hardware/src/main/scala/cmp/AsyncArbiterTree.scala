/*
 * Asynchronous arbiter that can be used to connect various synchronous 
 * or asynchronous resources to cores and arbitrate access to it.
 *
 * Author: Torur Biskopsto Strom (torur.strom@gmail.com)
 *
 */
package cmp

import chisel3._
import chisel3.util.HasBlackBoxResource

class AsyncArbiterIO extends Bundle
{
  val ack = Input(Bool())
  val req = Output(Bool())

  override def clone = new AsyncArbiterIO().asInstanceOf[this.type]
}

class AsyncArbiterTreeIO(cnt: Int) extends AsyncArbiterIO
{
  val cores = Vec(cnt, Flipped(new AsyncArbiterIO()))

  override def clone = new AsyncArbiterTreeIO(cnt).asInstanceOf[this.type]
}

class AsyncArbiter extends BlackBox {
  val io = IO(new AsyncArbiterIO() {
    val req1 = Input(Bool())
    val req2 = Input(Bool())
    val ack1 = Output(Bool())
    val ack2 = Output(Bool())
  })
}

abstract class AsyncArbiterBase(corecnt: Int) extends Module {
  val io = IO(new AsyncArbiterTreeIO(corecnt))
}

class AsyncArbiterTree(corecnt : Int) extends AsyncArbiterBase(corecnt) {

  val leafarbiters = (0 until math.ceil(corecnt/2).toInt).map(i =>
  {
    val arbiter = Module(new AsyncArbiter())
    val idx = i*2
    arbiter.io.req1 := io.cores(idx).req
    io.cores(idx).ack := arbiter.io.ack1
    if(idx < corecnt-1)
    {
      arbiter.io.req2 := io.cores(idx+1).req
      io.cores(idx+1).ack := arbiter.io.ack2
    }
    arbiter
  })



  val genarbiter = new ((IndexedSeq[AsyncArbiter]) => AsyncArbiter){
    def apply(children:IndexedSeq[AsyncArbiter]):AsyncArbiter =
    {
      val len = children.count(e => true)
      println(len)
      if(len < 2)
        return children(0)
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

      val parent = Module(new AsyncArbiter())

      parent.io.req1 := child1.io.req
      child1.io.ack := parent.io.ack1
      parent.io.req2 := child2.io.req
      child2.io.ack := parent.io.ack2
      return parent
    }
  }

  val par = genarbiter(leafarbiters)

  par.io.ack := io.ack
  io.req := par.io.req
}
