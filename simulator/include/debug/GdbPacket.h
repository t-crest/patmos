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
//  This represents the GDB RSP packet layout. It includes functions to
//  create and parse packets, checksum creation and checking.
//

#ifndef PATMOS_GDB_PACKET_H
#define PATMOS_GDB_PACKET_H

#include <string>

namespace patmos
{

  /// [packet format]
  /// [$.........#hh]
  /// [$<content>#<checksum>]
  const std::string startSeq = "$";   ///< sequence that marks the start of the
                                      ///< packet
  const std::string checkSeq = "#";   ///< sequence that marks the start of the 
                                      ///< checksum
  const int checksumLength = 2;       ///< 2 bits of checksum
  
  /// Represents a single gdb packet, including its content and checksum.
  class GdbPacket
  {
  public:
    /// Creates an empty packet
    GdbPacket();

    /// Creates a packet with the given content and the given checksum
    /// @param content packet content/body/data
    /// @param checksum checksum of the packet
    GdbPacket(std::string content, int checksum);

    /// @returns the content of the packet (body)
    std::string GetContent() const;

    /// @returns the checksum of the packet
    int GetChecksum() const;

    /// @returns the full packet string, including checksum, as it would be sent
    /// to the client
    std::string GetPacketString() const;

    /// returns true if the package is valid, i.e. the checksum is fitting
    ///  the content, otherwise false.
    bool IsValid() const;

  private:
    std::string m_content;
    int m_checksum;
  };

  /// Creates a new gdb packet with the given content (package body)
  ///  and returns it.
  /// @param content package body/data
  /// @returns a gdb packet containing the content
  GdbPacket CreateGdbPacket(const std::string &content);

  /// Parses the given gdb packet string and returns a gdb packet.
  /// @param gdbPacketString a full gdb packet string, including checksum, etc...
  /// @returns a gdb packet
  GdbPacket ParseGdbPacket(const std::string &gdbPacketString);

}
#endif // PATMOS_GDB_PACKET_H
