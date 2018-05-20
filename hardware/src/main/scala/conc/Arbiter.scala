/*
 * A local OCP arbiter with delay.
 *
 * Authors: Davide Laezza - Roberts Fanning - Wenhao Li
 */

package conc

import Chisel._
import conc.Util._
import ocp._
import patmos.Constants._

/*
    This module has roughly the same feature as NodeSPM. Briefly, it performs
    time-slotted arbitration between an OCP master and an OCP slave.

    The additional features are: the output of the core ID together with the
    OCP signals, when the time slots comes; and the (unused) possiility of
    having a delay of more than one clock cycle before the slave response.
 */
class Arbiter(
    id: Int,
    nrCores: Int,
    delay :Int = 1
) extends Module {

    val NO_DELAY = delay == 0

    /*
      slave     : the OCp slave port, to be connected to the arbitrated master
      master    : the OCP master port, to be connected to te slave
      cores     : the core ID, output thether with the master signals
    */
    val io = new Bundle() {
        val slave = new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH)
        val master = new OcpCoreMasterPort(ADDR_WIDTH, DATA_WIDTH)
        val core = UInt(OUTPUT, log2Up(nrCores))
    }

    // Saving one bit in state if there si no delay
    val waitingCmd :: waitingTurn :: rest = Enum(UInt(), if (NO_DELAY) 2 else 3)
    val waitingDva = if (NO_DELAY) UInt() else rest.head

    val state = RegInit(waitingCmd)

    // TODO: how to reset with a harmless IDLE command?
    val masterReg = RegInit(io.slave.M)

    // Register for the DVA reply, NULL by default
    val dvaRepl = Reg(init = OcpResp.NULL, next = OcpResp.NULL)

    // Counter for the time slot
    val cnt = RegInit(UInt(0, width = log2Up(nrCores)))
    cnt := Mux(cnt === UInt(nrCores - 1), UInt(0), cnt + UInt(1))

    // True when the time-slot comes
    val enabled = cnt === UInt(id)

    // True after `delay` clock cycles from the eabled signal
    val dva = cnt === UInt((id + delay) % nrCores)

    // Data comes from the SPM, response from the FSM
    io.slave.S.Resp := dvaRepl
    io.slave.S.Data := io.master.S.Data

    // Default OCP master signals to zero to allow external OR-ing
    io.master.M.Addr := UInt(0)
    io.master.M.Data := UInt(0)
    io.master.M.Cmd := UInt(0)
    io.master.M.ByteEn := UInt(0)

    // Default core ID to zero to allow external OR-ing
    io.core := UInt(0)

    // Finite state machine
    switch (state) {
        is (waitingCmd) {
            // Sampling while waiting for command
            masterReg := io.slave.M

            // Waiting for turn if a command is received
            state := Mux(io.slave.M.Cmd === OcpCmd.IDLE,
                    waitingCmd, waitingTurn)
        }

        is (waitingTurn) {

            // Only on time-slot
            when (enabled) {

                // Sending commands only for this cycle
                masterReg.Cmd := OcpCmd.IDLE
                io.master.M := masterReg
                io.core := UInt(id)

                /*
                    At synthesis time, switching to the appropriate state
                    depending on the delay
                */
                if (NO_DELAY) {
                    dvaRepl := OcpResp.DVA
                    state := waitingCmd
                }
                else {
                    state := waitingDva
                }
            }
        }

        // this state does not exist without a delay
        if (!NO_DELAY) {
            is (waitingDva) {

                // Only when the slave data are actually valid
                when (dva) {

                    // Sending va to the master and back to listening for commands
                    dvaRepl := OcpResp.DVA
                    state := waitingCmd
                }
            }
        }
    }
}

object Arbiter {
    def apply(id: Int, nrCores :Int, delay :Int) = {
        Module(new Arbiter(id, nrCores, delay))
    }

    def apply(id :Int, nrCores :Int) = {
        Module(new Arbiter(id, nrCores))
    }
}
