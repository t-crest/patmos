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
//  This represents the GDB RSP packet layer. It includes methods to send and 
//  retrieve packets via a GdbConnection
//

#ifndef PATMOS_GDB_PACKET_HANDLER_H
#define PATMOS_GDB_PACKET_HANDLER_H

#include "debug/GdbPacket.h"

#include <string>

namespace patmos
{

  class GdbConnection;

  /// Provides functions to read and write GdbPackets over the connection given
  /// at construction time.
  class GdbPacketHandler
  {
  public:
    /// @param con gdb connection, must be open and ready for read/write
    GdbPacketHandler(const GdbConnection &con);

    /// Writes the given gdb packet out via the connection.
    /// @param packet the packet that will be sent
    void WriteGdbPacket(const GdbPacket &packet) const;

    /// @returns exactly one gdb packet read from the connection
    GdbPacket ReadGdbPacket() const;

    /// @param useAck set to false to disable ack messages (useful for reliable
    ///  connection such as TCP or pipe)
    void SetUseAck(bool useAck);

    /// @returns true if this packet handler is currently using ack messages
    ///  to ensure correct transmission of messages
    bool IsUsingAck() const;

    /// Enable/Disable internal debugging. Will print packet contents and 
    /// checksums to stderr.
    /// @param debug if true, internal debugging will be enabled.
    void SetDebug(bool debug);

  private:
    const GdbConnection &m_con;
    bool m_useAck;
    bool m_debug;
  };
}
#endif // PATMOS_GDB_PACKET_HANDLER_H
