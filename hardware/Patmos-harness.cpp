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
#include <sys/poll.h>
#include <fcntl.h>

#include "VPatmos.h"
#if VM_TRACE
#include "verilated_fst_c.h"
#endif

#define OCMEM_ADDR_BITS 16

typedef uint64_t val_t;

using namespace std;

class Emulator
{
  vluint64_t m_tickcount;
  public: VPatmos *c;
  VerilatedFstC	*c_trace;
  // For Uart:
  bool UART_on;
  bool UART_idle;
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
    UART_idle = false;
    c->io_UartCmp_rx = 1; // keep UART tx high when idle

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
      c_trace = new VerilatedFstC;
      c->trace(c_trace, 99);
      c_trace->open("Patmos.fst");
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
      this->tick(STDIN_FILENO, STDOUT_FILENO);
    }
    c->reset = 0;
  }

  void tick(int uart_in,int uart_out)
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
      emu_uart(uart_in, uart_out);
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

  void emu_uart(int uart_in,int uart_out) {
    static unsigned char rx_buf;
    static unsigned rx_state;
    static unsigned char tx_buf;
    static unsigned tx_state;
    
    if (c->uartcmp_tx_baud_tick) {

      // Receive data from Patmos
      switch(rx_state) {
        default: //case 0
          if(c->io_UartCmp_tx == 0)
            rx_state++;
          break;

        case 1 ... 8:
        {
          if (c->io_UartCmp_tx == 1)
            rx_buf |= (1 << (rx_state - 1)); // Set the bit
          else
            rx_buf &= ~(1 << (rx_state - 1)); // Clear the bit
          
          rx_state++;
          break;
        }

        case 9:
          rx_state = 0;
          int w = write(uart_out, &rx_buf, 1);
          if (w != 1) {
            cerr << "patemu: error: Cannot write UART output" << endl;
          }
          break;
      }
      
      // Send data to Patmos
      switch(tx_state) {
        default: //case 0
          c->io_UartCmp_rx = 1;
          struct pollfd pfd;
          pfd.fd = uart_in;
          pfd.events = POLLIN;
          if (poll(&pfd, 1, 0) > 0) {
            unsigned char d;
            int r = read(uart_in, &tx_buf, 1);
            if (r == 1) {
              c->io_UartCmp_rx = 0;
              tx_state++;
            }
            else if(r != 0)
              cerr << "patemu: error: Cannot read UART input" << endl;
          }
          break;

        case 1 ... 8:
          c->io_UartCmp_rx = ((tx_buf >> (tx_state - 1)) & 1);
          tx_state++;
          break;

        case 9:
          c->io_UartCmp_rx = 1;
          tx_state = 0;
          break;
      }
      
      static unsigned idle_cnt = 0;
      const unsigned idle_cnt_max = 2;
    
      if((rx_state != 0) || (tx_state != 0))
          idle_cnt = 0;
      else if(idle_cnt < idle_cnt_max)
        idle_cnt++;
      
      UART_idle = idle_cnt >= idle_cnt_max;
    }
  }

  void emu_keys(void){
#ifdef IO_KEYS
    if ((rand() % 0x10000) == 0) {
      c->io_Keys_key = rand();
    }
#endif
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

        // copy from the buffer into memory
        for (size_t k = 0; k < phdr.p_memsz; k++)
        {

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

#ifdef EXTMEM_SSRAM32CTRL // TODO: test this
  void write_extmem(val_t address, val_t word)
  {
    ram_buf[address] = word; // This gives segmentation fault dumb on second run!
  }

  void init_extmem() {
    // only needed for random init
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
#elif defined EXTMEM_SRAMCTRL
  void write_extmem(val_t address, val_t word) {
    ram_buf[(address << 1) | 0] = word & 0xffff;
    ram_buf[(address << 1) | 1] = word >> 16;
  }

  void init_extmem() {
    // only needed for random init
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
#else
void write_extmem(val_t address, val_t word) {}
void init_extmem() {}
void emu_extmem() {}
#endif


  void print_state()
  {
    static unsigned int baseReg = 0;
    printf("0x%08x - 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
      ((baseReg + c->core0_pcNext - c->core0_relBaseNext) * 4),
      c->core0_rf_0,
      c->core0_rf_1,
      c->core0_rf_2,
      c->core0_rf_3,
      c->core0_rf_4,
      c->core0_rf_5,
      c->core0_rf_6,
      c->core0_rf_7,
      c->core0_rf_8,
      c->core0_rf_9,
      c->core0_rf_10,
      c->core0_rf_11,
      c->core0_rf_12,
      c->core0_rf_13,
      c->core0_rf_14,
      c->core0_rf_15,
      c->core0_rf_16,
      c->core0_rf_17,
      c->core0_rf_18,
      c->core0_rf_19,
      c->core0_rf_20,
      c->core0_rf_21,
      c->core0_rf_22,
      c->core0_rf_23,
      c->core0_rf_24,
      c->core0_rf_25,
      c->core0_rf_26,
      c->core0_rf_27,
      c->core0_rf_28,
      c->core0_rf_29,
      c->core0_rf_30,
      c->core0_rf_31
    );
    
    baseReg = c->core0_callRetBaseNext;
  }

};

static void usage(ostream &out, const char *name) {
  out << "Usage: " << name
      << " <options> [file]" << endl;
}

static void help(ostream &out) {
  out << endl << "Options:" << endl
      << "  -h            Print this help" << endl
      << "  -i            Initialize memory with random values" << endl
      << "  -l <N>        Stop after <N> cycles" << endl
      << "  -v            Dump wave forms file \"Patmos.fst\"" << endl
      << "  -r            Print register values in each cycle" << endl
      #ifdef IO_KEYS
      << "  -k            Simulate random input from keys" << endl
      #endif /* IO_KEYS */
      #ifdef IO_UART
      << "  -I <file>     Read input for UART from file <file>" << endl
      << "  -O <file>     Write output from UART to file <file>" << endl
      #endif
  ;
}


int main(int argc, char **argv, char **env)
{
  Verilated::commandArgs(argc, argv);
  Emulator *emu = new Emulator();
  int opt;
  int limit = -1;
  bool halt = false;
  int exitcode = EXIT_SUCCESS;
  bool reg_print = false;

  int uart_in = STDIN_FILENO;
  int uart_out = STDOUT_FILENO;
  bool keys = false;

  //Parse Arguments
  while ((opt = getopt(argc, argv, "hvl:iO:I:rk")) != -1){
    switch (opt) {
      case 'v':
        emu->setTrace();
        break;
      case 'l':
        limit = atoi(optarg);
        break;
      case 'i':
        emu->init_extmem();
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
      #endif
      case 'r':
        reg_print = true;
        break;
      #ifdef IO_KEYS
      case 'k':
      keys = true;
      break;
      #endif /* IO_KEYS */
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
  emu->tick(uart_in, uart_out);
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
  
  emu->c->envinfo_platform = 1; // set platform type to emulator
  emu->c->envinfo_entrypoint = entry; // set entry point

  emu->tick(uart_in, uart_out);

  if(reg_print){
    printf("Patmos start\n");
  }

  while (limit < 0 || emu->get_tick_count() < limit)
  {
    if(!halt && emu->c->envinfo_exit != 0) {
      halt = true;
      exitcode = emu->c->envinfo_exitcode;
    }

    halt |= Verilated::gotFinish();

    if (halt && (!emu->UART_on || emu->UART_idle)) {
      break;
    }

    emu->tick(uart_in, uart_out);

    if(keys){
      emu->emu_keys();
    }
    emu->emu_extmem();
    
    if (reg_print && emu->c->core0_enable) {
      emu->print_state();
    }
  }

  emu->stopTrace();
  if(reg_print){
    printf("PASSED\n");
  }
  exit(exitcode);
}


