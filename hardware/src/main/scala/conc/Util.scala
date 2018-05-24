/*
 * Utility code
 *
 * Author: Davide Laezza - Roberts Fanning - Wenhao Li
 */

package conc

import Chisel._

import ocp._

object Util {

    // This method concatenates an array of Bits
    def catAll[T <: Bits](items :Array[T]) = items.reduce(_ ## _)

    // This method OR-s an array of Bits
    def orAll[T <: Bits](items :Array[T]) = items.reduce(_ | _)

    // This method OR-s all the signal of an array of OCP master signals
    def orAllOcpMaster(ports :Array[OcpCoreMasterSignals]) = {
        val first = ports.head
        val out = new OcpCoreMasterSignals(first.Addr.getWidth,
        first.Data.getWidth)

        out.Addr := orAll(ports.map(_.Addr))
        out.ByteEn := orAll(ports.map(_.ByteEn))
        out.Cmd := orAll(ports.map(_.Cmd))
        out.Data := orAll(ports.map(_.Data))

        out
    }
}
