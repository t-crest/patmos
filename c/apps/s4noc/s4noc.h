/*
  Include file for the S4NOC evaluation.
*/

#ifndef LEN
#define LEN  4096 // in words
#endif

#ifndef BUF_LEN
#define BUF_LEN 16 // in words
#endif

#ifndef HANDSHAKE
#define HANDSHAKE 4
#endif

#ifndef DELAY
#define DELAY 10
#endif

#define S4NOC_ADDRESS 0xE8000000

#define IN_DATA 0
#define IN_SLOT 1
#define TX_FREE 2
#define RX_READY 3

// small NoC is 2x2, the other version is 3x3
#define SMALLNOC 1

#ifdef SMALLNOC
#define RCV 3
#define SEND_SLOT 0
#define CREDIT_SLOT 0
#else
//
#endif

volatile _IODEV int *timer_ptr = (volatile _IODEV int *) (PATMOS_IO_TIMER+4);
volatile _IODEV int *dead_ptr = (volatile _IODEV int *) PATMOS_IO_DEADLINE;

#define D_SPM_BASE 0x00000000
