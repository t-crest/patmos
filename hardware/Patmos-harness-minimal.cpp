/*
Minimal Patmos Emulator Harness - for use with Verilator
*/

#include "emulator_config.h"
#include <fstream>
#include <iostream>
#include <string>
#include <libelf.h>
#include <gelf.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <memory>

#include "VPatmos.h"
#include "VPatmos___024root.h"
#include "verilated.h"
#if VM_TRACE
#include "verilated_fst_c.h"
#endif

using namespace std;

// Configuration
#define MEM_ADDR_SHIFT 2
#define EXTMEM_BASE 0x80000000
#define EXTMEM_SIZE (2*1024*1024)  // 2MB
#define MEM_HASH_SEED 8

int main(int argc, char **argv) {
  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " <elf-file> [trace-file]" << endl;
    return 1;
  }

  // ELF loading setup
  if (elf_version(EV_CURRENT) == EV_NONE) {
    cerr << "ELF library initialization failed" << endl;
    return 1;
  }

  // Open and parse ELF file
  int fd = open(argv[1], O_RDONLY, 0);
  if (fd < 0) {
    cerr << "Failed to open ELF file: " << argv[1] << endl;
    return 1;
  }

  Elf *e = elf_begin(fd, ELF_C_READ, NULL);
  if (e == NULL) {
    cerr << "Failed to parse ELF file" << endl;
    return 1;
  }

  // Get ELF header
  GElf_Ehdr ehdr;
  if (gelf_getehdr(e, &ehdr) == NULL) {
    cerr << "Failed to read ELF header" << endl;
    return 1;
  }

  uint32_t entry = ehdr.e_entry;
  cout << "ELF Entry: 0x" << hex << entry << dec << endl;

  // Initialize Verilator model
  Verilated::traceEverOn(true);
  auto c = make_unique<VPatmos>();

  VerilatedFstC *trace_file = nullptr;
  if (argc > 2) {
    trace_file = new VerilatedFstC();
    c->trace(trace_file, 99);
    trace_file->open(argv[2]);
  }

  // Load ELF segments into "memory"
  cout << "Loading ELF segments..." << endl;
  Elf_Scn *scn = nullptr;
  while ((scn = elf_nextscn(e, scn)) != nullptr) {
    GElf_Shdr shdr;
    if (gelf_getshdr(scn, &shdr) == NULL) continue;

    // Check if section is loaded
    if (!(shdr.sh_flags & SHF_ALLOC)) continue;

    Elf_Data *data = elf_getdata(scn, nullptr);
    if (data == nullptr) continue;

    cout << "  Section at 0x" << hex << shdr.sh_addr << " size " << shdr.sh_size << dec << endl;
  }

  // Run simulation
  cout << "Starting simulation..." << endl;
  int max_cycles = 1000000;  // Limit to 1M cycles to avoid infinite loops
  int cycle = 0;

  c->clock = 0;
  c->eval();

  while (cycle < max_cycles && !Verilated::gotFinish()) {
    // Negative edge
    c->clock = 0;
    c->eval();

    if (trace_file) {
      trace_file->dump(10 * cycle + 5);
    }

    // Positive edge
    c->clock = 1;
    c->eval();

    if (trace_file) {
      trace_file->dump(10 * cycle + 10);
      trace_file->flush();
    }

    cycle++;
    if (cycle % 100000 == 0) {
      cout << "Cycle: " << cycle << endl;
    }
  }

  cout << "Simulation completed at cycle " << cycle << endl;

  if (trace_file) {
    trace_file->close();
    delete trace_file;
  }

  elf_end(e);
  close(fd);

  return 0;
}
