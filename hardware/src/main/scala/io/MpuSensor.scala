/*
   Copyright 2018 Technical University of Denmark, DTU Compute.
   All rights reserved.

   MPU6050 sensor interface for I2C

   Author: Oktay Baris (okba@dtu.dk)

 */

package io

import Chisel._
import Node._
import ocp._

object MpuSensor extends DeviceObject {
    def init(params : Map[String, String]) = {
    }

    def create(params: Map[String, String]) : MpuSensor = {
      Module(new MpuSensor())
    }

    trait Pins {
      val mpuSensorPins = new Bundle() {
          // I2C pins
          val scl_out  = Bits(OUTPUT, width = 1)
          // val sda_inout  = Bits(OUTPUT, width = 1)  // this is an inout pin
          //val sda_inout  = Bits(INPUT, width = 1)
          val sda_out  = Bits(OUTPUT, width = 1)
          val sda_in  = Bits(INPUT, width = 1)
          val we_out = Bits (OUTPUT, 1)
          //val reset = Bits (INPUT,1)
      }
    }
}

//bundle for the blackbox
class MpuSensorIO() extends Bundle {
      //outputs
      val readdata_0  = Bits(OUTPUT, width = 32)
      val readdata_1  = Bits(OUTPUT, width = 32)
      val readdata_2  = Bits(OUTPUT, width = 32)
      val readdata_3  = Bits(OUTPUT, width = 32)
      val readdata_4  = Bits(OUTPUT, width = 32)
      val readdata_5  = Bits(OUTPUT, width = 32)
      val readdata_6  = Bits(OUTPUT, width = 32)  //For future
      val readdata_7  = Bits(OUTPUT, width = 32)  //For future
      val readdata_8  = Bits(OUTPUT, width = 32)  //For future
      val readdata_9  = Bits(OUTPUT, width = 32)  //For future
      // I2C pins
      val scl_out  = Bits(OUTPUT, width = 1)
      // val sda_inout  = Bits(OUTPUT, width = 1)  // this is an inout pin
      //val sda_inout  = Bits(INPUT, width = 1)
      val sda_out  = Bits(OUTPUT, width = 1)
      val sda_in  = Bits(INPUT, width = 1)
      val we_out  = Bits(OUTPUT, width = 1)
      //val reset = Bits(INPUT,1)

}

//blackbox
class MpuSensorBB() extends BlackBox {
    val io = new MpuSensorIO()
    // rename component
    setModuleName("imu_mpu")
    // rename signals
    renameClock( "clk", "clk")
    addClock(Driver.implicitClock)
    reset.setName("reset")
    //outputs
    io.readdata_0.setName("readdata_0")
    io.readdata_1.setName("readdata_1")
    io.readdata_2.setName("readdata_2")
    io.readdata_3.setName("readdata_3")
    io.readdata_4.setName("readdata_4")
    io.readdata_5.setName("readdata_5")
    // outputs for the future usage
    io.readdata_6.setName("readdata_6")
    io.readdata_7.setName("readdata_7")
    io.readdata_8.setName("readdata_8")
    io.readdata_9.setName("readdata_9")
    //I2C
    io.scl_out.setName("scl_out")
    //io.mpuI2cPins.sda_inout.setName("sda_inout") //Inout
    //io.sda_inout.setName("sda_inout")  //In
    io.sda_out.setName("sda_out")   //out
    io.sda_in.setName("sda_in")   //in
    io.we_out.setName("we_out")
    //io.reset.setName("reset")


}

//wrapper
class MpuSensor() extends CoreDevice() {
      override val io = new CoreDeviceIO() with MpuSensor.Pins

      val bb = Module(new MpuSensorBB())

      // OCP Registers
      val ocpDataReg = Reg(Bits(width = 32))
      val ocpRespReg = Reg(Bits(width = 2))
      ocpRespReg := OcpResp.NULL

      // Connections to OCP master
      io.ocp.S.Resp := ocpRespReg
      io.ocp.S.Data := ocpDataReg

      // pin connection between the blackbox and the wrapper
      io.mpuSensorPins.scl_out := bb.io.scl_out

      bb.io.sda_in := io.mpuSensorPins.sda_in
      io.mpuSensorPins.sda_out := bb.io.sda_out
      io.mpuSensorPins.we_out := bb.io.we_out // enable for tristate

      // Address decoding for Reads
      when(io.ocp.M.Cmd === OcpCmd.RD) {
          ocpRespReg := OcpResp.DVA

          // address decoding
          switch(io.ocp.M.Addr(5,2)) {
            // Reading from readdata_0 register
            is(Bits("b0000")) {
              ocpDataReg := bb.io.readdata_0
            }

            // Reading from readdata_1 register
            is(Bits("b0001")) {
              ocpDataReg := bb.io.readdata_1
            }

            // Reading from readdata_2 register
            is(Bits("b0010")) {
              ocpDataReg := bb.io.readdata_2
            }

            // Reading from readdata_3 register
            is(Bits("b0011")) {
              ocpDataReg := bb.io.readdata_3
            }

            // Reading from readdata_4 register
            is(Bits("b0100")) {
              ocpDataReg := bb.io.readdata_4
            }

            // Reading from readdata_5 register
            is(Bits("b0101")) {
              ocpDataReg := bb.io.readdata_5
            }

            // Reading from readdata_6 register
            is(Bits("b0110")) {
              ocpDataReg := bb.io.readdata_6
              //ocpDataReg := UInt(101)
            }

            // Reading from readdata_7 register
            is(Bits("b0111")) {
              ocpDataReg := bb.io.readdata_7
            }

            // Reading from readdata_8 register
            is(Bits("b1000")) {
              ocpDataReg := bb.io.readdata_8
            }

            // Reading from readdata_9 register
            is(Bits("b1001")) {
              ocpDataReg := bb.io.readdata_9
            }


          }
      }

}


