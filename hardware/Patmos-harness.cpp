
#include "VPatmos.h"
#include "verilated.h"
#if VM_TRACE
#include "verilated_vcd_c.h"
#endif
#include <iostream>


// Override Verilator definition so first $finish ends simulation
// Note: VL_USER_FINISH needs to be defined when compiling Verilator code
void vl_finish(const char* filename, int linenum, const char* hier) {
  Verilated::flushCall();
  exit(0);
}

int main(int argc, char **argv, char **env) {
    Verilated::commandArgs(argc, argv);
    VerilatedVcdC *m_trace;
    VPatmos* top = new VPatmos;
    std::string vcdfile = "build/Patmos.vcd";
    std::vector<std::string> args(argv+1, argv+argc);
    std::vector<std::string>::const_iterator it;
    Verilated::traceEverOn(true);
    int cnt = 0;
    top->reset = 1;
    top->eval();
    top->clock = 1;
    top->eval();
    top->clock = 0;
    top->eval();
    top->clock = 1;
    top->eval();
    top->clock = 0;
    top->eval();
    top->reset = 0;
    top->eval();
    // Tick the clock until we are done
    printf("This is a hacked harness \n");
    while(!Verilated::gotFinish() && cnt != 20) {
	    top->clock = 1;
	    top->eval();
        printf("pc: %d \n", top->Patmos__DOT__cores_0__DOT__fetch__DOT__pcReg);
	    top->clock = 0;
	    top->eval();
        cnt++;
    }

    exit(0);
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
