#ifndef PLATFORM_H
#define PLATFORM_H

#include <libs/libTinyosPatmos.h>

#define _IODEV __attribute__((address_space(1)))
#define _UNCACHED __attribute__((address_space(3)))

//----------------------PINS-----------------------------------------------------
#define LED (*((volatile _IODEV int *)0xf0090000))
#define KEYS (*((volatile _IODEV int *)0xf00a0000))

//----------------------UART-----------------------------------------------------
#define UART_BASE_ADDR 0xf0080000
#define UART2_BASE_ADDR 0xF00E0000

#define UART_CTRL (*((volatile _IODEV int *)UART_BASE_ADDR))
#define UART_BUFFER (*((volatile _IODEV int *)(UART_BASE_ADDR+4)))

#define TX_TRANSMIT_READY 0
#define RX_DATA_AVAILABLE 1

// check which address is correct
#define UART2_CTRL (*((volatile _IODEV int *)UART2_BASE_ADDR))
#define UART2_BUFFER (*((volatile _IODEV int *)(UART2_BASE_ADDR+4)))

//----------------------DEADLINE---------------------------------------------------
#define DEADLINE (*((volatile _IODEV int *)0xf0030000))

//----------------------CPU_INFO---------------------------------------------------
#define EXTMEM_BURST_MASK 0x0000FF00
#define EXTMEM_BURST_SHIFT 8
#define EXTMEM_COMBINEDWRITES_MASK 0x000000FF

typedef struct
{
	uint8_t burstLength; //8-15 bit
	uint8_t combinedWritesToExtMem; //7-0 bit
} extMemConf_t;

#define CACHE_TYPE_MASK 0xFF000000
#define CACHE_TYPE_SHIFT 24
#define CACHE_POLICY_MASK 0x00FF0000
#define CACHE_POLICY_SHIFT 16
#define CACHE_ASSOCIATIVITY_MASK 0x0000FFFF

typedef struct
{
	uint8_t cacheType; //31-24
	uint8_t replacementPolicy; //23-16
	uint16_t associativity; //15-0 
} chacheConf_t;

#define CPUINFO_BASE_ADDR 0xF0000000

#define CPU_INFO_CORE_ID (*((volatile _IODEV int *)CPUINFO_BASE_ADDR))
#define CPU_INFO_FREQ (*((volatile _IODEV int *)(CPUINFO_BASE_ADDR+4)))
#define CPU_INFO_NUM_CORES (*((volatile _IODEV int *)(CPUINFO_BASE_ADDR+8)))
#define CPU_INFO_FEATURES (*((volatile _IODEV int *)(CPUINFO_BASE_ADDR+12)))
#define CPU_INFO_EXTMEM_SIZE (*((volatile _IODEV int *)(CPUINFO_BASE_ADDR+16)))
#define CPU_INFO_EXTMEM_CONF (*((volatile _IODEV int *)(CPUINFO_BASE_ADDR+20)))
#define CPU_INFO_ICACHE_SIZE (*((volatile _IODEV int *)(CPUINFO_BASE_ADDR+24)))
#define CPU_INFO_ICACHE_CONF (*((volatile _IODEV int *)(CPUINFO_BASE_ADDR+28)))
#define CPU_INFO_DCACHE_SIZE (*((volatile _IODEV int *)(CPUINFO_BASE_ADDR+32)))
#define CPU_INFO_DCACHE_CONF (*((volatile _IODEV int *)(CPUINFO_BASE_ADDR+36)))
#define CPU_INFO_SCACHE_SIZE (*((volatile _IODEV int *)(CPUINFO_BASE_ADDR+40)))
#define CPU_INFO_SCACHE_CONF (*((volatile _IODEV int *)(CPUINFO_BASE_ADDR+44)))
#define CPU_INFO_ISPM_SIZE (*((volatile _IODEV int *)(CPUINFO_BASE_ADDR+48)))
#define CPU_INFO_DSPM_SIZE (*((volatile _IODEV int *)(CPUINFO_BASE_ADDR+52)))

#define CPU_INFO_BOOT_BASE_ADDR 0xF0018000

#define CPU_INFO_BOOT_SRC_START (*((volatile _IODEV int *)CPU_INFO_BOOT_BASE_ADDR))
#define CPU_INFO_BOOT_SRC_SIZE (*((volatile _IODEV int *)(CPU_INFO_BOOT_BASE_ADDR+4)))
#define CPU_INFO_BOOT_DST_START (*((volatile _IODEV int *)(CPU_INFO_BOOT_BASE_ADDR+8)))
#define CPU_INFO_BOOT_DST_SIZE (*((volatile _IODEV int *)(CPU_INFO_BOOT_BASE_ADDR+12)))

//----------------------MMU--------------------------------------------------------------
typedef struct
{
	bool readable; //31
	bool writeable; //30
	bool executable; //29
	uint32_t offset;
} mmuConf_t;

#define MMU_NUM_SEGMENTS 8

#define MMU_READ_MASK 	0x80000000
#define MMU_WRITE_MASK 	0x40000000
#define MMU_EXEC_MASK 	0x20000000
#define MMU_OFFSET_MASK 0x1FFFFFFF

#define MMU_READ_BIT 31
#define MMU_WRITE_BIT 30
#define MMU_EXEC_BIT 29

#define MMU_SEGMENT_READABLE MMU_READ_MASK
#define MMU_SEGMENT_WRITEABLE MMU_WRITE_MASK
#define MMU_SEGMENT_EXECUTABLE MMU_EXEC_MASK

#define IO_MMU_BASE_ADDRESS ((volatile _IODEV int *)0xF0070000)

#define IO_MMU_SEGMENT_BASE(I) ((volatile _IODEV int * const)&(((volatile _IODEV int * const)(IO_MMU_BASE_ADDRESS))[2*I]))
#define IO_MMU_SEGMENT_CONF(I) (((volatile _IODEV int *)(IO_MMU_BASE_ADDRESS))[2*I+4])

//----------------------EXCEPTIONS_INTERRUPTS------------------------------------------
#define MAX_EXCEPTION 31

#define EXCUNIT_BASE_ADDR 0xF0010000

#define EXCEPTION_STATUS (*((volatile _IODEV int *)EXCUNIT_BASE_ADDR))
#define EXCEPTION_MASK (*((volatile _IODEV int *)(EXCUNIT_BASE_ADDR+4)))
#define EXCEPTION_PEND (*((volatile _IODEV int *)(EXCUNIT_BASE_ADDR+8)))
#define EXCEPTION_SOURCE (*((volatile _IODEV int *)(EXCUNIT_BASE_ADDR+12)))
#define EXCEPTION_SLEEP (*((volatile _IODEV int *)(EXCUNIT_BASE_ADDR+16)))
#define EXCEPTION_CACHECTRL (*((volatile _IODEV int *)(EXCUNIT_BASE_ADDR+20)))

// 'I' stands for the number of the desired Exception
typedef void (*exc_handler_t)(void);
#define EXCEPTION_HANDLER_VECTOR(I) (((_IODEV exc_handler_t volatile * const)(EXCUNIT_BASE_ADDR+0x80))[I])

#define EXCEPTION_ILLEGAL_OPERATION 0
#define EXCEPTION_ILLEGAL_ADDRESS 1
#define EXCEPTION_INTERRUPT_CLOCK 16
#define EXCEPTION_INTERRUPT_USEC 17

//----------------------TIMER------------------------------------------------------
#define TIMER_BASE_ADDR 0xf0020000

#define TIMER0_CYCLES_HIGH_WORD (*((volatile _IODEV int *)TIMER_BASE_ADDR))
#define TIMER0_CYCLES_LOW_WORD (*((volatile _IODEV int *)(TIMER_BASE_ADDR+4)))
#define TIMER1_MICROS_HIGH_WORD (*((volatile _IODEV int *)(TIMER_BASE_ADDR+8)))
#define TIMER1_MICROS_LOW_WORD (*((volatile _IODEV int *)(TIMER_BASE_ADDR+12)))

//----------------------ETHERNET--------------------------------------------------------
#define ETHERNET_BASE_ADDR 0xf00d0000

#define ETHERNET (*((volatile _IODEV int *)ETHERNET_BASE_ADDR))

#define ETHERNET_MAX_BUFFER_OFFSET 0xEFFF

#define ETHERNET_MAX_BUFFER_INDEX (ETHERNET_MAX_BUFFER_OFFSET+1)/4 // 15360 Indexes
#define ETHERNET_RX_TX_BUFFER(I) (((_IODEV (void*) volatile * const)(ETHERNET_BASE_ADDR))[I])


#define ETHERNET_CONTROLLER_REGISTERS_MIN_OFFSET 0xF000
#define ETHERNET_CONTROLLER_REGISTERS_MAX_OFFSET 0xFFFF

#define ETHERNET_CONTROLLER_REGISTERS_MAX_INDEX (ETHERNET_CONTROLLER_REGISTERS_MAX_OFFSET-ETHERNET_CONTROLLER_REGISTERS_MIN_OFFSET+1)/4 // 400 Indexes
#define ETHERNET_CONTROLLER_REGISTERS(I) (((_IODEV (void*) volatile * const)(ETHERNET_BASE_ADDR+ETHERNET_CONTROLLER_REGISTERS_MIN_OFFSET))[I])

//---------------------------------------------------------------------------------------

#endif