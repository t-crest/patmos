/*
   Copyright 2018 Technical University of Denmark, DTU Compute.
   All rights reserved.

   MPU6050 sensor interface for I2C

   Author: Oktay Baris (okba@dtu.dk)

 */

package io

import chisel3._
import chisel3.util._
import ocp._

object MpuSensor extends DeviceObject {
    def init(params : Map[String, String]) = {
    }

    def create(params: Map[String, String]) : MpuSensor = {
      Module(new MpuSensor())
    }
}

//bundle for the blackbox
class MpuSensorIO() extends Bundle {
      //outputs
      val readdata_0  = Output(UInt(32.W))
      val readdata_1  = Output(UInt(32.W))
      val readdata_2  = Output(UInt(32.W))
      val readdata_3  = Output(UInt(32.W))
      val readdata_4  = Output(UInt(32.W))
      val readdata_5  = Output(UInt(32.W))
      val readdata_6  = Output(UInt(32.W))  //For future
      val readdata_7  = Output(UInt(32.W))  //For future
      val readdata_8  = Output(UInt(32.W))  //For future
      val readdata_9  = Output(UInt(32.W))  //For future
      // I2C pins
      val scl_out  = Output(UInt(1.W))
      // val sda_inout  = Output(UInt(1.W))  // this is an inout pin
      //val sda_inout  = Input(UInt(1.W))
      val sda_out  = Output(UInt(1.W))
      val sda_in  = Input(UInt(1.W))
      val we_out  = Output(UInt(1.W))
      //val reset = Bits(INPUT,1)

}

//blackbox
class MpuSensorBB() extends BlackBox {
    val io = new MpuSensorIO()
    //throw new Error("BlackBox wrapper for MpuSensor needs update for Chisel 3")
    // rename component
    //should be commented out when chisel3 is used
    /*setModuleName("imu_mpu")
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
    //io.reset.setName("reset")*/
    

}

//wrapper
class MpuSensor() extends CoreDevice() {
      override val io = new CoreDeviceIO() with patmos.HasPins {
        val pins: Bundle {
          val scl_out: UInt
          val sda_out: UInt
          val sda_in: UInt
          val we_out: UInt
        } = new Bundle() {
          // I2C pins
          val scl_out  = Output(UInt(1.W))
          // val sda_inout  = Output(UInt(1.W))  // this is an inout pin
          //val sda_inout  = Input(UInt(1.W))
          val sda_out  = Output(UInt(1.W))
          val sda_in  = Input(UInt(1.W))
          val we_out = Output(UInt(1.W))
          //val reset = Bits (INPUT,1)
        }
      }

      val bb = Module(new MpuSensorBB())

      // OCP Registers
      val ocpDataReg = Reg(UInt(32.W))
      val ocpRespReg = Reg(UInt(2.W))
      ocpRespReg := OcpResp.NULL

      // Connections to OCP master
      io.ocp.S.Resp := ocpRespReg
      io.ocp.S.Data := ocpDataReg

      // pin connection between the blackbox and the wrapper
      io.pins.scl_out := bb.io.scl_out

      bb.io.sda_in := io.pins.sda_in
      io.pins.sda_out := bb.io.sda_out
      io.pins.we_out := bb.io.we_out // enable for tristate

      // Address decoding for Reads
      when(io.ocp.M.Cmd === OcpCmd.RD) {
          ocpRespReg := OcpResp.DVA

          // address decoding
          switch(io.ocp.M.Addr(5,2)) {
            // Reading from readdata_0 register
            is("b0000".U) {
              ocpDataReg := bb.io.readdata_0
            }

            // Reading from readdata_1 register
            is("b0001".U) {
              ocpDataReg := bb.io.readdata_1
            }

            // Reading from readdata_2 register
            is("b0010".U) {
              ocpDataReg := bb.io.readdata_2
            }

            // Reading from readdata_3 register
            is("b0011".U) {
              ocpDataReg := bb.io.readdata_3
            }

            // Reading from readdata_4 register
            is("b0100".U) {
              ocpDataReg := bb.io.readdata_4
            }

            // Reading from readdata_5 register
            is("b0101".U) {
              ocpDataReg := bb.io.readdata_5
            }

            // Reading from readdata_6 register
            is("b0110".U) {
              ocpDataReg := bb.io.readdata_6
              //ocpDataReg := 101.U
            }

            // Reading from readdata_7 register
            is("b0111".U) {
              ocpDataReg := bb.io.readdata_7
            }

            // Reading from readdata_8 register
            is("b1000".U) {
              ocpDataReg := bb.io.readdata_8
            }

            // Reading from readdata_9 register
            is("b1001".U) {
              ocpDataReg := bb.io.readdata_9
            }


          }
      }

}


