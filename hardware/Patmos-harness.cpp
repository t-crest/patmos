
#include <fstream>
#include <iostream>
#include "VPatmos.h"
#include "verilated.h"
#if VM_TRACE
#include "verilated_vcd_c.h"
#endif
#include <iostream>
#include <string>

using namespace std;

class Emulator
{
  unsigned long m_tickcount;
  VPatmos *c;

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
  ostream* outputTarget = &std::cout;

public:
  Emulator(void)
  {
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
  }

  ~Emulator(void)
  {
    delete c;
    c = NULL;
  }

  void reset(void)
  {
    c->reset = 1;
    // Make sure any inheritance gets applied
    this->tick();
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

    // Toggle the clock
    // Rising edge
    c->clock = 1;
    c->eval();
    // Falling edge
    c->clock = 0;
    c->eval();

    //UART emulation
    if (UART_on)
    {
      UART_tick();
    }
  }

  long int get_tick_count(void)
  {
    return m_tickcount;
  }

  bool done(void) { return (Verilated::gotFinish()); }

  // UART methods below
  void UART_init(int set_baudrate, int set_freq)
  {
    baudrate = set_baudrate;
    freq = set_freq;
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

  void UART_to_file(string path){
    static ofstream outFile;
    if (!outFile.is_open()){
      outFile.open(path);
    }
    outputTarget = &outFile;
  }

  void UART_to_console(void){
    outputTarget = &cout;
  }
};

// Override Verilator definition so first $finish ends simulation
// Note: VL_USER_FINISH needs to be defined when compiling Verilator code
void vl_finish(const char *filename, int linenum, const char *hier)
{
  Verilated::flushCall();
  exit(0);
}

int main(int argc, char **argv, char **env)
{
  Verilated::commandArgs(argc, argv);
  Emulator *emu = new Emulator();
  emu->reset();
  emu->tick();
  emu->UART_init(115200, 80000000);
  //emu->UART_to_file("uart_dump.txt");
  emu->tick();
  emu->UART_write("hej");

  while (!emu->done() && emu->get_tick_count() != 10000000)
  {
    emu->tick();
    //printf("%d \n", emu->get_tick_count());
  }
  printf("This is a hacked harness \n");
  exit(EXIT_SUCCESS);

  /*#if VM_TRACE
    Verilated::traceEverOn(true);
    VL_PRINTF("Enabling waves..");
    VerilatedVcdC* tfp = new VerilatedVcdC;
    top->trace(tfp, 99);
    tfp->open(vcdfile.c_str());
#endif

#if VM_TRACE
    if (tfp) tfp->close();
    delete tfp;
#endif
    delete top;
    exit(0);*/
}
