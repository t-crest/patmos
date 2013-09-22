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
//  Test implementation of the gdb server.
//

#include <iostream>
#include "debug/GdbServer.h"
#include "debug/TcpConnection.h"
#include "debug/DebugInterface.h"

using namespace patmos;

namespace
{
  class TestInterface : public DebugInterface
  {
    virtual HostInfo GetHostInfo() const
    {
      HostInfo info;
      return info;
    }
    virtual void SetDebugClient(DebugClient &debugClient)
    {
    }
    virtual void AddBreakpoint(Breakpoint bp)
    {
    }
    virtual void RemoveBreakpoint(Breakpoint bp)
    {
    }
  };
}

int main(int argc, char **argv)
{
  const int listenPort = 1234;

  std::cerr << "using port " << listenPort << std::endl;

  std::cerr << "establishing connection...";
  TcpConnection con(listenPort);
  std::cerr << " done." << std::endl;

  TestInterface debugInterface;
  GdbServer server(debugInterface, con);

  std::cerr << "starting gdb server" << std::endl;
  server.Start();
}
