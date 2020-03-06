
#include "VPatmos.h"
#include "verilated.h"
#include "veri_api.h"
#if VM_TRACE
#include "verilated_vcd_c.h"
#endif
#include <iostream>
class Patmos_api_t: public sim_api_t<VerilatorDataWrapper*> {
    public:
    Patmos_api_t(VPatmos* _dut) {
        dut = _dut;
        main_time = 0L;
        is_exit = false;
#if VM_TRACE
        tfp = NULL;
#endif
    }
    void init_sim_data() {
        sim_data.inputs.clear();
        sim_data.outputs.clear();
        sim_data.signals.clear();

        sim_data.inputs.push_back(new VerilatorCData(&(dut->clock)));
        sim_data.inputs.push_back(new VerilatorCData(&(dut->reset)));
        sim_data.inputs.push_back(new VerilatorSData(&(dut->io_SRamCtrl_ramIn_din)));
        sim_data.inputs.push_back(new VerilatorCData(&(dut->io_UartCmp_rx)));
        sim_data.inputs.push_back(new VerilatorCData(&(dut->io_Keys_key)));
        sim_data.outputs.push_back(new VerilatorCData(&(dut->io_SRamCtrl_ramOut_nub)));
        sim_data.outputs.push_back(new VerilatorCData(&(dut->io_SRamCtrl_ramOut_nlb)));
        sim_data.outputs.push_back(new VerilatorCData(&(dut->io_SRamCtrl_ramOut_nwe)));
        sim_data.outputs.push_back(new VerilatorCData(&(dut->io_SRamCtrl_ramOut_noe)));
        sim_data.outputs.push_back(new VerilatorCData(&(dut->io_SRamCtrl_ramOut_nce)));
        sim_data.outputs.push_back(new VerilatorSData(&(dut->io_SRamCtrl_ramOut_dout)));
        sim_data.outputs.push_back(new VerilatorCData(&(dut->io_SRamCtrl_ramOut_doutEna)));
        sim_data.outputs.push_back(new VerilatorIData(&(dut->io_SRamCtrl_ramOut_addr)));
        sim_data.outputs.push_back(new VerilatorCData(&(dut->io_UartCmp_tx)));
        sim_data.outputs.push_back(new VerilatorSData(&(dut->io_Leds_led)));
        sim_data.signals.push_back(new VerilatorCData(&(dut->reset)));
        sim_data.signal_map["Patmos.reset"] = 0;
    }
#if VM_TRACE
     void init_dump(VerilatedVcdC* _tfp) { tfp = _tfp; }
#endif
    inline bool exit() { return is_exit; }

    // required for sc_time_stamp()
    virtual inline double get_time_stamp() {
        return main_time;
    }

    private:
    VPatmos* dut;
    bool is_exit;
    vluint64_t main_time;
#if VM_TRACE
    VerilatedVcdC* tfp;
#endif
    virtual inline size_t put_value(VerilatorDataWrapper* &sig, uint64_t* data, bool force=false) {
        return sig->put_value(data);
    }
    virtual inline size_t get_value(VerilatorDataWrapper* &sig, uint64_t* data) {
        return sig->get_value(data);
    }
    virtual inline size_t get_chunk(VerilatorDataWrapper* &sig) {
        return sig->get_num_words();
    }
    virtual inline void reset() {
        dut->reset = 1;
        step();
    }
    virtual inline void start() {
        dut->reset = 0;
    }
    virtual inline void finish() {
        dut->eval();
        is_exit = true;
    }
    virtual inline void step() {
        dut->clock = 0;
        dut->eval();
#if VM_TRACE
        if (tfp) tfp->dump(main_time);
#endif
        main_time++;
        dut->clock = 1;
        dut->eval();
#if VM_TRACE
        if (tfp) tfp->dump(main_time);
#endif
        main_time++;
    }
    virtual inline void update() {
        // This seems to force a full eval of circuit, so registers with alternate clocks are update correctly
        dut->eval();
        // This was the original call, did not refresh registers when some  other clock transitioned
        // dut->_eval_settle(dut->__VlSymsp);
    }
};

// The following isn't strictly required unless we emit (possibly indirectly) something
// requiring a time-stamp (such as an assert).
static Patmos_api_t * _Top_api;
double sc_time_stamp () { return _Top_api->get_time_stamp(); }

// Override Verilator definition so first $finish ends simulation
// Note: VL_USER_FINISH needs to be defined when compiling Verilator code
void vl_finish(const char* filename, int linenum, const char* hier) {
  Verilated::flushCall();
  exit(0);
}

int main(int argc, char **argv, char **env) {
    Verilated::commandArgs(argc, argv);
    VPatmos* top = new VPatmos;
    std::string vcdfile = "build/Patmos.vcd";
    std::vector<std::string> args(argv+1, argv+argc);
    std::vector<std::string>::const_iterator it;
    int cnt = 0;
    int cntu = 0;
    unsigned char uartc = 'f';
    // Tick the clock until we are done
    printf("This is a hacked harness \n");
    while(!Verilated::gotFinish() && cnt != 10) {
	top->clock = 1;
	top->eval();
	top->clock = 0;
	top->eval();
        cnt++;
    }

    exit(0);
#if VM_TRACE
    Verilated::traceEverOn(true);
    VL_PRINTF("Enabling waves..");
    VerilatedVcdC* tfp = new VerilatedVcdC;
    top->trace(tfp, 99);
    tfp->open(vcdfile.c_str());
#endif
    Patmos_api_t api(top);
    _Top_api = &api; /* required for sc_time_stamp() */
    api.init_sim_data();
    api.init_channels();
#if VM_TRACE
    api.init_dump(tfp);
#endif
    //while(!api.exit()) api.tick();
#if VM_TRACE
    if (tfp) tfp->close();
    delete tfp;
#endif
    delete top;
    exit(0);
}
