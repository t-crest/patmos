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
// This is a tiny assembler embedded into C++ using operator overloading that
// allows to write small test programs.
// The header merely exports some simple test functions.
//

#ifndef PATMOS_ASSEMBLER_H
#define PATMOS_ASSEMBLER_H

namespace patmos
{
  /// A simple test program.
  void test1(bool debug = false);

  /// Another simple test program.
  void test2(bool debug = false);

  /// Yet another simple test program.
  void test3(bool debug = false);

  /// Yet another simple test program.
  void test4(bool debug = false);

  /// Yet another simple test program.
  void test5(bool debug = false);

  /// Yet another simple test program.
  void test6(bool debug = false);

  /// Yet another simple test program.
  void test7(bool debug = false);
}

#endif // PATMOS_ASSEMBLER_H


