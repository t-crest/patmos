/*
 * I2C interface for Patmos
 *
 * Authors: 
 * 	Kasper Juul Hesse Rasmussen (s183735@student.dtu.dk)
 * 	Tjalfe Egholm Rude (s183694@student.dtu.dk)
 *
 */
package io

import Chisel._
import ocp._

object I2Controller extends DeviceObject {
	var sclFreq = 100000
	var respectStretch = false

  def init(params : Map[String, String]) = {
	  sclFreq = getPosIntParam(params, "sclFreq")
	  respectStretch = getBoolParam(params, "respectStretch")
  }

  def create(params: Map[String, String]) : I2Controller = {
    Module(new I2Controller(sclFreq, respectStretch))
  }
}

class I2Controller(sclFreq: Int, respectStretch: Boolean) extends CoreDevice() {
  override val io = new CoreDeviceIO() with patmos.HasPins {
    override val pins = new Bundle() {
		 val sdaIn = Bool(INPUT)
		 val sdaOut = Bool(OUTPUT)
		 val sclOut = Bool(OUTPUT)
		 val sclIn = Bool(INPUT)
		 val i2cEn = Bool(OUTPUT)
		 val busy = Bool(OUTPUT)
		 val sclClk = Bool(OUTPUT)
		 val sdaClk = Bool(OUTPUT)
    }
  }

  //Shortcuts for accessing external pins
  val sdaIn = io.pins.sdaIn
  val sdaOut = io.pins.sdaOut
  val sclOut = io.pins.sclOut
  val sclIn = io.pins.sclIn
  val i2cEn = io.pins.i2cEn



	val CLOCK_FREQ = 80000000 //Board clock speed (80MHz)
	val OUT_FREQ = sclFreq //i2c bus speed (100kHz)
	var DIVIDER = (CLOCK_FREQ/OUT_FREQ)/4 //Width of 1/4 clock cycle //Default value: 200

//   if(testing) { //When testing, we want a lower divider (20 instead of 125)
// 	  DIVIDER = 20
//   }
	/*
	======= Signals =======
	*/

	//i2c Timing generation
	val sclClk = RegInit(false.B) //Internal clock ena/disable scl
	val sdaClk = RegInit(false.B) //Internal clock ena/disabling sda
	val sdaClkPrev = RegNext(sdaClk) //Previous value, used to detect rising/falling edge
	val clkCnt = RegInit(UInt(0, width=32))  //Counter used to generate sda/sclClk
	val stretch = Wire(init=false.B) //Is the slave device clock stretching
	val sdaRising = sdaClk && !sdaClkPrev //Rising edge of sdaClk
	val sdaFalling = !sdaClk && sdaClkPrev //Falling edge of sdaClk

	io.pins.sclClk := sclClk
   io.pins.sdaClk := sdaClk

	//I2C read registers
	val readVals = Reg(Vec(4,RegInit(UInt(0, width=8))))
	val readCount = RegInit(UInt(0, width=3))
	val readFlag =  RegInit(false.B)

	

	//i2c process State machine
	val ready::start::command::slv_ack1::wr::rd::slv_ack2::mstr_ack::stop::Nil = Enum(UInt(), 9) //States
	val i2cState = RegInit(ready) //State register
	when(i2cState === ready) {
		i2cEn := false.B
	} .otherwise {
		i2cEn := true.B
	}
	/*
	0:ready
	1:start
	2:command
	3:slv_ack1
	4:wr
	5:rd
	6:slv_ack2
	7:mstr_ack
	8:stop
	*/

	//i2c read/write dataWr
	val i2c = Reg(new OcpI2CVals) //Bundle of dataWr, addr values for i2c

	//i2c state machine helper signals
	//val i2c_data_rd = Vec(8, Bool()) //Data read from slave
	val sclEn = RegInit(false.B) //Enable for scl (used in all other states than start/stop)
	val sdaOutval = RegInit(true.B) //Value that should be written on sdaOut. Defaults true for high impedance
	val bitcnt = RegInit(UInt(7, width=3)) //Which bit we're currently reading/writing
	val ackError = RegInit(false.B) //Is there an acknowledge error from slave?

//	val ocpAddr = Reg(UInt(width=7)) //Address of the current slave device
//	val ocpRw = Reg(UInt(width=1)) //Read(1) or write(0)
	//end 
	val ocp = Reg(new OcpI2CVals)
//	val ocpReg = Reg(ocp)


//	val addrRW = Reg(UInt(width=8)) //Concatenated version of address and read/write bit
//	val dataWr = Reg(UInt(width=8)) //Data to write to slave

	//OCP signals
//	val ocp_data_wr = Reg(UInt(width=8)) //Write dataWr received from OCP
//	val ocp_data_rd = Reg(Vec(8, Bool())) //Data received from slave


	//Miscellaneous signals
	val busy = RegInit(false.B) //Is the i2c system currently processing a transaction
	val ena = RegInit(false.B) //Should we send dataWr? (Set by OCP)
	io.pins.busy := busy



	/*
	======= I2C-timing generation ========
	*/
	//Generates the timing for sclClk and sdaClk
	when(clkCnt === (DIVIDER*4-1).U) { //End of timing cycle
		clkCnt := 0.U
	} .elsewhen(!stretch) { //Only increment when slave is not stretching
		clkCnt := clkCnt + 1.U
	}

	//Use above generated timing to toggle scl/sdaClk
	when(clkCnt < (DIVIDER-1).U) { //First 1/4, both clocks are off
		sclClk := false.B
		sdaClk := false.B
	} .elsewhen((DIVIDER-1).U <= clkCnt && clkCnt < (2*DIVIDER-1).U) { //Second 1/4, enable sda before scl
		sclClk := false.B
		sdaClk := true.B
	} .elsewhen((2*DIVIDER-1).U <= clkCnt && clkCnt < (3*DIVIDER-1).U) { //Third 1/4, enable both, check for stretching
		sclClk := true.B
		sdaClk := true.B
		//Check if the slave is clock strething
		when(sclIn || !(respectStretch.B)) { //If input clock is high, no stretching is occuring
			stretch := false.B
		} .otherwise {
			stretch := true.B //Slave is clock stretching
		}
	} .otherwise { //Last 1/4, disable sdaClk and keep sclClk high
		sdaClk := false.B
		sclClk := true.B
	}

	/*
	======== START/STOP CONDITION ========
	*/
	//Generate start/stop condition
	//This makes sure that SDA goes low before SCL,
	//and conversely that SCL goes high before SDA
	//In all other states, the output follows outval being written
	when(i2cState === start) {
		sdaOut := sdaClkPrev
	} .elsewhen(i2cState === stop) {
		sdaOut := !sdaClkPrev
	} .otherwise {
		sdaOut := sdaOutval
	}

	sclOut := Mux(sclEn && !sclClk, false.B, true.B)

	/*
	======== I2C STATE MACHINE ========
	*/
	when(sdaRising) { //State machine updates on rising edge
	//Most elements of the state machine update on the rising edge.
	//A couple of things update on the falling edge, however (see below)
		switch(i2cState) {
			is(ready) { //Ready/idle state
				when(ena) { //Start read/write process
					busy := true.B //Set busy flag, signal that we're processing transaction

					//Load values from ocp registers into i2c registers
					i2c.addrRW := ocp.addrRW
					i2c.dataWr := ocp.dataWr
					i2cState := start //Set next-state
				} .otherwise { //Stay here
					busy := false.B
					i2cState := ready
				}
			}

			is(start) { //Start state, latch in first value to write out
				busy := true.B //Keep busy flag high
				sdaOutval := i2c.addrRW(bitcnt)
				i2cState := command
			}

			is(command) { //Send slave address and read/write command
				when(bitcnt === 0.U) { //Finished sending ocpAddr and ocpRw bit
					sdaOutval := true.B //Latch dataWr to high impedance to allow slave to pull low
					bitcnt := 7.U //Reset bitcnt
					i2cState := slv_ack1 //Go to slave ack bit
				} .otherwise { //Still sending
					sdaOutval := i2c.addrRW(bitcnt-1.U) //Preload next value
					bitcnt := bitcnt - 1.U //Count down
					i2cState := command
				}
			}

			is(slv_ack1) { //Slave acknowledgement from ocpAddr and ocpRw bit
				when(i2c.addrRW(0) === 0.U) { //Write command. Extract from i2c to make sure it hasn't been altered by new OCP command
					sdaOutval := i2c.dataWr(bitcnt) //Load first bit to write
					i2cState := wr
				} .otherwise { //Read command
					sdaOutval := 1.U //Latch to high impedance to allow reading
					i2cState := rd
				}
			}

			is(wr) { //Write dataWr to slave
				busy := true.B //Keep busy high
				when(bitcnt === 0.U) { //Finished writing
					sdaOutval := 1.U //Latch to high to allow slave acknowledgement
					bitcnt := 7.U //Reset bitcnt
					i2cState := slv_ack2 
				} .otherwise { //Still writing
					sdaOutval := i2c.dataWr(bitcnt-1.U) //Preload next value
					bitcnt := bitcnt - 1.U //Count down
					i2cState := wr
				}
			}

			is(slv_ack2) { //Slave acknowledge after writing dataWr
				when(ena) { //Is there more dataWr to send?
					busy := false.B //Set busy to signal that we're ready for more dataWr
					i2c.addrRW := ocp.addrRW //Load new address and read/write bit
					i2c.dataWr := ocp.dataWr //Load new value to write
					when(ocp.addrRW === i2c.addrRW) { //When current address rw-bit matches upcoming values
						sdaOutval := ocp.dataWr(bitcnt) //Load new value to write
						i2cState := wr //Return to write state
					} .otherwise { //Address and/or rw bit does not match
						i2cState := start //Return to start
					}
				} .otherwise { //Enable is zero
					i2cState := stop // No more to do, exit out
				}
			}

			is(rd) {
				//The actual read is performed on the falling edge of sdaClk
				busy := true.B //Keep busy flag high
				when(bitcnt === 0.U) { //Finished reading

					when(ena && (i2c.addrRW === ocp.addrRW)) {//When enabled and current addr matches upcoming addr
						sdaOutval := false.B //Pull low to acknowledge received data
					} .otherwise {
						sdaOutval := true.B //Pull high to send no-ack and break current read
					}

					bitcnt := 7.U //Reset bitcnt
					i2cState := mstr_ack //Proceed to acknowledge state

					readVals(readCount) := i2c.dataRd //Store read data in another register
					readCount := readCount + 1.U //Keep track
					i2c.dataRd := 0.U //Reset our read data
				} .otherwise { //Still counting
					bitcnt := bitcnt - 1.U
					i2cState := rd
				}
			}

			is(mstr_ack) {
				when(ena) { //Enable is high, there is more to do
					busy := false.B //reset busy flag to signal OCP we're ready for more
					i2c.addrRW := ocp.addrRW //Load new address
					i2c.dataWr := ocp.dataWr //Preload dataWr if next command is a write command
					when(i2c.addrRW === ocp.addrRW) { //If addr and RW bit have not changed
						sdaOutval := true.B //Pull to high-impedance to allow read
						i2cState := rd 
					} .otherwise { //Addr and/or rw bit has been changed, start over
						i2cState := start
						readFlag := true.B
					}
				} .otherwise { //Nothing more to do
					i2cState := stop
					readFlag := true.B
				}
			}

			is(stop) {
				busy := false.B //Signal to OCP that we're finished
				i2cState := ready //Return to ready state
			}
		}
	} .elsewhen(sdaFalling) {
		switch(i2cState) {
			is(start) {
				when(!sclEn) { //If scl output is not enabled, do so
				//By updating this in the falling edge of sdaClk, we make sure to let sda go low before scl
					sclEn := true.B
					ackError := false.B //Clear acknowledge error bit
				}
			}

			is(slv_ack1) { //First slave ack after sending address
				when(sdaIn || ackError) { //If sda is high, the slave has not acknowledged
					ackError := true.B //Set error flag high
				}
			}

			is(rd) {
				// when(testing.B){
				// 	i2c.dataRd(bitcnt) := 1.U
				// } otherwise{
					i2c.dataRd(bitcnt) := sdaIn //Latch in current value on sda
				// }
			}

			is(slv_ack2) { //Slave ack after sending dataWr
				when(sdaIn || ackError) { //If sda is high, the slave has not acknowledged
					ackError := true.B //Set error flag high
				}	
			}

			is(stop) { 
				sclEn := false.B //Disable scl output, satisfies start/stop condition
			}
		}
	}

	/*
	======== OCP INTERFACE AND STATE MACHINE ========
	TODO
	Make sure that user retrieves read data
	*/
	val ocpM = io.ocp.M //Shortcut to access master register
	val ocpS = io.ocp.S //Shortcut to access slave output register
	val ocpIdle :: ocpLoad :: ocpListen :: ocpMore :: ocpReadWaiter :: Nil = Enum(UInt(), 5)

	val ocpState = RegInit(ocpIdle) //The current state of the ocp/i2c transaction interface
	val ocpCmd = (ocpM.Cmd === OcpCmd.RD || ocpM.Cmd === OcpCmd.WR) //If a command has been received
	val ocpAvailable = RegInit(true.B) //Is OCP available for more dataWr?
	val bytesToRead = RegInit(UInt(0, width=3))

	val respReg = RegInit(OcpResp.NULL) //Ocp data response
	val dataReg = RegInit(UInt(0, width=32))

	respReg := OcpResp.NULL //Default assignment response to Null
	dataReg := 0.U //Default assign to all zeros

	//Ocp and i2c connection state machine
	switch(ocpState) {
		is(ocpIdle) {
			when(!ocpAvailable) { //Idle and new command received
				ocpState := ocpLoad
			}
		}

		is(ocpLoad) { //Load new dataWr and ocpAddr into the i2c system
			ena := true.B //Set i2c enable high
			when(busy && ocp.addrRW(0) ) { //Performing a read operation
				ocpState := ocpReadWaiter
			} .elsewhen (busy && !ocp.addrRW(0) ) { //when busy goes high, i2c has latched in values
				ocpState := ocpListen
				ocpAvailable := true.B //Can now load new values via OCP
			}
		}

		is(ocpListen) {
			ena := false.B //Disable i2c per default
			when(!ocpAvailable) { //More dataWr is available on OCP
				ocpState := ocpMore
			} .elsewhen(!busy) { //Transaction is finished
				ocpState := ocpIdle
			}
		}

		is(ocpMore) { //Waiting state before additional dataWr is processed
			ena := true.B //Enable high, signal i2c that more is coming
			when(!busy) { //Finished processing current transaction
				ocpState := ocpLoad
			}
			//Need to write some code to exit this state if we get stuck here
		}

		is(ocpReadWaiter) {
			when(readCount === (bytesToRead-1.U) && busy && !RegNext(busy)) { //We're about to read the final byte, listen for new commands
				ocpState := ocpListen
				ocpAvailable := true.B
			}
		}
	}

	/*
	========== ADDRESS HANDLING OF C-COMMANDS =======
	*/
	when(ocpCmd) {
		switch(ocpM.Addr(5,2)) {
			//Load new command
			is(Bits("b0000")) {
				when(ocpAvailable) {
					ocp.addrRW := ocpM.Data(15,8) //Address of the slave device + rw bit
					ocp.dataWr := ocpM.Data(7,0) //Data to write to slave
					bytesToRead := ocpM.Data(18,16); //If a read command was sent, this is how many bytes to read before pulling enable low
					ocpAvailable := false.B
				}
			}
			//Read ocpAvailable flag
			is(Bits("b0001")) {
				dataReg := ocpAvailable
				// dataReg := Cat(UInt(0, width=31), ocpAvailable)
			}
			//Retrieve read data and reset readVals
			is(Bits("b0010")){
				dataReg := Cat(readVals(3),readVals(2),readVals(1),readVals(0))
				readCount := 0.U
				readVals(0) := 0.U
				readVals(1) := 0.U
				readVals(2) := 0.U
				readVals(3) := 0.U
				readFlag := false.B
			}
			//Retrieve readFlag from system
			is(Bits("b0100")){
				dataReg := readFlag 
			}
		}
	}

	//Always respond DVA right after a command has been received
	when(ocpCmd) {
		respReg := OcpResp.DVA
	}
	ocpS.Resp := respReg
	ocpS.Data := dataReg
}

/**
Bundle that allows for easier access and grouping of dataWr, dataRd, address
*/
class OcpI2CVals extends Bundle {
	val dataWr = UInt(width=8) //Data read from slave device
	val dataRd = UInt(width=8) //Data writing to slave device. Not used in OCP instance of this bundle
	val addrRW = UInt(width=8) //Concatenated version of address and rw.
}