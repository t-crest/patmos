/*
   Copyright 2012 Technical University of Denmark, DTU Compute.
   All rights reserved.

   This file is part of the Patmos simulator.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

      1. Redistributions of source code must retain the above copyright notice,
         this list of conditions and the following disclaimer.

      2. Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
   NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   The views and conclusions contained in the software and documentation are
   those of the authors and should not be interpreted as representing official
   policies, either expressed or implied, of the copyright holder.
 */

//
// basic UART interface for I/O.
//

#ifndef PATMOS_UART_H
#define PATMOS_UART_H

#include <istream>
#include <ostream>
#include <cstdio>

#include "memory-map.h"
#include "exception.h"

namespace patmos
{
  /// A simple UART implementation allowing memory-mapped I/O.
  class uart_t : public mapped_device_t
  {
  private:
    /// Address at which the UART's status can be read/written.
    const uword_t Status_address;

    /// Address at which data can be read from/written to the UART.
    const uword_t Data_address;

    /// Stream providing the data read from the UART.
    std::istream &In_stream;

    /// Flag indicating whether the input stream is a tty.
    /// When true, this disables the signaling of EOF when the stream's buffer
    /// becomes empty.
    bool IsTTY;

    /// Stream to store data that is written to the UART.
    std::ostream &Out_stream;

    /// bit position of the parity-error bit (PAE).
    static const uword_t PAE = 2;

    /// bit position of the data-available bit (DAV).
    static const uword_t DAV = 1;

    /// bit position of the transmit-ready bit (TRE).
    static const uword_t TRE = 0;

  protected:
    /// A simulated access to the UART's status register.
    /// @param value A pointer to a destination to store the value read from
    /// the UART.
    /// @return True when the data is available from the UART.
    virtual bool read_status(byte_t *value)
    {
      // always accept data for transmission (TRE = 1), but data is only
      // available when there is something in the stream (DAV=0 or 1).
      // when the input stream reaches the end-of-file (EOF), signal a parity
      // error, i.e., PAE = 1.
      // TODO EOF does not work in the hardware. Instead, use escape characters
      //      or something to signal EOF over UART.
      *value = (1 << TRE);
      if (In_stream.rdbuf()->in_avail())
        *value |= (1 << DAV);
      else if (In_stream.eof() || !IsTTY)
        *value |= (1 << PAE);

      return true;
    }

    /// A simulated access to the UART's status register.
    /// @param value A pointer to a destination to read the value to write.
    /// @return True when the data is written to the UART.
    virtual bool write_control(byte_t *value)
    {
      return true;
    }

    /// A simulated access to the UART's data register.
    /// @param value A pointer to a destination to store the value read from
    /// the UART.
    /// @return True when the data is available from the UART.
    virtual bool read_data(byte_t *value)
    {
      In_stream.read(reinterpret_cast<char*>(value), sizeof(byte_t));
      std::streamsize num = In_stream.gcount();
      if (num == 0) return false;

      assert(num == sizeof(byte_t));
      return true;
    }

    /// A simulated peek access to the UART's data register
    /// @param value A pointer to a destination to store the value read from
    /// the UART.
    virtual bool peek_data(byte_t *value)
    {
      *value = (byte_t)In_stream.peek();
      return true;
    }

    /// A simulated access to a UART's data register.
    /// @param value The value to be written to the UART.
    /// @return True when the data is written finally to the UART, false
    /// otherwise.
    virtual bool write_data(byte_t *value)
    {
      Out_stream.write(reinterpret_cast<char*>(value), sizeof(byte_t));
      Out_stream.flush();
      return true;
    }

  public:
    /// Construct a new memory-mapped UART.
    /// @param base_address The address of the UART device
    /// @param in_stream Stream providing data read from the UART.
    /// @param istty Flag indicating whether the input stream is a TTY.
    /// @param out_stream Stream storing data written to the UART.
    uart_t(uword_t base_address,
           std::istream &in_stream, bool istty, std::ostream &out_stream) :
        mapped_device_t(base_address, 8),
        Status_address(base_address+0x00),
        Data_address(base_address+0x04),
        In_stream(in_stream), IsTTY(istty),
        Out_stream(out_stream)
    {
      // Ensure that we can check the rd buffer of the streams
      assert(In_stream.rdbuf() && Out_stream.rdbuf() &&
             "UART expects streams associated with a buffer.");
    }

    /// A simulated access to a read port.
    /// @param address The memory address to read from.
    /// @param value A pointer to a destination to store the value read from
    /// the memory.
    /// @param size The number of bytes to read.
    /// @return True when the data is available from the read port.
    virtual bool read(simulator_t &s, uword_t address, byte_t *value, uword_t size)
    {
      if (address == Status_address && size == 4)
        return read_status(value+3);
      else if (address == Data_address && size == 4)
        return read_data(value+3);
      else
        simulation_exception_t::unmapped(address);
      return true;
    }

    /// A simulated access to a read port. Does not update the device state or
    /// simulate timing, just reads the value.
    /// @param address The memory address to read from.
    /// @param value A pointer to a destination to store the value read from
    /// the memory.
    /// @param size The number of bytes to read.
    virtual void peek(simulator_t &s, uword_t address, byte_t *value, uword_t size) {
      if (address == Status_address && size == 4)
        read_status(value+3);
      else if (address == Data_address && size == 4)
        peek_data(value+3);
      else
        simulation_exception_t::unmapped(address);
    }

    /// A simulated access to a write port.
    /// @param address The memory address to write to.
    /// @param value The value to be written to the memory.
    /// @param size The number of bytes to write.
    /// @return True when the data is written finally to the memory, false
    /// otherwise.
    virtual bool write(simulator_t &s, uword_t address, byte_t *value, uword_t size)
    {
      if (address == Status_address && size == 4)
        return write_control(value+3);
      else if (address == Data_address && size == 4)
        return write_data(value+3);
      else
        simulation_exception_t::unmapped(address);
      return true;
    }
  };
}

#endif // PATMOS_UART_H

