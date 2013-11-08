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
// Main file of the Patmos Simulator.
//

#include "command-line.h"
#include "loader.h"
#include "data-cache.h"
#include "instruction.h"
#include "method-cache.h"
#include "simulation-core.h"
#include "stack-cache.h"
#include "streams.h"
#include "symbol.h"
#include "memory-map.h"
#include "uart.h"
#include "rtc.h"
#include "interrupts.h"

#include <unistd.h>
#include <termios.h>
#include <signal.h>

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
                                              unsigned int refresh_cycles)
{
  if (cores > 1) {
    return *new patmos::tdm_memory_t(size, burst_size, posted, cores, cpu_id,
                                     burst_time, read_delay, refresh_cycles);    
  } 
  else if (cores == 1) {
    if (burst_time == 0 && read_delay == 0)
      return *new patmos::ideal_memory_t(size);
    else if (page_size == 0)
      return *new patmos::fixed_delay_memory_t(size, burst_size, posted, 
                                              burst_time, read_delay);
    else
      return *new patmos::variable_burst_memory_t(size, burst_size, page_size,
                                              posted, burst_time, read_delay);
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

  switch (dck.policy)
  {
    case patmos::SAC_IDEAL:
      return *new patmos::ideal_data_cache_t(gm);
    case patmos::SAC_NO:
      return *new patmos::no_data_cache_t(gm);
    case patmos::SAC_LRU:
      return *new patmos::set_assoc_data_cache_t<true>(gm, dck.associativity, num_blocks, line_size);
    case patmos::SAC_FIFO:
      return *new patmos::set_assoc_data_cache_t<false>(gm, dck.associativity, num_blocks, line_size);
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

  switch (isck.policy) {
    case patmos::SAC_IDEAL:
      return *new patmos::instr_cache_wrapper_t<true>(new patmos::ideal_data_cache_t(gm));
    case patmos::SAC_NO:
      return *new patmos::no_instr_cache_t(gm);
    case patmos::SAC_LRU:
    {
      patmos::data_cache_t *lru =
        new patmos::set_assoc_data_cache_t<true>(gm, isck.associativity, num_blocks, line_size);

      return *new patmos::instr_cache_wrapper_t<true>(lru);
    }
    case patmos::SAC_FIFO:
    {
      patmos::data_cache_t *fifo =
        new patmos::set_assoc_data_cache_t<false>(gm, isck.associativity, num_blocks, line_size);

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
/// @param block_size The size of a cache block in bytes.
/// @param gm Global memory accessed on stack cache fills/spills.
/// @return An instance of the requested stack cache kind.
static patmos::stack_cache_t &create_stack_cache(patmos::stack_cache_e sck,
                                                 unsigned int size,
                                                 unsigned int block_size,
                                                 patmos::memory_t &gm)
{
  switch(sck)
  {
    case patmos::SC_IDEAL:
      return *new patmos::ideal_stack_cache_t(gm);
    case patmos::SC_BLOCK:
    {
      // convert size to number of blocks
      unsigned int num_blocks = (size - 1) / block_size + 1;

      return *new patmos::block_stack_cache_t(gm, num_blocks, block_size);
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
    ("maxc,c", boost::program_options::value<unsigned int>()->default_value(std::numeric_limits<unsigned int>::max(), "inf."), "stop simulation after the given number of cycles")
    ("binary,b", boost::program_options::value<std::string>()->default_value("-"), "binary or elf-executable file (stdin: -)")
    ("output,o", boost::program_options::value<std::string>()->default_value("-"), "output execution trace in file (stdout: -)")
    ("debug", boost::program_options::value<unsigned int>()->implicit_value(0), "enable step-by-step debug tracing after cycle")
    ("debug-fmt", boost::program_options::value<patmos::debug_format_e>()->default_value(patmos::DF_DEFAULT), "format of the debug trace (short, trace, instr, blocks, calls, default, long, all)")
    ("debug-file", boost::program_options::value<std::string>()->default_value("-"), "output debug trace in file (stderr: -)")
    ("debug-nopc", "do not print PC and cycles counter in debug output")
    ("print-stats", boost::program_options::value<patmos::address_t>(), "print statistics for a given function only.")
    ("flush-caches", boost::program_options::value<patmos::address_t>(), "flush all caches when reaching the given address (can be a symbol name).")
    ("slot-stats,a", "show instruction statistics per slot")
    ("instr-stats,i", "show more detailed statistics per instruction")
    ("quiet,q", "disable statistics output");

  boost::program_options::options_description memory_options("Memory options");
  memory_options.add_options()
    ("gsize,g",  boost::program_options::value<patmos::byte_size_t>()->default_value(patmos::NUM_MEMORY_BYTES), "global memory size in bytes")
    ("gtime,G",  boost::program_options::value<unsigned int>()->default_value(patmos::NUM_MEMORY_BLOCK_BYTES/4 + 5), "global memory transfer time per burst in cycles")
    ("tdelay,t", boost::program_options::value<unsigned int>()->default_value(0), "read delay to global memory per request in cycles")
    ("trefresh", boost::program_options::value<unsigned int>()->default_value(0), "refresh cycles per TDM round")
    ("bsize",    boost::program_options::value<unsigned int>()->default_value(patmos::NUM_MEMORY_BLOCK_BYTES), "burst size (and alignment) of the memory system.")
    ("psize",    boost::program_options::value<unsigned int>()->default_value(0), "Memory page size. Enables variable burst lengths for single-core.")
    ("posted,p", boost::program_options::value<unsigned int>()->default_value(0), "Enable posted writes (sets max queue size)")
    ("lsize,l",  boost::program_options::value<patmos::byte_size_t>()->default_value(patmos::NUM_LOCAL_MEMORY_BYTES), "local memory size in bytes");

  boost::program_options::options_description cache_options("Cache options");
  cache_options.add_options()
    ("dcsize,d", boost::program_options::value<patmos::byte_size_t>()->default_value(patmos::NUM_DATA_CACHE_BYTES), "data cache size in bytes")
    ("dckind,D", boost::program_options::value<patmos::set_assoc_cache_type>()->default_value(patmos::set_assoc_cache_type(patmos::SAC_LRU,2)), "kind of set-associative data cache, defaults to lru2 (ideal, no, lruN, fifoN)")
    ("dlsize",   boost::program_options::value<patmos::byte_size_t>()->default_value(0), "size of a data cache line in bytes, defaults to burst size if set to 0")

    ("scsize,s", boost::program_options::value<patmos::byte_size_t>()->default_value(patmos::NUM_STACK_CACHE_BYTES), "stack cache size in bytes")
    ("sckind,S", boost::program_options::value<patmos::stack_cache_e>()->default_value(patmos::SC_IDEAL), "kind of stack cache (ideal, block)")
    ("sbsize",   boost::program_options::value<patmos::byte_size_t>()->default_value(patmos::NUM_STACK_CACHE_BLOCK_BYTES), "stack cache block size in bytes")

    ("icache,C", boost::program_options::value<patmos::instr_cache_e>()->default_value(patmos::IC_MCACHE), "kind of instruction cache (mcache, icache)")
    ("ickind,K", boost::program_options::value<patmos::set_assoc_cache_type>()->default_value(patmos::set_assoc_cache_type(patmos::SAC_IDEAL,1)), "kind of set-associative I-cache (ideal, no, lruN, fifoN")
    ("ilsize",   boost::program_options::value<patmos::byte_size_t>()->default_value(0), "size of an I-cache line in bytes, defaults to burst size if set to 0")

    ("mcsize,m", boost::program_options::value<patmos::byte_size_t>()->default_value(patmos::NUM_METHOD_CACHE_BYTES), "method cache / instruction cache size in bytes")
    ("mckind,M", boost::program_options::value<patmos::method_cache_e>()->default_value(patmos::MC_IDEAL), "kind of method cache (ideal, lru, fifo)")
    ("mcmethods",boost::program_options::value<unsigned int>()->default_value(patmos::NUM_METHOD_CACHE_MAX_METHODS), "Maximum number of methods in the method cache, defaults to number of blocks if zero")
    ("mbsize",   boost::program_options::value<patmos::byte_size_t>()->default_value(patmos::NUM_METHOD_CACHE_BLOCK_BYTES), "method cache block size in bytes, defaults to burst size if zero");

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
    ("uart_offset", boost::program_options::value<patmos::address_t>()->default_value(patmos::UART_OFFSET), "offset where the UART device is mapped")
    ("led_offset", boost::program_options::value<patmos::address_t>()->default_value(patmos::LED_OFFSET), "offset where the LED device is mapped");
  
  boost::program_options::options_description uart_options("UART options");
  uart_options.add_options()
    ("in,I", boost::program_options::value<std::string>()->default_value("-"), "input file for UART simulation (stdin: -)")
    ("out,O", boost::program_options::value<std::string>()->default_value("-"), "output file for UART simulation (stdout: -)");

  boost::program_options::options_description interrupt_options("Interrupt options");
  interrupt_options.add_options()
    ("interrupt", boost::program_options::value<unsigned int>()->default_value(0), "enable interval interrupts");

  boost::program_options::positional_options_description pos;
  pos.add("binary", 1);

  boost::program_options::options_description cmdline_options;
  cmdline_options.add(generic_options).add(memory_options).add(cache_options)
                 .add(sim_options).add(uart_options).add(interrupt_options);

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
  std::string binary(vm["binary"].as<std::string>());
  std::string output(vm["output"].as<std::string>());

  std::string uart_in(vm["in"].as<std::string>());
  std::string uart_out(vm["out"].as<std::string>());

  std::string debug_out(vm["debug-file"].as<std::string>());

  unsigned int cores = vm["cores"].as<unsigned int>();
  unsigned int cpuid = vm["cpuid"].as<unsigned int>();
  double       freq = vm["freq"].as<double>();
  unsigned int mmbase = vm["mmbase"].as<patmos::address_t>().value();
  unsigned int mmhigh = vm["mmhigh"].as<patmos::address_t>().value();
  
  unsigned int cpuinfo_offset = vm["cpuinfo_offset"].as<patmos::address_t>().value();
  unsigned int excunit_offset = vm["excunit_offset"].as<patmos::address_t>().value();
  unsigned int timer_offset = vm["timer_offset"].as<patmos::address_t>().value();
  unsigned int uart_offset = vm["uart_offset"].as<patmos::address_t>().value();
  unsigned int led_offset = vm["led_offset"].as<patmos::address_t>().value();

  unsigned int gsize = vm["gsize"].as<patmos::byte_size_t>().value();
  unsigned int lsize = vm["lsize"].as<patmos::byte_size_t>().value();
  unsigned int dcsize = vm["dcsize"].as<patmos::byte_size_t>().value();
  unsigned int dlsize = vm["dlsize"].as<patmos::byte_size_t>().value();
  unsigned int scsize = vm["scsize"].as<patmos::byte_size_t>().value();
  unsigned int sbsize = vm["sbsize"].as<patmos::byte_size_t>().value();
  unsigned int mcsize = vm["mcsize"].as<patmos::byte_size_t>().value();
  unsigned int mbsize = vm["mbsize"].as<patmos::byte_size_t>().value();
  unsigned int ilsize = vm["ilsize"].as<patmos::byte_size_t>().value();
  unsigned int mcmethods= vm["mcmethods"].as<unsigned int>();

  unsigned int gtime = vm["gtime"].as<unsigned int>();
  unsigned int bsize = vm["bsize"].as<unsigned int>();
  unsigned int psize = vm["psize"].as<unsigned int>();
  unsigned int posted = vm["posted"].as<unsigned int>();
  unsigned int tdelay = vm["tdelay"].as<unsigned int>();
  unsigned int trefresh = vm["trefresh"].as<unsigned int>();

  patmos::set_assoc_cache_type dck = vm["dckind"].as<patmos::set_assoc_cache_type>();
  patmos::stack_cache_e sck = vm["sckind"].as<patmos::stack_cache_e>();
  patmos::instr_cache_e ick = vm["icache"].as<patmos::instr_cache_e>();
  patmos::method_cache_e mck = vm["mckind"].as<patmos::method_cache_e>();
  patmos::set_assoc_cache_type isck = vm["ickind"].as<patmos::set_assoc_cache_type>();

  patmos::debug_format_e debug_fmt= vm["debug-fmt"].as<patmos::debug_format_e>();
  bool debug_nopc = vm.count("debug-nopc") > 0;
  unsigned int debug_cycle = vm.count("debug") ?
                                       vm["debug"].as<unsigned int>() :
                                       std::numeric_limits<unsigned int>::max();
  unsigned int max_cycle = vm["maxc"].as<unsigned int>();

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
  
  unsigned int interrupt_enabled = vm["interrupt"].as<unsigned int>();

  bool slot_stats = (vm.count("slot-stats") != 0);
  bool instr_stats = (vm.count("instr-stats") != 0);

  if (!mbsize) mbsize = bsize;
  
  // the exit code, initialized by default to signal an error.
  int exit_code = -1;

  // input/output streams
  std::istream *in = NULL;
  std::istream *uin = NULL;

  std::ostream *out = NULL;
  std::ostream *uout = NULL;

  std::ostream *dout = NULL;

  // setup simulation framework
  patmos::memory_t &gm = create_global_memory(cores, cpuid, gsize, bsize, psize,
                                              posted, gtime, tdelay, trefresh);
  patmos::stack_cache_t &sc = create_stack_cache(sck, scsize, sbsize, gm);
  patmos::instr_cache_t &ic = create_instr_cache(ick, isck, mck, mcsize, 
                                                 ilsize ? ilsize : bsize, 
                                                 mbsize, mcmethods, gm);
  patmos::data_cache_t &dc = create_data_cache(dck, dcsize, 
                                               dlsize ? dlsize : bsize, gm);

  try
  {
    // open streams
    in = patmos::get_stream<std::ifstream>(binary, std::cin);
    out = patmos::get_stream<std::ofstream>(output, std::cout);

    uin = patmos::get_stream<std::ifstream>(uart_in, std::cin);
    uout = patmos::get_stream<std::ofstream>(uart_out, std::cout);

    dout = patmos::get_stream<std::ofstream>(debug_out, std::cerr);

    // check if the uart input stream is a tty.
    bool uin_istty = (uin == &std::cin) && isatty(STDIN_FILENO);

    assert(in && out && uin && uout && dout);

    // finalize simulation framework
    patmos::ideal_memory_t lm(lsize);
    patmos::memory_map_t mm(lm, mmbase, mmhigh);
    
    patmos::symbol_map_t sym;

    patmos::interrupt_handler_t interrupt_handler;

    patmos::simulator_t s(gm, mm, dc, ic, sc, sym, interrupt_handler);

    // set up timer device
    patmos::rtc_t rtc(mmbase+timer_offset, s, freq);
    if (interrupt_enabled) {
      interrupt_handler.enable_interrupts();
    }
    
    // setup IO mapped devices
    patmos::cpuinfo_t cpuinfo(mmbase+cpuinfo_offset, cpuid, freq);
    patmos::excunit_t excunit(mmbase+excunit_offset);
    patmos::uart_t uart(mmbase+uart_offset, *uin, uin_istty, *uout);
    patmos::led_t leds(mmbase+led_offset, *uout);

    mm.add_device(cpuinfo);
    mm.add_device(excunit);
    mm.add_device(uart);
    mm.add_device(leds);
    mm.add_device(rtc);

    // load input program
    patmos::section_list_t text;
    patmos::loader_t *loader = patmos::create_loader(*in);
    patmos::uword_t entry = loader->get_program_entry();
    
    if (!loader->is_ELF()) {
      // some output for compatibility
      std::cerr << boost::format("Loaded: %1% bytes\n") 
                   % loader->get_binary_size();
    }
    
    loader->load_symbols(sym, text);
    loader->load_to_memory(gm);
    
    // setup stats reset trigger
    if (print_stats) {
      print_stats_func.parse(sym);
      s.Dbg_stack.print_function_stats(print_stats_func.value(), *out);
    }
   
    if (flush_caches) {
      flush_caches_addr.parse(sym);
      s.flush_caches_at(flush_caches_addr.value());
    }
   
    // start execution
    try
    {
      s.run(entry, debug_cycle, debug_fmt, *dout, debug_nopc, 
            max_cycle, instr_stats);
      s.print_stats(*out, slot_stats, instr_stats);
    }
    catch (patmos::simulation_exception_t e)
    {
      switch(e.get_kind())
      {
        case patmos::simulation_exception_t::HALT:
          // get the exit code
          exit_code = e.get_info();

	  if (!vm.count("quiet") && !print_stats) {
            s.print_stats(*out, slot_stats, instr_stats);
	  }
          break;
        default:
          std::cerr << e.to_string(sym);
	  std::cerr << s.Dbg_stack;
      }
    }
  }
  catch(std::ios_base::failure f)
  {
    std::cerr << f.what() << "\n";
  }

  // free memory/cache instances
  // note: no need to free the local memory here.
  delete &gm;
  delete &dc;
  delete &ic;
  delete &sc;

  // free streams
  patmos::free_stream(in, std::cin);
  patmos::free_stream(out, std::cout);

  patmos::free_stream(uin, std::cin);
  patmos::free_stream(uout, std::cout);

  patmos::free_stream(dout, std::cerr);

  return exit_code;
}
