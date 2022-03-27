#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <machine/patmos.h>
#include <machine/spm.h>

#include "libaudio/audio_singlecore.h"
#include "libaudio/audio_singlecore.c"

//#define SIM

#define DETAILED_TIMING

#define INLINE_PREFIX   static inline
//#define INLINE_PREFIX

/* Timer related code */

static _iodev_ptr_t const timer_ptr_high = (_iodev_ptr_t)(PATMOS_IO_TIMER);
static _iodev_ptr_t const timer_ptr_low = (_iodev_ptr_t)(PATMOS_IO_TIMER + 0x04);

INLINE_PREFIX uint32_t get_time32()
{
  return *timer_ptr_low;
}

INLINE_PREFIX uint64_t get_time64()
{
  uint64_t ret = *timer_ptr_low;
  ret |= ((uint64_t)*timer_ptr_high) << 32;
  return ret;
}

/* GENERAL CONSTANTS */
const uint32_t BUFFER_SIZE = 128;

const uint32_t BLOCK_SIZE = 8;
const uint32_t DELAY_BUFFER_BLOCKS = 1 << 12;
const uint32_t DELAY_BUFFER_SIZE = DELAY_BUFFER_BLOCKS * BLOCK_SIZE;
int16_t delay_buffer[DELAY_BUFFER_SIZE] __attribute__((aligned(16)));

const uint32_t SAMPLING_FREQUENCY = 80000000 / (6 * 256);
const uint32_t BLOCK_FREQUENCY = SAMPLING_FREQUENCY / BLOCK_SIZE;

/* SW Implementation */
const int16_t dist_lut[1024] = {
    #include "lut.h"
};

INLINE_PREFIX int16_t gain1(int16_t s, uint8_t g)
{
    int32_t in = s;
    int32_t out = in * g >> 6;
    return out;
}

INLINE_PREFIX int16_t gain2(int16_t s, uint8_t g)
{
    int32_t in = s;
    int32_t out = in * g >> 5;
    if (out > (1 << 15) - 1)
        out = (1 << 15) - 1;
    else if (out < -(1 << 15))
        out = -(1 << 15);
    return out;
}

INLINE_PREFIX int16_t mix(int16_t s1, int16_t s2, uint8_t m)
{
    int32_t in1 = s1;
    int32_t in2 = s2;
    int32_t diff = in2 - in1;
    int32_t out = in1 + ((diff * m) >> 6);
    return out;
}

INLINE_PREFIX int16_t distortion(int16_t s, uint8_t g)
{
    uint16_t in = s;
    if (s < 0)
        in = -s;
    if (in == 1 << 15)
        in = in - 1;
        
    uint32_t gain = in * g;
    uint16_t index = gain >> 11;
    uint16_t fract = gain & ((1 << 11) - 1);
    
    int16_t lut1;
    int16_t lut2 = dist_lut[index];
    if (index == 0)
        lut1 = 0;
    else
        lut1 = dist_lut[index - 1];
    
    int16_t diff = lut2 - lut1;
    int16_t interp = ((int32_t)diff * fract) >> 11;
    
    int16_t out = lut1 + interp;    
    return s < 0 ? -out : out;
}

INLINE_PREFIX int16_t delay(int16_t s, uint16_t max_len, uint16_t len, uint8_t m, uint8_t f)
{
    static uint16_t cur_len = 8 * BLOCK_SIZE;
    static uint16_t wrPtr = 0;
    static int16_t interpVal = 0;
    static bool interp = false;
    
    int16_t del_in;
    int16_t del_out;
    int16_t out;
    
    uint16_t rdPtr;
    if (wrPtr < cur_len)
        rdPtr = wrPtr - cur_len + max_len + 1;
    else
        rdPtr = wrPtr - cur_len;
    
    if (cur_len < len)
    {
        del_in = delay_buffer[rdPtr];
        interpVal = del_in;
        cur_len += 1;
    }
    else if (cur_len > len)
    {
        if (!interp)
        {
            int16_t newInterpVal = delay_buffer[rdPtr]; 
            del_in = ((int32_t)interpVal + newInterpVal) >> 1;
            interpVal = newInterpVal;
            
            interp = true;
        }
        else
        {
            del_in = interpVal;
        
            interp = false;
            cur_len -= 1;
        }
    }
    else
    {
        del_in = delay_buffer[rdPtr];
        interpVal = del_in;
    }
        
    del_out = mix(s, del_in, f);
    delay_buffer[wrPtr] = del_out;
    
    if (wrPtr >= max_len)
        wrPtr = 0;
    else
        wrPtr++;
    
    out = mix(s, del_in, m);
    return out;
}

void swRun()
{
    volatile _SPM int16_t *sample_i;
    volatile _SPM int16_t *sample_o;
    volatile _SPM int16_t *dummy;

    const uint32_t SAMPLE_I_ADDR = 0;
    const uint32_t SAMPLE_O_ADDR = sizeof(int32_t);
    const uint32_t DUMMY_ADDR = 2 * sizeof(int32_t);

    sample_i = (volatile _SPM int16_t *) SAMPLE_I_ADDR;
    sample_o = (volatile _SPM int16_t *) SAMPLE_O_ADDR;
    dummy = (volatile _SPM int16_t *) DUMMY_ADDR;

    uint8_t mod_en = 3;
    uint8_t dist_gain = 64 * 3 / 20;
    uint8_t dist_outgain = 64 / 10;
    uint16_t del_maxlen = DELAY_BUFFER_SIZE - BLOCK_SIZE;
    uint16_t del_len = SAMPLING_FREQUENCY / 4;
    uint8_t del_mix = 64 / 4;
    uint8_t del_fb = 64 / 20;
    uint8_t del_outgain = 64 * 3 / 5;
    
    // Timed code starts here:
    uint32_t iterations = 0;
    uint64_t total_time = 0;
    uint64_t fx_time = 0;
   
    asm volatile ("" ::: "memory");
    uint64_t t_start = get_time64();
    asm volatile ("" ::: "memory");

    int16_t sample = 0;
    while(*keyReg & 1)
    {    
        // Read input sample from AudioInterface.
        #ifndef SIM 
            getInputBufferSPM(sample_i, dummy);
        #else
            *sample_i = sample + 2048;
        #endif
        
        sample = *sample_i;
        
        #ifdef DETAILED_TIMING
            asm volatile ("" ::: "memory");
            uint64_t start = get_time64();
            asm volatile ("" ::: "memory");
        #endif

        if (mod_en & 1)
        {
            sample = distortion(sample, dist_gain);
            sample = gain1(sample, dist_outgain);
        }
        
        if (mod_en & 2)
        {
            sample = delay(sample, del_maxlen, del_len, del_mix, del_fb);
            sample = gain2(sample, del_outgain);
        }
        
        #ifdef DETAILED_TIMING
            asm volatile ("" ::: "memory");
            uint64_t end = get_time64();
            fx_time += end - start;
            asm volatile ("" ::: "memory");
        #endif

        *sample_o = sample;
        
        // Write output sample to AudioInterface.
        #ifndef SIM
            setOutputBufferSPM(sample_o, sample_o);
        #endif
        
        iterations++;
    }
    
    asm volatile ("" ::: "memory");
    uint64_t t_end = get_time64();
    asm volatile ("" ::: "memory");
    
    total_time = t_end - t_start;
    
    printf("[SW] %ld iterations:\n  total time: %lld (%lld)\n  fx time: %lld (%lld)\n\n", iterations, total_time, total_time / iterations, fx_time, fx_time / iterations);
}



/* COP Implementation */
#define FUNC_MOD_EN         ".word 0x03433001"
#define FUNC_DIST_GAIN      ".word 0x03453001"
#define FUNC_DIST_OUTGAIN   ".word 0x03473001"
#define FUNC_DEL_BUF        ".word 0x03493001"
#define FUNC_DEL_LEN        ".word 0x034B3001"
#define FUNC_DEL_MAXLEN     ".word 0x034D3001"
#define FUNC_DEL_MIX        ".word 0x034F3001"
#define FUNC_DEL_FB         ".word 0x03513001"
#define FUNC_DEL_OUTGAIN    ".word 0x03533001"

void copRun()
{
    volatile _SPM int16_t *sample_i;
    volatile _SPM int16_t *sample_o;
    volatile _SPM int16_t *dummy;

    const uint32_t SAMPLE_I_ADDR = 0;
    const uint32_t SAMPLE_O_ADDR = sizeof(int32_t);
    const uint32_t DUMMY_ADDR = 2 * sizeof(int32_t);

    sample_i = (volatile _SPM int16_t *) SAMPLE_I_ADDR;
    sample_o = (volatile _SPM int16_t *) SAMPLE_O_ADDR;
    dummy = (volatile _SPM int16_t *) DUMMY_ADDR;

    // Set delay buffer address.
    register int16_t *delay_buffer_addr __asm__ ("19") = delay_buffer;
    asm (FUNC_DEL_BUF : : "r"(delay_buffer_addr));
    
    register uint32_t config_data __asm__ ("19");
    
    // Enable both modules.
    config_data = 3;
    asm (FUNC_MOD_EN : : "r"(config_data));

    // Set distortion gain to ~15%.
    config_data = 64 * 3 / 20;
    asm (FUNC_DIST_GAIN : : "r"(config_data));
    
    // Set distortion output gain to ~10%.
    config_data = 64 / 10;
    asm (FUNC_DIST_OUTGAIN : : "r"(config_data));
    
    // Set maximum delay length.
    config_data = DELAY_BUFFER_BLOCKS - 1;
    asm (FUNC_DEL_MAXLEN : : "r"(config_data));
    
    // Set delay length to ~250 ms.
    config_data = BLOCK_FREQUENCY / 4;
    asm (FUNC_DEL_LEN : : "r"(config_data));
    
    // Set mix to ~25%.
    config_data = 64 / 4;
    asm (FUNC_DEL_MIX : : "r"(config_data));

    // Set feedback to ~5%.
    config_data = 64 / 20;
    asm (FUNC_DEL_FB : : "r"(config_data));
    
    // Set delay output gain to ~120% (NOTE: 32 is 100%).
    config_data = 64 * 3 / 5;
    asm (FUNC_DEL_OUTGAIN : : "r"(config_data));
    
    // Timed code starts here:
    uint32_t iterations = 0;
    uint64_t total_time = 0;
    uint64_t fx_time = 0;
   
    asm volatile ("" ::: "memory");
    uint64_t t_start = get_time64();
    asm volatile ("" ::: "memory");
    
    // First block of samples.
    for (int i = 0; i < BLOCK_SIZE; ++i)
    {
        // Read input sample from AudioInterface.
        #ifndef SIM 
            getInputBufferSPM(sample_i, dummy);
        #else
            *sample_i = 20 + (i << 6);
        #endif
        
        #ifdef DETAILED_TIMING
            asm volatile ("" ::: "memory");
            uint64_t start = get_time64();
            asm volatile ("" ::: "memory");
        #endif
            
        // Move sample to Coprocessor.
        register int32_t sample_i_ext __asm__ ("19") = *sample_i;
        asm (".word 0x03413001"     // unpredicated COP_WRITE to COP0 with FUNC = 00000, RA = 10011, RB = 00000
            :
            : "r"(sample_i_ext));
        
        #ifdef DETAILED_TIMING
            asm volatile ("" ::: "memory");
            uint64_t end = get_time64();
            fx_time += end - start;
            asm volatile ("" ::: "memory");
        #endif
    }

    while(*keyReg & 2)
    {
        // Process block of input samples.
        for (int i = 0; i < BLOCK_SIZE; ++i)
        {
            // Read input sample from AudioInterface.
            #ifndef SIM 
                getInputBufferSPM(sample_i, dummy);
            #else
                *sample_i = 20 + (i << 6);
            #endif
            
            #ifdef DETAILED_TIMING
                asm volatile ("" ::: "memory");
                uint64_t start = get_time64();
                asm volatile ("" ::: "memory");
            #endif

            // Move sample to Coprocessor.
            register int32_t sample_i_ext __asm__ ("19") = *sample_i;
            asm (".word 0x03413001"     // unpredicated COP_WRITE to COP0 with FUNC = 00000, RA = 10011, RB = 00000
                :
                : "r"(sample_i_ext));
            
            #ifdef DETAILED_TIMING
                asm volatile ("" ::: "memory");
                uint64_t end = get_time64();
                fx_time += end - start;
                asm volatile ("" ::: "memory");
            #endif
        }
        
        // Process block of output samples.
        for (int i = 0; i < BLOCK_SIZE; ++i)
        {
            #ifdef DETAILED_TIMING
                asm volatile ("" ::: "memory");
                uint64_t start = get_time64();
                asm volatile ("" ::: "memory");
            #endif
            
            // Move sample from Coprocessor.
            register int32_t sample_o_ext __asm__ ("19");
            asm (".word 0x03660003"     // unpredicated COP_READ from COP0 with FUNC = 00000, RA = 00000, RD = 10011
                : "=r"(sample_o_ext)
                : 
                : "19" );
            
            #ifdef DETAILED_TIMING
                asm volatile ("" ::: "memory");
                uint64_t end = get_time64();
                fx_time += end - start;
                asm volatile ("" ::: "memory");
            #endif
            
            *sample_o = sample_o_ext;
        
            // Write output sample to AudioInterface.
            #ifndef SIM
                setOutputBufferSPM(sample_o, sample_o);
            #endif
            
            iterations++;
        }
    }
    
    // Last block of samples.
    for (int i = 0; i < BLOCK_SIZE; ++i)
    {
        #ifdef DETAILED_TIMING
            asm volatile ("" ::: "memory");
            uint64_t start = get_time64();
            asm volatile ("" ::: "memory");
        #endif
            
        // Move sample from Coprocessor.
        register int32_t sample_o_ext __asm__ ("19");
        asm (".word 0x03660003"     // unpredicated COP_READ from COP0 with FUNC = 00000, RA = 00000, RD = 10011
            : "=r"(sample_o_ext)
            : 
            : "19" );

        #ifdef DETAILED_TIMING
            asm volatile ("" ::: "memory");
            uint64_t end = get_time64();
            fx_time += end - start;
            asm volatile ("" ::: "memory");
        #endif
        
        *sample_o = sample_o_ext;
        
        // Write output sample to AudioInterface.
        #ifndef SIM
            setOutputBufferSPM(sample_o, sample_o);
        #endif
        
        iterations++;
    }
    
    asm volatile ("" ::: "memory");
    uint64_t t_end = get_time64();
    asm volatile ("" ::: "memory");
    
    total_time = t_end - t_start;
    
    printf("[COP] %ld iterations:\n  total time: %lld (%lld)\n  cop wait time: %lld (%lld)\n\n", iterations, total_time, total_time / iterations, fx_time, fx_time / iterations);
}

int main()
{
    #ifndef SIM
        setup(0);

        setInputBufferSize(BUFFER_SIZE);
        setOutputBufferSize(BUFFER_SIZE);

        *audioAdcEnReg = 1;
        *audioDacEnReg = 1;
    #endif
    
    while(*keyReg & 4) {
        swRun();
        copRun();
    }
    
    return EXIT_SUCCESS;
}
