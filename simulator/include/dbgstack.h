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
// Helper class for debug stack handling.
//

#ifndef PATMOS_DBGSTACK_H
#define PATMOS_DBGSTACK_H

#include <limits>
#include <ostream>
#include <vector>


#include "basic-types.h"

namespace patmos
{
  /// forward defs
  class simulator_t;


  /// Profiling information for functions.
  class dbgstack_t
  {
    private:

      /// dbgstack_frame_t - Data of a stack frame.
      class dbgstack_frame_t {
      public:
        // constructor
        dbgstack_frame_t(simulator_t &sim, uword_t func);

        uword_t function;
        uword_t ret_base;
        uword_t ret_offs;
        uword_t caller_tos_stackcache;
        uword_t caller_tos_shadowstack;
        
        // Should we print statistics when returning from this function?
        bool print_stats;
      };

      /// stack - Debug stack.
      std::vector<dbgstack_frame_t> stack;

      simulator_t &sim;
      
      std::ostream *debug_out;
      
      uword_t print_function;

      /// print_stackframe - Print a single debug stack frame to the stream
      /// @param callee the stack frame of the callee, or null if not available
      void print_stackframe(std::ostream &os, unsigned depth,
                            const dbgstack_frame_t &frame,
                            const dbgstack_frame_t *callee) const;
    public:
      /// Constructor
      dbgstack_t(simulator_t &s) : sim(s), debug_out(0), 
         print_function(std::numeric_limits<unsigned int>::max())
      {
      }

      void print_function_stats(uword_t address, std::ostream &debug_out);
      
      /// initialize - Initialize the debug stack.
      void initialize(uword_t entry);

      /// finalize - Finalize the debug stack
      void finalize(void);

      /// is_active_frame - Returns true if a given frame is currently active.
      /// @param frame the frame to check
      bool is_active_frame(const dbgstack_frame_t &frame) const;

      /// push - Push the current state as a stack frame on the debug stack.
      /// Only used for debugging.
      void push(uword_t target);

      /// pop - Pop the top stack frame from the debug stack,
      /// if the given return info matches.
      /// Only used for debugging.
      void pop(uword_t return_base, uword_t return_offset);

      /// print - Print a debug stack trace.
      /// @param os An output stream.
      std::ostream &print(std::ostream &os) const;

  };

  std::ostream &operator<<(std::ostream &os, const dbgstack_t &dbgstack);

} // end namespace patmos


#endif // PATMOS_DBGSTACK_H
