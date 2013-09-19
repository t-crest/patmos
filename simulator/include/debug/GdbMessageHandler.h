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
//  This handles the different GDB RSP messages. It implements the server side
//  of the high level GDB RSP protocol.
//

#ifndef PATMOS_GDB_MESSAGE_HANDLER_H
#define PATMOS_GDB_MESSAGE_HANDLER_H

#include <string>
#include <boost/shared_ptr.hpp>

namespace patmos
{
  class GdbPacketHandler;
  class GdbMessage;
  typedef boost::shared_ptr<GdbMessage> GdbMessagePtr;
  class GdbResponseMessage;

  class GdbMessageHandler
  {
  public:
    GdbMessageHandler(const GdbPacketHandler &packetHandler);
    GdbMessagePtr ReadGdbMessage() const;
    void SendGdbMessage(const GdbResponseMessage &message) const;
  
  private:
    const GdbPacketHandler &m_packetHandler;
  };

}
#endif // PATMOS_GDB_MESSAGE_HANDLER_H
