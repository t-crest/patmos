/*
   Copyright 2015 Technical University of Denmark, DTU Compute.
   All rights reserved.

   This file is part of the time-predictable VLIW processor Patmos.

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

#include <fstream>
#include <iostream>

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <gelf.h>
#include <libelf.h>

#include "Patmos.h"
#include "emulator_config.h"

ostream *out = &cout;
char *program_name = NULL;

#define OCMEM_ADDR_BITS 16

#ifdef EXTMEM_SSRAM32CTRL
static uint32_t *ram_buf;
#define SRAM_CYCLES 3

static void write_extmem(val_t address, val_t word) {
  ram_buf[address] = word;
}

static void init_extmem(Patmos_t *c, bool random) {
  // Get SRAM properties
  uint32_t addr_bits = c->Patmos__io_sSRam32CtrlPins_ramOut_addr.width();
  uint32_t cells = 1 << addr_bits;

  // Check data width and allocate buffer
  assert(c->Patmos__io_sSRam32CtrlPins_ramOut_dout.width() == 32);
  ram_buf = (uint32_t *)calloc(cells, sizeof(uint32_t));
  if (ram_buf == NULL) {
    cerr << program_name << ": error: Cannot allocate memory for SRAM emulation" << endl;
    exit(EXIT_FAILURE);
  }

  // Initialize with random data
  if (random) {
    for (int i = 0; i < cells; i++) {
      write_extmem(i, rand());
    }
  }
}

static void emu_extmem(Patmos_t *c) {
  static uint32_t addr_cnt;
  static uint32_t address;
  static uint32_t counter;

  // Start of request
  if (!c->Patmos__io_sSRam32CtrlPins_ramOut_nadsc.to_bool()) {
    address = c->Patmos__io_sSRam32CtrlPins_ramOut_addr.to_ulong();
    addr_cnt = address;
    counter = 0;
  }

  // Advance address for burst
  if (!c->Patmos__io_sSRam32CtrlPins_ramOut_nadv.to_bool()) {
    addr_cnt++;
  }

  // Read from external memory
  if (!c->Patmos__io_sSRam32CtrlPins_ramOut_noe.to_bool()) {
    counter++;
    if (counter >= SRAM_CYCLES) {
      c->Patmos__io_sSRam32CtrlPins_ramIn_din = ram_buf[address];
      if (address <= addr_cnt) {
        address++;
      }
    }
  }

  // Write to external memory
  if (c->Patmos__io_sSRam32CtrlPins_ramOut_nbwe.to_ulong() == 0) {
    uint32_t nbw = c->Patmos__io_sSRam32CtrlPins_ramOut_nbw.to_ulong();
    uint32_t mask = 0x00000000;
    for (unsigned i = 0; i < 4; i++) {
      if ((nbw & (1 << i)) == 0) {
        mask |= 0xff << (i*8);
      }
    }

    ram_buf[address] &= ~mask;
    ram_buf[address] |= mask & c->Patmos__io_sSRam32CtrlPins_ramOut_dout.to_ulong();

    if (address <= addr_cnt) {
      address++;
    }
  }
}
#endif /* EXTMEM_SSRAM32CTRL */

#ifdef EXTMEM_SRAMCTRL
static uint16_t *ram_buf;

static void write_extmem(val_t address, val_t word) {
  ram_buf[(address << 1) | 0] = word & 0xffff;
  ram_buf[(address << 1) | 1] = word >> 16;
}

static void init_extmem(Patmos_t *c, bool random) {
  // Get SRAM properties
  uint32_t addr_bits = c->Patmos_ramCtrl__addrReg.width();
  uint32_t cells = 1 << addr_bits;

  // Check data width and allocate buffer
  assert(c->Patmos__io_sRamCtrlPins_ramOut_dout.width() == 16);
  ram_buf = (uint16_t *)calloc(cells, sizeof(uint16_t));
  if (ram_buf == NULL) {
    cerr << program_name << ": error: Cannot allocate memory for SRAM emulation" << endl;
    exit(EXIT_FAILURE);
  }

  // Initialize with random data
  if (random) {
    for (int i = 0; i < cells/2; i++) {
      write_extmem(i, rand());
    }
  }
}

static void emu_extmem(Patmos_t *c) {
  uint32_t address = c->Patmos_ramCtrl__addrReg.to_ulong();

  // Read from external memory unconditionally
  c->Patmos__io_sRamCtrlPins_ramIn_din = ram_buf[address];

  // Write to external memory
  if (!c->Patmos__io_sRamCtrlPins_ramOut_nwe.to_bool()) {
    uint16_t mask = 0x0000;
    if (!c->Patmos__io_sRamCtrlPins_ramOut_nub.to_bool()) {
      mask |= 0xff00;
    }
    if (!c->Patmos__io_sRamCtrlPins_ramOut_nlb.to_bool()) {
      mask |= 0x00ff;
    }
    ram_buf[address] &= ~mask;
    ram_buf[address] |= mask & c->Patmos__io_sRamCtrlPins_ramOut_dout.to_ulong();
  }
}
#endif /* EXTMEM_SRAMCTRL */

#ifdef IO_CPUINFO
static void emu_cpuinfo(Patmos_t *c) {
    c->Patmos__io_cpuInfoPins_id = 0;
    c->Patmos__io_cpuInfoPins_cnt = 1;
}
#endif /* IO_CPUINFO */

#ifdef IO_KEYS
static void emu_keys(Patmos_t *c, bool enable) {
  if (enable) {
    if ((rand() % 0x10000) == 0) {
      c->Patmos__io_keysPins_key = rand();
    }
  }
}
#endif /* IO_KEYS */

#ifdef IO_UART
static void emu_uart(Patmos_t *c, int uart_in, int uart_out) {
  static unsigned baud_counter = 0;

  // Pass on data from UART
  if (c->Patmos_core_iocomp_Uart__io_ocp_M_Cmd.to_ulong() == 0x1
      && (c->Patmos_core_iocomp_Uart__io_ocp_M_Addr.to_ulong() & 0xff) == 0x04) {
    unsigned char d = c->Patmos_core_iocomp_Uart__io_ocp_M_Data.to_ulong();
    int w = write(uart_out, &d, 1);
    if (w != 1) {
      cerr << program_name << ": error: Cannot write UART output" << endl;
    }
  }

  // Pass on data to UART
  bool baud_tick = c->Patmos_core_iocomp_Uart__tx_baud_tick.to_bool();
  if (baud_tick) {
    baud_counter = (baud_counter + 1) % 10;
  }
  if (baud_tick && baud_counter == 0) {
    struct pollfd pfd;
    pfd.fd = uart_in;
    pfd.events = POLLIN;
    if (poll(&pfd, 1, 0) > 0) {
      unsigned char d;
      int r = read(uart_in, &d, 1);
      if (r != 0) {
        if (r != 1) {
          cerr << program_name << ": error: Cannot read UART input" << endl;
        } else {
          c->Patmos_core_iocomp_Uart__rx_state = 0x3; // rx_stop_bit
          c->Patmos_core_iocomp_Uart__rx_baud_tick = 1;
          c->Patmos_core_iocomp_Uart__rxd_reg2 = 1;
          c->Patmos_core_iocomp_Uart__rx_buff = d;
        }
      }
    }
  }
}
#endif /* IO_UART */

#ifdef IO_ETHMAC
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <arpa/inet.h>

static int ethmac_alloc_tap(const char *ip_addr)
{
  struct ifreq ifr;
  int fd, err;

  // open tunnel device
  if ((fd = open("/dev/net/tun", O_RDWR)) < 0) {
    cerr << program_name << ": error: Opening tun device: " << strerror(errno) << endl;
    return fd;
  }

  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, "pat0", IFNAMSIZ);

  // Create tap device
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
  if ((err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ){
    cerr << program_name << ": error: Creating tap device: " << strerror(errno) << endl;
    close(fd);
    return err;
  }

  // We need to create a socket and work on that to set up the tap device
  int skfd;
  if ((skfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) < 0) {
    cerr << program_name << ": error: Opening socket for tap device: " << strerror(errno) << endl;
    close(fd);
    return err;
  }

  // Set IP address of device
  struct sockaddr_in *addr = (struct sockaddr_in *)&ifr.ifr_addr;
  addr->sin_family = AF_INET;
  inet_pton(AF_INET, ip_addr, &addr->sin_addr);
  if ((err = ioctl(skfd, SIOCSIFADDR, (void *) &ifr)) < 0 ){
    cerr << program_name << ": error: Setting address for tap device: " << strerror(errno) << endl;
    close(fd);
    return err;
  }

  // Get device up and running
  if ((err = ioctl(skfd, SIOCGIFFLAGS, (void *) &ifr)) < 0 ) {
    cerr << program_name << ": error: Getting flags for tap device: " << strerror(errno) << endl;
    close(fd);
    return err;
  }
  ifr.ifr_flags |= IFF_UP | IFF_RUNNING;
  if ((err = ioctl(skfd, SIOCSIFFLAGS, (void *) &ifr)) < 0 ){
    cerr << program_name << ": error: Setting tap device up and running: " << strerror(errno) << endl;
    close(fd);
    return err;
  }

  return fd;
}

static void emu_ethmac(Patmos_t *c, int ethmac_tap) {

  static int rx = 0;
  static int rx_ready = 0;
  static uint32_t rx_addr = 0;
  static uint32_t rx_length = 0;

  static int tx = 0;
  static int tx_ready = 0;
  static uint32_t tx_addr = 0;
  static uint32_t tx_length = 0;

  static uint8_t buffer [0xefff];

  uint32_t cmd  = c->Patmos_core_iocomp_EthMac_bb__MCmd.to_ulong();
  uint32_t addr = c->Patmos_core_iocomp_EthMac_bb__MAddr.to_ulong() & 0xffff;
  uint32_t data = c->Patmos_core_iocomp_EthMac_bb__MData.to_ulong();
  c->Patmos_core_iocomp_EthMac_bb__respReg = 0;

  if (cmd == 0x1) {
    if (addr < 0xf000) {
      buffer[addr] = data >> 24;
      buffer[addr+1] = data >> 16;
      buffer[addr+2] = data >> 8;
      buffer[addr+3] = data;
    } else {
      switch (addr) {
      case 0xf004:
        if (data & 0x4) { rx_ready = 0; }
        if (data & 0x1) { tx_ready = 0; }
        break;
      case 0xf400: tx_length = data >> 16; tx = data & 0x8000; break;
      case 0xf404: tx_addr = data; break;
      case 0xf600: rx_length = data >> 16; rx = data & 0x8000; break;
      case 0xf604: rx_addr = data; break;
      }
    }
    c->Patmos_core_iocomp_EthMac_bb__respReg = 0x1;
  } else if (cmd == 0x2) {
    if (addr < 0xf000) {
      data = ((buffer[addr] << 24) | (buffer[addr+1] << 16) |
              (buffer[addr+2] << 8) | (buffer[addr+3]));
    } else {
      switch (addr) {
      case 0xf004: data = (rx_ready << 2) | (tx_ready << 0); break;
      }
    }
    c->Patmos_core_iocomp_EthMac_bb__dataReg = data;
    c->Patmos_core_iocomp_EthMac_bb__respReg = 0x1;
  }

  if (tx && !tx_ready) {
    if (write(ethmac_tap, &buffer[tx_addr], tx_length) < 0) {
      cerr << program_name << ": error: Cannot write to tap device" << endl;
    }
    tx = 0;
    tx_ready = 1;
  }

  if (rx && !rx_ready) {
    struct pollfd pfd;
    pfd.fd = ethmac_tap;
    pfd.events = POLLIN;
    if (poll(&pfd, 1, 0) > 0) {
      ssize_t len = read(ethmac_tap, &buffer[rx_addr], 0x600);
      if (len > 0) {
        rx = 0;
        rx_ready = 1;
      } else if (len < 0) {
        cerr << program_name << ": error: Cannot read from tap device" << endl;
      }
    }
  }
}
#endif /* IO_ETHMAC */

// Read an elf executable image into the on-chip memories
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
      //assert(phdr.p_vaddr == phdr.p_paddr);
      assert(phdr.p_filesz <= phdr.p_memsz);

      // copy from the buffer into the on-chip memories
      for (size_t k = 0; k < phdr.p_memsz; k++) {

        if ((phdr.p_flags & PF_X) != 0 &&
            ((phdr.p_paddr + k) >> OCMEM_ADDR_BITS) == 0x1 &&
            ((phdr.p_paddr + k) & 0x3) == 0) {
          // Address maps to ISPM and is at a word boundary
          val_t word = k >= phdr.p_filesz ? 0 :
            (((val_t)elfbuf[phdr.p_offset + k + 0] << 24) |
             ((val_t)elfbuf[phdr.p_offset + k + 1] << 16) |
             ((val_t)elfbuf[phdr.p_offset + k + 2] << 8) |
             ((val_t)elfbuf[phdr.p_offset + k + 3] << 0));
          val_t addr = ((phdr.p_paddr + k) - (0x1 << OCMEM_ADDR_BITS)) >> 3;
          unsigned size = (sizeof(c->Patmos_core_fetch_MemBlock__mem.contents) /
                           sizeof(c->Patmos_core_fetch_MemBlock__mem.contents[0]));
          assert(addr < size && "Instructions mapped to ISPM exceed size");

          // Write to even or odd block
          if (((phdr.p_paddr + k) & 0x4) == 0) {
            c->Patmos_core_fetch_MemBlock__mem.put(addr, word);
          } else {
            c->Patmos_core_fetch_MemBlock_1__mem.put(addr, word);
          }
        }

        if (((phdr.p_paddr + k) & 0x3) == 0) {
          // Address maps to SRAM and is at a word boundary
          val_t word = k >= phdr.p_filesz ? 0 :
            (((val_t)elfbuf[phdr.p_offset + k + 0] << 24) |
             ((val_t)elfbuf[phdr.p_offset + k + 1] << 16) |
             ((val_t)elfbuf[phdr.p_offset + k + 2] << 8) |
             ((val_t)elfbuf[phdr.p_offset + k + 3] << 0));

          val_t addr = ((phdr.p_paddr + k) >> 2);
          write_extmem(addr, word);
        }
      }
    }
  }

  // get entry point
  val_t entry = hdr.e_entry;

  elf_end(elf);

  return entry;
}

static void init_icache(Patmos_t *c, val_t entry) {
  if (entry != 0) {
    if (entry >= 0x20000) {
      #ifdef ICACHE_METHOD
      //init for method cache
      c->Patmos_core_fetch__pcReg = -1;
      c->Patmos_core_icache_repl__hitReg = 0;
      #endif /* ICACHE_METHOD */
      #ifdef ICACHE_LINE
      //init for icache
      c->Patmos_core_fetch__pcReg = (entry >> 2) - 1;
      #endif /* ICACHE_LINE */
      c->Patmos_core_fetch__relBaseReg = 0;
      c->Patmos_core_fetch__relocReg = (entry >> 2) - 1;
      c->Patmos_core_fetch__selCache = 1;
      c->Patmos_core_icache_repl__selCacheReg = 1;
    } else {
      // pcReg for ispm starts at entry point - ispm base
      c->Patmos_core_fetch__pcReg = ((entry - 0x10000) >> 2) - 1;
      c->Patmos_core_fetch__relBaseReg = (entry - 0x10000) >> 2;
      c->Patmos_core_fetch__relocReg = 0x10000 >> 2;
      c->Patmos_core_fetch__selSpm = 1;
      c->Patmos_core_icache_repl__selSpmReg = 1;
    }
    c->Patmos_core_icache_repl__callRetBaseReg = (entry >> 2);
    #ifdef ICACHE_METHOD
    c->Patmos_core_icache_ctrl__callRetBaseReg = (entry >> 2);
    #endif /* ICACHE_METHOD */
    #ifdef ICACHE_LINE
    c->Patmos_core_fetch__relBaseReg = (entry >> 2);
    #endif /* ICACHE_LINE */
  }
}

static void stat_icache(Patmos_t *c, bool halt) {
  static uint cache_miss = 0;
  static uint cache_hits = 0;
  static uint exec_cycles = 0;
  static uint cache_stall_cycles = 0;
  static uint max_function_size = 0;
  static float hit_rate = 0;
  //count all cycles till the program terminats
  exec_cycles++;
  #ifdef ICACHE_METHOD
  //count everytime a new method is written to the cache
  if (c->Patmos_core_icache_ctrl__io_ctrlrepl_wTag.to_bool()) {
    cache_miss++;
    if (c->Patmos_core_icache_ctrl__io_ctrlrepl_wData.to_ulong() > max_function_size) {
      max_function_size = c->Patmos_core_icache_ctrl__io_ctrlrepl_wData.to_ulong();
    }
  }
  //everytime a method is called from the cache, todo: find a better way to measure hits
  if (c->Patmos_core_fetch__io_memfe_doCallRet.to_bool() &&
      c->Patmos_core_icache_repl__io_replctrl_hit.to_bool() &&
      c->Patmos_core_icache_ctrl__stateReg.to_ulong() == 0 &&
      c->Patmos_core_icache__io_ena_in.to_bool() &&
      !c->Patmos_core_icache_ctrl__io_ctrlrepl_instrStall.to_bool()) {
    cache_hits++;
  }
  #endif /* ICACHE_METHOD */
  #ifdef ICACHE_LINE
  //add stats for instruction cache measurements
  if (c->Patmos_core_icache_ctrl__io_ctrlrepl_wTag.to_bool()) {
    cache_miss++;
  }
  if (c->Patmos_core_fetch__io_ena.to_bool()) {
    if (c->Patmos_core_icache_repl__hitEven.to_bool()) {
      cache_hits++;
    }
    if (c->Patmos_core_icache_repl__hitOdd.to_bool()) {
      cache_hits++;
    }
  }
  #endif /* ICACHE_LINE */

  //pipeline stalls caused by the instruction cache
  if (!c->Patmos_core_icache__io_ena_out.to_bool()) {
    cache_stall_cycles++;
  }
  //program terminats, write to output
  if (halt == true) {
    hit_rate = (float)((float)cache_hits /  (float)(cache_hits + cache_miss))*(float)100;

    *out << "exec_cycles: " << exec_cycles << "\n"
         << "cache_hits: " << cache_hits << "\n"
         << "cache_misses: " << cache_miss << "\n"
         << "hit rate: " << hit_rate << "\n"
         <<  "cache_stall_cycles: " << cache_stall_cycles << "\n"
         << "max function size: " << max_function_size << "\n";
  }
}

static void print_sc_state(Patmos_t *c) {
  // fill
  if ((c->Patmos_core_dcache_sc__stateReg.to_ulong() == 1) ||
      (c->Patmos_core_dcache_sc__stateReg.to_ulong() == 2)) {
    if (c->Patmos_core_dcache_sc__mb_wrEna.to_bool()) {
      for (unsigned int i = 0; i < 4; i++) {
        std::cerr << "f:" << (c->Patmos_core_dcache_sc__transferAddrReg.to_ulong() + i - 4)
                  << " > " << (((c->Patmos_core_dcache_sc__mb_wrData.to_ulong() << (i*8)) >> 24) & 0xFF)
                  << "\n";
      }
    }
  }
  // spill
  else if ((c->Patmos_core_dcache_sc__stateReg.to_ulong() == 3) ||
           (c->Patmos_core_dcache_sc__stateReg.to_ulong() == 4)) {
    if (c->Patmos_core_dcache_sc__io_toMemory_M_DataValid.to_bool() &&
        c->Patmos_core_dcache_sc__io_toMemory_M_DataByteEn.to_ulong()) {
      for (unsigned int i = 0; i < 4; i++) {
        std::cerr << "s:" << (c->Patmos_core_dcache_sc__transferAddrReg.to_ulong() + i - 4)
                  << " < " << (((c->Patmos_core_dcache_sc__mb_rdData.to_ulong() << (i*8)) >> 24) & 0xFF)
                  << "\n";
      }
    }
  }
}

static void print_state(Patmos_t *c) {
  static unsigned int baseReg = 0;
  *out << ((baseReg + c->Patmos_core_fetch__pcReg.to_ulong()) * 4 - c->Patmos_core_fetch__relBaseReg.to_ulong() * 4) << " - ";
  baseReg = c->Patmos_core_icache_repl__callRetBaseReg.to_ulong();

  for (unsigned i = 0; i < 32; i++) {
    *out << c->Patmos_core_decode_rf__rf.get(i).to_ulong() << " ";
  }

  *out << endl;
}

static void usage(ostream &out, const char *name) {
  out << "Usage: " << name
      << " <options> [file]" << endl;
}

static void help(ostream &out) {
  out << endl << "Options:" << endl
      #ifdef IO_ETHMAC
      << "  -e <addr>     Provide virtual network interface with IP address <addr>" << endl
      #endif /* IO_ETHMAC */
      << "  -h            Print this help" << endl
      << "  -i            Initialize memory with random values" << endl
      #ifdef IO_KEYS
      << "  -k            Simulate random input from keys" << endl
      #endif /* IO_KEYS */
      << "  -l <N>        Stop after <N> cycles" << endl
      << "  -p            Print instruction cache statistics" << endl
      << "  -r            Print register values in each cycle" << endl
      << "  -s            Trace stack cache spilling/filling" << endl
      << "  -v            Dump wave forms file \"Patmos.vcd\"" << endl
      #ifdef IO_UART
      << "  -I <file>     Read input for UART from file <file>" << endl
      << "  -O <file>     Write output from UART to file <file>" << endl
      #endif /* IO_UART */
    ;
}

int main (int argc, char* argv[]) {
  Patmos_t* c = new Patmos_t();

  int opt;

  bool random = false;
  int  lim = -1;
  bool print_stat = false;
  bool quiet = true;
  bool vcd = false;
  bool sc_trace = false;

  #ifdef IO_KEYS
  bool keys = false;
  #endif /* IO_KEYS */
  #ifdef IO_UART
  int uart_in = STDIN_FILENO;
  int uart_out = STDOUT_FILENO;
  #endif /* IO_UART */
  #ifdef IO_ETHMAC
  int ethmac_tap = -1;
  #endif /* IO_ETHMAC */

  program_name = argv[0];

  // Parse command line arguments
  while ((opt = getopt(argc, argv, "e:hikl:nprsvI:O:")) != -1) {
    switch (opt) {
    #ifdef IO_ETHMAC
    case 'e':
      ethmac_tap = ethmac_alloc_tap(optarg);
      break;
    #endif /* IO_ETHMAC */
    case 'i':
      random = true;
      break;
    #ifdef IO_KEYS
    case 'k':
      keys = true;
      break;
    #endif /* IO_KEYS */
    case 'l':
      lim = atoi(optarg);
      break;
    case 'p':
      print_stat = true;
      break;
    case 'r':
      quiet = false;
      break;
    case 's':
      sc_trace = true;
      break;
    case 'v':
      vcd = true;
      break;
    #ifdef IO_UART
    case 'I':
      if (strcmp(optarg, "-") == 0) {
        uart_in = STDIN_FILENO;
      } else {
        uart_in = open(optarg, O_RDONLY);
        if (uart_in < 0) {
          cerr << argv[0] << "error: Cannot open input file " << optarg << endl;
          exit(EXIT_FAILURE);
        }
      }
      break;
    case 'O':
      if (strcmp(optarg, "-") == 0) {
        uart_out = STDOUT_FILENO;
      } else {
        uart_out = open(optarg, O_WRONLY|O_CREAT|O_TRUNC, 0644);
        if (uart_out < 0) {
          cerr << argv[0] << ": error: Cannot open output file " << optarg << endl;
          exit(EXIT_FAILURE);
        }
      }
      break;
    #endif /* IO_UART */
    case 'h':
      usage(cout, argv[0]);
      help(cout);
      exit(EXIT_SUCCESS);
    default: /* '?' */
      usage(cerr, argv[0]);
      cerr << "Try '" << argv[0] << " -h' for more information" << endl;
      exit(EXIT_FAILURE);
    }
  }

  // Initialize random generator for initialization
  srand(0);

  // Initialize internal state
  // TODO: Randomizing internal state seems to be broken
  // c->init(random);
  c->init();

  // Initalize external memory
  init_extmem(c, random);

  // Parse ELF file, if present
  val_t entry = 0;
  if (optind < argc) {
    ifstream *fs = new ifstream(argv[optind]);
    if (!fs->good()) {
      cerr << argv[0] << ": error: Cannot open elf file " << argv[optind] << endl;
      exit(EXIT_FAILURE);
    }
    entry = readelf(*fs, c);
  }

  // Create vcd trace file if necessary
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

  // Initialize instruction cache for entry point
  init_icache(c, entry);

  // Main emulation loop
  bool halt = false;
  for (int t = 0; lim < 0 || t < lim; t++) {
    dat_t<1> reset = LIT<1>(0);

    c->clock_lo(reset);
    c->clock_hi(reset);

    // Emulate external devices
    emu_extmem(c);
    #ifdef IO_CPUINFO
    emu_cpuinfo(c);
    #endif /* IO_CPUINFO */
    #ifdef IO_KEYS
    emu_keys(c, keys);
    #endif /* IO_KEYS */
    #ifdef IO_UART
    emu_uart(c, uart_in, uart_out);
    #endif /* IO_UART */
    #ifdef IO_ETHMAC
    emu_ethmac(c, ethmac_tap);
    #endif /* IO_ETHMAC */

    // Print tracing information
    if (vcd) {
      c->dump(f, t);
    }
    if (!quiet && c->Patmos_core__enableReg.to_bool()) {
      print_state(c);
    }
    if (sc_trace) {
      print_sc_state(c);
    }

    // Return to address 0 halts the execution after one more iteration
    if (halt) {
      break;
    }
    if ((c->Patmos_core_memory__memReg_mem_brcf.to_bool()
         || c->Patmos_core_memory__memReg_mem_ret.to_bool())
        && c->Patmos_core_icache_repl__callRetBaseReg.to_ulong() == 0) {
      halt = true;
    }

    // Record/print statistics
    if (print_stat) {
      stat_icache(c, halt);
    }
  }

  // TODO: adapt comparison tool so this can be removed
  if (!quiet) {
    *out << "PASSED" << endl;
  }

  // Pass on return value from processor
  return c->Patmos_core_decode_rf__rf.get(1).to_ulong();
}
