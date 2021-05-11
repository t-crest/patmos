#include "benchmark.h"

// io-device pointers
static _iodev_ptr_t const sha_ptr = (_iodev_ptr_t)0xf00b0000;
static _iodev_ptr_t const sha_ptr_write = (_iodev_ptr_t)0xf00b0080;
static _iodev_ptr_t const sha_ptr_read = (_iodev_ptr_t)0xf00b0040;

static uint32_t msg_block_cursor = 0;

// reset io-device seed and transmission state
void io_reset(void)
{
  *sha_ptr = 0;
  msg_block_cursor = 0;
}

// send next block and start computation
// returns true when last block has been sent
// must only be called when prior computation is completed
INLINE_PREFIX bool io_send_next_block(void)
{
  uint32_t *buf = msg_buf_word + (msg_block_cursor * BLOCK_WORDS);
  for(int32_t i = 0; i < BLOCK_WORDS; ++i)
  {
    *(sha_ptr_write + i) = buf[i];
  }
  *sha_ptr = 1;
  return ++msg_block_cursor >= msg_blocks;
}

// retrieves the hash
// must only be called when prior computation is completed
INLINE_PREFIX void io_retrieve_hash(void)
{
  for(int32_t i = 0; i < HASH_WORDS; ++i)
  {
    hash_buf_word[i] = *(sha_ptr_read + i);
  }
}

// attempts to retrieve hash
// returns true if successful
INLINE_PREFIX bool io_try_retrieve_hash(void)
{
  if(*sha_ptr == 0)
  {
    io_retrieve_hash();
    return true;
  }
  return false;
}

// waits until the current computation is completed
INLINE_PREFIX void io_busy_wait(void)
{
  while(*sha_ptr);
}

void benchmark(uint32_t *busy_time_s, uint32_t *busy_time_r, uint32_t *idle_time)
{
  io_reset();
  asm volatile ("" ::: "memory");
  
  uint32_t busy_acc_s = 0;
  uint32_t busy_acc_r = 0;
  uint32_t idle_acc = 0;
  uint32_t busy_start;
  uint32_t idle_start;
  
  bool done = false;
  
  while(!done)
  {
    busy_start = get_time32();
    
    asm volatile ("" ::: "memory");
    done = io_send_next_block();
    asm volatile ("" ::: "memory");
    
    idle_start = get_time32();
    asm volatile ("" ::: "memory");
    busy_acc_s += (idle_start - busy_start);
    
    asm volatile ("" ::: "memory");
    io_busy_wait();
    asm volatile ("" ::: "memory");
    
    #ifdef TIME_IDLE
      busy_start = get_time32();
      idle_acc += (busy_start - idle_start);
    #endif
  }
  
  #ifndef TIME_IDLE
    busy_start = get_time32();
  #endif
  
  asm volatile ("" ::: "memory");
  io_retrieve_hash();
  asm volatile ("" ::: "memory");
  
  idle_start = get_time32();
  busy_acc_r += (idle_start - busy_start);
  
  asm volatile ("" ::: "memory");
  *busy_time_s = busy_acc_s;
  *busy_time_r = busy_acc_r;
  *idle_time = idle_acc;
}
