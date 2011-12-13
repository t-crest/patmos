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
// Core simulation loop of the Patmos Simulator.
//

#include "simulation-core.h"
#include "instruction.h"

namespace patmos
{
  void decoder_t::decode(instruction_data_t *result)
  {
    // get the index into the program
    word_t pc = S.PC / 4;

    unsigned int i;
    bool fetch_next = true;
    for(i = 0; i < NUM_SLOTS; i++)
    {
      // get the first instruction of the bundle
      if (fetch_next && pc < (word_t)S.Program.size())
      {
        result[i] = S.Program[pc];
        pc++;
      }
      else
      {
        result[i] = instruction_data_t();
      }

      fetch_next = !result[i].Bundle_end;
    }
  }
}
