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


#include "dbgstack.h"
#include "simulation-core.h"
#include "stack-cache.h"
#include "symbol.h"

#include <iostream>
#include <sstream>
#include <boost/format.hpp>
#include <climits>


namespace patmos
{

  dbgstack_t::dbgstack_frame_t::
  dbgstack_frame_t(simulator_t &sim, uword_t func) 
   : function(func), print_stats(false)
  {
    // We could use r30/r31 here ?!
    ret_base = sim.BASE;
    ret_offs = sim.nPC-sim.BASE;

    // if rsp has not been set yet, use int_max for now
    uword_t sp = sim.GPR.get(rsp).get();
    caller_tos_shadowstack = sp ? sp : INT_MAX;
    caller_tos_stackcache  = sim.Stack_cache.size();
    // TODO remember state of GPR, if debug is enabled.
  }


  void dbgstack_t::print_function_stats(uword_t address, std::ostream &dbg)
  {
    print_function = address;
    debug_out = &dbg;
  }
  
  void dbgstack_t::initialize(uword_t entry)
  {
    push(entry);
  }


  void dbgstack_t::finalize()
  {
  }


  bool dbgstack_t::is_active_frame(const dbgstack_frame_t &frame) const
  {
    // TODO this might break if the stack pointers are modified in the delay
    //      slots (?)
    
    // check if the frame stack pointers are below the current pointers
    if (frame.caller_tos_shadowstack < sim.GPR.get(rsp).get()) {
      // we are currently further down the shadow stack
      return false;
    }
    // Note that at the moment we store the size of the stack cache,
    // *not* the TOS address.
    if (frame.caller_tos_stackcache > sim.Stack_cache.size()) {
      return false;
    }
    // check if the function of the current frame contains
    // the current subfunction
    return frame.function == sim.BASE ||
              sim.Symbols.covers(frame.function, sim.BASE);
  }



  void dbgstack_t::push(uword_t target)
  {
    if (!stack.empty()) {
      // Check if the call is coming from the TOS.
      if (!is_active_frame(stack.back())) {
        // We are resuming after some longjmp or so..
        // For now, just nuke the whole stack
        while (!stack.empty()) {
          stack.pop_back();
        }
      }
    }
    // Create a new stack frame
    stack.push_back( dbgstack_frame_t(sim, target) );
    
    if (target == print_function) {
      // TODO this should be moved into a separate class, managing stats 
      //      printing with multiple targets, ..
      // TODO should we print the stats up to here?
      sim.reset_stats();
      stack.back().print_stats = true;
    }
  }


  void dbgstack_t::pop(uword_t return_base, uword_t return_offset)
  {
    if (stack.empty()) return;

    // check if we are truly returning,
    // otherwise do not pop ... yet (if this is a longjmp)
    dbgstack_frame_t &frame = stack.back();
    
    if (frame.ret_base == return_base &&
        frame.ret_offs == return_offset) 
    {
      if (frame.print_stats) {
        sim.print_stats(*debug_out, true, true);
      }
      stack.pop_back();
    }
  }


  void dbgstack_t::print_stackframe(std::ostream &os, unsigned depth,
                                           const dbgstack_frame_t &frame,
                                           const dbgstack_frame_t *callee)
                                           const
  {
    os << boost::format("#%d 0x%x ") % depth % frame.function;
    sim.Symbols.print(os, frame.function, true);

    // TODO print registers r3-r8 from call state, as well as varargs
    os << "()";
    os << boost::format(": $rsp 0x%x stack cache size 0x%x\n")
          % frame.caller_tos_shadowstack % frame.caller_tos_stackcache;

    // Print the current location, if any
    uword_t base = 0, offset = 0;
    if (callee) {
      base   = callee->ret_base;
      offset = callee->ret_offs;
    } else if (is_active_frame(frame)) {
      base   = sim.BASE;
      offset = sim.PC - sim.BASE;
    }

    if (base || offset) {
      os << boost::format("   at 0x%x (base: 0x%x ") % (base + offset) % base;
      sim.Symbols.print(os, base);
      os << boost::format(", offset: 0x%x ") % offset;
      sim.Symbols.print(os, base + offset);
      os << ")\n";
    }

    // TODO print current state using the callee infos (if debug/verbose is enabled?)
  }


  std::ostream &dbgstack_t::print(std::ostream &os) const
  {
    // TODO Obviously, it would be nicer to print the actual stack
    // frame, but since we have up to three different stacks and no
    // easy way to determine the stack frame size, it is much easier
    // to just keep track of the stack frames separately. This has the
    // advantage that we can store more debug info in the frames.
    // This could be removed when we get GDB support into the simulator ;)

    os << "Stacktrace:\n";

    if (stack.empty()) {
      print_stackframe(os, 0, dbgstack_frame_t(sim, sim.BASE), 0);
      return os;
    }

    const dbgstack_frame_t *callee_frame = NULL;
    unsigned i = 0;
    for (std::vector<dbgstack_frame_t>::const_reverse_iterator
         fi=stack.rbegin(), fe=stack.rend(); fi!=fe; ++fi) {

      print_stackframe(os, i++, *fi, callee_frame);
      callee_frame = &*fi;
    }
    return os;
  }

  std::ostream &operator<<(std::ostream &os, const dbgstack_t &dbgstack)
  {
    return dbgstack.print(os);
  }

} // end namespace patmos
