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
#define HANDSHAKE 2
#endif

#define S4NOC_ADDRESS 0xE8000000

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
