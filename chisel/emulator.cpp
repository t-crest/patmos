#include <fstream>
#include <iostream>

#include <stdio.h>
#include <unistd.h>
#include <gelf.h>
#include <libelf.h>

#include "Patmos.h"

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
    cout << "readelf: ELF file must be of kind ELF.\n";
    exit(EXIT_FAILURE);
  }

  // get elf header
  GElf_Ehdr hdr;
  GElf_Ehdr *tmphdr = gelf_getehdr(elf, &hdr);
  assert(tmphdr);

  if (hdr.e_machine != 0xBEEB) {
    cout << "readelf: unsupported architecture: ELF file is not a Patmos ELF file.\n";
    exit(EXIT_FAILURE);
  }
  
  // check class
  int ec = gelf_getclass(elf);
  if (ec != ELFCLASS32) {
    cout << "readelf: unsupported architecture: ELF file is not a 32bit Patmos ELF file.\n";
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

		if (((phdr.p_paddr + k) >> 21) == 0x1 && 
			((phdr.p_paddr + k) & 0x3) == 0) {
		  // Address maps to ISPM and is at a word boundary
		  val_t word = k >= phdr.p_filesz ? 0 :
			(((val_t)elfbuf[phdr.p_offset + k + 0] << 24) |
			 ((val_t)elfbuf[phdr.p_offset + k + 1] << 16) |
			 ((val_t)elfbuf[phdr.p_offset + k + 2] << 8) |
			 ((val_t)elfbuf[phdr.p_offset + k + 3] << 0));
		  val_t addr = ((phdr.p_paddr + k) - (0x1 << 21)) >> 3;

		  // Write to even or odd block
		  if (((phdr.p_paddr + k) & 0x4) == 0) {
			c->Patmos_fetch__memEven.put(addr, word);
		  } else {
			c->Patmos_fetch__memOdd.put(addr, word);
		  }
		}

		if (((phdr.p_paddr + k) >> 21) == 0x2) {
		  // Address maps to data SPM
		  val_t byte = k >= phdr.p_filesz ? 0 : elfbuf[phdr.p_offset + k];
		  val_t addr = ((phdr.p_paddr + k) - (0x2 << 21)) >> 2;
		  switch ((phdr.p_paddr + k) & 0x3) {
		  case 0: c->Patmos_memory_spm__mem0.put(addr, byte); break;
		  case 1: c->Patmos_memory_spm__mem1.put(addr, byte); break;
		  case 2: c->Patmos_memory_spm__mem2.put(addr, byte); break;
		  case 3: c->Patmos_memory_spm__mem3.put(addr, byte); break;
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
	cout << (pc - 2) << " - ";

	sval_t rf [32];
	rf[0] = c->Patmos_decode_rf__rf_0.to_ulong();
	rf[1] = c->Patmos_decode_rf__rf_1.to_ulong();
	rf[2] = c->Patmos_decode_rf__rf_2.to_ulong();
	rf[3] = c->Patmos_decode_rf__rf_3.to_ulong();
	rf[4] = c->Patmos_decode_rf__rf_4.to_ulong();
	rf[5] = c->Patmos_decode_rf__rf_5.to_ulong();
	rf[6] = c->Patmos_decode_rf__rf_6.to_ulong();
	rf[7] = c->Patmos_decode_rf__rf_7.to_ulong();
	rf[8] = c->Patmos_decode_rf__rf_8.to_ulong();
	rf[9] = c->Patmos_decode_rf__rf_9.to_ulong();
	rf[10] = c->Patmos_decode_rf__rf_10.to_ulong();
	rf[11] = c->Patmos_decode_rf__rf_11.to_ulong();
	rf[12] = c->Patmos_decode_rf__rf_12.to_ulong();
	rf[13] = c->Patmos_decode_rf__rf_13.to_ulong();
	rf[14] = c->Patmos_decode_rf__rf_14.to_ulong();
	rf[15] = c->Patmos_decode_rf__rf_15.to_ulong();
	rf[16] = c->Patmos_decode_rf__rf_16.to_ulong();
	rf[17] = c->Patmos_decode_rf__rf_17.to_ulong();
	rf[18] = c->Patmos_decode_rf__rf_18.to_ulong();
	rf[19] = c->Patmos_decode_rf__rf_19.to_ulong();
	rf[20] = c->Patmos_decode_rf__rf_20.to_ulong();
	rf[21] = c->Patmos_decode_rf__rf_21.to_ulong();
	rf[22] = c->Patmos_decode_rf__rf_22.to_ulong();
	rf[23] = c->Patmos_decode_rf__rf_23.to_ulong();
	rf[24] = c->Patmos_decode_rf__rf_24.to_ulong();
	rf[25] = c->Patmos_decode_rf__rf_25.to_ulong();
	rf[26] = c->Patmos_decode_rf__rf_26.to_ulong();
	rf[27] = c->Patmos_decode_rf__rf_27.to_ulong();
	rf[28] = c->Patmos_decode_rf__rf_28.to_ulong();
	rf[29] = c->Patmos_decode_rf__rf_29.to_ulong();
	rf[30] = c->Patmos_decode_rf__rf_30.to_ulong();
	rf[31] = c->Patmos_decode_rf__rf_31.to_ulong();

	for (int i = 0; i < 32; i++) {
	  cout << rf[i] << " ";
	}

	cout << endl;
}

int main (int argc, char* argv[]) {
  Patmos_t* c = new Patmos_t();

  int opt;
  int lim = -1;
  bool vcd = false;
  bool quiet = false;
  
  while ((opt = getopt(argc, argv, "qvl:")) != -1) {
	switch (opt) {
	case 'q':
	  quiet = true;
	  break;
	case 'v':
	  vcd = true;
	  break;
	case 'l':
	  lim = atoi(optarg);
	  break;
	default: /* '?' */
	  cerr << "Usage: " << argv[0] << "[-l cycles] [file]" << endl;
	  exit(EXIT_FAILURE);
	}
  }

  c->init();

  val_t entry = 0;
  if (optind < argc) {
	ifstream fs;
	fs.open(argv[optind]);
	entry = readelf(fs, c);
  }

  FILE *f = vcd ? fopen("Patmos.vcd", "w") : NULL;
  
  if (!quiet) {
	cout << "Patmos start" << endl;
  }

  // Assert reset for a few cycles
  for(int i = 0; i < 5; i++) {
    dat_t<1> reset = LIT<1>(1);
    c->clock_lo(reset);
    c->clock_hi(reset);
  }

  if (entry != 0) {
	c->Patmos_fetch__pc = (entry >> 2) - 1;
	c->Patmos_memory__baseReg = entry;
  }

  // Main emulation loop
  bool halt = false;
  for (int t = 0; lim < 0 || t < lim; t++) {
    dat_t<1> reset = LIT<1>(0);

    c->clock_lo(reset);
    c->clock_hi(reset);

	if (vcd) {
	  c->dump(f, t);
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
	cout << "PASSED" << endl;
  }

  // Pass on return value from processor
  return c->Patmos_decode_rf__rf_1.to_ulong();
}
