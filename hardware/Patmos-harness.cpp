/* 
NOTES ON STUFF MISSING FROM THE OLD EMULATOR
- generating the emulator_config. These care hardcoded below for now
- VCD dump
- init_extmem - should not be needed as the current state of the new emulator 
  uses hashmap for memory. Only for testing width of data channel to memory.
  but i haven't found a way to get signal width from verilator.

*/
#include "emulator_config.h"


//OTHER INTEMEDIATE HARDCODE




#include <fstream>
#include <iostream>
#include <string>
#include <libelf.h>
#include <gelf.h>

#include "VPatmos.h"
#include "verilated.h"
#if VM_TRACE
#include "verilated_vcd_c.h"
#endif
#if CORE_COUNT > 1
#include "VPatmos_PatmosCore.h"
#endif

#define OCMEM_ADDR_BITS 16

typedef uint64_t val_t;

using namespace std;

class Emulator
{
  unsigned long m_tickcount;
  public: VPatmos *c;
  VerilatedVcdC	*c_trace;
  // For Uart:
  bool UART_on;
  int baudrate;
  int freq;
  char in_byte;
  char out_byte;
  int sample_counter_out;
  int sample_counter_in;
  int bit_counter_out;
  int bit_counter_in;
  char state;
  bool writing;
  string write_str;
  int write_cntr;
  int write_len;
  bool trace;
  ostream *outputTarget = &std::cout;

  //elf - mem - ram
  #ifdef EXTMEM_SSRAM32CTRL
  uint32_t *ram_buf; 
  #endif /* EXTMEM_SSRAM32CTRL */

  #ifdef EXTMEM_SRAMCTRL
  uint16_t *ram_buf; 
  #endif /* EXTMEM_SRAMCTRL */

public:
  Emulator(void)
  {
    Verilated::traceEverOn(true);
    c = new VPatmos;
    m_tickcount = 0l;

    //for UART
    UART_on = false;
    baudrate = 0;
    freq = 0;
    in_byte = 0;
    out_byte = 0;
    sample_counter_out = 0;
    sample_counter_in = 0;
    bit_counter_out = 0;
    bit_counter_in = 0;
    state = 'i'; // 0:idle 1:receiving
    writing = false;
    write_cntr = 0;
    write_len = 0;
    c->io_UartCmp_rx = 1; // keep UART tx high when idle
    outputTarget = &cout; // default uart print to terminal

    #ifdef EXTMEM_SSRAM32CTRL
    ram_buf = (uint32_t *)calloc(1 << EXTMEM_ADDR_BITS, sizeof(uint32_t));
    #endif /* EXTMEM_SSRAM32CTRL */

    #ifdef EXTMEM_SRAMCTRL
    ram_buf = (uint16_t *)calloc(1 << EXTMEM_ADDR_BITS, sizeof(uint16_t));
    #endif /* EXTMEM_SRAMCTRL */

    trace = false;
    
  }

  ~Emulator(void)
  {
    delete c;
    c = NULL;

    if (trace){
      if (c_trace) {
        c_trace->close();
        c_trace = NULL;
      }
    }
  }

  void setTrace(){
    trace = true;
    if (!c_trace){
      c_trace = new VerilatedVcdC;
			c->trace(c_trace, 99);
			c_trace->open("Patmos.vcd");
    }
  }

  void stopTrace(){
    if (trace){
      if (c_trace) {
        c_trace->close();
        c_trace = NULL;
      }
    }
    trace = false;
  }

  void reset(int cycles)
  {
    c->reset = 1;
    // Make sure any inheritance gets applied
    for (int i = 0; i < cycles; i++)
    {
      this->tick();
    }
    c->reset = 0;
  }

  void tick(void)
  {
    // Increment our own internal time reference
    m_tickcount++;

    // Make sure any combinatorial logic depending upon
    // inputs that may have changed before we called tick()
    // has settled before the rising edge of the clock.
    c->clock = 0;
    c->eval();

    if (trace) {
      c_trace->dump(10*m_tickcount+5);
    }
    // Toggle the clock
    // Rising edge
    c->clock = 1;
    c->eval();

    //UART emulation
    if (UART_on)
    {
      UART_tick();
    }

    if (trace) {
      c_trace->dump(10*m_tickcount+10);
      c_trace->flush();
    }
  }

  long int get_tick_count(void)
  {
    return m_tickcount;
  }

  bool done(void) { return (Verilated::gotFinish()); }

  // UART methods below
  void UART_init()
  {
    baudrate = BAUDRATE;
    freq = FREQ;
    UART_on = true;
  }

  void UART_tick(void)
  {
    if (state == 'i')
    { // idle wait for start bit
      if (c->io_UartCmp_tx == 0)
      {
        state = 'r'; //receiving
      }
    }
    else if (state == 'r')
    {
      sample_counter_out++;
      if (sample_counter_out == ((freq / baudrate) + 1))
      { //+1 as i to go one clock tick to futher before sampling
        UART_read_bit();
        sample_counter_out = 0;
      }
    }

    if (writing)
    {
      sample_counter_in++;
      if (sample_counter_in == ((freq / baudrate) + 1))
      {
        sample_counter_in = 0;
        if (bit_counter_in == 0)
        { //start bit
          c->io_UartCmp_rx = 0;
          bit_counter_in++;
        }
        else if ((bit_counter_in > 0) && (bit_counter_in <= 8))
        { // data bits
          c->io_UartCmp_rx = (write_str[write_cntr] >> (8 - bit_counter_in)) & 1;
          bit_counter_in++;
        }
        else
        { //stop bits
          c->io_UartCmp_rx = 1;
          if (bit_counter_in == 9)
          {
            bit_counter_in++;
          }
          else
          {
            bit_counter_in = 0;
            write_cntr++;
            if (write_cntr == write_len)
            {
              writing = false;
              write_cntr = 0;
            }
          }
        }
      }
    }
  }

  void UART_read_bit(void)
  {
    bit_counter_out++;
    if (bit_counter_out == 9)
    {
      *outputTarget << out_byte;
      out_byte = 0;
      bit_counter_out = 0;
      state = 'i';
    }
    else
    {
      out_byte = (c->io_UartCmp_tx << (bit_counter_out - 1)) | out_byte;
    }
  }

  void UART_write(string in_str)
  {
    if (writing)
    {
      printf("UART are still writing");
      return;
    }
    write_str = in_str;
    write_len = write_str.length();
    writing = true;
  }

  void UART_to_file(string path)
  {
    static ofstream outFile;
    if (!outFile.is_open())
    {
      outFile.open(path);
    }
    outputTarget = &outFile;
  }

  void UART_to_console(void)
  {
    outputTarget = &cout;
  }

  //static val_t readelf(istream &is, Patmos_t *c)
  val_t readelf(istream &is)
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
      for (unsigned i = 0; i < count; i++)
        elfbuf.push_back(buf[i]);
    }

    // check libelf version
    elf_version(EV_CURRENT);

    // open elf binary
    Elf *elf = elf_memory((char *)&elfbuf[0], elfbuf.size());
    assert(elf);

    // check file kind
    Elf_Kind ek = elf_kind(elf);
    if (ek != ELF_K_ELF)
    {
      cerr << "readelf: ELF file must be of kind ELF.\n";
      exit(EXIT_FAILURE);
    }

    // get elf header
    GElf_Ehdr hdr;
    GElf_Ehdr *tmphdr = gelf_getehdr(elf, &hdr);
    assert(tmphdr);

    if (hdr.e_machine != 0xBEEB)
    {
      cerr << "readelf: unsupported architecture: ELF file is not a Patmos ELF file.\n";
      exit(EXIT_FAILURE);
    }

    // check class
    int ec = gelf_getclass(elf);
    if (ec != ELFCLASS32)
    {
      cerr << "readelf: unsupported architecture: ELF file is not a 32bit Patmos ELF file.\n";
      exit(EXIT_FAILURE);
    }

    // get program headers
    size_t n;
    int ntmp = elf_getphdrnum(elf, &n);
    assert(ntmp == 0);

    for (size_t i = 0; i < n; i++)
    {
      // get program header
      GElf_Phdr phdr;
      GElf_Phdr *phdrtmp = gelf_getphdr(elf, i, &phdr);
      assert(phdrtmp);

      if (phdr.p_type == PT_LOAD)
      {
        // some assertions
        //assert(phdr.p_vaddr == phdr.p_paddr);
        assert(phdr.p_filesz <= phdr.p_memsz);

        // copy from the buffer into the on-chip memories
        for (size_t k = 0; k < phdr.p_memsz; k++)
        {
          if ((phdr.p_flags & PF_X) != 0 &&
              ((phdr.p_paddr + k) >> OCMEM_ADDR_BITS) == 0x1 &&
              ((phdr.p_paddr + k) & 0x3) == 0)
          {
            // Address maps to ISPM and is at a word boundary

            val_t word = k >= phdr.p_filesz ? 0 : 
              (((val_t)elfbuf[phdr.p_offset + k + 0] << 24) |
               ((val_t)elfbuf[phdr.p_offset + k + 1] << 16) |
               ((val_t)elfbuf[phdr.p_offset + k + 2] << 8) | 
               ((val_t)elfbuf[phdr.p_offset + k + 3] << 0));
            val_t addr = ((phdr.p_paddr + k) - (0x1 << OCMEM_ADDR_BITS)) >> 3;
            #if CORE_COUNT == 1
            unsigned size = (sizeof(c->Patmos__DOT__cores_0__DOT__fetch__DOT__MemBlock__DOT__mem) / //ANTHON THIS MIGHT BE WRONG - SHOULD BE OKAY
                             sizeof(c->Patmos__DOT__cores_0__DOT__fetch__DOT__MemBlock__DOT__mem[0]));
            #endif
            #if CORE_COUNT > 1
            unsigned size = (sizeof(c->__PVT__Patmos__DOT__cores_0->__PVT__fetch__DOT__MemBlock__DOT__mem) / //ANTHON THIS MIGHT BE WRONG - SHOULD BE OKAY
                             sizeof(c->__PVT__Patmos__DOT__cores_0->__PVT__fetch__DOT__MemBlock__DOT__mem[0]));
            #endif
            assert(addr < size && "Instructions mapped to ISPM exceed size");
  
            // Write to even or odd block
            #if CORE_COUNT == 1
            if (((phdr.p_paddr + k) & 0x4) == 0)
            {
              
              c->Patmos__DOT__cores_0__DOT__fetch__DOT__MemBlock__DOT__mem[addr] = word;
            }
            else
            {
              c->Patmos__DOT__cores_0__DOT__fetch__DOT__MemBlock_1__DOT__mem[addr] = word;
            }
            #endif
            #if CORE_COUNT > 1
            if (((phdr.p_paddr + k) & 0x4) == 0)
            {

              c->__PVT__Patmos__DOT__cores_0->__PVT__fetch__DOT__MemBlock__DOT__mem[addr] = word;
            }
            else
            {
              c->__PVT__Patmos__DOT__cores_0->__PVT__fetch__DOT__MemBlock_1__DOT__mem[addr] = word;
            }
            #endif

          }

          if (((phdr.p_paddr + k) & 0x3) == 0)
          {
            // Address maps to SRAM and is at a word boundary
            val_t word = k >= phdr.p_filesz ? 0 : (((val_t)elfbuf[phdr.p_offset + k + 0] << 24) | ((val_t)elfbuf[phdr.p_offset + k + 1] << 16) | ((val_t)elfbuf[phdr.p_offset + k + 2] << 8) | ((val_t)elfbuf[phdr.p_offset + k + 3] << 0));

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

  #ifdef EXTMEM_SSRAM32CTRL //TODO test this
  void write_extmem(val_t address, val_t word)
  {
    ram_buf[address] = word; // This gives segmentation fault dumb on second run!
  }

  void init_extmem() {
    //only needed for random init
    for (int i = 0; i < (1 << EXTMEM_ADDR_BITS); i++) {
      write_extmem(i, rand());
    }
  } 

static void emu_extmem() {
  static uint32_t addr_cnt;
  static uint32_t address;
  static uint32_t counter;

  // Start of request
  if (c->io_sSRam32CtrlPins_ramOut_nadsc != 1) {
    address = c->Patmos__io_sSRam32CtrlPins_ramOut_addr.to_ulong();
    addr_cnt = address;
    counter = 0;
  }

  // Advance address for burst
  if (c->io_sSRam32CtrlPins_ramOut_nadv != 1) {
    addr_cnt++;
  }

  // Read from external memory
  if (c->io_sSRam32CtrlPins_ramOut_noe != 1) {
    counter++;
    if (counter >= SRAM_CYCLES) {
      c->io_sSRam32CtrlPins_ramIn_din = ram_buf[address];
      if (address <= addr_cnt) {
        address++;
      }
    }
  }

  // Write to external memory
  if (c->io_sSRam32CtrlPins_ramOut_nbwe == 0) {
    uint32_t nbw = c->io_sSRam32CtrlPins_ramOut_nbw;
    uint32_t mask = 0x00000000;
    for (unsigned i = 0; i < 4; i++) {
      if ((nbw & (1 << i)) == 0) {
        mask |= 0xff << (i*8);
      }
    }

    ram_buf[address] &= ~mask;
    ram_buf[address] |= mask & ((unsigned long int) c->io_sSRam32CtrlPins_ramOut_dout);

    if (address <= addr_cnt) {
      address++;
    }
  }
}

#endif /*EXTMEM_SSRAM32CTRL*/

#ifdef EXTMEM_SRAMCTRL
  void write_extmem(val_t address, val_t word) {
    ram_buf[(address << 1) | 0] = word & 0xffff;
    ram_buf[(address << 1) | 1] = word >> 16;
  }

  void init_extmem() {
    //only needed for random init
    for (int i = 0; i < (1 << EXTMEM_ADDR_BITS)/2; i++) {
      write_extmem(i, rand());
    }
  }

  void emu_extmem() {
    uint32_t address = (uint32_t) c->io_SRamCtrl_ramOut_addr;
    // Read from external memory unconditionally
    c->io_SRamCtrl_ramIn_din = ram_buf[address];

    // Write to external memory
    if (c->io_SRamCtrl_ramOut_nwe != 1) {
      uint16_t mask = 0x0000;
      if (c->io_SRamCtrl_ramOut_nub != 1) {
        mask |= 0xff00;
      }
      if (c->io_SRamCtrl_ramOut_nlb != 1) {
        mask |= 0x00ff;
      }
      ram_buf[address] &= ~mask;
      ram_buf[address] |= mask & ((unsigned long int) c->io_SRamCtrl_ramOut_dout);
    }
  }
#endif /*EXTMEM_SRAMCTRL*/

  void init_icache(val_t entry)
  {
    
    tick();
    if (entry != 0)
    {
      if (entry >= 0x20000)
      {
#ifdef ICACHE_METHOD
        //init for method cache
#if CORE_COUNT == 1
        c->Patmos__DOT__cores_0__DOT__fetch__DOT__pcNext = -1;
        c->Patmos__DOT__cores_0__DOT__icache__DOT__repl__DOT__hitNext = 0;
// add multicore support, at the moment only for the method cache and not the ISPM
#endif
       

#if CORE_COUNT > 1
        c->__PVT__Patmos__DOT__cores_0->fetch__DOT__pcNext = -1;
        c->__PVT__Patmos__DOT__cores_0->icache__DOT__repl__DOT__hitNext = 0;

        c->__PVT__Patmos__DOT__cores_1->fetch__DOT__pcNext = -1;
        c->__PVT__Patmos__DOT__cores_1->icache__DOT__repl__DOT__hitNext = 0;
#endif
#if CORE_COUNT > 2
        c->__PVT__Patmos__DOT__cores_2->fetch__DOT__pcNext = -1;
        c->__PVT__Patmos__DOT__cores_2->icache__DOT__repl__DOT__hitNext = 0;
#endif
#if CORE_COUNT > 3
        c->__PVT__Patmos__DOT__cores_3->fetch__DOT__pcNext = -1;
        c->__PVT__Patmos__DOT__cores_3->icache__DOT__repl__DOT__hitNext = 0;
#endif
#if CORE_COUNT > 4
        c->__PVT__Patmos__DOT__cores_4->fetch__DOT__pcNext = -1;
        c->__PVT__Patmos__DOT__cores_4->icache__DOT__repl__DOT__hitNext = 0;
#endif
#if CORE_COUNT > 5
        c->__PVT__Patmos__DOT__cores_5->fetch__DOT__pcNext = -1;
        c->__PVT__Patmos__DOT__cores_5->icache__DOT__repl__DOT__hitNext = 0;
#endif
#if CORE_COUNT > 6
        c->__PVT__Patmos__DOT__cores_6->fetch__DOT__pcNext = -1;
        c->__PVT__Patmos__DOT__cores_6->icache__DOT__repl__DOT__hitNext = 0;
#endif
#if CORE_COUNT > 7
        c->__PVT__Patmos__DOT__cores_7->fetch__DOT__pcNext = -1;
        c->__PVT__Patmos__DOT__cores_7->icache__DOT__repl__DOT__hitNext = 0;
#endif
#endif /* ICACHE_METHOD */
#ifdef ICACHE_LINE
        //init for icache
        #if CORE_COUNT == 1
        c->Patmos__DOT__cores_0__DOT__fetch__DOT__pcNext = (entry >> 2) - 1;
        #endif
        #if CORE_COUNT > 1
        c->__PVT__Patmos__DOT__cores_0->fetch__DOT__pcNext = (entry >> 2) - 1;
        #endif
#endif /* ICACHE_LINE */
#if CORE_COUNT == 1
        c->Patmos__DOT__cores_0__DOT__fetch__DOT__relBaseNext = 0;
        c->Patmos__DOT__cores_0__DOT__fetch__DOT__relocNext = (entry >> 2) - 1;
        c->Patmos__DOT__cores_0__DOT__fetch__DOT__selCacheNext = 1;
        c->Patmos__DOT__cores_0__DOT__icache__DOT__repl__DOT__selCacheNext = 1;
#endif
#if CORE_COUNT > 1
        c->__PVT__Patmos__DOT__cores_0->fetch__DOT__relBaseNext = 0;
        c->__PVT__Patmos__DOT__cores_0->fetch__DOT__relocNext = (entry >> 2) - 1;
        c->__PVT__Patmos__DOT__cores_0->fetch__DOT__selCacheNext = 1;
        c->__PVT__Patmos__DOT__cores_0->icache__DOT__repl__DOT__selCacheNext = 1;

        c->__PVT__Patmos__DOT__cores_1->fetch__DOT__relBaseNext = 0;
        c->__PVT__Patmos__DOT__cores_1->fetch__DOT__relocNext = (entry >> 2) - 1;
        c->__PVT__Patmos__DOT__cores_1->fetch__DOT__selCacheNext = 1;
        c->__PVT__Patmos__DOT__cores_1->icache__DOT__repl__DOT__selCacheNext = 1;
#endif
#if CORE_COUNT > 2
        c->__PVT__Patmos__DOT__cores_2->fetch__DOT__relBaseNext = 0;
        c->__PVT__Patmos__DOT__cores_2->fetch__DOT__relocNext = (entry >> 2) - 1;
        c->__PVT__Patmos__DOT__cores_2->fetch__DOT__selCacheNext = 1;
        c->__PVT__Patmos__DOT__cores_2->icache__DOT__repl__DOT__selCacheNext = 1;
#endif
#if CORE_COUNT > 3
        c->__PVT__Patmos__DOT__cores_3->fetch__DOT__relBaseNext = 0;
        c->__PVT__Patmos__DOT__cores_3->fetch__DOT__relocNext = (entry >> 2) - 1;
        c->__PVT__Patmos__DOT__cores_3->fetch__DOT__selCacheNext = 1;
        c->__PVT__Patmos__DOT__cores_3->icache__DOT__repl__DOT__selCacheNext = 1;
#endif
#if CORE_COUNT > 4
        c->__PVT__Patmos__DOT__cores_4->fetch__DOT__relBaseNext = 0;
        c->__PVT__Patmos__DOT__cores_4->fetch__DOT__relocNext = (entry >> 2) - 1;
        c->__PVT__Patmos__DOT__cores_4->fetch__DOT__selCacheNext = 1;
        c->__PVT__Patmos__DOT__cores_4->icache__DOT__repl__DOT__selCacheNext = 1;
#endif
#if CORE_COUNT > 5
        c->__PVT__Patmos__DOT__cores_5->fetch__DOT__relBaseNext = 0;
        c->__PVT__Patmos__DOT__cores_5->fetch__DOT__relocNext = (entry >> 2) - 1;
        c->__PVT__Patmos__DOT__cores_5->fetch__DOT__selCacheNext = 1;
        c->__PVT__Patmos__DOT__cores_5->icache__DOT__repl__DOT__selCacheNext = 1;
#endif
#if CORE_COUNT > 6
        c->__PVT__Patmos__DOT__cores_6->fetch__DOT__relBaseNext = 0;
        c->__PVT__Patmos__DOT__cores_6->fetch__DOT__relocNext = (entry >> 2) - 1;
        c->__PVT__Patmos__DOT__cores_6->fetch__DOT__selCacheNext = 1;
        c->__PVT__Patmos__DOT__cores_6->icache__DOT__repl__DOT__selCacheNext = 1;
#endif
#if CORE_COUNT > 7
        c->__PVT__Patmos__DOT__cores_7->fetch__DOT__relBaseNext = 0;
        c->__PVT__Patmos__DOT__cores_7->fetch__DOT__relocNext = (entry >> 2) - 1;
        c->__PVT__Patmos__DOT__cores_7->fetch__DOT__selCacheNext = 1;
        c->__PVT__Patmos__DOT__cores_7->icache__DOT__repl__DOT__selCacheNext = 1;
#endif
      }
      else
      {
        // pcReg for ispm starts at entry point - ispm base
#if CORE_COUNT == 1
        c->Patmos__DOT__cores_0__DOT__fetch__DOT__pcNext = ((entry - 0x10000) >> 2) - 1;
        c->Patmos__DOT__cores_0__DOT__fetch__DOT__relBaseNext = (entry - 0x10000) >> 2;
        c->Patmos__DOT__cores_0__DOT__fetch__DOT__relocNext = 0x10000 >> 2;
        c->Patmos__DOT__cores_0__DOT__fetch__DOT__selSpmNext = 1;
        c->Patmos__DOT__cores_0__DOT__icache__DOT__repl__DOT__selSpmNext = 1;
#endif
#if CORE_COUNT > 1
        c->__PVT__Patmos__DOT__cores_0->fetch__DOT__pcNext = ((entry - 0x10000) >> 2) - 1;
        c->__PVT__Patmos__DOT__cores_0->fetch__DOT__relBaseNext = (entry - 0x10000) >> 2;
        c->__PVT__Patmos__DOT__cores_0->fetch__DOT__relocNext = 0x10000 >> 2;
        c->__PVT__Patmos__DOT__cores_0->fetch__DOT__selSpmNext = 1;
        c->__PVT__Patmos__DOT__cores_0->icache__DOT__repl__DOT__selSpmNext = 1;
#endif 
      }
#if CORE_COUNT == 1
      c->Patmos__DOT__cores_0__DOT__icache__DOT__repl__DOT__callRetBaseNext = (entry >> 2);
#endif 
#if CORE_COUNT > 1
      c->__PVT__Patmos__DOT__cores_0->icache__DOT__repl__DOT__callRetBaseNext = (entry >> 2);

      c->__PVT__Patmos__DOT__cores_1->icache__DOT__repl__DOT__callRetBaseNext = (entry >> 2);
#endif
#if CORE_COUNT > 2
      c->__PVT__Patmos__DOT__cores_2->icache__DOT__repl__DOT__callRetBaseNext = (entry >> 2);
#endif
#if CORE_COUNT > 3
      c->__PVT__Patmos__DOT__cores_3->icache__DOT__repl__DOT__callRetBaseNext = (entry >> 2);
#endif
#if CORE_COUNT > 4
      c->__PVT__Patmos__DOT__cores_4->icache__DOT__repl__DOT__callRetBaseNext = (entry >> 2);
#endif
#if CORE_COUNT > 5
      c->__PVT__Patmos__DOT__cores_5->icache__DOT__repl__DOT__callRetBaseNext = (entry >> 2);
#endif
#if CORE_COUNT > 6
      c->__PVT__Patmos__DOT__cores_6->icache__DOT__repl__DOT__callRetBaseNext = (entry >> 2);
#endif
#if CORE_COUNT > 7
      c->__PVT__Patmos__DOT__cores_7->icache__DOT__repl__DOT__callRetBaseNext = (entry >> 2);
#endif

#ifdef ICACHE_METHOD
#if CORE_COUNT == 1
      c->Patmos__DOT__cores_0__DOT__icache__DOT__ctrl__DOT__callRetBaseNext = (entry >> 2);
#endif 
#if CORE_COUNT > 1
      c->__PVT__Patmos__DOT__cores_0->icache__DOT__ctrl__DOT__callRetBaseNext = (entry >> 2);

      c->__PVT__Patmos__DOT__cores_1->icache__DOT__ctrl__DOT__callRetBaseNext = (entry >> 2);
#endif
#if CORE_COUNT > 2
      c->__PVT__Patmos__DOT__cores_2->icache__DOT__ctrl__DOT__callRetBaseNext = (entry >> 2);
#endif
#if CORE_COUNT > 3
      c->__PVT__Patmos__DOT__cores_3->icache__DOT__ctrl__DOT__callRetBaseNext = (entry >> 2);
#endif
#if CORE_COUNT > 4
      c->__PVT__Patmos__DOT__cores_4->icache__DOT__ctrl__DOT__callRetBaseNext = (entry >> 2);
#endif
#if CORE_COUNT > 5
      c->__PVT__Patmos__DOT__cores_5->icache__DOT__ctrl__DOT__callRetBaseNext = (entry >> 2);
#endif
#if CORE_COUNT > 6
      c->__PVT__Patmos__DOT__cores_6->icache__DOT__ctrl__DOT__callRetBaseNext = (entry >> 2);
#endif
#if CORE_COUNT > 7
      c->__PVT__Patmos__DOT__cores_7->icache__DOT__ctrl__DOT__callRetBaseNext = (entry >> 2);
#endif
#endif /* ICACHE_METHOD */
#ifdef ICACHE_LINE
#if CORE_COUNT == 1
      c->Patmos__DOT__cores_0__DOT__fetch__DOT__relBaseNext = (entry >> 2);
#endif
#if CORE_COUNT > 1
      c->__PVT__Patmos__DOT__cores_0->fetch__DOT__relBaseNext = (entry >> 2);
#endif
#endif /* ICACHE_LINE */
    }
  }
};

// Override Verilator definition so first $finish ends simulation
// Note: VL_USER_FINISH needs to be defined when compiling Verilator code
void vl_finish(const char *filename, int linenum, const char *hier)
{
  Verilated::flushCall();
  exit(0);
}

static void usage(ostream &out, const char *name) {
  out << "Usage: " << name
      << " <options> [file]" << endl;
}

static void help(ostream &out) {
  out << endl << "Options:" << endl
      << "  -h            Print this help" << endl
      << "  -i            Initialize memory with random values" << endl
      << "  -l <N>        Stop after <N> cycles" << endl
      << "  -v            Dump wave forms file \"Patmos.vcd\"" << endl
      << "  -O <file>     Write output from UART to file <file>" << endl
  ;
}
   

int main(int argc, char **argv, char **env)
{
  Verilated::commandArgs(argc, argv);
  Emulator *emu = new Emulator();
  int opt;
  int limit = -1;
  bool halt = false;

  //Parse Arguments
  while ((opt = getopt(argc, argv, "hvl:iO:")) != -1){
    switch (opt) {
      case 'v':
        emu->setTrace();
        break;
      case 'l':
        limit = atoi(optarg);
        break;
      case 'i':
        emu->init_extmem();
      case 'O':
        emu->UART_to_file(optarg);
        break;
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

  
  emu->reset(1);
  emu->tick();
  emu->UART_init();

  val_t entry = 0;
  if (optind < argc)
  {
    ifstream *fs = new ifstream(argv[optind]);
    if (!fs->good())
    {
      cerr << "Error: Cannot open elf file " << endl;
      exit(EXIT_FAILURE);
    }
    entry = emu->readelf(*fs);
  }

  emu->reset(5);
  emu->tick();

  emu->init_icache(entry);


  int cnt = 0;
  int waituart = 0;
  while (limit < 0 || emu->get_tick_count() < limit)
  {
    cnt++;
    emu->tick();
    emu->emu_extmem();
     // Return to address 0 halts the execution after one more iteration
    if (halt) {
      if(waituart == 80000-(1000*(CORE_COUNT-1))){
        break;
      }else{
        waituart++;
      }
    }
    #if CORE_COUNT == 1
    if ((emu->c->Patmos__DOT__cores_0__DOT__memory__DOT__memReg_mem_brcf == 1
         || emu->c->Patmos__DOT__cores_0__DOT__memory__DOT__memReg_mem_ret == 1)
        && emu->c->Patmos__DOT__cores_0__DOT__icache__DOT__repl__DOT__callRetBaseReg == 0) {
      halt = true;
    }
    #endif
    #if CORE_COUNT > 1
    if ((emu->c->__PVT__Patmos__DOT__cores_0->__PVT__memory__DOT__memReg_mem_brcf == 1
         || emu->c->__PVT__Patmos__DOT__cores_0->__PVT__memory__DOT__memReg_mem_ret == 1)
        && emu->c->__PVT__Patmos__DOT__cores_0->__PVT__icache__DOT__repl__DOT__callRetBaseReg == 0) {
      halt = true;
    }
    #endif
  }

  emu->stopTrace();
  exit(EXIT_SUCCESS);
}


