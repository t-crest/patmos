/*
   Copyright 2012 Technical University of Denmark, DTU Compute.
   All rights reserved.

   This file is part of the Patmos simulator.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

      1. Redistributions of source code must retain the above copyright notice,
         this list of conditions and the following disclaimer.

      2. Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
   NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   The views and conclusions contained in the software and documentation are
   those of the authors and should not be interpreted as representing official
   policies, either expressed or implied, of the copyright holder.
 */

//
// Main file of the Patmos Simulator.
//

#include "command-line.h"
#include "loader.h"
#include "dbgstack.h"
#include "data-cache.h"
#include "instruction.h"
#include "method-cache.h"
#include "simulation-core.h"
#include "stack-cache.h"
#include "streams.h"
#include "symbol.h"
#include "memory-map.h"
#include "noc.h"
#include "uart.h"
#include "rtc.h"
#include "excunit.h"

#include <unistd.h>
#include <termios.h>
#include <signal.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>

#include <boost/program_options.hpp>


/// Construct a global memory for the simulation.
/// @param burst_time Access time in cycles for memory accesses.
/// @param size The requested size of the memory in bytes.
/// @return An instance of the requested memory.
static patmos::memory_t &create_global_memory(unsigned int cores,
                                              unsigned int cpu_id,
                                              unsigned int size,
                                              unsigned int burst_size,
                                              unsigned int page_size,
                                              unsigned int posted,
                                              unsigned int burst_time,
                                              unsigned int read_delay,
                                              unsigned int refresh_cycles,
                                              bool randomize_mem, 
                                              patmos::mem_check_e chkreads)
{
  if (cores > 1) {
    return *new patmos::tdm_memory_t(size, burst_size, posted, cores, cpu_id,
                                     burst_time, read_delay, refresh_cycles,
                                     randomize_mem, chkreads);
  } 
  else if (cores == 1) {
    if (burst_time == 0 && read_delay == 0)
      return *new patmos::ideal_memory_t(size, randomize_mem, chkreads);
    else if (page_size == 0)
      return *new patmos::fixed_delay_memory_t(size, burst_size, posted, 
                                              burst_time, read_delay,
                                              randomize_mem, chkreads);
    else
      return *new patmos::variable_burst_memory_t(size, burst_size, page_size,
                                              posted, burst_time, read_delay,
                                              randomize_mem, chkreads);
  } 
  else {
    std::cerr << "Invalid number of cores.\n";
    abort();
  }
}

/// Construct a data cache for the simulation.
/// @param dck The kind of the data cache requested.
/// @param size The requested size of the data cache in bytes.
/// @param line_size The size of one cache line.
/// @param gm Global memory accessed on a cache miss.
/// @return An instance of a data cache.
static patmos::data_cache_t &create_data_cache(patmos::set_assoc_cache_type dck,
                                               unsigned int size,
                                               unsigned int line_size,
                                               patmos::memory_t &gm)
{
  unsigned int num_blocks = (size - 1)/line_size + 1;
  // Make it fully-associative?
  unsigned int assoc = dck.associativity > 0 ? dck.associativity : num_blocks;

  switch (dck.policy)
  {
    case patmos::SAC_IDEAL:
      return *new patmos::ideal_data_cache_t(gm);
    case patmos::SAC_NO:
      return *new patmos::no_data_cache_t(gm);
    case patmos::SAC_DM:
      assert(dck.associativity == 1);
      // Fallthrough to LRU with 1-way assoc to model direct mapped cache      
    case patmos::SAC_LRU:
      return *new patmos::set_assoc_data_cache_t<true>(gm, assoc, num_blocks, line_size);
    case patmos::SAC_FIFO:
      return *new patmos::set_assoc_data_cache_t<false>(gm, assoc, num_blocks, line_size);
    case patmos::SAC_LRU_WB:
          return *new patmos::set_assoc_data_cache_wb_t<true>(gm, assoc, num_blocks, line_size);
    case patmos::SAC_FIFO_WB:
         return *new patmos::set_assoc_data_cache_wb_t<false>(gm, assoc, num_blocks, line_size);
  };
}


static patmos::data_cache_t &create_stack_data_cache(patmos::set_assoc_cache_type dck,
                                               unsigned int size,
                                               unsigned int line_size,
                                               patmos::memory_t &gm, patmos::memory_t &dc)
{
  unsigned int num_blocks = (size - 1)/line_size + 1;
  // Make it fully-associative?
  unsigned int assoc = dck.associativity > 0 ? dck.associativity : num_blocks;

  switch (dck.policy)
  {
    case patmos::SAC_IDEAL:
      return *new patmos::ideal_data_cache_t(gm);
    case patmos::SAC_NO:
      return *new patmos::no_data_cache_t(dc);
    case patmos::SAC_DM:
      assert(dck.associativity == 1);
      // Fallthrough to LRU with 1-way assoc to model direct mapped cache
    case patmos::SAC_LRU:
      return *new patmos::set_assoc_data_cache_t<true>(gm, assoc, num_blocks, line_size);
    case patmos::SAC_FIFO:
      return *new patmos::set_assoc_data_cache_t<false>(gm, assoc, num_blocks, line_size);
  };
}

/// Construct a method cache for the simulation.
/// @param mck The kind of the method cache requested.
/// @param size The requested size of the method cache in bytes.
/// @param block_size The size of one cache block in bytes.
/// @param assoc Associativity of the method cache.
/// @param gm Global memory accessed on a cache miss.
/// @return An instance of the requested method  cache kind.
static patmos::instr_cache_t &create_method_cache(patmos::method_cache_e mck,
                                                 unsigned int size,
                                                 unsigned int block_size,
                                                 unsigned int max_methods,
                                                 patmos::memory_t &gm)
{
  // convert size to number of blocks
  unsigned int num_blocks = ((size - 1)/block_size) + 1;
  
  switch(mck)
  {
    case patmos::MC_IDEAL:
      return *new patmos::ideal_method_cache_t(gm);
    case patmos::MC_LRU:
      return *new patmos::lru_method_cache_t(gm, num_blocks, block_size, max_methods);
    case patmos::MC_FIFO:
      return *new patmos::fifo_method_cache_t(gm, num_blocks, block_size, max_methods);
  }

  assert(false);
  abort();
}

static patmos::instr_cache_t &create_iset_cache(patmos::set_assoc_cache_type isck,
       unsigned int size, unsigned int line_size,
       patmos::memory_t &gm)
{
  unsigned int num_blocks = ((size - 1)/line_size) + 1;
  // Make it fully-associative?
  unsigned int assoc = isck.associativity > 0 ? isck.associativity : num_blocks;

  
  switch (isck.policy) {
    case patmos::SAC_IDEAL:
      return *new patmos::instr_cache_wrapper_t<true>(new patmos::ideal_data_cache_t(gm));
    case patmos::SAC_NO:
      return *new patmos::no_instr_cache_t(gm);
    case patmos::SAC_DM:
      assert(isck.associativity == 1);
      // Fallthrough to LRU with 1-way assoc to model direct mapped cache
    case patmos::SAC_LRU:
    {
      patmos::data_cache_t *lru =
        new patmos::set_assoc_data_cache_t<true>(gm, assoc, num_blocks, line_size);

      return *new patmos::instr_cache_wrapper_t<true>(lru);
    }
    case patmos::SAC_FIFO:
    {
      patmos::data_cache_t *fifo =
        new patmos::set_assoc_data_cache_t<false>(gm, assoc, num_blocks, line_size);

      return *new patmos::instr_cache_wrapper_t<true>(fifo);
    }
  }
}

static patmos::instr_cache_t &create_instr_cache(patmos::instr_cache_e ick, 
       patmos::set_assoc_cache_type isck, patmos::method_cache_e mck,
       unsigned int size, unsigned int line_size, unsigned int block_size,
       unsigned int mcmethods, patmos::memory_t &gm)
{
  switch (ick) {
    case patmos::IC_MCACHE:
      return create_method_cache(mck, size, block_size, mcmethods, gm);
    case patmos::IC_ICACHE:
      return create_iset_cache(isck, size, line_size, gm);
    default:
      abort();
  }
}

/// Construct a stack cache for the simulation.
/// @param sck The kind of the stack cache requested.
/// @param size The requested size of the stack cache in bytes.
/// @param bsize The size of burst transfers in bytes.
/// @param gm Global memory accessed on stack cache fills/spills.
/// @param dc Data cache to use if the stack cache type is dcache.
/// @return An instance of the requested stack cache kind.
static patmos::stack_cache_t &create_stack_cache(patmos::stack_cache_e sck,
                                                 unsigned int size,
                                                 unsigned int bsize,
                                                 patmos::memory_t &gm, 
                                                 patmos::memory_t &dc)
{
  switch(sck)
  {
    case patmos::SC_IDEAL:{

      return *new patmos::ideal_stack_cache_t(gm);
}
    case patmos::SC_BLOCK:
    {
      // The stack cache always uses a granularity of words for allocation
      unsigned int num_blocks = (size - 1) / 4 + 1;

      return *new patmos::block_stack_cache_t(gm, num_blocks, 4);
    }
    case patmos::SC_ABLOCK:
    {
      // convert size to number of blocks
      unsigned int num_blocks = (size - 1) / bsize + 1;
        
      return *new patmos::block_aligned_stack_cache_t(gm, num_blocks, bsize);
    }
    case patmos::SC_LBLOCK:
    {
      // convert size to number of blocks
      unsigned int num_blocks = (size - 1) / 4 + 1;
        
      return *new patmos::block_lazy_stack_cache_t(gm, num_blocks, 4);
    }
    case patmos::SC_DCACHE:
    {

      return *new patmos::proxy_stack_cache_t(dc);
    }
  }

  assert(false);
  abort();
}

/// Disable the line buffering 
void disable_line_buffering()
{
  // is it a regular stream anyways?
  if (isatty(STDIN_FILENO))
  {
    struct termios tio;

    // ignore SIGTTOU (which would stop background process)
    signal(SIGTTOU, SIG_IGN);

    // get the termios flags for stdin
    tcgetattr(STDIN_FILENO, &tio);

    // disable buffering
    tio.c_lflag &=(~ICANON);

    // reset the termios flags
    tcsetattr(STDIN_FILENO, TCSANOW, &tio);
  }
}

int main(int argc, char **argv)
{
  // the UART simulation may invoke cin.rdbuf()->in_avail(), which does not work
  // properly when cin is synced with stdio. we thus disable it here, since we
  // are not using stdio anyway.
  disable_line_buffering();
  std::cin.sync_with_stdio(false);

  // define command-line options
  boost::program_options::options_description generic_options(
    "Generic options:\n for memory/cache sizes the following units are allowed:"
    " k, m, g, or kb, mb, gb");
  generic_options.add_options()
    ("help,h", "produce help message")
    ("maxc,c", boost::program_options::value<unsigned int>()->default_value(0, "inf."), "stop simulation after the given number of cycles")
    ("binary,b", boost::program_options::value<std::string>(), "binary or elf-executable file (stdin: -)")
    ("debug", boost::program_options::value<unsigned int>()->implicit_value(0), "enable step-by-step debug tracing after cycle")
    ("debug-fmt", boost::program_options::value<patmos::debug_format_e>()->default_value(patmos::DF_DEFAULT), 
                  "format of the debug trace (short, trace, instr, blocks, calls, calls-indent, default, long, all)")
    ("debug-file", boost::program_options::value<std::string>()->default_value(""), "output debug trace in file (stdout: -)")
    ("debug-intrs", "print out all status changes of the exception unit.")
    ("debug-nopc", "do not print PC and cycles counter in debug output")
    ("debug-access", boost::program_options::value<patmos::address_t>(), "print accesses to the given address or symbol.")
    ("stats-file,o", boost::program_options::value<std::string>()->default_value(""), "write statistics to a file (stdout: -)")
    ("print-stats", boost::program_options::value<patmos::address_t>(), "print statistics for a given function only.")
    ("flush-caches", boost::program_options::value<patmos::address_t>(), "flush all caches when reaching the given address (can be a symbol name).")
    ("hitmiss-stats", "Print hit/miss cache accesses (requires '--full')")
    ("full,V", "full statistics output")
    ("verbose,v", "enable short statistics output");

  boost::program_options::options_description memory_options("Memory options");
  memory_options.add_options()
    ("gsize,g",  boost::program_options::value<patmos::byte_size_t>()->default_value(patmos::NUM_MEMORY_BYTES), "global memory size in bytes")
    ("gtime,G",  boost::program_options::value<unsigned int>()->default_value(patmos::NUM_MEMORY_BLOCK_BYTES/4 + 3), 
                 "global memory transfer time per burst in cycles")
    ("tdelay,t", boost::program_options::value<int>()->default_value(0), "read delay to global memory per request in cycles")
    ("trefresh", boost::program_options::value<unsigned int>()->default_value(0), "refresh cycles per TDM round")
    ("bsize",    boost::program_options::value<unsigned int>()->default_value(patmos::NUM_MEMORY_BLOCK_BYTES), "burst size (and alignment) of the memory system.")
    ("psize",    boost::program_options::value<patmos::byte_size_t>()->default_value(0), "Memory page size. Enables variable burst lengths for single-core.")
    ("posted,p", boost::program_options::value<unsigned int>()->default_value(0), "Enable posted writes (sets max queue size)")
    ("lsize,l",  boost::program_options::value<patmos::byte_size_t>()->default_value(patmos::NUM_LOCAL_MEMORY_BYTES), "local memory size in bytes")
    ("mem-rand", boost::program_options::value<unsigned int>()->default_value(0), "Initialize memories with random data")
    ("chkreads", boost::program_options::value<patmos::mem_check_e>()->default_value(patmos::MCK_NONE), 
                 "Check for reads of uninitialized data, either per byte (warn, err) or per access (warn-addr, err-addr). Disables the data cache.");

  boost::program_options::options_description noc_options("Network-on-chip options");
  noc_options.add_options()
    ("nocbase",          boost::program_options::value<patmos::address_t>()->default_value(patmos::NOC_BASE_ADDRESS), "base address of the NOC device map address range")
    ("noc_route_offset", boost::program_options::value<patmos::address_t>()->default_value(patmos::NOC_DMA_P_OFFSET), "offset of the NOC routing information device map")
    ("noc_st_offset",    boost::program_options::value<patmos::address_t>()->default_value(patmos::NOC_DMA_ST_OFFSET), "offset of the NOC slot table device map")
    ("noc_spm_offset",   boost::program_options::value<patmos::address_t>()->default_value(patmos::NOC_SPM_OFFSET), "offset of the NOC SPM")
    ("nocsize",          boost::program_options::value<patmos::byte_size_t>()->default_value(patmos::NOC_SPM_SIZE),   "size of the NOC SPM");
    
  boost::program_options::options_description cache_options("Cache options");
  cache_options.add_options()
    ("dcsize,d", boost::program_options::value<patmos::byte_size_t>()->default_value(patmos::NUM_DATA_CACHE_BYTES), "data cache size in bytes")
    ("dckind,D", boost::program_options::value<patmos::set_assoc_cache_type>()->default_value(patmos::set_assoc_cache_type(patmos::SAC_DM,2)),
                 "kind of direct mapped/fully-/set-associative data cache, defaults to lru2 (ideal, no, dm, lru[N], fifo[N], lruwb[N], fifowb[N])")
    ("dlsize",   boost::program_options::value<patmos::byte_size_t>()->default_value(0), "size of a data cache line in bytes, defaults to burst size if set to 0")

    ("scsize,s", boost::program_options::value<patmos::byte_size_t>()->default_value(patmos::NUM_STACK_CACHE_BYTES), "stack cache size in bytes")
    ("sckind,S", boost::program_options::value<patmos::stack_cache_e>()->default_value(patmos::SC_BLOCK), "kind of stack cache (ideal, block, ablock, lblock, dcache)")

    ("icache,C", boost::program_options::value<patmos::instr_cache_e>()->default_value(patmos::IC_MCACHE), "kind of instruction cache (mcache, icache)")
    ("ickind,K", boost::program_options::value<patmos::set_assoc_cache_type>()->default_value(patmos::set_assoc_cache_type(patmos::SAC_LRU,2)), 
                 "kind of direct mapped/fully-/set-associative I-cache (ideal, no, dm, lru[N], fifo[N]")
    ("ilsize",   boost::program_options::value<patmos::byte_size_t>()->default_value(0), "size of an I-cache line in bytes, defaults to burst size if set to 0")

    ("mcsize,m", boost::program_options::value<patmos::byte_size_t>()->default_value(patmos::NUM_METHOD_CACHE_BYTES), 
                 "method cache / instruction cache size in bytes")
    ("mckind,M", boost::program_options::value<patmos::method_cache_e>()->default_value(patmos::MC_FIFO), "kind of method cache (ideal, lru, fifo)")
    ("mcmethods",boost::program_options::value<unsigned int>()->default_value(patmos::NUM_METHOD_CACHE_MAX_METHODS), 
                 "Maximum number of methods in the method cache, defaults to number of blocks if zero")
    ("mbsize",   boost::program_options::value<patmos::byte_size_t>()->default_value(patmos::NUM_METHOD_CACHE_BLOCK_BYTES), 
                 "method cache block size in bytes, defaults to burst size if zero")
	("scdcsize,z", boost::program_options::value<patmos::byte_size_t>()->default_value(patmos::NUM_DATA_STACK_CACHE_BYTES), "stack data cache size in bytes")
	("scdckind,Z", boost::program_options::value<patmos::set_assoc_cache_type>()->default_value(patmos::set_assoc_cache_type(patmos::SAC_LRU,2)),
	             "kind of direct mapped/fully-/set-associative stack data cache, defaults to lru2 (ideal, no, dm, lru[N], fifo[N])");


  boost::program_options::options_description sim_options("Simulator options");
  sim_options.add_options()
    ("cpuid",  boost::program_options::value<unsigned int>()->default_value(0), "Set CPU ID in the simulator")
    ("cores,N", boost::program_options::value<unsigned int>()->default_value(1), "Set number of CPUs (enables memory TDM)")
    ("freq",   boost::program_options::value<double>()->default_value(80.0), "Set CPU Frequency in Mhz")
    ("mmbase", boost::program_options::value<patmos::address_t>()->default_value(patmos::IOMAP_BASE_ADDRESS), "base address of the IO device map address range")
    ("mmhigh", boost::program_options::value<patmos::address_t>()->default_value(patmos::IOMAP_HIGH_ADDRESS), "highest address of the IO device map address range")
    ("cpuinfo_offset", boost::program_options::value<patmos::address_t>()->default_value(patmos::CPUINFO_OFFSET), "offset where the cpuinfo device is mapped")
    ("excunit_offset", boost::program_options::value<patmos::address_t>()->default_value(patmos::EXCUNIT_OFFSET), "offset where the exception unit is mapped")
    ("timer_offset", boost::program_options::value<patmos::address_t>()->default_value(patmos::TIMER_OFFSET), "offset where the timer device is mapped")
    ("perfcounters_offset", boost::program_options::value<patmos::address_t>()->default_value(patmos::PERFCOUNTERS_OFFSET), "offset where the performance counters device is mapped")
    ("uart_offset", boost::program_options::value<patmos::address_t>()->default_value(patmos::UART_OFFSET), "offset where the UART device is mapped")
    ("led_offset", boost::program_options::value<patmos::address_t>()->default_value(patmos::LED_OFFSET), "offset where the LED device is mapped");
  
  boost::program_options::options_description uart_options("UART options");
  uart_options.add_options()
    ("in,I", boost::program_options::value<std::string>()->default_value("-"), "input file for UART simulation (stdin: -)")
    ("out,O", boost::program_options::value<std::string>()->default_value("-"), "output file for UART simulation (stdout: -)");

  boost::program_options::options_description interrupt_options("Interrupt options");
  interrupt_options.add_options()
    ("interrupt", boost::program_options::value<int>()->default_value(1), "enable or disable interrupts");

  boost::program_options::positional_options_description pos;
  pos.add("binary", 1);

  boost::program_options::options_description cmdline_options;
  cmdline_options.add(generic_options).add(memory_options).add(cache_options)
                 .add(noc_options).add(sim_options).add(uart_options)
                 .add(interrupt_options);

  // process command-line options
  boost::program_options::variables_map vm;
  try
  {
    boost::program_options::store(
                          boost::program_options::command_line_parser(argc, argv)
                            .options(cmdline_options).positional(pos).run(), vm);
    boost::program_options::notify(vm);

    // help message
    if (vm.count("help")) {
      std::cout << cmdline_options << "\n";
      return 1;
    }
  }
  catch(boost::program_options::error &e)
  {
    std::cerr << cmdline_options << "\n" << e.what() << "\n\n";
    return 1;
  }

  // get some command-line  options
  std::string binary;
  if (vm.count("binary")) {
    binary = (vm["binary"].as<std::string>());
  } else {
    std::cout << "No program to simulate specified. Use --help for more options.\n";
    return 1;
  }
  std::string stats_out(vm["stats-file"].as<std::string>());

  std::string uart_in(vm["in"].as<std::string>());
  std::string uart_out(vm["out"].as<std::string>());

  std::string debug_out(vm["debug-file"].as<std::string>());

  unsigned int cores = vm["cores"].as<unsigned int>();
  unsigned int cpuid = vm["cpuid"].as<unsigned int>();
  double       freq = vm["freq"].as<double>();
  unsigned int mmbase = vm["mmbase"].as<patmos::address_t>().value();
  unsigned int mmhigh = vm["mmhigh"].as<patmos::address_t>().value();
  
  unsigned int nocbase = vm["nocbase"].as<patmos::address_t>().value();
  unsigned int noc_route_offset = vm["noc_route_offset"].as<patmos::address_t>().value();
  unsigned int noc_st_offset = vm["noc_st_offset"].as<patmos::address_t>().value();
  unsigned int noc_spm_offset = vm["noc_spm_offset"].as<patmos::address_t>().value();
  unsigned int nocsize = vm["nocsize"].as<patmos::byte_size_t>().value();
  
  unsigned int cpuinfo_offset = vm["cpuinfo_offset"].as<patmos::address_t>().value();
  unsigned int excunit_offset = vm["excunit_offset"].as<patmos::address_t>().value();
  unsigned int timer_offset = vm["timer_offset"].as<patmos::address_t>().value();
  unsigned int perfcounters_offset = vm["perfcounters_offset"].as<patmos::address_t>().value();
  unsigned int uart_offset = vm["uart_offset"].as<patmos::address_t>().value();
  unsigned int led_offset = vm["led_offset"].as<patmos::address_t>().value();

  unsigned int gsize = vm["gsize"].as<patmos::byte_size_t>().value();
  unsigned int lsize = vm["lsize"].as<patmos::byte_size_t>().value();
  unsigned int dcsize = vm["dcsize"].as<patmos::byte_size_t>().value();
  unsigned int scdcsize = vm["scdcsize"].as<patmos::byte_size_t>().value();
  unsigned int dlsize = vm["dlsize"].as<patmos::byte_size_t>().value();
  unsigned int scsize = vm["scsize"].as<patmos::byte_size_t>().value();
  unsigned int mcsize = vm["mcsize"].as<patmos::byte_size_t>().value();
  unsigned int mbsize = vm["mbsize"].as<patmos::byte_size_t>().value();
  unsigned int ilsize = vm["ilsize"].as<patmos::byte_size_t>().value();
  unsigned int mcmethods= vm["mcmethods"].as<unsigned int>();

  unsigned int gtime = vm["gtime"].as<unsigned int>();
  unsigned int bsize = vm["bsize"].as<unsigned int>();
  unsigned int psize = vm["psize"].as<patmos::byte_size_t>().value();
  unsigned int posted = vm["posted"].as<unsigned int>();
           int tdelay = vm["tdelay"].as<int>();
  unsigned int trefresh = vm["trefresh"].as<unsigned int>();

  patmos::set_assoc_cache_type dck = vm["dckind"].as<patmos::set_assoc_cache_type>();
  patmos::set_assoc_cache_type scdck = vm["scdckind"].as<patmos::set_assoc_cache_type>();
  patmos::stack_cache_e sck = vm["sckind"].as<patmos::stack_cache_e>();
  patmos::instr_cache_e ick = vm["icache"].as<patmos::instr_cache_e>();
  patmos::method_cache_e mck = vm["mckind"].as<patmos::method_cache_e>();
  patmos::set_assoc_cache_type isck = vm["ickind"].as<patmos::set_assoc_cache_type>();

  patmos::debug_format_e debug_fmt= vm["debug-fmt"].as<patmos::debug_format_e>();
  bool debug_nopc = vm.count("debug-nopc") > 0;
  bool debug_intrs = vm.count("debug-intrs") > 0;
  uint64_t debug_cycle = vm.count("debug") ?
                                vm["debug"].as<unsigned int>() :
                                std::numeric_limits<uint64_t>::max();
  uint64_t max_cycle = vm["maxc"].as<unsigned int>();
  if (!max_cycle) {
    max_cycle = std::numeric_limits<uint64_t>::max();
  }
  
  // TODO make the option accept a list of addresses/symbol names
  bool debug_accesses = vm.count("debug-access") > 0;
  patmos::address_t debug_access_addr;
  if (debug_accesses) {
    debug_access_addr = vm["debug-access"].as<patmos::address_t>();
  }
  
  bool print_stats = vm.count("print-stats") > 0;
  patmos::address_t print_stats_func;
  if (print_stats) {
    print_stats_func = vm["print-stats"].as<patmos::address_t>();
  }

  bool flush_caches = vm.count("flush-caches") > 0;
  patmos::address_t flush_caches_addr;
  if (flush_caches) {
    flush_caches_addr = vm["flush-caches"].as<patmos::address_t>();
  }
  
  bool excunit_enabled = vm["interrupt"].as<int>() > 0;

  bool randomize_mem = vm["mem-rand"].as<unsigned int>() > 0;
  patmos::mem_check_e chkreads = vm["chkreads"].as<patmos::mem_check_e>();
  
  bool hitmiss_stats = (vm.count("hitmiss-stats") != 0);
  bool long_stats = (vm.count("full") != 0);
  bool verbose = (vm.count("verbose") != 0) || long_stats;

  if (!mbsize) mbsize = bsize;
  
  if (chkreads != patmos::MCK_NONE) {
    // Disable the data cache with --chkreads
    // TODO we should warn about this / abort if the user sets -D .. need to make
    // -D have an implicit_value instead of default_value for this.
    dck.policy = patmos::SAC_NO;
    scdck.policy = patmos::SAC_NO;
  }
  
  // the exit code, initialized by default to signal an error.
  int exit_code = -1;

  // input/output streams
  std::istream *in = NULL;
  std::istream *uin = NULL;

  std::ostream *sout = NULL;
  std::ostream *uout = NULL;

  std::ostream *dout = NULL;

  // Seed rand with a fixed value so that every simulation run produces the same
  // results.
  srand(0);
  
  // setup simulation framework
  patmos::memory_t &gm = create_global_memory(cores, cpuid, gsize, 
                                              bsize, psize,
                                              posted, gtime, tdelay, trefresh,
                                              randomize_mem, chkreads);
  patmos::instr_cache_t &ic = create_instr_cache(ick, isck, mck, mcsize, 
                                                 ilsize ? ilsize : bsize, 
                                                 mbsize, mcmethods, gm);
  patmos::data_cache_t &dc = create_data_cache(dck, dcsize, 
                                               dlsize ? dlsize : bsize, gm);

  patmos::stack_cache_t &sc = create_stack_cache(sck, scsize, bsize, gm, dc);

  patmos::data_cache_t &scdc = create_stack_data_cache(scdck, scdcsize,
                                               dlsize ? dlsize : bsize, gm, dc);

  try
  {
    // open streams
    in = patmos::get_stream<std::ifstream>(binary, std::cin);

    uin = patmos::get_stream<std::ifstream>(uart_in, std::cin);
    uout = patmos::get_stream<std::ofstream>(uart_out, std::cout);

    dout = patmos::get_stream<std::ofstream>(debug_out, std::cerr);
    sout = patmos::get_stream<std::ofstream>(stats_out, std::cerr);


    // check if the uart input stream is a tty.
    bool uin_istty = (uin == &std::cin) && isatty(STDIN_FILENO);

    assert(in && sout && uin && uout && dout);

    // finalize simulation framework
    // setup exception unit
    patmos::excunit_t excunit(mmbase+excunit_offset);
    excunit.enable_interrupts(excunit_enabled);
    excunit.enable_debug(debug_intrs);

    // TODO initialize the SPM with random data as well?
    patmos::ideal_memory_t lm(lsize, false, chkreads);
    patmos::ideal_memory_t nm(nocsize, false, patmos::MCK_NONE);
    patmos::memory_map_t mm(lm, std::min(mmbase,nocbase), mmhigh);
    
    patmos::symbol_map_t sym;

    patmos::simulator_t s(gm, mm, dc, ic, sc, scdc, sym, excunit);

    // setup statistics printing
    patmos::stats_options_t &stats_options = s.Dbg_stack.get_stats_options();
    stats_options.short_stats = !long_stats;
    stats_options.instruction_stats = long_stats;
    stats_options.profiling_stats = long_stats;
    stats_options.hitmiss_stats = hitmiss_stats;
    
    // set up timer device
    patmos::rtc_t rtc(s, mmbase+timer_offset, freq);
    rtc.enable_debug(debug_intrs);
    
    // setup IO mapped devices
    patmos::cpuinfo_t cpuinfo(mmbase+cpuinfo_offset, cpuid, freq, cores);
    patmos::perfcounters_t perfcounters(mmbase+perfcounters_offset);
    patmos::uart_t uart(mmbase+uart_offset, *uin, uin_istty, *uout);
    patmos::led_t leds(mmbase+led_offset, *uout);
    patmos::noc_t noc(nocbase, nocbase+noc_route_offset, nocbase+noc_st_offset,
                      nocbase+noc_spm_offset, nocsize, nm);

    mm.add_device(cpuinfo);
    mm.add_device(excunit);
    mm.add_device(perfcounters);
    mm.add_device(uart);
    mm.add_device(leds);
    mm.add_device(rtc);
    mm.add_device(noc);

    // load input program
    patmos::section_list_t text;
    patmos::loader_t *loader = patmos::create_loader(*in);
    patmos::uword_t entry = loader->get_program_entry();
    
    if (!loader->is_ELF()) {
      // some output for compatibility
      std::cerr << boost::format("Loaded: %1% bytes\n") 
                   % loader->get_binary_size();
    }

    try
    {
      loader->load_symbols(sym, text);
      loader->load_to_memory(s, gm);
    }
    catch (patmos::simulation_exception_t e) 
    {
      switch(e.get_kind()) {
        case patmos::simulation_exception_t::UNMAPPED:
          // Unmapped access during loading of code
          if (e.get_cycle() == 0) {
            std::cerr << boost::format("Unmapped access to 0x%x while loading program.\n") % e.get_info();
            break;
          }
          // intended fallthrough to default
        default:
          std::cerr << e.to_string(sym);
      }
      goto _cleanup;
    }
    
    if (debug_accesses) {
      debug_access_addr.parse(sym);
      s.Debug_mem_address.insert(debug_access_addr.value());
    }
    
    // setup stats reset trigger
    if (print_stats) {
      print_stats_func.parse(sym);
      s.Dbg_stack.print_function_stats(print_stats_func.value(), *sout);
    }
  
    if (flush_caches) {
      flush_caches_addr.parse(sym);
      s.flush_caches_at(flush_caches_addr.value());
    }
  
    // start execution
    bool success = false;
    try
    {
      s.run(entry, debug_cycle, debug_fmt, *dout, debug_nopc, 
            max_cycle, long_stats);
      success = true;
    }
    catch (patmos::simulation_exception_t e)
    {
      switch(e.get_kind())
      {
        case patmos::simulation_exception_t::HALT:
          // get the exit code
          exit_code = e.get_info();
          success = true;
          break;
        default:
          std::cerr << e.to_string(sym);
	  std::cerr << s.Dbg_stack;
      }
    }
    
    if (success) {
      if (verbose && !print_stats) {
        s.print_stats(*sout);
      }
      if (verbose) {
        *sout << "Pasim options:\n  ";
        
        // TODO make this more generic.. somehow.
        
        if (vm["maxc"].as<unsigned int>())
          *sout << " --maxc=" << max_cycle;
        if (flush_caches)
          *sout << " --flush-caches=" << flush_caches_addr;
        *sout << " --cpuid=" << cpuid << " --cores=" << cores;
        *sout << " --freq=" << freq;
        *sout << " --interrupt=" << excunit_enabled;            

        *sout << "\n  ";
        *sout << " --mmbase=" << mmbase << " --mmhigh=" << mmhigh;
        *sout << " --cpuinfo_offset=" << cpuinfo_offset;
        *sout << " --excunit_offset=" << excunit_offset;
        *sout << " --timer_offset=" << timer_offset;
        *sout << " --perfcounters_offset=" << perfcounters_offset;
        *sout << " --uart_offset=" << uart_offset;
        *sout << " --led_offset=" << led_offset;
        
        *sout << "\n  ";
        *sout << " --gsize=" << gsize;
        *sout << " --gtime=" << gtime;
        *sout << " --tdelay=" << tdelay << " --trefresh=" << trefresh;
        *sout << " --bsize=" << bsize << " --psize=" << psize;
        *sout << " --posted=" << posted; 
        
        *sout << "\n  ";
        *sout << " --nocbase=" << nocbase;
        *sout << " --noc_route_offset=" << noc_route_offset;
        *sout << " --noc_st_offset=" << noc_st_offset;
        *sout << " --noc_spm_offset=" << noc_spm_offset;
        *sout << " --nocsize=" << nocsize;
        
        *sout << "\n  ";
        *sout << " --lsize=" << lsize;
        *sout << " --dckind=" << dck;
        *sout << " --dcsize=" << dcsize << " --dlsize=" << dlsize;
        *sout << " --scdckind=" << scdck;
        *sout << " --scdcsize=" << scdcsize << " --dlsize=" << dlsize;
        *sout << " --sckind=" << sck;
        *sout << " --scsize=" << scsize;
        
        *sout << "\n  ";
        *sout << " --icache=" << ick << " --ickind=" << isck;
        *sout << " --ilsize=" << ilsize;
        *sout << " --mckind=" << mck;
        *sout << " --mcsize=" << mcsize << " --mbsize=" << mbsize;
        *sout << " --mcmethods=" << mcmethods;
        
        *sout << "\n\n";
      }
    }
  }
  catch(std::ios_base::failure f)
  {
    std::cerr << f.what() << "\n";
  }

_cleanup:
  // free memory/cache instances
  // note: no need to free the local memory here.
  delete &gm;
  delete &dc;
  delete &scdc;
  delete &ic;
  delete &sc;

  // free streams
  patmos::free_stream(in);

  patmos::free_stream(uin);
  patmos::free_stream(uout);

  patmos::free_stream(dout);
  patmos::free_stream(sout);

  return exit_code;
}
