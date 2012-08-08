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
#include "data-cache.h"
#include "instruction.h"
#include "method-cache.h"
#include "simulation-core.h"
#include "stack-cache.h"
#include "streams.h"
#include "symbol.h"
#include "uart.h"

#include <gelf.h>
#include <libelf.h>

#include <unistd.h>
#include <termios.h>

#include <fstream>
#include <iostream>

#include <boost/program_options.hpp>

/// Test whether the file is an elf executable image or a raw binary stream.
/// @param is The input stream to test.
/// @return True, in case the file appears to be an elf executable image, false
/// otherwise.
static bool is_elf(std::istream &is)
{
  char data[4];

  for(unsigned int i = 0; i < 4; i++)
  {
    data[i] = is.get();
  }

  bool result = (data[0] == '\177') && (data[1] == 'E') &&
                (data[2] == 'L') && (data[3] == 'F');

  for(unsigned int i = 0; i < 4; i++)
  {
    is.putback(data[3 - i]);
  }

  return result;
}

/// Read an elf executable image into the simulator's main memory.
/// @param is The input stream to read from.
/// @param m The main memory to load to.
/// @param msize Maximal number of bytes to load.
/// @param symbols Map to store symbol information, if available.
/// @return The entry point of the elf executable.
static patmos::uword_t readelf(std::istream &is, patmos::memory_t &m,
                               unsigned int msize,
                               patmos::symbol_map_t &symbols)
{
  std::vector<char> elfbuf;
  elfbuf.reserve(1 << 20);
  // read the whole stream.
  while (!is.eof())
  {
    char buf[128];

    // read into buffer
    is.read(&buf[0], sizeof(buf));

    // check how much was read
    std::streamsize count = is.gcount();
    assert(count <= 128);

    // write into main memory
    for(unsigned int i = 0; i < count; i++)
      elfbuf.push_back(buf[i]);
  }

  // check libelf version
  elf_version(EV_CURRENT);

  // open elf binary
  Elf *elf = elf_memory((char*)&elfbuf[0], elfbuf.size());
  assert(elf);

  // check file kind
  Elf_Kind ek = elf_kind(elf);
  if (ek != ELF_K_ELF) {
    std::cout << "readelf: ELF file must be of kind ELF.\n";
    exit(1);
  }

  // get elf header
  GElf_Ehdr hdr;
  GElf_Ehdr *tmphdr = gelf_getehdr(elf, &hdr);
  assert(tmphdr);

  if (hdr.e_machine != 0xBEEB) {
    std::cout << "readelf: unsupported architecture: ELF file is not a Patmos ELF file.\n";
    exit(1);
  }
  
  // check class
  int ec = gelf_getclass(elf);
  if (ec != ELFCLASS32) {
    std::cout << "readelf: unsupported architecture: ELF file is not a 32bit Patmos ELF file.\n";
    exit(1);
  }

  // get program headers
  size_t n;
  int ntmp = elf_getphdrnum (elf, &n);
  assert(ntmp == 0);

  for(size_t i = 0; i < n; i++)
  {
    // get program header
    GElf_Phdr phdr;
    GElf_Phdr *phdrtmp = gelf_getphdr(elf, i, &phdr);
    assert(phdrtmp);

    if (phdr.p_type == PT_LOAD)
    {
      // some assertions
      assert(phdr.p_vaddr == phdr.p_paddr);
      assert(phdr.p_filesz <= phdr.p_memsz);
      assert(phdr.p_paddr + phdr.p_memsz <= msize);

      // copy from the buffer into the main memory
      m.write_peek(phdr.p_paddr,
                   reinterpret_cast<patmos::byte_t*>(&elfbuf[phdr.p_offset]),
                   phdr.p_filesz);
    }
  }

  // get sections
  ntmp = elf_getshdrnum(elf, &n);
  assert(ntmp == 0);

  // read symbol information
  for(size_t i = 0; i < n; i++)
  {
    Elf_Scn *sec =  elf_getscn (elf, i);
    assert(sec);

    // get section header
    GElf_Shdr shdr;
    gelf_getshdr(sec, &shdr);
    GElf_Shdr *shdrtmp = gelf_getshdr(sec, &shdr);
    assert(shdrtmp);

    if (shdr.sh_type == SHT_SYMTAB)
    {
      int num_entries = shdr.sh_size/shdr.sh_entsize;
      Elf_Data *data = elf_getdata(sec, NULL);
      assert(data);

      for(int j = 0; j != num_entries; j++)
      {
        GElf_Sym sym;
        GElf_Sym *tmpsym = gelf_getsym(data, j, &sym);
        assert(tmpsym);
        char *name = elf_strptr(elf, shdr.sh_link, sym.st_name);
        assert(name);

        // construct a symbol and store it for later use, i.e., the symbol map
        // is queried during simulation to find symbol names associated with
        // addresses.
        patmos::symbol_info_t sym_info(sym.st_value, sym.st_size, name);
        symbols.add(sym_info);
      }
    }
  }

  // ensure that the symbol map is sorted.
  symbols.sort();

  // get entry point
  patmos::uword_t entry = hdr.e_entry;

  elf_end(elf);

  return entry;
}

/// Read a raw binary image into the simulator's main memory.
/// @param is The input stream to read from.
/// @param m The main memory to load to.
/// @param msize Maximal number of bytes to load.
static void readbin(std::istream &is, patmos::memory_t &m, unsigned int msize)
{
  std::streamsize offset = 0;
  while (!is.eof())
  {
    patmos::byte_t buf[128];

    // read into buffer
    is.read(reinterpret_cast<char*>(&buf[0]), sizeof(buf));

    // check how much was read
    std::streamsize count = is.gcount();
    assert((count <= 128) && (offset + count < msize));

    // write into main memory
    m.write_peek(offset, buf, count);

    offset += count;
  }

  // some output
  std::cerr << boost::format("Loaded: %1% bytes\n") % offset;
}

/// Construct a global memory for the simulation.
/// @param time Access time in cycles for memory accesses.
/// @param size The requested size of the memory in bytes.
/// @return An instance of the requested memory.
static patmos::memory_t &create_global_memory(unsigned int time,
                                              unsigned int size)
{
  if (time == 0)
    return *new patmos::ideal_memory_t(size);
  else
    return *new patmos::fixed_delay_memory_t<>(size, time);
}

/// Construct a data cache for the simulation.
/// @param size The requested size of the data cache in bytes.
/// @param gm Global memory accessed on a cache miss.
/// @return An instance of a data cache.
static patmos::data_cache_t &create_data_cache(unsigned int size,
                                               patmos::memory_t &gm)
{
  return *new patmos::ideal_data_cache_t(gm);
}

/// Construct a method cache for the simulation.
/// @param mck The kind of the method cache requested.
/// @param size The requested size of the method cache in bytes.
/// @param gm Global memory accessed on a cache miss.
/// @return An instance of the requested method  cache kind.
static patmos::method_cache_t &create_method_cache(patmos::method_cache_e mck,
                                                 unsigned int size,
                                                 patmos::memory_t &gm)
{
  switch(mck)
  {
    case patmos::MC_IDEAL:
      return *new patmos::ideal_method_cache_t(gm);
    case patmos::MC_LRU:
    {
      // convert size to number of blocks
      unsigned int num_blocks = std::ceil((float)size/
                                   (float)patmos::NUM_METHOD_CACHE_BLOCK_BYTES);

      return *new patmos::lru_method_cache_t<>(gm, num_blocks);
    }
  }

  assert(false);
  abort();
}

/// Construct a stack cache for the simulation.
/// @param sck The kind of the stack cache requested.
/// @param size The requested size of the stack cache in bytes.
/// @param gm Global memory accessed on stack cache fills/spills.
/// @return An instance of the requested stack cache kind.
static patmos::stack_cache_t &create_stack_cache(patmos::stack_cache_e sck,
                                                 unsigned int size,
                                                 patmos::memory_t &gm)
{
  switch(sck)
  {
    case patmos::SC_IDEAL:
      return *new patmos::ideal_stack_cache_t();
    case patmos::SC_BLOCK:
    {
      // convert size to number of blocks
      unsigned int num_blocks = std::ceil((float)size/
                                    (float)patmos::NUM_STACK_CACHE_BLOCK_BYTES);

      return *new patmos::block_stack_cache_t<>(gm, num_blocks,
                                          patmos::NUM_STACK_CACHE_TOTAL_BLOCKS);
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
    ("debug", boost::program_options::value<unsigned int>()->implicit_value(0), "enable step-by-step debug tracing after cycle");

  boost::program_options::options_description memory_options("Memory options");
  memory_options.add_options()
    ("gsize,g", boost::program_options::value<patmos::byte_size_t>()->default_value(patmos::NUM_MEMORY_BYTES), "global memory size in bytes")
    ("gtime,G", boost::program_options::value<unsigned int>()->default_value(0), "access delay to global memory in cycles")
    ("lsize,l", boost::program_options::value<patmos::byte_size_t>()->default_value(patmos::NUM_LOCAL_MEMORY_BYTES), "local memory size in bytes");

  boost::program_options::options_description cache_options("Cache options");
  cache_options.add_options()
    ("dcsize,d", boost::program_options::value<patmos::byte_size_t>()->default_value(patmos::NUM_DATA_CACHE_BYTES), "data cache size in bytes")

    ("scsize,s", boost::program_options::value<patmos::byte_size_t>()->default_value(patmos::NUM_STACK_CACHE_BYTES), "stack cache size in bytes")
    ("sckind,S", boost::program_options::value<patmos::stack_cache_e>()->default_value(patmos::SC_IDEAL), "kind of method cache (ideal, block)")

    ("mcsize,m", boost::program_options::value<patmos::byte_size_t>()->default_value(patmos::NUM_METHOD_CACHE_BYTES), "method cache size in bytes")
    ("mckind,M", boost::program_options::value<patmos::method_cache_e>()->default_value(patmos::MC_IDEAL), "kind of method cache (ideal, lru)");

  boost::program_options::options_description uart_options("UART options");
  uart_options.add_options()
    ("in,I", boost::program_options::value<std::string>()->default_value("-"), "input file for UART simulation (stdin: -)")
    ("out,O", boost::program_options::value<std::string>()->default_value("-"), "output file for UART simulation (stdout: -)")
    ("ustatus", boost::program_options::value<unsigned int>()->default_value(patmos::UART_STATUS_ADDRESS), "address where the UART's status register is mapped")
    ("udata", boost::program_options::value<unsigned int>()->default_value(patmos::UART_DATA_ADDRESS), "address where the UART's data register is mapped");

  boost::program_options::positional_options_description pos;
  pos.add("binary", 1);

  boost::program_options::options_description cmdline_options;
  cmdline_options.add(generic_options).add(memory_options).add(cache_options)
                 .add(uart_options);

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

  unsigned int ustatus = vm["ustatus"].as<unsigned int>();
  unsigned int udata = vm["udata"].as<unsigned int>();

  unsigned int gsize = vm["gsize"].as<patmos::byte_size_t>().value();
  unsigned int lsize = vm["lsize"].as<patmos::byte_size_t>().value();
  unsigned int dcsize = vm["dcsize"].as<patmos::byte_size_t>().value();
  unsigned int scsize = vm["scsize"].as<patmos::byte_size_t>().value();
  unsigned int mcsize = vm["mcsize"].as<patmos::byte_size_t>().value();

  unsigned int gtime = vm["gtime"].as<unsigned int>();

  patmos::stack_cache_e sck = vm["sckind"].as<patmos::stack_cache_e>();
  patmos::method_cache_e mck = vm["mckind"].as<patmos::method_cache_e>();

  unsigned int debug_cycle = vm.count("debug") ?
                                       vm["debug"].as<unsigned int>() :
                                       std::numeric_limits<unsigned int>::max();
  unsigned int max_cycle = vm["maxc"].as<unsigned int>();

  // the exit code, initialized by default to signal an error.
  int exit_code = -1;

  // input/output streams
  std::istream *in = NULL;
  std::istream *uin = NULL;

  std::ostream *out = NULL;
  std::ostream *uout = NULL;

  // setup simulation framework
  patmos::memory_t &gm = create_global_memory(gtime, gsize);
  patmos::stack_cache_t &sc = create_stack_cache(sck, scsize, gm);
  patmos::method_cache_t &mc = create_method_cache(mck, mcsize, gm);
  patmos::data_cache_t &dc = create_data_cache(dcsize, gm);

  try
  {
    // open streams
    in = patmos::get_stream<std::ifstream>(binary, std::cin);
    out = patmos::get_stream<std::ofstream>(output, std::cout);

    uin = patmos::get_stream<std::ifstream>(uart_in, std::cin);
    uout = patmos::get_stream<std::ofstream>(uart_out, std::cout);

    // check if the uart input stream is a tty.
    bool uin_istty = (uin == &std::cin) && isatty(STDIN_FILENO);

    assert(in && out && uin && uout);

    // finalize simulation framework
    patmos::ideal_memory_t lm(lsize);
    patmos::uart_t uart(lm, ustatus, udata, *uin, uin_istty, *uout);
    patmos::symbol_map_t sym;

    patmos::simulator_t s(gm, uart, dc, mc, sc, sym);

    // load input program
    patmos::uword_t entry = 0;
    if (is_elf(*in))
      entry = readelf(*in, gm, gsize, sym);
    else
      readbin(*in, gm, gsize);

    // start execution
    try
    {
      s.run(entry, debug_cycle, max_cycle);
      s.print_stats(*out);
    }
    catch (patmos::simulation_exception_t e)
    {
      switch(e.get_kind())
      {
        case patmos::simulation_exception_t::CODE_EXCEEDED:
          std::cerr << boost::format("Cycle %1%: Method cache size exceeded: "
                                    "%2$08x%3%: %4$08x\n")
                    % e.get_cycle() % e.get_pc() % sym.find(e.get_pc())
                    % e.get_info();
          break;
        case patmos::simulation_exception_t::STACK_EXCEEDED:
          std::cerr << boost::format("Cycle %1%: Stack size exceeded: "
                                     "%2$08x%3%\n")
                    % e.get_cycle() % e.get_pc() % sym.find(e.get_pc());
          break;
        case patmos::simulation_exception_t::UNMAPPED:
          std::cerr << boost::format("Cycle %1%: Unmapped memory access: "
                                     "%2$08x%3%: %4$08x\n")
                    % e.get_cycle() % e.get_pc() % sym.find(e.get_pc())
                    % e.get_info();
          break;
        case patmos::simulation_exception_t::ILLEGAL:
          std::cerr << boost::format("Cycle %1%: Illegal instruction: "
                                     "%2$08x%3%: %4$08x\n")
                    % e.get_cycle() % e.get_pc() % sym.find(e.get_pc())
                    % e.get_info();
          break;
        case patmos::simulation_exception_t::UNALIGNED:
          std::cerr << boost::format("Cycle %1%: Unaligned memory access: "
                                     "%2$08x%3%: %4$08x\n")
                    % e.get_cycle() % e.get_pc() % sym.find(e.get_pc())
                    % e.get_info();
          break;
        case patmos::simulation_exception_t::HALT:
          // get the exit code
          exit_code = e.get_info();
          s.print_stats(*out);
          break;
        default:
          std::cerr << "Unknown simulation error.\n";
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
  delete &mc;
  delete &sc;

  // free streams
  patmos::free_stream(in, std::cin);
  patmos::free_stream(out, std::cout);

  patmos::free_stream(uin, std::cin);
  patmos::free_stream(uout, std::cout);

  return exit_code;
}
