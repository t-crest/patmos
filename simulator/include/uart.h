//
//  This file is part of the Patmos Simulator.
//  The Patmos Simulator is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  The Patmos Simulator is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with the Patmos Simulator. If not, see <http://www.gnu.org/licenses/>.
//
//
// basic UART interface for I/O.
//

#ifndef PATMOS_UART_H
#define PATMOS_UART_H

#include <istream>
#include <ostream>
#include <cstdio>

namespace patmos
{
  /// Default address of the UART status register.
  static const uword_t UART_STATUS_ADDRESS = 0x80000000;

  /// Default address of the UART data register.
  static const uword_t UART_DATA_ADDRESS = 0x80000001;

  /// A simple UART implementation allowing memory-mapped I/O.
  class uart_t : public memory_t
  {
  private:
    /// Memory onto which the UART is mapped.
    memory_t &Memory;

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
      *value = (1 << TRE);
      if (In_stream.rdbuf()->in_avail())
        *value |= (1 << DAV);
      else if (In_stream.eof() || !IsTTY)
        *value |= (1 << PAE);

      return true;
    }

    /// A simulated access to the UART's data register.
    /// @param value A pointer to a destination to store the value read from
    /// the UART.
    /// @return True when the data is available from the UART.
    virtual bool read_data(byte_t *value)
    {
      In_stream.read(reinterpret_cast<char*>(value), sizeof(byte_t));
      assert(In_stream.gcount() == sizeof(byte_t));
      return true;
    }

    /// A simulated access to a UART's data register.
    /// @param value The value to be written to the UART.
    /// @return True when the data is written finally to the UART, false
    /// otherwise.
    virtual bool write_data(byte_t *value)
    {
      Out_stream.write(reinterpret_cast<char*>(value), sizeof(byte_t));
      return true;
    }

  public:
    /// Construct a new memory-mapped UART.
    /// @param memory The memory onto which the UART is memory mapped.
    /// @param status_address The address from/to which the UART's status can be
    /// read/written.
    /// @param data_address The address through which data can be exchanged with
    /// the UART.
    /// @param in_stream Stream providing data read from the UART.
    /// @param istty Flag indicating whether the input stream is a TTY.
    /// @param out_stream Stream storing data written to the UART.
    uart_t(memory_t &memory, uword_t status_address, uword_t data_address,
           std::istream &in_stream, bool istty, std::ostream &out_stream) :
        Memory(memory), Status_address(status_address),
        Data_address(data_address), In_stream(in_stream), IsTTY(istty),
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
    virtual bool read(uword_t address, byte_t *value, uword_t size)
    {
      if (address == Status_address && size == 1)
        return read_status(value);
      else if (address == Data_address && size == 1)
        return read_data(value);
      else
        return Memory.read(address, value, size);
    }

    /// A simulated access to a write port.
    /// @param address The memory address to write to.
    /// @param value The value to be written to the memory.
    /// @param size The number of bytes to write.
    /// @return True when the data is written finally to the memory, false
    /// otherwise.
    virtual bool write(uword_t address, byte_t *value, uword_t size)
    {
      if (address == Data_address && size == 1)
        return write_data(value);
      else
        return Memory.write(address, value, size);
    }

    /// Read some values from the memory -- DO NOT SIMULATE TIMING.
    /// @param address The memory address to read from.
    /// @param value A pointer to a destination to store the value read from
    /// the memory.
    /// @param size The number of bytes to read.
    virtual void read_peek(uword_t address, byte_t *value, uword_t size)
    {
      Memory.read_peek(address, value, size);
    }

    /// Write some values into the memory -- DO NOT SIMULATE TIMING, just write.
    /// @param address The memory address to write to.
    /// @param value The value to be written to the memory.
    /// @param size The number of bytes to write.
    virtual void write_peek(uword_t address, byte_t *value, uword_t size)
    {
      Memory.write_peek(address, value, size);
    }

    /// Check if the memory is busy handling some request.
    /// @return False in case the memory is currently handling some request,
    /// otherwise true.
    virtual bool is_ready()
    {
      return Memory.is_ready();
    }

    /// Notify the memory that a cycle has passed.
    virtual void tick()
    {
      Memory.tick();
    }

    /// Print the internal state of the memory to an output stream.
    /// @param os The output stream to print to.
    virtual void print(std::ostream &os) const
    {
      Memory.print(os);
    }
  };
}

#endif // PATMOS_UART_H

