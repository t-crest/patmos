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
//  gdb server implementation, used for gdb debugging.
//

#ifndef PATMOS_GDB_SERVER_H
#define PATMOS_GDB_SERVER_H

#include <string>
#include <boost/scoped_ptr.hpp>

#include "debug/DebugClient.h"
#include "debug/GdbPacketHandler.h"
#include "debug/GdbMessageHandler.h"

namespace patmos
{

  class GdbConnection;

  typedef boost::scoped_ptr<GdbPacketHandler> GdbPacketHandlerPtr;
  typedef boost::scoped_ptr<GdbMessageHandler> GdbMessageHandlerPtr;

  class GdbConnectionFailedException : public std::exception
  {
  public:
    GdbConnectionFailedException();
    ~GdbConnectionFailedException() throw();
    virtual const char* what() const throw();
  
  private:
    std::string m_whatMessage;
  };

  class GdbServer : public DebugClient
  {
  public:
    GdbServer(DebugInterface &debugInterface,
        const GdbConnection &connection);

    /*
     * starts to listen on the connection given at construction time.
     * this will block until the initial handshake with the gdb client
     * program is done. Depending on gdb client commands, the program
     * will then continue or not
     */
    void Start();

    // Implement DebugClient
    virtual void BreakpointHit(const Breakpoint &bp);
  
  private:
    DebugInterface &m_debugInterface;
    const GdbConnection &m_connection;
    GdbPacketHandlerPtr m_packetHandler;
    GdbMessageHandlerPtr m_messageHandler;
  };

}
#endif // PATMOS_GDB_SERVER_H
