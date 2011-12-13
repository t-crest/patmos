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
// Decode instructions given by a pointer.
// 
// TODO: Implement an actual decoder that supports binary encoded instruction.
//

#ifndef PATMOS_DECODER_H
#define PATMOS_DECODER_H

namespace patmos
{
  // forward declaration
  class simulator_t;
  class instruction_data_t;

  /// Interface to decoder Patmos instructions.
  class decoder_t
  {
  private:
    /// The parent Patmos simulator of this decoder.
    /// This reference is used to access and update the global processor state.
    simulator_t &S;

    unsigned int I;
  public:
    /// Construct a new instance of a Patmos decoder.
    /// @param s The parent Patmos simulator of the decoder.
    decoder_t(simulator_t &s) : S(s), I(0)
    {
    }
    
    void decode(instruction_data_t *result);
  };
}

#endif // PATMOS_DECODER_H

