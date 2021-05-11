#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <machine/patmos.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

/*------------------------------------------SWITCHES FOR BENCHMARK SETTINGS-------------------------------------------*/
#ifndef REPETITIONS
  #define REPETITIONS       (5)                           // number of repetitions per benchmark run
#endif

// #define BENCHMARK_SW                                   // benchmark software implementation
// #define BENCHMARK_IO                                   // benchmark io-device implementation
// #define BENCHMARK_COP                                  // benchmark coprocessor implementation

#ifdef BENCHMARK_SW
  #define BENCHMARK_NAME    "software"
#endif
#ifdef BENCHMARK_IO
  #define BENCHMARK_NAME    "io-device"
#endif
#ifdef BENCHMARK_COP
  #define BENCHMARK_NAME    "coprocessor"
#endif

#if !(defined BENCHMARK_SW || defined BENCHMARK_IO || defined BENCHMARK_COP)
  #error "Cannot compile benchmark without any implementation."
#endif
#if (defined BENCHMARK_SW && defined BENCHMARK_IO) ||   \
    (defined BENCHMARK_SW && defined BENCHMARK_COP) ||  \
    (defined BENCHMARK_IO && BENCHMARK_COP)
  #error "Cannot compile benchmark with more than one implementation."
#endif

/*-----------------------------------------GENERAL DEFINITIONS FOR BENCHMARK------------------------------------------*/
#define INLINE_PREFIX       static inline                 // for inlining of benchmark-related leaf-functions
#define DATA_ALIGNMENT      __attribute__((aligned(16)))  // for burst-length alignment of msg- and hash-buffers
#define TIME_IDLE                                         // enable timing of idle-periods (i.e. busy-waiting)

#ifdef BENCHMARK_COP
  #define HASH_MODIFIER     volatile _UNCACHED
#else
  #define HASH_MODIFIER
#endif

/*--------------------------------------------SHA-256 RELATED DEFINITIONS---------------------------------------------*/
#define HASH_WORDS          (8)                           // 8 words per hash
#define BLOCK_WORDS         (16)                          // 16 words per data block
#define TERMINATION_WORDS   (2)                           // 2 words required for length termination

#define PADDING_BYTES       (1)                           // at least 1 byte required for padding (0x80)

#define HASH_BYTES          (HASH_WORDS * sizeof(uint32_t))
#define BLOCK_BYTES         (BLOCK_WORDS * sizeof(uint32_t))
#define TERMINATION_BYTES   (TERMINATION_WORDS * sizeof(uint32_t))

#define MAX_MSG_WORDS       (8 * BLOCK_WORDS)
#define MAX_MSG_BYTES       (MAX_MSG_WORDS * sizeof(uint32_t))

/*---------------------------------------------SHA-256 RELATED VARIABLES----------------------------------------------*/
extern char msg_buf[MAX_MSG_BYTES] DATA_ALIGNMENT;
extern uint32_t *msg_buf_word;
extern uint32_t msg_blocks;

extern HASH_MODIFIER char hash_buf[HASH_BYTES] DATA_ALIGNMENT;
extern HASH_MODIFIER uint32_t *hash_buf_word;

/*------------------------------------------DEFINITIONS FOR TIMER IO-DEVICE-------------------------------------------*/
// define timer io-device pointer
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

/*---------------------------------------------DECLARATION FOR BENCHMARK----------------------------------------------*/
// every implementation has to provide a benchmark-function
// the parameters are not fixed here in order to allow variations for different implementations
void benchmark();

#endif
