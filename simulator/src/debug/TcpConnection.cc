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
//  Implementation of the Tcp Connection interface.
//

#include "debug/TcpConnection.h"

#include <sstream>

namespace patmos
{

  //////////////////////////////////////////////////////////////////
  // Exceptions
  //////////////////////////////////////////////////////////////////

  TcpConnectionCreationException::TcpConnectionCreationException(
      int port)
    : GdbException("")
  {
    std::stringstream ss;
    ss << "Could not initialize a TCP/IP listener for port "
       << port;
    m_whatMessage = ss.str();
  }

  //////////////////////////////////////////////////////////////////
  // TcpConnection implementation
  //////////////////////////////////////////////////////////////////
  
  TcpConnection::TcpConnection(int port)
    : m_io_service(),
      m_endpoint(ip::tcp::v4(), port),
      m_acceptor(m_io_service, m_endpoint)
  {
    m_acceptor.accept(*m_stream.rdbuf());
  }

  void TcpConnection::PutChar(char c) const
  {
    m_stream << c;
  }

  char TcpConnection::GetChar() const
  {
    char c;
    m_stream >> c;
    return c;
  }

  void TcpConnection::Write(const std::string &str) const
  {
    m_stream << str;
  }

}
