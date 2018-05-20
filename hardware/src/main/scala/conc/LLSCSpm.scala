/*
 * A scratchpad memory supporting LL/SC
 *
 * Author: Davide Laezza - Roberts Fanning - Wenhao Li
 */

package conc

import Chisel._

import ocp._
import patmos._
import patmos.Constants._

// Dirtybits module creates a dirty bit for each core at every addressable memory
// location. These dirty bits are a fundemental part of our Load-Link
// Store-Conditional protocol as they determine a cores permission to write
// to a specific memory location.
class DirtyBits (
    nrCores :Int,
    size :Int
) extends Module {
    import DirtyBits._

    /*
      addr     : the address of the memory location whose dirty bits are operated
      core     : the core id whose dirty bit is dealt with
      wrEnable : write enable to allowing for the writing of the dirtybits
      data     : data is the input status of a dirty bit for the core
      coreBit  : coreBit is the output status of the dirty bit for the core

    */
    val io = new Bundle() {
        val addr = Bits(INPUT, log2Up(size))
        val core = Bits(INPUT, log2Up(nrCores))
        val wrEnable = Bool(INPUT)
        val data = Bits(INPUT, 1)
        val coreBit = Bits(OUTPUT, 1)
    }

    // Dirty bits
    // initialize hardware memory for number of cores dirty bits with desired size
    val dirtyBits = Mem(makeInitLocation(nrCores), size)

    val location = dirtyBits(io.addr)
    val coreBit = (location.toUInt >> io.core)(0)

    // Read
    io.coreBit := coreBit

    // Write: when the dirtybit write enable signal is high one of the
    //        following can occur: either a dirty bit is written to pristine
    //        on a read command, or all dirty bits are written to dirty on a
    //        write command.
    when (io.wrEnable) {
        switch (io.data) {

            // Making the bit for the core pristine
            // the mask is used for shift the pristine status bits to the
            // correseponding bit posintion e.g
            // if io.core == b100 (4) then
            //    mask = shift left 4 position with 0
            is (PRISTINE) {
                val mask = ~(UInt(1) << io.core)
                location := location & mask
            }

            // Making the bits dirty for every core
            // with a write command, all cores dirtybits are set as dirty, since
            // after writing to the memory at the location has been changed, so
            // any cores who have read (their dirty bit is pristine) will have
            // their dirty bit set to dirty informing them that the address
            // has been written to.
            is (DIRTY) {
                location := Fill(nrCores, DIRTY)
            }
        }
    }
}

/*
* The scala thing static field and functions can be seen in this object.
*/
//The DirtyBits values are specified with 0 indicating Pristine and 1 Dirty
object DirtyBits {
    val PRISTINE = Bits(0)
    val DIRTY = Bits(1)

    //DirtyBits are applied to each memory address for each core.
    def apply(nrCores :Int, size: Int) = {
        Module(new DirtyBits(nrCores, size))
    }

    //All DirtyBits for each core and address are initalised as Pristine, since
    //no cores have initally written to memory.
    def makeInitLocation(nrCores :Int) = {
        val location = Fill(nrCores, PRISTINE)
        location.setWidth(nrCores)
        location
    }
}

/*
* A special scratchpad memory is created to perform the hardware support load
* linked store conditianl protocol. This implementation requires no new
* instructions for the instruction set since all normal read and write commands
* to the LLSCSpm will be either load linked reads or store conditional writes.
*/

class LLSCSpm(
    //The granularity specifies the amount of memory assigned to a set of
    //DirtyBits. For instances a set DirtyBits for each core could cover a byte,
    //a word, a double word etc.
    val granularity :Int,
    nrCores: Int,           //number of cores
    size: Int,               //size of each core memory
    //whether to send the store conditional results on data as response for the
    //write ot=r to store it in a register for later read
    resultOnData :Boolean = false
) extends Module {
    import DirtyBits._
    import LLSCSpm._

    // memory coherence error checks.
    if (!isPow2 (granularity)) {
        sys.error (s"LLSCSpm: granularity must be a power of 2, but $granularity was provided.")
    }

    //Sub-word write has not been implemented in DirtyBits
    if (granularity < 4) {
        sys.error (s"LLSCSpm: granularity must be at least 4, but $granularity was provided.")
    }

    if (!isPow2 (size)) {
        sys.error (s"LLSCSpm: size must be a power of 2, but $size was provided.")
    }

    if (granularity > size) {
        sys.error (s"LLSCSpm: granularity has to be less than size, but granularity $granularity and size $size were provided.")
    }

    /*
    Open core protocol io bundle
    */

    val io = new Bundle() {
        val slave = new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH)
        val core = UInt(INPUT, log2Up(nrCores))
    }

    // The address of the result register, if not sending
    // the result when writing
    val resultRegAddr = UInt(size)

    //binary value of the SPM address
    val addrBits = log2Up(size / BYTES_PER_WORD)

    // generate byte memories
    val mem = new Array[MemBlockIO](BYTES_PER_WORD)
    for (i <- 0 until BYTES_PER_WORD) {
        mem(i) = MemBlock(size / BYTES_PER_WORD, BYTE_WIDTH, bypass = false).io
    }
    //A DirtyBits module is created with the number of cores and the number of
    //memory addresses being watched by a set of dirtybits.
    val dirtyBits = DirtyBits(nrCores, size / granularity)

    //The inputs of the DirtyBits module are initialised.
    //Address is shifted by the granularity since a set of DirtyBits covers
    //multiple memory locations.
    dirtyBits.io.addr := io.slave.M.Addr >> log2Up(granularity)
    dirtyBits.io.core := io.core // the cores are set.

    // Default wires set not to write
    dirtyBits.io.wrEnable := Bool(false)
    dirtyBits.io.data := Bits(0)

    /* check of dirtry bits status:
    if the dirty bit status of memory is prestine then return true
    else return false
    */

    val isPristine = dirtyBits.io.coreBit === PRISTINE

    /*
        Should only write if the write command is received and the dirty bit
        of the current core is pristine.
    */
    val shouldWrite = io.slave.M.Cmd === OcpCmd.WR && isPristine

    // This register contains a bit for every core, with the result of
    // the last store conditional. In fact, it is just an unused wire
    // if the results are sent upon write
    val resultsReg = if (resultOnData)
            Bits(0)
        else
            RegInit(UInt(0, width = nrCores))

    /* To connect with the Ocp slave */
    val cmdReg = RegNext(io.slave.M.Cmd)

    // This register holds the resut of a store conditional. If they are to be
    // returned on OCP data, the register is assigned in write command,
    // otheriwse it samples the result for the current core from resultReg.
    val dataReg = if (resultOnData)
            RegInit(io.slave.M.Data)
        else
            RegNext((resultsReg >> io.core)(0))

    // Whether the read command is to read the result of a store conditional,
    // instead of a memory cell
    val readResult = io.slave.M.Addr === resultRegAddr

    // Whether at the previous clock sycle the result address was read. An
    // unused wire if the result is sent upon write
    val hasReadResult = if (resultOnData)
            Bool(false)
        else
            RegNext(readResult)

    // store conditional
    /*
        Setting the mask to 0 also when the write is not alowed by
        LL/SC protocol.
    */
    val stmsk = Mux(shouldWrite, io.slave.M.ByteEn, Bits(0))
    for (i <- 0 until BYTES_PER_WORD) {
        mem(i) <= (stmsk(i), io.slave.M.Addr(addrBits + 1, 2),
            io.slave.M.Data(BYTE_WIDTH*(i+1)-1, BYTE_WIDTH*i))
    }

    val rdData = mem.map(_(io.slave.M.Addr(addrBits + 1, 2))).reduceLeft((x,y) => y ## x)

    /* slave response
       mutilplexed by Ocp write || read
       if the command is write || read holds then DVA, else null
    */
    io.slave.S.Resp := Mux(cmdReg === OcpCmd.WR || cmdReg === OcpCmd.RD,
        OcpResp.DVA, OcpResp.NULL)

    if (resultOnData) {
        // When sending the results on OCP data wires, the data should come
        // from the memory on read commands and from the result register
        // on writes
        io.slave.S.Data := Mux(cmdReg === OcpCmd.RD, rdData, dataReg)
    }
    else {
        // When sending the results on OCP data wires, the data should come
        // from the memory on read commands and from the result register
        // on writes
        io.slave.S.Data := Mux(hasReadResult, dataReg, rdData)
    }

    switch (io.slave.M.Cmd) {
      //For a read command write enable is set to true and then the dirty bit
      //for the reading core is set to pristine at the address of the read
      //command. No conditions need to be checked since a read command is
      //always possible
        is (OcpCmd.RD) {

            // When sending the results on OCP data wires, every read command
            // is to the memory, so the dirty bits need to be updated
            if (resultOnData) {
                dirtyBits.io.wrEnable := Bool(true)
                dirtyBits.io.data := PRISTINE
            }

            // When the result is stored in a register for later reading,
            // the dirty bits only need to be updated when not reading the
            // result register
            else {
                when (~readResult) {
                    dirtyBits.io.wrEnable := Bool(true)
                    dirtyBits.io.data := PRISTINE
                }
            }
        }
        //When a core attempts a write command we must first check if that core's
        //dirty bit is still pristine meaning that no other cores have written
        //to that address. If that condition is satisfied write enable is set to
        //true to allow for all cores dirtybits to be set to dirty using DIRTY.
        //The result is reported as either a success or a failure.
        is (OcpCmd.WR) {
            when (isPristine) {
                dirtyBits.io.wrEnable := Bool(true)
                dirtyBits.io.data := DIRTY

                // When the store conditional result is returned on OCP data
                // wires, writing to the result in the result register.
                // Otherwise, the corresponding bit in the result register is
                // set for later reading
                if (resultOnData) {
                    dataReg := Result.SUCCESS
                }
                else {
                    resultsReg := resultsReg & ~(UInt(1) << io.core)
                }
            }.otherwise {

                // When the store conditional result is returned on OCP data
                // wires, writing to the result in the result register.
                // Otherwise, the corresponding bit in the result register is
                // reset for later reading
                if (resultOnData) {
                    dataReg := Result.FAIL
                }
                else {
                    resultsReg := resultsReg | UInt(1) << io.core
                }
            }
        }
    }
}

object LLSCSpm {
    object Result {
        val SUCCESS = UInt(0)
        val FAIL = UInt(1)
    }

    def apply(granularity :Int, nrCores: Int, size: Int, resultOnData :Boolean = false) = {
        Module(new LLSCSpm(granularity, nrCores, size, resultOnData))
    }
}

class LLSCSpmTester(dut: LLSCSpm, resultOnData :Boolean = false) extends Tester(dut) {
    import LLSCSpmTester._

  println("LL/SC SPM Tester")

  def read(addr: Int) = {
    poke(dut.io.slave.M.Addr, addr)
    poke(dut.io.slave.M.Cmd, 2) // OcpCmd.RD
    step(1)
    poke(dut.io.slave.M.Cmd, 0) // OcpCmd.IDLE
    while (peek(dut.io.slave.S.Resp) != 1) {
      step(1)
    }
    peek(dut.io.slave.S.Data)
    dut.io.slave.S.Data
  }

  // This is the write for when the results have to be read from
  // the result register
  def writeNoResult(addr: Int, data: Int) = {
    poke(dut.io.slave.M.Addr, addr)
    poke(dut.io.slave.M.Data, data)
    poke(dut.io.slave.M.Cmd, 1) // OcpCmd.WR
    poke(dut.io.slave.M.ByteEn, 0x0f)
    step(1)
    poke(dut.io.slave.M.Cmd, 0) // OcpCmd.IDLE
    while (peek(dut.io.slave.S.Resp) != 1) {
      step(1)
    }
    read(SIZE)
  }

  // This is the write for when the results are returned on the data wires
  def writeResult(addr: Int, data: Int) = {
    poke(dut.io.slave.M.Addr, addr)
    poke(dut.io.slave.M.Data, data)
    poke(dut.io.slave.M.Cmd, 1) // OcpCmd.WR
    poke(dut.io.slave.M.ByteEn, 0x0f)
    step(1)
    poke(dut.io.slave.M.Cmd, 0) // OcpCmd.IDLE
    while (peek(dut.io.slave.S.Resp) != 1) {
      step(1)
    }
    peek(dut.io.slave.S.Data)
    dut.io.slave.S.Data
  }

  // Choosing the right write based on the parameter
  val write = if (resultOnData) writeResult _ else writeNoResult _

  poke(dut.io.core, 0)

  // Basic read-write test

  for (i <- 0.until(1024, GRANULARITY)) {
    expect(write(i, i * 0x100 + 0xa), 0)
  }
  step(1)
  for (i <- 0.until(1024, GRANULARITY)) {
    expect(read(i), i * 0x100 + 0xa)
  }

  // LL/SC tests

  poke(dut.io.core, 0)

  // Write to the same memory location without reading in between should fail
  expect(write(0, 0xFF), 0)
  expect(write(0, 0xF0), 1)

  // Failed writes should not affect memory
  expect(read(0), 0xFF)

  // Write to the same memory location with a read from the same core in
  expect(write(0, 0xFF), 0)
  expect(read(0), 0xFF)
  expect(write(0, 0xF0), 0)

  // Write to memory in the same granularity block should fail
  if (GRANULARITY != 1) {
      expect(write(GRANULARITY * 9, 0xFF), 0)
      expect(write(GRANULARITY * 9 + 1, 0xF0), 1)
  }

  // Write to memory in different granularity blocks should succeed
  expect(write(GRANULARITY * 5, 0xFF), 0)
  expect(write(GRANULARITY * 3, 0xF0), 0)

  // Write frrom a different core should invalidate subsequent writes
  // from other cores
  poke(dut.io.core, 0)
  read(GRANULARITY * 8)
  poke(dut.io.core, 1)
  read(GRANULARITY * 8)
  expect(write(GRANULARITY * 8, 0xF0), 0)
  poke(dut.io.core, 0)
  expect(write(GRANULARITY * 8, 0xFF), 1)

  // The first write after a read shoudl always succeed
  poke(dut.io.core, 0)
  read(GRANULARITY * 8)
  poke(dut.io.core, 1)
  read(GRANULARITY * 8)
  expect(write(GRANULARITY * 8, 0xF0), 0)
  poke(dut.io.core, 0)
  expect(read(GRANULARITY * 8), 0xF0)
  expect(write(GRANULARITY * 8, 0xFF), 0)

  // Unlike CAS, LL/SC doesn't siffer the ABA problem
  val aValue = 0x19
  val bValue = 0x84

  poke(dut.io.core, 0)
  read(GRANULARITY * 8)                     // So that next write doesn't fail
  expect(write(GRANULARITY * 8, aValue), 0) // a value is written by core 0
  expect(read(GRANULARITY * 8), aValue)     // a value is read

  poke(dut.io.core, 1)
  expect(read(GRANULARITY * 8), aValue)     // So that next write doesn't fail
  expect(write(GRANULARITY * 8, bValue), 0) // value b is written by core 1
  expect(read(GRANULARITY * 8), bValue)     // So that next write doesn't fail
  expect(write(GRANULARITY * 8, aValue), 0) // vaue a is written again by core 1
  poke(dut.io.core, 0)
  expect(write(GRANULARITY * 8, 0xFF), 1)   // Store Conditional fails
}

object LLSCSpmTester {
    val GRANULARITY = 32
    val SIZE = 1024

    def main(args: Array[String]): Unit = {

        // Write result has to be read from the result register
        chiselMainTest(Array("--genHarness", "--test", "--backend", "c",
                "--compile", "--vcd", "--targetDir", "generated"),
            () => LLSCSpm(GRANULARITY, 4, SIZE)) {
                c => new LLSCSpmTester(c)
        }

        // Write result is returned on data wires
        chiselMainTest(Array("--genHarness", "--test", "--backend", "c",
                "--compile", "--vcd", "--targetDir", "generated"),
            () => LLSCSpm(GRANULARITY, 4, SIZE, true)) {
                c => new LLSCSpmTester(c, true)
        }
    }
}
