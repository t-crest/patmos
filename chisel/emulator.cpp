#include <fstream>
#include <iostream>

#include <stdio.h>
#include <unistd.h>
#include <gelf.h>
#include <libelf.h>

#include "Patmos.h"

istream *in = &cin;
ostream *out = &cout;

static uint ssram_buf[524288]; //2MB
  
/// Read an elf executable image into the on-chip memories
static val_t readelf(istream &is, Patmos_t *c)
{
  vector<unsigned char> elfbuf;
  elfbuf.reserve(1 << 20);

  // read the whole stream.
  while (!is.eof())
  {
	char buf[1024];

    // read into buffer
    is.read(&buf[0], sizeof(buf));

    // check how much was read
    streamsize count = is.gcount();
    assert(count <= 1024);

    // write into main memory
    for(unsigned i = 0; i < count; i++)
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
    cerr << "readelf: ELF file must be of kind ELF.\n";
    exit(EXIT_FAILURE);
  }

  // get elf header
  GElf_Ehdr hdr;
  GElf_Ehdr *tmphdr = gelf_getehdr(elf, &hdr);
  assert(tmphdr);

  if (hdr.e_machine != 0xBEEB) {
    cerr << "readelf: unsupported architecture: ELF file is not a Patmos ELF file.\n";
    exit(EXIT_FAILURE);
  }
  
  // check class
  int ec = gelf_getclass(elf);
  if (ec != ELFCLASS32) {
    cerr << "readelf: unsupported architecture: ELF file is not a 32bit Patmos ELF file.\n";
    exit(EXIT_FAILURE);
  }

  // get program headers
  size_t n;
  int ntmp = elf_getphdrnum (elf, &n);
  assert(ntmp == 0);

  for(size_t i = 0; i < n; i++) {
    // get program header
    GElf_Phdr phdr;
    GElf_Phdr *phdrtmp = gelf_getphdr(elf, i, &phdr);
    assert(phdrtmp);

    if (phdr.p_type == PT_LOAD) {
      // some assertions
      assert(phdr.p_vaddr == phdr.p_paddr);
      assert(phdr.p_filesz <= phdr.p_memsz);

      // copy from the buffer into the on-chip memories
	  for (size_t k = 0; k < phdr.p_memsz; k++) {

	    //if (((phdr.p_paddr + k) >> 23) == 0x1 && 
	      if (((phdr.p_paddr + k) >> 16) == 0x1 && 
			((phdr.p_paddr + k) & 0x3) == 0) {
		  // Address maps to ISPM and is at a word boundary
		  val_t word = k >= phdr.p_filesz ? 0 :
			(((val_t)elfbuf[phdr.p_offset + k + 0] << 24) |
			 ((val_t)elfbuf[phdr.p_offset + k + 1] << 16) |
			 ((val_t)elfbuf[phdr.p_offset + k + 2] << 8) |
			 ((val_t)elfbuf[phdr.p_offset + k + 3] << 0));
		  // val_t addr = ((phdr.p_paddr + k) - (0x1 << 23)) >> 3;
		   val_t addr = ((phdr.p_paddr + k) - (0x1 << 16)) >> 3;

		  unsigned size = (sizeof(c->Patmos_fetch__memEven.contents) / 
						   sizeof(c->Patmos_fetch__memEven.contents[0]));
		  assert(addr < size && "Instructions mapped to ISPM exceed size");

		  // Write to even or odd block
		  if (((phdr.p_paddr + k) & 0x4) == 0) {
			c->Patmos_fetch__memEven.put(addr, word);
		  } else {
			c->Patmos_fetch__memOdd.put(addr, word);
		  }
		}

		if (((phdr.p_paddr + k) & 0x3) == 0) {
		  val_t word = k >= phdr.p_filesz ? 0 :
			(((val_t)elfbuf[phdr.p_offset + k + 0] << 24) |
			 ((val_t)elfbuf[phdr.p_offset + k + 1] << 16) |
			 ((val_t)elfbuf[phdr.p_offset + k + 2] << 8) |
			 ((val_t)elfbuf[phdr.p_offset + k + 3] << 0));

		  val_t addr = ((phdr.p_paddr + k) >> 2);
		  ssram_buf[addr] = word;
		}

		// if (((phdr.p_paddr + k) >> 17) >= 0x1 && ((phdr.p_paddr + k) & 0x3) == 0) {
		//   val_t word = k >= phdr.p_filesz ? 0 :
		// 	(((val_t)elfbuf[phdr.p_offset + k + 0] << 24) |
		// 	 ((val_t)elfbuf[phdr.p_offset + k + 1] << 16) |
		// 	 ((val_t)elfbuf[phdr.p_offset + k + 2] << 8) |
		// 	 ((val_t)elfbuf[phdr.p_offset + k + 3] << 0));

		//   val_t addr = ((phdr.p_paddr + k) - (0x1 << 17)) >> 2;
		//   //*out << k << "MCache - Word:" << word << "addr:" << addr << "phdr_memsz" << phdr.p_memsz << "phdr_addr"<< phdr.p_paddr << "\n";;
		//   ssram_buf[addr] = word;

		// }

		if (((phdr.p_paddr + k) >> 16) == 0x0) {
		  // Address maps to data SPM
		  val_t byte = k >= phdr.p_filesz ? 0 : elfbuf[phdr.p_offset + k];
		  val_t addr = (phdr.p_paddr + k) >> 2;
		  
		  unsigned size = (sizeof(c->Patmos_globMem__mem0.contents) /
						   sizeof(c->Patmos_globMem__mem0.contents[0]));


		  assert (addr < size && "Data mapped to DSPM exceed size");

		  switch ((phdr.p_paddr + k) & 0x3) {
		  case 0: c->Patmos_globMem__mem3.put(addr, byte); break;
		  case 1: c->Patmos_globMem__mem2.put(addr, byte); break;
		  case 2: c->Patmos_globMem__mem1.put(addr, byte); break;
		  case 3: c->Patmos_globMem__mem0.put(addr, byte); break;
		  }
		}

	  }
    }
  }

  // get entry point
  val_t entry = hdr.e_entry;

  elf_end(elf);

  return entry;
}

static void print_state(Patmos_t *c) {
	sval_t pc = c->Patmos_memory__io_memwb_pc.to_ulong();
	*out << (pc - 2) << " - ";

	for (unsigned i = 0; i < 32; i++) {
	  *out << c->Patmos_decode_rf__rf.get(i).to_ulong() << " ";
	}

	*out << endl;
}

static void extSsramSim(Patmos_t *c) {
  static int addr_cnt;
  static int address;
  static int counter;
  //*out << "noe:" << c->Patmos__io_sramPins_ram_out_noe.to_ulong() << " nadv: " << c->Patmos__io_sramPins_ram_out_nadv.to_ulong() << " nadsc:" << c->Patmos__io_sramPins_ram_out_nadsc.to_ulong() << " addr:" << c->Patmos__io_sramPins_ram_out_addr.to_ulong() << "\n";
  if (c->Patmos__io_sramPins_ram_out_nadsc.to_ulong() == 0) {
    address = c->Patmos__io_sramPins_ram_out_addr.to_ulong();
    addr_cnt = c->Patmos__io_sramPins_ram_out_addr.to_ulong();
    counter = 0;
  }
  if (c->Patmos__io_sramPins_ram_out_nadv.to_ulong() == 0) {
    addr_cnt++;
  }
  if (c->Patmos__io_sramPins_ram_out_noe.to_ulong() == 0) {
    counter++;
    if (counter >= 3) {
      c->Patmos__io_sramPins_ram_in_din = ssram_buf[address];
      if (address <= addr_cnt) {
	address++;
      }
    }
  }
}

int main (int argc, char* argv[]) {
  Patmos_t* c = new Patmos_t();

  int opt;
  int lim = -1;
  bool vcd = false;
  bool uart = false;
  bool quiet = false;

  while ((opt = getopt(argc, argv, "quvl:I:O:")) != -1) {
	switch (opt) {
	case 'q':
	  quiet = true;
	  break;
	case 'u':
	  uart = true;
	  break;
	case 'v':
	  vcd = true;
	  break;
	case 'l':
	  lim = atoi(optarg);
	  break;
	case 'I':
	  if (strcmp(optarg, "-") == 0) {
		in = &cin;
	  } else {
		in = new ifstream(optarg);
		if (!in->good()) {
		  cerr << argv[0] << "error: Cannot open input file " << optarg << endl;
		  exit(EXIT_FAILURE);
		}
	  }
	  break;
	case 'O':
	  if (strcmp(optarg, "-") == 0) {
		out = &cout;
	  } else {
		out = new ofstream(optarg);
		if (!out->good()) {
		  cerr << argv[0] << ": error: Cannot open output file " << optarg << endl;
		  exit(EXIT_FAILURE);
		}
	  }
	  break;
	default: /* '?' */
	  cerr << "Usage: " << argv[0]
		   << "[-q] [-u] [-v] [-l cycles] [-I file] [-O file] [file]" << endl;
	  exit(EXIT_FAILURE);
	}
  }

  c->init();

  val_t entry = 0;
  if (optind < argc) {
	ifstream *fs = new ifstream(argv[optind]);
	if (!fs->good()) {
	  cerr << argv[0] << ": error: Cannot open elf file " << argv[optind] << endl;
	  exit(EXIT_FAILURE);
	}
	entry = readelf(*fs, c);
  }

  FILE *f = vcd ? fopen("Patmos.vcd", "w") : NULL;
  
  if (!quiet) {
	*out << "Patmos start" << endl;
  }

  // Assert reset for a few cycles
  for(int i = 0; i < 5; i++) {
    dat_t<1> reset = LIT<1>(1);
    c->clock_lo(reset);
    c->clock_hi(reset);
  }

  if (entry != 0) {
    if (entry >= 0x20000) {
      c->Patmos_fetch__pcReg = 0; //pcReg for method cache starts at 0
    }
    else {
      c->Patmos_fetch__pcReg = ((entry - 0x10000) >> 2) - 1; //pcReg for ispm starts at entry point - ispm base
    }
    c->Patmos_fetch__baseReg = (entry >> 2); //base address = entry point address
    //c->Patmos_memory__baseReg = entry;
    //*out << "baseReg: " << (entry >> 2) << "\n";
    // for (int i = 32768; i < 32868; i++) {
    //   *out << "sram[i] " << i << " " << ssram_buf[i] << "\n";
    // }

  }

  // Main emulation loop
  bool halt = false;
  for (int t = 0; lim < 0 || t < lim; t++) {
    dat_t<1> reset = LIT<1>(0);

    c->clock_lo(reset);
    c->clock_hi(reset);

    extSsramSim(c);

	if (vcd) {
	  c->dump(f, t);
	}

	if (uart) {
	  // Pass on data from UART
	  if (c->Patmos_iocomp_uart__io_ocp_M_Cmd.to_ulong() == 0x5
	  	  && c->Patmos_iocomp_uart__io_ocp_M_Addr.to_ulong() == 0x04) {
	  	*out << (char)c->Patmos_iocomp_uart__io_ocp_M_Data.to_ulong();
	  }
	}

	if (!quiet) {
	  print_state(c);
	}
	
	// Return to address 0 halts the execution after one more iteration
	if (halt) {
	  break;
	}
	if (c->Patmos_memory__memReg_mem_ret.to_bool()
		&& c->Patmos_memory__memReg_mem_callRetBase.to_ulong() == 0) {
	  halt = true;
	}
  }

  // TODO: adapt comparison tool so this can be removed
  if (!quiet) {
	*out << "PASSED" << endl;
  }

  // Pass on return value from processor
  return c->Patmos_decode_rf__rf.get(1).to_ulong();
}
