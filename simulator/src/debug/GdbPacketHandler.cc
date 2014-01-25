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

#include "debug/GdbPacketHandler.h"
#include "debug/GdbConnection.h"

#include <sstream>
#include <algorithm>
#include <iostream>
#include <iomanip>

namespace
{
  const std::string ackSeq = "+";     // acknowledgement sequence
  const std::string failSeq = "-";    // failure sequence / request retransmission
  const int maxRetransmissions = 10;  // maximum number of retransmissions

  // helper functions
  
  using namespace patmos;

  bool StrEndsWith(std::string s, std::string e)
  {
    const int sLen = s.length();
    const int eLen = e.length();

    if (sLen < eLen)
      return false;
    
    return (s.substr(sLen - eLen, eLen) == e);
  }

  bool GetAck(const patmos::GdbConnection &con)
  {
    std::stringstream ss;
    // we want to either read ackSeq OR failSeq
    // it is safe to read the shorter sequence
    const int minChars = std::min(ackSeq.length(), failSeq.length());
    int i = 0;
    for (; i < minChars; ++i)
      ss << con.GetChar();

    if (ss.str() == ackSeq)
      return true;
    else if (ss.str() == failSeq)
      return false;
    
    // continue for the longer sequence
    const int maxChars = std::max(ackSeq.length(), failSeq.length());
    for (; i < maxChars; ++i)
      ss << con.GetChar();

    if (ss.str() == ackSeq)
      return true;
    else
      return false;
  }

  void DebugWrite(const GdbPacket &packet)
  {
    std::cerr << "GdbPacketHandler::WriteGdbPacket > " << 
      packet.GetPacketString()<< std::endl;
  }

  void DebugRead(const GdbPacket &packet, bool isValid)
  {
    int checksum = packet.GetChecksum();
    // if invalid, calc checksum of the given packet
    if (!isValid)
    {
      GdbPacket checksumHelper = CreateGdbPacket(packet.GetContent());
      checksum = checksumHelper.GetChecksum();
    }
    std::cerr << "GdbPacketHandler::ReadGdbPacket  < " << 
      packet.GetPacketString() << "(" << packet.GetContent() <<
      ", checksum: " << std::hex << std::setw(2) << checksum << 
      ", is valid: " << isValid << ")" << std::endl;
  }
}

namespace patmos
{

  //////////////////////////////////////////////////////////////////
  // Exceptions
  //////////////////////////////////////////////////////////////////

  GdbMaxRetransmissionsException::GdbMaxRetransmissionsException()
    : m_whatMessage("Error: GdbServer: Transmitting a packet exceeded the maximum number of retransmissions.")
  {
  }
  GdbMaxRetransmissionsException::~GdbMaxRetransmissionsException() throw()
  {
  }
  const char* GdbMaxRetransmissionsException::what() const throw()
  {
    return m_whatMessage.c_str();
  }

  //////////////////////////////////////////////////////////////////
  // GdbPacketHandler implementation
  //////////////////////////////////////////////////////////////////
  
  GdbPacketHandler::GdbPacketHandler(const GdbConnection &con)
    : m_con(con), m_useAck(true), m_debug(false)
  {
  }

  void GdbPacketHandler::WriteGdbPacket(const GdbPacket &packet) const
  {
    int r = 0; // retransmission count
    do
    {
      if (r++ > maxRetransmissions)
        throw GdbMaxRetransmissionsException();

      if (m_debug)
        DebugWrite(packet);

      m_con.Write(packet.GetPacketString());
    } while (IsUsingAck() && !GetAck(m_con));
  }

  GdbPacket GdbPacketHandler::ReadGdbPacket() const
  {
    bool isValid = false;
    GdbPacket packet;

    do
    {
      std::stringstream ss;
      
      // read from connection as long as there is no check sequence
      do
      {
        ss << m_con.GetChar();
      } while (!StrEndsWith(ss.str(),checkSeq));

      // now read the checksum
      for (int i = 0; i < checksumLength; ++i)
        ss << m_con.GetChar();

      packet = ParseGdbPacket(ss.str());
      isValid = packet.IsValid();
      
      if (m_debug)
        DebugRead(packet, isValid);

      if (IsUsingAck() && !isValid)
        m_con.Write(failSeq);

    } while (IsUsingAck() && !isValid);

    if (IsUsingAck())
      m_con.Write(ackSeq);
    
    return packet;
  }

  void GdbPacketHandler::SetUseAck(bool useAck)
  {
    m_useAck = useAck;
  }

  bool GdbPacketHandler::IsUsingAck() const
  {
    return m_useAck;
  }

  void GdbPacketHandler::SetDebug(bool debug)
  {
    m_debug = debug;
  }

}
