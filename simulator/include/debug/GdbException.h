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
//  Generic gdb exception
//

#ifndef PATMOS_GDB_EXCEPTION_H
#define PATMOS_GDB_EXCEPTION_H

#include <exception>
#include <string>

namespace patmos
{
  /// Generic exception used to throw gdb server exceptions
  class GdbException : public std::exception
  {
  public:
    GdbException(const std::string &whatMessage);
    ~GdbException() throw();
    virtual const char* what() const throw();
  
  protected:
    std::string m_whatMessage;
  };

}
#endif // PATMOS_GDB_EXCEPTION_H
