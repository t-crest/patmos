/*
 * system.h - SOPC Builder system and BSP software package information
 *
 * Machine generated for CPU 'DOC_CPU' in SOPC Builder design 'DOC_Multi_Axis4'
 * SOPC Builder design path: ../../DOC_Multi_Axis4.sopcinfo
 *
 * Generated: Tue Mar 10 13:36:00 CET 2015
 */

/*
 * DO NOT MODIFY THIS FILE
 *
 * Changing this file will have subtle consequences
 * which will almost certainly lead to a nonfunctioning
 * system. If you do modify this file, be aware that your
 * changes will be overwritten and lost when this file
 * is generated again.
 *
 * DO NOT MODIFY THIS FILE
 */

/*
 * License Agreement
 *
 * Copyright (c) 2008
 * Altera Corporation, San Jose, California, USA.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * This agreement shall be governed in all respects by the laws of the State
 * of California and by the laws of the United States of America.
 */

#ifndef __SYSTEM_H_
#define __SYSTEM_H_

/* Include definitions from linker script generator */
//#include "linker.h"


/*
 * CPU configuration
 *
 */

//#define ALT_CPU_ARCHITECTURE "altera_nios2_qsys"
//#define ALT_CPU_BIG_ENDIAN 0
//#define ALT_CPU_BREAK_ADDR 0x00008820
//#define ALT_CPU_CPU_FREQ 100000000u
//#define ALT_CPU_CPU_ID_SIZE 1
//#define ALT_CPU_CPU_ID_VALUE 0x00000000
//#define ALT_CPU_CPU_IMPLEMENTATION "fast"
//#define ALT_CPU_DATA_ADDR_WIDTH 0x1d
//#define ALT_CPU_DCACHE_LINE_SIZE 32
//#define ALT_CPU_DCACHE_LINE_SIZE_LOG2 5
//#define ALT_CPU_DCACHE_SIZE 2048
//#define ALT_CPU_EXCEPTION_ADDR 0x00000020
//#define ALT_CPU_FLUSHDA_SUPPORTED
//#define ALT_CPU_FREQ 100000000
//#define ALT_CPU_HARDWARE_DIVIDE_PRESENT 0
//#define ALT_CPU_HARDWARE_MULTIPLY_PRESENT 1
//#define ALT_CPU_HARDWARE_MULX_PRESENT 0
//#define ALT_CPU_HAS_DEBUG_CORE 1
//#define ALT_CPU_HAS_DEBUG_STUB
//#define ALT_CPU_HAS_JMPI_INSTRUCTION
//#define ALT_CPU_ICACHE_LINE_SIZE 32
//#define ALT_CPU_ICACHE_LINE_SIZE_LOG2 5
//#define ALT_CPU_ICACHE_SIZE 4096
//#define ALT_CPU_INITDA_SUPPORTED
//#define ALT_CPU_INST_ADDR_WIDTH 0x1a
//#define ALT_CPU_NAME "DOC_CPU"
//#define ALT_CPU_NUM_OF_SHADOW_REG_SETS 0
//#define ALT_CPU_RESET_ADDR 0x03000000


/*
 * CPU configuration (with legacy prefix - don't use these anymore)
 *
 */

//#define NIOS2_BIG_ENDIAN 0
//#define NIOS2_BREAK_ADDR 0x00008820
//#define NIOS2_CPU_FREQ 100000000u
//#define NIOS2_CPU_ID_SIZE 1
//#define NIOS2_CPU_ID_VALUE 0x00000000
//#define NIOS2_CPU_IMPLEMENTATION "fast"
//#define NIOS2_DATA_ADDR_WIDTH 0x1d
//#define NIOS2_DCACHE_LINE_SIZE 32
//#define NIOS2_DCACHE_LINE_SIZE_LOG2 5
//#define NIOS2_DCACHE_SIZE 2048
//#define NIOS2_EXCEPTION_ADDR 0x00000020
//#define NIOS2_FLUSHDA_SUPPORTED
//#define NIOS2_HARDWARE_DIVIDE_PRESENT 0
//#define NIOS2_HARDWARE_MULTIPLY_PRESENT 1
//#define NIOS2_HARDWARE_MULX_PRESENT 0
//#define NIOS2_HAS_DEBUG_CORE 1
//#define NIOS2_HAS_DEBUG_STUB
//#define NIOS2_HAS_JMPI_INSTRUCTION
//#define NIOS2_ICACHE_LINE_SIZE 32
//#define NIOS2_ICACHE_LINE_SIZE_LOG2 5
//#define NIOS2_ICACHE_SIZE 4096
//#define NIOS2_INITDA_SUPPORTED
//#define NIOS2_INST_ADDR_WIDTH 0x1a
//#define NIOS2_NUM_OF_SHADOW_REG_SETS 0
//#define NIOS2_RESET_ADDR 0x03000000


/*
 * DOC_DC_Link configuration
 *
 */

#define ALT_MODULE_CLASS_DOC_DC_Link ssg_emb_dc_ballast
#define DOC_DC_LINK_BASE 0xF00B0100
#define DOC_DC_LINK_IRQ -1
#define DOC_DC_LINK_IRQ_INTERRUPT_CONTROLLER_ID -1
#define DOC_DC_LINK_NAME "/dev/DOC_DC_Link"
#define DOC_DC_LINK_SPAN 64
#define DOC_DC_LINK_TYPE "ssg_emb_dc_ballast"


/*
 * DOC_DC_Link_P configuration
 *
 */

#define ALT_MODULE_CLASS_DOC_DC_Link_P ssg_emb_dc_ballast
#define DOC_DC_LINK_P_BASE 0xF00B0200
#define DOC_DC_LINK_P_IRQ -1
#define DOC_DC_LINK_P_IRQ_INTERRUPT_CONTROLLER_ID -1
#define DOC_DC_LINK_P_NAME "/dev/DOC_DC_Link_P"
#define DOC_DC_LINK_P_SPAN 64
#define DOC_DC_LINK_P_TYPE "ssg_emb_dc_ballast"


/*
 * Define for each module class mastered by the CPU
 *
 */

#define __ALTERA_AVALON_JTAG_UART
#define __ALTERA_AVALON_NEW_SDRAM_CONTROLLER
#define __ALTERA_AVALON_ONCHIP_MEMORY2
#define __ALTERA_AVALON_PERFORMANCE_COUNTER
#define __ALTERA_AVALON_PIO
#define __ALTERA_AVALON_SYSID_QSYS
#define __ALTERA_GENERIC_TRISTATE_CONTROLLER
#define __ALTERA_NIOS2_QSYS
#define __ALTPLL
#define __DSPBA_AUTO_AVALON_IF_TOP
#define __ENDAT22_AVALON_IP
#define __MB119Y
#define __SSG_EMB_DC_BALLAST
#define __SSG_EMB_DSM
#define __SSG_EMB_PWM
#define __SSG_EMB_SD_ADC


/*
 * FOC_fixed_point configuration
 *
 */

//#define ALT_MODULE_CLASS_FOC_fixed_point dspba_auto_avalon_if_top
//#define FOC_FIXED_POINT_BASE 0x102000
//#define FOC_FIXED_POINT_IRQ -1
//#define FOC_FIXED_POINT_IRQ_INTERRUPT_CONTROLLER_ID -1
//#define FOC_FIXED_POINT_NAME "/dev/FOC_fixed_point"
//#define FOC_FIXED_POINT_SPAN 1024
//#define FOC_FIXED_POINT_TYPE "dspba_auto_avalon_if_top"


/*
 * FOC_floating_point configuration
 *
 */

//#define ALT_MODULE_CLASS_FOC_floating_point dspba_auto_avalon_if_top
//#define FOC_FLOATING_POINT_BASE 0x103000
//#define FOC_FLOATING_POINT_IRQ -1
//#define FOC_FLOATING_POINT_IRQ_INTERRUPT_CONTROLLER_ID -1
//#define FOC_FLOATING_POINT_NAME "/dev/FOC_floating_point"
//#define FOC_FLOATING_POINT_SPAN 1024
//#define FOC_FLOATING_POINT_TYPE "dspba_auto_avalon_if_top"


/*
 * IO_IN_Buttons configuration
 *
 */

#define ALT_MODULE_CLASS_IO_IN_Buttons altera_avalon_pio
#define IO_IN_BUTTONS_BASE 0xF00B0300
#define IO_IN_BUTTONS_BIT_CLEARING_EDGE_REGISTER 0
#define IO_IN_BUTTONS_BIT_MODIFYING_OUTPUT_REGISTER 0
#define IO_IN_BUTTONS_CAPTURE 1
#define IO_IN_BUTTONS_DATA_WIDTH 4
#define IO_IN_BUTTONS_DO_TEST_BENCH_WIRING 1
#define IO_IN_BUTTONS_DRIVEN_SIM_VALUE 0
#define IO_IN_BUTTONS_EDGE_TYPE "RISING"
#define IO_IN_BUTTONS_FREQ 50000000
#define IO_IN_BUTTONS_HAS_IN 1
#define IO_IN_BUTTONS_HAS_OUT 0
#define IO_IN_BUTTONS_HAS_TRI 0
#define IO_IN_BUTTONS_IRQ -1
#define IO_IN_BUTTONS_IRQ_INTERRUPT_CONTROLLER_ID -1
#define IO_IN_BUTTONS_IRQ_TYPE "NONE"
#define IO_IN_BUTTONS_NAME "/dev/IO_IN_Buttons"
#define IO_IN_BUTTONS_RESET_VALUE 0
#define IO_IN_BUTTONS_SPAN 16
#define IO_IN_BUTTONS_TYPE "altera_avalon_pio"


/*
 * System configuration
 *
 */

//#define ALT_DEVICE_FAMILY "Cyclone IV E"
//#define ALT_ENHANCED_INTERRUPT_API_PRESENT
//#define ALT_IRQ_BASE NULL
//#define ALT_LOG_PORT "/dev/null"
//#define ALT_LOG_PORT_BASE 0x0
//#define ALT_LOG_PORT_DEV null
//#define ALT_LOG_PORT_TYPE ""
//#define ALT_NUM_EXTERNAL_INTERRUPT_CONTROLLERS 0
//#define ALT_NUM_INTERNAL_INTERRUPT_CONTROLLERS 1
//#define ALT_NUM_INTERRUPT_CONTROLLERS 1
//#define ALT_STDERR "/dev/jtag_uart"
//#define ALT_STDERR_BASE 0x3800450
//#define ALT_STDERR_DEV jtag_uart
//#define ALT_STDERR_IS_JTAG_UART
//#define ALT_STDERR_PRESENT
//#define ALT_STDERR_TYPE "altera_avalon_jtag_uart"
//#define ALT_STDIN "/dev/jtag_uart"
//#define ALT_STDIN_BASE 0x3800450
//#define ALT_STDIN_DEV jtag_uart
//#define ALT_STDIN_IS_JTAG_UART
//#define ALT_STDIN_PRESENT
//#define ALT_STDIN_TYPE "altera_avalon_jtag_uart"
//#define ALT_STDOUT "/dev/jtag_uart"
//#define ALT_STDOUT_BASE 0x3800450
//#define ALT_STDOUT_DEV jtag_uart
//#define ALT_STDOUT_IS_JTAG_UART
//#define ALT_STDOUT_PRESENT
//#define ALT_STDOUT_TYPE "altera_avalon_jtag_uart"
//#define ALT_SYSTEM_NAME "DOC_Multi_Axis4"


/*
 * TCM_Data_Memory configuration
 *
 */

//#define ALT_MODULE_CLASS_TCM_Data_Memory altera_avalon_onchip_memory2
//#define TCM_DATA_MEMORY_ALLOW_IN_SYSTEM_MEMORY_CONTENT_EDITOR 0
//#define TCM_DATA_MEMORY_ALLOW_MRAM_SIM_CONTENTS_ONLY_FILE 0
//#define TCM_DATA_MEMORY_BASE 0x4000
//#define TCM_DATA_MEMORY_CONTENTS_INFO ""
//#define TCM_DATA_MEMORY_DUAL_PORT 0
//#define TCM_DATA_MEMORY_GUI_RAM_BLOCK_TYPE "AUTO"
//#define TCM_DATA_MEMORY_INIT_CONTENTS_FILE "DOC_Multi_Axis4_TCM_Data_Memory"
//#define TCM_DATA_MEMORY_INIT_MEM_CONTENT 1
//#define TCM_DATA_MEMORY_INSTANCE_ID "NONE"
//#define TCM_DATA_MEMORY_IRQ -1
//#define TCM_DATA_MEMORY_IRQ_INTERRUPT_CONTROLLER_ID -1
//#define TCM_DATA_MEMORY_NAME "/dev/TCM_Data_Memory"
//#define TCM_DATA_MEMORY_NON_DEFAULT_INIT_FILE_ENABLED 0
//#define TCM_DATA_MEMORY_RAM_BLOCK_TYPE "AUTO"
//#define TCM_DATA_MEMORY_READ_DURING_WRITE_MODE "DONT_CARE"
//#define TCM_DATA_MEMORY_SINGLE_CLOCK_OP 0
//#define TCM_DATA_MEMORY_SIZE_MULTIPLE 1
//#define TCM_DATA_MEMORY_SIZE_VALUE 8192
//#define TCM_DATA_MEMORY_SPAN 8192
//#define TCM_DATA_MEMORY_TYPE "altera_avalon_onchip_memory2"
//#define TCM_DATA_MEMORY_WRITABLE 1


/*
 * TCM_Instruction_Memory configuration
 *
 */

//#define ALT_MODULE_CLASS_TCM_Instruction_Memory altera_avalon_onchip_memory2
//#define TCM_INSTRUCTION_MEMORY_ALLOW_IN_SYSTEM_MEMORY_CONTENT_EDITOR 0
//#define TCM_INSTRUCTION_MEMORY_ALLOW_MRAM_SIM_CONTENTS_ONLY_FILE 0
//#define TCM_INSTRUCTION_MEMORY_BASE 0x0
//#define TCM_INSTRUCTION_MEMORY_CONTENTS_INFO ""
//#define TCM_INSTRUCTION_MEMORY_DUAL_PORT 1
//#define TCM_INSTRUCTION_MEMORY_GUI_RAM_BLOCK_TYPE "AUTO"
//#define TCM_INSTRUCTION_MEMORY_INIT_CONTENTS_FILE "DOC_Multi_Axis4_TCM_Instruction_Memory"
//#define TCM_INSTRUCTION_MEMORY_INIT_MEM_CONTENT 1
//#define TCM_INSTRUCTION_MEMORY_INSTANCE_ID "NONE"
//#define TCM_INSTRUCTION_MEMORY_IRQ -1
//#define TCM_INSTRUCTION_MEMORY_IRQ_INTERRUPT_CONTROLLER_ID -1
//#define TCM_INSTRUCTION_MEMORY_NAME "/dev/TCM_Instruction_Memory"
//#define TCM_INSTRUCTION_MEMORY_NON_DEFAULT_INIT_FILE_ENABLED 0
//#define TCM_INSTRUCTION_MEMORY_RAM_BLOCK_TYPE "AUTO"
//#define TCM_INSTRUCTION_MEMORY_READ_DURING_WRITE_MODE "DONT_CARE"
//#define TCM_INSTRUCTION_MEMORY_SINGLE_CLOCK_OP 0
//#define TCM_INSTRUCTION_MEMORY_SIZE_MULTIPLE 1
//#define TCM_INSTRUCTION_MEMORY_SIZE_VALUE 16384
//#define TCM_INSTRUCTION_MEMORY_SPAN 16384
//#define TCM_INSTRUCTION_MEMORY_TYPE "altera_avalon_onchip_memory2"
//#define TCM_INSTRUCTION_MEMORY_WRITABLE 1


/*
 * altpll_0 configuration
 *
 */

//#define ALTPLL_0_BASE 0x1980000
//#define ALTPLL_0_IRQ -1
//#define ALTPLL_0_IRQ_INTERRUPT_CONTROLLER_ID -1
//#define ALTPLL_0_NAME "/dev/altpll_0"
//#define ALTPLL_0_SPAN 16
//#define ALTPLL_0_TYPE "altpll"
//#define ALT_MODULE_CLASS_altpll_0 altpll


/*
 * cfi_flash configuration
 *
 */

//#define ALT_MODULE_CLASS_cfi_flash altera_generic_tristate_controller
//#define CFI_FLASH_BASE 0x3000000
//#define CFI_FLASH_HOLD_VALUE 60
//#define CFI_FLASH_IRQ -1
//#define CFI_FLASH_IRQ_INTERRUPT_CONTROLLER_ID -1
//#define CFI_FLASH_NAME "/dev/cfi_flash"
//#define CFI_FLASH_SETUP_VALUE 60
//#define CFI_FLASH_SIZE 8388608u
//#define CFI_FLASH_SPAN 8388608
//#define CFI_FLASH_TIMING_UNITS "ns"
//#define CFI_FLASH_TYPE "altera_generic_tristate_controller"
//#define CFI_FLASH_WAIT_VALUE 160


/*
 * hal configuration
 *
 */

#define ALT_MAX_FD 32
#define ALT_SYS_CLK none
#define ALT_TIMESTAMP_CLK none

 /*
 * pio_pfc configuration
 *
 */

#define ALT_MODULE_CLASS_pio_pfc altera_avalon_pio
#define PIO_PFC_BASE 0xF00B0080
#define PIO_PFC_BIT_CLEARING_EDGE_REGISTER 0
#define PIO_PFC_BIT_MODIFYING_OUTPUT_REGISTER 0
#define PIO_PFC_CAPTURE 0
#define PIO_PFC_DATA_WIDTH 2
#define PIO_PFC_DO_TEST_BENCH_WIRING 0
#define PIO_PFC_DRIVEN_SIM_VALUE 0
#define PIO_PFC_EDGE_TYPE "NONE"
#define PIO_PFC_FREQ 50000000
#define PIO_PFC_HAS_IN 1
#define PIO_PFC_HAS_OUT 1
#define PIO_PFC_HAS_TRI 0
#define PIO_PFC_IRQ -1
#define PIO_PFC_IRQ_INTERRUPT_CONTROLLER_ID -1
#define PIO_PFC_IRQ_TYPE "NONE"
#define PIO_PFC_NAME "/dev/pio_pfc"
#define PIO_PFC_RESET_VALUE 0
#define PIO_PFC_SPAN 16
#define PIO_PFC_TYPE "altera_avalon_pio"

/*
 * sysid_0 configuration
 *
 */

#define ALT_MODULE_CLASS_sysid_0 altera_avalon_sysid_qsys
#define SYSID_0_BASE 0xF00B0000
#define SYSID_0_ID 13709566
#define SYSID_0_IRQ -1
#define SYSID_0_IRQ_INTERRUPT_CONTROLLER_ID -1
#define SYSID_0_NAME "/dev/sysid_0"
#define SYSID_0_SPAN 8
#define SYSID_0_TIMESTAMP 1425989756
#define SYSID_0_TYPE "altera_avalon_sysid_qsys"

/*
 * drive0_DOC_ADC configuration
 *
 */

#define ALT_MODULE_CLASS_drive0_DOC_ADC ssg_emb_sd_adc
#define DRIVE0_DOC_ADC_BASE 0xF00B1000
#define DRIVE0_DOC_ADC_IRQ 1
#define DRIVE0_DOC_ADC_IRQ_INTERRUPT_CONTROLLER_ID 0
#define DRIVE0_DOC_ADC_NAME "/dev/drive0_DOC_ADC"
#define DRIVE0_DOC_ADC_SPAN 64
#define DRIVE0_DOC_ADC_TYPE "ssg_emb_sd_adc"


/*
 * drive0_DOC_ADC_POW configuration
 *
 */

#define ALT_MODULE_CLASS_drive0_DOC_ADC_POW ssg_emb_sd_adc
#define DRIVE0_DOC_ADC_POW_BASE 0xF00B1080
#define DRIVE0_DOC_ADC_POW_IRQ -1
#define DRIVE0_DOC_ADC_POW_IRQ_INTERRUPT_CONTROLLER_ID -1
#define DRIVE0_DOC_ADC_POW_NAME "/dev/drive0_DOC_ADC_POW"
#define DRIVE0_DOC_ADC_POW_SPAN 64
#define DRIVE0_DOC_ADC_POW_TYPE "ssg_emb_sd_adc"


/*
 * drive0_DOC_Biss configuration
 *
 */

#define ALT_MODULE_CLASS_drive0_DOC_Biss mb119y
#define DRIVE0_DOC_BISS_BASE 0xF00B2000
#define DRIVE0_DOC_BISS_IRQ -1
#define DRIVE0_DOC_BISS_IRQ_INTERRUPT_CONTROLLER_ID -1
#define DRIVE0_DOC_BISS_NAME "/dev/drive0_DOC_Biss"
#define DRIVE0_DOC_BISS_SPAN 256
#define DRIVE0_DOC_BISS_TYPE "mb119y"


/*
 * drive0_DOC_EnDat configuration
 *
 */

//#define ALT_MODULE_CLASS_drive0_DOC_EnDat EnDat22_AVALON_IP
//#define DRIVE0_DOC_ENDAT_BASE 0x3900400
//#define DRIVE0_DOC_ENDAT_IRQ -1
//#define DRIVE0_DOC_ENDAT_IRQ_INTERRUPT_CONTROLLER_ID -1
//#define DRIVE0_DOC_ENDAT_NAME "/dev/drive0_DOC_EnDat"
//#define DRIVE0_DOC_ENDAT_SPAN 1024
//#define DRIVE0_DOC_ENDAT_TYPE "EnDat22_AVALON_IP"


/*
 * drive0_DOC_PWM configuration
 *
 */

#define ALT_MODULE_CLASS_drive0_DOC_PWM ssg_emb_pwm
#define DRIVE0_DOC_PWM_BASE 0xF00B1040
#define DRIVE0_DOC_PWM_IRQ -1
#define DRIVE0_DOC_PWM_IRQ_INTERRUPT_CONTROLLER_ID -1
#define DRIVE0_DOC_PWM_NAME "/dev/drive0_DOC_PWM"
#define DRIVE0_DOC_PWM_SPAN 64
#define DRIVE0_DOC_PWM_TYPE "ssg_emb_pwm"


/*
 * drive0_DOC_SM configuration
 *
 */

#define ALT_MODULE_CLASS_drive0_DOC_SM ssg_emb_dsm
#define DRIVE0_DOC_SM_BASE 0xF00B10C0
#define DRIVE0_DOC_SM_IRQ -1
#define DRIVE0_DOC_SM_IRQ_INTERRUPT_CONTROLLER_ID -1
#define DRIVE0_DOC_SM_NAME "/dev/drive0_DOC_SM"
#define DRIVE0_DOC_SM_SPAN 8
#define DRIVE0_DOC_SM_TYPE "ssg_emb_dsm"


///*
// * drive1_DOC_ADC configuration
// *
// */
//
//#define ALT_MODULE_CLASS_drive1_DOC_ADC ssg_emb_sd_adc
//#define DRIVE1_DOC_ADC_BASE 0x3a00100
//#define DRIVE1_DOC_ADC_IRQ -1
//#define DRIVE1_DOC_ADC_IRQ_INTERRUPT_CONTROLLER_ID -1
//#define DRIVE1_DOC_ADC_NAME "/dev/drive1_DOC_ADC"
//#define DRIVE1_DOC_ADC_SPAN 64
//#define DRIVE1_DOC_ADC_TYPE "ssg_emb_sd_adc"
//
//
///*
// * drive1_DOC_ADC_POW configuration
// *
// */
//
//#define ALT_MODULE_CLASS_drive1_DOC_ADC_POW ssg_emb_sd_adc
//#define DRIVE1_DOC_ADC_POW_BASE 0x3a00800
//#define DRIVE1_DOC_ADC_POW_IRQ -1
//#define DRIVE1_DOC_ADC_POW_IRQ_INTERRUPT_CONTROLLER_ID -1
//#define DRIVE1_DOC_ADC_POW_NAME "/dev/drive1_DOC_ADC_POW"
//#define DRIVE1_DOC_ADC_POW_SPAN 64
//#define DRIVE1_DOC_ADC_POW_TYPE "ssg_emb_sd_adc"
//
//
///*
// * drive1_DOC_Biss configuration
// *
// */
//
//#define ALT_MODULE_CLASS_drive1_DOC_Biss mb119y
//#define DRIVE1_DOC_BISS_BASE 0x3a00900
//#define DRIVE1_DOC_BISS_IRQ -1
//#define DRIVE1_DOC_BISS_IRQ_INTERRUPT_CONTROLLER_ID -1
//#define DRIVE1_DOC_BISS_NAME "/dev/drive1_DOC_Biss"
//#define DRIVE1_DOC_BISS_SPAN 256
//#define DRIVE1_DOC_BISS_TYPE "mb119y"
//
//
///*
// * drive1_DOC_EnDat configuration
// *
// */
//
//#define ALT_MODULE_CLASS_drive1_DOC_EnDat EnDat22_AVALON_IP
//#define DRIVE1_DOC_ENDAT_BASE 0x3a00400
//#define DRIVE1_DOC_ENDAT_IRQ -1
//#define DRIVE1_DOC_ENDAT_IRQ_INTERRUPT_CONTROLLER_ID -1
//#define DRIVE1_DOC_ENDAT_NAME "/dev/drive1_DOC_EnDat"
//#define DRIVE1_DOC_ENDAT_SPAN 1024
//#define DRIVE1_DOC_ENDAT_TYPE "EnDat22_AVALON_IP"
//
//
///*
// * drive1_DOC_PWM configuration
// *
// */
//
//#define ALT_MODULE_CLASS_drive1_DOC_PWM ssg_emb_pwm
//#define DRIVE1_DOC_PWM_BASE 0x3a00200
//#define DRIVE1_DOC_PWM_IRQ -1
//#define DRIVE1_DOC_PWM_IRQ_INTERRUPT_CONTROLLER_ID -1
//#define DRIVE1_DOC_PWM_NAME "/dev/drive1_DOC_PWM"
//#define DRIVE1_DOC_PWM_SPAN 64
//#define DRIVE1_DOC_PWM_TYPE "ssg_emb_pwm"
//
//
///*
// * drive1_DOC_SM configuration
// *
// */
//
//#define ALT_MODULE_CLASS_drive1_DOC_SM ssg_emb_dsm
//#define DRIVE1_DOC_SM_BASE 0x3a00040
//#define DRIVE1_DOC_SM_IRQ -1
//#define DRIVE1_DOC_SM_IRQ_INTERRUPT_CONTROLLER_ID -1
//#define DRIVE1_DOC_SM_NAME "/dev/drive1_DOC_SM"
//#define DRIVE1_DOC_SM_SPAN 8
//#define DRIVE1_DOC_SM_TYPE "ssg_emb_dsm"
//
//
///*
// * drive2_DOC_ADC configuration
// *
// */
//
//#define ALT_MODULE_CLASS_drive2_DOC_ADC ssg_emb_sd_adc
//#define DRIVE2_DOC_ADC_BASE 0x3c00100
//#define DRIVE2_DOC_ADC_IRQ -1
//#define DRIVE2_DOC_ADC_IRQ_INTERRUPT_CONTROLLER_ID -1
//#define DRIVE2_DOC_ADC_NAME "/dev/drive2_DOC_ADC"
//#define DRIVE2_DOC_ADC_SPAN 64
//#define DRIVE2_DOC_ADC_TYPE "ssg_emb_sd_adc"
//
//
///*
// * drive2_DOC_ADC_POW configuration
// *
// */
//
//#define ALT_MODULE_CLASS_drive2_DOC_ADC_POW ssg_emb_sd_adc
//#define DRIVE2_DOC_ADC_POW_BASE 0x3c00800
//#define DRIVE2_DOC_ADC_POW_IRQ -1
//#define DRIVE2_DOC_ADC_POW_IRQ_INTERRUPT_CONTROLLER_ID -1
//#define DRIVE2_DOC_ADC_POW_NAME "/dev/drive2_DOC_ADC_POW"
//#define DRIVE2_DOC_ADC_POW_SPAN 64
//#define DRIVE2_DOC_ADC_POW_TYPE "ssg_emb_sd_adc"
//
//
///*
// * drive2_DOC_Biss configuration
// *
// */
//
//#define ALT_MODULE_CLASS_drive2_DOC_Biss mb119y
//#define DRIVE2_DOC_BISS_BASE 0x3c00900
//#define DRIVE2_DOC_BISS_IRQ -1
//#define DRIVE2_DOC_BISS_IRQ_INTERRUPT_CONTROLLER_ID -1
//#define DRIVE2_DOC_BISS_NAME "/dev/drive2_DOC_Biss"
//#define DRIVE2_DOC_BISS_SPAN 256
//#define DRIVE2_DOC_BISS_TYPE "mb119y"
//
//
///*
// * drive2_DOC_EnDat configuration
// *
// */
//
//#define ALT_MODULE_CLASS_drive2_DOC_EnDat EnDat22_AVALON_IP
//#define DRIVE2_DOC_ENDAT_BASE 0x3c00400
//#define DRIVE2_DOC_ENDAT_IRQ -1
//#define DRIVE2_DOC_ENDAT_IRQ_INTERRUPT_CONTROLLER_ID -1
//#define DRIVE2_DOC_ENDAT_NAME "/dev/drive2_DOC_EnDat"
//#define DRIVE2_DOC_ENDAT_SPAN 1024
//#define DRIVE2_DOC_ENDAT_TYPE "EnDat22_AVALON_IP"
//
//
///*
// * drive2_DOC_PWM configuration
// *
// */
//
//#define ALT_MODULE_CLASS_drive2_DOC_PWM ssg_emb_pwm
//#define DRIVE2_DOC_PWM_BASE 0x3c00200
//#define DRIVE2_DOC_PWM_IRQ -1
//#define DRIVE2_DOC_PWM_IRQ_INTERRUPT_CONTROLLER_ID -1
//#define DRIVE2_DOC_PWM_NAME "/dev/drive2_DOC_PWM"
//#define DRIVE2_DOC_PWM_SPAN 64
//#define DRIVE2_DOC_PWM_TYPE "ssg_emb_pwm"
//
//
///*
// * drive2_DOC_SM configuration
// *
// */
//
//#define ALT_MODULE_CLASS_drive2_DOC_SM ssg_emb_dsm
//#define DRIVE2_DOC_SM_BASE 0x3c00040
//#define DRIVE2_DOC_SM_IRQ -1
//#define DRIVE2_DOC_SM_IRQ_INTERRUPT_CONTROLLER_ID -1
//#define DRIVE2_DOC_SM_NAME "/dev/drive2_DOC_SM"
//#define DRIVE2_DOC_SM_SPAN 8
//#define DRIVE2_DOC_SM_TYPE "ssg_emb_dsm"
//
//
///*
// * drive3_DOC_ADC configuration
// *
// */
//
//#define ALT_MODULE_CLASS_drive3_DOC_ADC ssg_emb_sd_adc
//#define DRIVE3_DOC_ADC_BASE 0x3e00100
//#define DRIVE3_DOC_ADC_IRQ -1
//#define DRIVE3_DOC_ADC_IRQ_INTERRUPT_CONTROLLER_ID -1
//#define DRIVE3_DOC_ADC_NAME "/dev/drive3_DOC_ADC"
//#define DRIVE3_DOC_ADC_SPAN 64
//#define DRIVE3_DOC_ADC_TYPE "ssg_emb_sd_adc"
//
//
///*
// * drive3_DOC_ADC_POW configuration
// *
// */
//
//#define ALT_MODULE_CLASS_drive3_DOC_ADC_POW ssg_emb_sd_adc
//#define DRIVE3_DOC_ADC_POW_BASE 0x3d00800
//#define DRIVE3_DOC_ADC_POW_IRQ -1
//#define DRIVE3_DOC_ADC_POW_IRQ_INTERRUPT_CONTROLLER_ID -1
//#define DRIVE3_DOC_ADC_POW_NAME "/dev/drive3_DOC_ADC_POW"
//#define DRIVE3_DOC_ADC_POW_SPAN 64
//#define DRIVE3_DOC_ADC_POW_TYPE "ssg_emb_sd_adc"
//
//
///*
// * drive3_DOC_Biss configuration
// *
// */
//
//#define ALT_MODULE_CLASS_drive3_DOC_Biss mb119y
//#define DRIVE3_DOC_BISS_BASE 0x3d00900
//#define DRIVE3_DOC_BISS_IRQ -1
//#define DRIVE3_DOC_BISS_IRQ_INTERRUPT_CONTROLLER_ID -1
//#define DRIVE3_DOC_BISS_NAME "/dev/drive3_DOC_Biss"
//#define DRIVE3_DOC_BISS_SPAN 256
//#define DRIVE3_DOC_BISS_TYPE "mb119y"
//
//
///*
// * drive3_DOC_EnDat configuration
// *
// */
//
//#define ALT_MODULE_CLASS_drive3_DOC_EnDat EnDat22_AVALON_IP
//#define DRIVE3_DOC_ENDAT_BASE 0x3e00400
//#define DRIVE3_DOC_ENDAT_IRQ -1
//#define DRIVE3_DOC_ENDAT_IRQ_INTERRUPT_CONTROLLER_ID -1
//#define DRIVE3_DOC_ENDAT_NAME "/dev/drive3_DOC_EnDat"
//#define DRIVE3_DOC_ENDAT_SPAN 1024
//#define DRIVE3_DOC_ENDAT_TYPE "EnDat22_AVALON_IP"
//
//
///*
// * drive3_DOC_PWM configuration
// *
// */
//
//#define ALT_MODULE_CLASS_drive3_DOC_PWM ssg_emb_pwm
//#define DRIVE3_DOC_PWM_BASE 0x3e00200
//#define DRIVE3_DOC_PWM_IRQ -1
//#define DRIVE3_DOC_PWM_IRQ_INTERRUPT_CONTROLLER_ID -1
//#define DRIVE3_DOC_PWM_NAME "/dev/drive3_DOC_PWM"
//#define DRIVE3_DOC_PWM_SPAN 64
//#define DRIVE3_DOC_PWM_TYPE "ssg_emb_pwm"
//
//
///*
// * drive3_DOC_SM configuration
// *
// */
//
//#define ALT_MODULE_CLASS_drive3_DOC_SM ssg_emb_dsm
//#define DRIVE3_DOC_SM_BASE 0x3e00040
//#define DRIVE3_DOC_SM_IRQ -1
//#define DRIVE3_DOC_SM_IRQ_INTERRUPT_CONTROLLER_ID -1
//#define DRIVE3_DOC_SM_NAME "/dev/drive3_DOC_SM"
//#define DRIVE3_DOC_SM_SPAN 8
//#define DRIVE3_DOC_SM_TYPE "ssg_emb_dsm"




/*
 * jtag_uart configuration
 *
 */

//#define ALT_MODULE_CLASS_jtag_uart altera_avalon_jtag_uart
//#define JTAG_UART_BASE 0x3800450
//#define JTAG_UART_IRQ 0
//#define JTAG_UART_IRQ_INTERRUPT_CONTROLLER_ID 0
//#define JTAG_UART_NAME "/dev/jtag_uart"
//#define JTAG_UART_READ_DEPTH 64
//#define JTAG_UART_READ_THRESHOLD 8
//#define JTAG_UART_SPAN 8
//#define JTAG_UART_TYPE "altera_avalon_jtag_uart"
//#define JTAG_UART_WRITE_DEPTH 64
//#define JTAG_UART_WRITE_THRESHOLD 8


/*
 * performance_counter_0 configuration
 *
 */

//#define ALT_MODULE_CLASS_performance_counter_0 altera_avalon_performance_counter
//#define PERFORMANCE_COUNTER_0_BASE 0x8000
//#define PERFORMANCE_COUNTER_0_HOW_MANY_SECTIONS 3
//#define PERFORMANCE_COUNTER_0_IRQ -1
//#define PERFORMANCE_COUNTER_0_IRQ_INTERRUPT_CONTROLLER_ID -1
//#define PERFORMANCE_COUNTER_0_NAME "/dev/performance_counter_0"
//#define PERFORMANCE_COUNTER_0_SPAN 64
//#define PERFORMANCE_COUNTER_0_TYPE "altera_avalon_performance_counter"




/*
 * sdram configuration
 *
 */

//#define ALT_MODULE_CLASS_sdram altera_avalon_new_sdram_controller
//#define SDRAM_BASE 0x2000000
//#define SDRAM_CAS_LATENCY 2
//#define SDRAM_CONTENTS_INFO
//#define SDRAM_INIT_NOP_DELAY 0.0
//#define SDRAM_INIT_REFRESH_COMMANDS 2
//#define SDRAM_IRQ -1
//#define SDRAM_IRQ_INTERRUPT_CONTROLLER_ID -1
//#define SDRAM_IS_INITIALIZED 1
//#define SDRAM_NAME "/dev/sdram"
//#define SDRAM_POWERUP_DELAY 100.0
//#define SDRAM_REFRESH_PERIOD 15.625
//#define SDRAM_REGISTER_DATA_IN 1
//#define SDRAM_SDRAM_ADDR_WIDTH 0x16
//#define SDRAM_SDRAM_BANK_WIDTH 2
//#define SDRAM_SDRAM_COL_WIDTH 8
//#define SDRAM_SDRAM_DATA_WIDTH 32
//#define SDRAM_SDRAM_NUM_BANKS 4
//#define SDRAM_SDRAM_NUM_CHIPSELECTS 1
//#define SDRAM_SDRAM_ROW_WIDTH 12
//#define SDRAM_SHARED_DATA 0
//#define SDRAM_SIM_MODEL_BASE 1
//#define SDRAM_SPAN 16777216
//#define SDRAM_STARVATION_INDICATOR 0
//#define SDRAM_TRISTATE_BRIDGE_SLAVE ""
//#define SDRAM_TYPE "altera_avalon_new_sdram_controller"
//#define SDRAM_T_AC 5.5
//#define SDRAM_T_MRD 3
//#define SDRAM_T_RCD 20.0
//#define SDRAM_T_RFC 70.0
//#define SDRAM_T_RP 20.0
//#define SDRAM_T_WR 14.0


/*
 * svm_dump_mem configuration
 *
 */

#define ALT_MODULE_CLASS_svm_dump_mem altera_avalon_onchip_memory2
#define SVM_DUMP_MEM_ALLOW_IN_SYSTEM_MEMORY_CONTENT_EDITOR 1
#define SVM_DUMP_MEM_ALLOW_MRAM_SIM_CONTENTS_ONLY_FILE 0
#define SVM_DUMP_MEM_BASE 0x4000000
#define SVM_DUMP_MEM_CONTENTS_INFO ""
#define SVM_DUMP_MEM_DUAL_PORT 0
#define SVM_DUMP_MEM_GUI_RAM_BLOCK_TYPE "AUTO"
#define SVM_DUMP_MEM_INIT_CONTENTS_FILE "DOC_Multi_Axis4_svm_dump_mem"
#define SVM_DUMP_MEM_INIT_MEM_CONTENT 0
#define SVM_DUMP_MEM_INSTANCE_ID "SVM"
#define SVM_DUMP_MEM_IRQ -1
#define SVM_DUMP_MEM_IRQ_INTERRUPT_CONTROLLER_ID -1
#define SVM_DUMP_MEM_NAME "/dev/svm_dump_mem"
#define SVM_DUMP_MEM_NON_DEFAULT_INIT_FILE_ENABLED 0
#define SVM_DUMP_MEM_RAM_BLOCK_TYPE "AUTO"
#define SVM_DUMP_MEM_READ_DURING_WRITE_MODE "DONT_CARE"
#define SVM_DUMP_MEM_SINGLE_CLOCK_OP 0
#define SVM_DUMP_MEM_SIZE_MULTIPLE 1
#define SVM_DUMP_MEM_SIZE_VALUE 65536
#define SVM_DUMP_MEM_SPAN 65536
#define SVM_DUMP_MEM_TYPE "altera_avalon_onchip_memory2"
#define SVM_DUMP_MEM_WRITABLE 1


/*
 * sys_console_debug_ram configuration
 *
 */

#define ALT_MODULE_CLASS_sys_console_debug_ram altera_avalon_onchip_memory2
#define SYS_CONSOLE_DEBUG_RAM_ALLOW_IN_SYSTEM_MEMORY_CONTENT_EDITOR 0
#define SYS_CONSOLE_DEBUG_RAM_ALLOW_MRAM_SIM_CONTENTS_ONLY_FILE 0
#define SYS_CONSOLE_DEBUG_RAM_BASE 0x3800000
#define SYS_CONSOLE_DEBUG_RAM_CONTENTS_INFO ""
#define SYS_CONSOLE_DEBUG_RAM_DUAL_PORT 0
#define SYS_CONSOLE_DEBUG_RAM_GUI_RAM_BLOCK_TYPE "AUTO"
#define SYS_CONSOLE_DEBUG_RAM_INIT_CONTENTS_FILE "DOC_Multi_Axis4_sys_console_debug_ram"
#define SYS_CONSOLE_DEBUG_RAM_INIT_MEM_CONTENT 1
#define SYS_CONSOLE_DEBUG_RAM_INSTANCE_ID "NONE"
#define SYS_CONSOLE_DEBUG_RAM_IRQ -1
#define SYS_CONSOLE_DEBUG_RAM_IRQ_INTERRUPT_CONTROLLER_ID -1
#define SYS_CONSOLE_DEBUG_RAM_NAME "/dev/sys_console_debug_ram"
#define SYS_CONSOLE_DEBUG_RAM_NON_DEFAULT_INIT_FILE_ENABLED 0
#define SYS_CONSOLE_DEBUG_RAM_RAM_BLOCK_TYPE "AUTO"
#define SYS_CONSOLE_DEBUG_RAM_READ_DURING_WRITE_MODE "DONT_CARE"
#define SYS_CONSOLE_DEBUG_RAM_SINGLE_CLOCK_OP 0
#define SYS_CONSOLE_DEBUG_RAM_SIZE_MULTIPLE 1
#define SYS_CONSOLE_DEBUG_RAM_SIZE_VALUE 1024
#define SYS_CONSOLE_DEBUG_RAM_SPAN 1024
#define SYS_CONSOLE_DEBUG_RAM_TYPE "altera_avalon_onchip_memory2"
#define SYS_CONSOLE_DEBUG_RAM_WRITABLE 1

#endif /* __SYSTEM_H_ */
