#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <machine/patmos.h>
#include <machine/spm.h>

#include "libaudio/audio_singlecore.h"
#include "libaudio/audio_singlecore.c"


//#define SIM


const uint32_t BUFFER_SIZE = 128;

const uint32_t BLOCK_SIZE = 8;
const uint32_t DELAY_BUFFER_BLOCKS = 1 << 12;
const uint32_t DELAY_BUFFER_SIZE = DELAY_BUFFER_BLOCKS * BLOCK_SIZE;
int16_t delay_buffer[DELAY_BUFFER_SIZE] __attribute__((aligned(16)));

const uint32_t SAMPLING_FREQUENCY = 80000000 / (6 * 256);
const uint32_t BLOCK_FREQUENCY = SAMPLING_FREQUENCY / BLOCK_SIZE;

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
    
    // Set mix to ~20%.
    config_data = 64 / 4;
    asm (FUNC_DEL_MIX : : "r"(config_data));

    // Set feedback to ~5%.
    config_data = 64 / 20;
    asm (FUNC_DEL_FB : : "r"(config_data));
    
    // Set delay output gain to ~120% (NOTE: 32 is 100%).
    config_data = 64 * 3 / 5;
    asm (FUNC_DEL_OUTGAIN : : "r"(config_data));
    
    // First block of samples.
    for (int i = 0; i < BLOCK_SIZE; ++i)
    {
        // Read input sample from AudioInterface.
        #ifndef SIM 
            getInputBufferSPM(sample_i, dummy);
        #else
            *sample_i = 20 + (i << 6);
        #endif
        
        // Move sample to Coprocessor.
        register int32_t sample_i_ext __asm__ ("19") = *sample_i;
        asm (".word 0x03413001"     // unpredicated COP_WRITE to COP0 with FUNC = 00000, RA = 10011, RB = 00000
            :
            : "r"(sample_i_ext));
    }

    while(*keyReg != 3)
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
            
            // Move sample to Coprocessor.
            register int32_t sample_i_ext __asm__ ("19") = *sample_i;
            asm (".word 0x03413001"     // unpredicated COP_WRITE to COP0 with FUNC = 00000, RA = 10011, RB = 00000
                :
                : "r"(sample_i_ext));
        }
        
        // Process block of output samples.
        for (int i = 0; i < BLOCK_SIZE; ++i)
        {
            // Move sample from Coprocessor.
            register int32_t sample_o_ext __asm__ ("19");
            asm (".word 0x03660003"     // unpredicated COP_READ from COP0 with FUNC = 00000, RA = 00000, RD = 10011
                : "=r"(sample_o_ext)
                : 
                : "19" );
            *sample_o = sample_o_ext;
        
            // Write output sample to AudioInterface.
            #ifndef SIM
                setOutputBufferSPM(sample_o, sample_o);
            #endif
        }
    }
    
    // Last block of samples.
    for (int i = 0; i < BLOCK_SIZE; ++i)
    {
        // Move sample from Coprocessor.
        register int32_t sample_o_ext __asm__ ("19");
        asm (".word 0x03660003"     // unpredicated COP_READ from COP0 with FUNC = 00000, RA = 00000, RD = 10011
            : "=r"(sample_o_ext)
            : 
            : "19" );     
        *sample_o = sample_o_ext;

        // Write output sample to AudioInterface.
        #ifndef SIM
            setOutputBufferSPM(sample_o, sample_o);
        #endif
    }
}

int main()
{
    #ifndef SIM
        setup(0);

        setInputBufferSize(BUFFER_SIZE);
        setOutputBufferSize(BUFFER_SIZE);

        *audioAdcEnReg = 1;
        for(unsigned int i=0; i < BUFFER_SIZE * 1536; i++)
        {
            *audioDacEnReg = 0;
        }
        *audioDacEnReg = 1;
    #endif
    
    copRun();

    return EXIT_SUCCESS;
}
