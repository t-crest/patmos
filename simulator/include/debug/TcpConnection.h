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

#ifndef PATMOS_TCP_CONNECTION_H
#define PATMOS_TCP_CONNECTION_H

#include "debug/GdbException.h"
#include "debug/GdbConnection.h"
#include <boost/asio.hpp>

using namespace boost::asio;

namespace patmos
{

  class TcpConnectionCreationException : public GdbException
  {
  public:
    TcpConnectionCreationException(int port);
  };

  /*
   * Implementation of a gdb connection using TCP/IP sockets
   */
  class TcpConnection : public GdbConnection
  {
  public:
    TcpConnection(int port);

    virtual void PutChar(char c) const;
    virtual char GetChar() const;
    virtual void Write(const std::string &str) const;

  private:
    io_service m_io_service;
    ip::tcp::endpoint m_endpoint;
    ip::tcp::acceptor m_acceptor;
    mutable ip::tcp::iostream m_stream;
  };

}
#endif // PATMOS_TCP_CONNECTION_H
