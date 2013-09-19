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
//  This represents the GDB RSP packet layer. It includes a representation of
//  gdb rsp packets and methods to send and retrieve packets via a
//  GdbConnection
//

#include "debug/GdbPacket.h"

#include <sstream>
#include <iomanip>

namespace
{

  // helper functions
  int CalcChecksum(std::string content)
  {
    int checksum = 0;
    for (int i=0; i<content.length(); ++i)
    {
      checksum += content[i];
    }
    checksum = checksum % 256;
    return checksum;
  }

  int ParseChecksum(std::string s)
  {
    unsigned int checksum;
    std::stringstream ss;
    ss << std::hex << s;
    ss >> checksum;
    return checksum;
  }

}

namespace patmos
{

  //////////////////////////////////////////////////////////////////
  // GdbPacket implementation
  //////////////////////////////////////////////////////////////////
  
  GdbPacket::GdbPacket()
    : m_content(""),
      m_checksum(0)
  {
  }

  GdbPacket::GdbPacket(std::string content, int checksum)
    : m_content(content),
      m_checksum(checksum)
  {
  }

  std::string GdbPacket::GetContent() const
  {
    return m_content;
  }

  int GdbPacket::GetChecksum() const
  {
    return m_checksum;
  }

  std::string GdbPacket::GetPacketString() const
  {
    std::stringstream ss;
    ss << startSeq << m_content << checkSeq << 
      std::hex << std::setw(2) << std::setfill('0') << m_checksum;
    return ss.str();
  }

  bool GdbPacket::IsValid() const
  {
    return CalcChecksum(m_content) == m_checksum;
  }

  GdbPacket CreateGdbPacket(const std::string &content)
  {
    return GdbPacket(content, CalcChecksum(content));
  }

  GdbPacket ParseGdbPacket(const std::string &gdbPacketString)
  {
    const int packetLength = gdbPacketString.length();

    // check if packet starts with "startSeq"
    const int startSeqPos = gdbPacketString.find(startSeq);
    if (startSeqPos == std::string::npos)
      return GdbPacket();
    const int contentStart = startSeqPos + startSeq.length();

    // check if packet contains "checkSeq"
    const int checkSeqPos = gdbPacketString.find(checkSeq, contentStart);
    if (checkSeqPos == std::string::npos)
      return GdbPacket();
    const int contentEnd = checkSeqPos;

    // check if there is any content
    const int contentLength = contentEnd - contentStart;
    if (contentLength <= 0)
      return GdbPacket();

    // check if the checksum is there
    const int checksumStart = checkSeqPos + checkSeq.length();
    if (checksumStart + checksumLength > packetLength)
      return GdbPacket();

    std::string content = 
      gdbPacketString.substr(contentStart, contentLength);
    int checksum = 
      ParseChecksum(gdbPacketString.substr(checksumStart, checksumLength));
    return GdbPacket(content, checksum);
  }

}
