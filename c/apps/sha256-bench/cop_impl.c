#include "benchmark.h"

// reset coprocessor seed
void cop_reset()
{
  asm (".word 0x3400001");    // unpredicated COP_WRITE to COP0 with FUNC = 00000, RA = 00000, RB = 00000
}

// sends the message blocks
// must only be called when prior computation is completed
INLINE_PREFIX void cop_send_blocks()
{
  register uint32_t *msg_buf_word_reg __asm__ ("19") = msg_buf_word;
  register uint32_t msg_blocks_reg __asm__ ("20") = msg_blocks;
  asm (".word 0x34B3A01"      // unpredicated COP_WRITE to COP0 with FUNC = 00101, RA = 10011, RB = 10100
        :
        : "r"(msg_buf_word_reg), "r"(msg_blocks_reg));
}

// retrieves the hash
// must only be called when prior computation is completed
INLINE_PREFIX void cop_retrieve_hash()
{
  register volatile _UNCACHED uint32_t *hash_buf_word_reg __asm__ ("17") = hash_buf_word;
  asm (".word 0x3471001"      // unpredicated COP_WRITE to COP0 with FUNC = 00010, RA = 00011, RB = 00000
        :
        : "r"(hash_buf_word_reg));
}

// waits until the current computation is completed
INLINE_PREFIX void cop_busy_wait()
{
  register uint32_t flag __asm__ ("18") = 1;
  while(flag)
  {
    //printf("flag: %lu\n", flag);
	  asm (	".word 0x3640083"   // unpredicated COP_READ to COP0 with FUNC = 00001, RA = 00000, RD = 10010
		      : "=r"(flag)
		      :
		      : "18");
	}

}

void benchmark(uint32_t *busy_time_s, uint32_t *busy_time_r, uint32_t *idle_time)
{
  cop_reset();
  asm volatile ("" ::: "memory");
  
  uint32_t busy_acc_s = 0;
  uint32_t busy_acc_r = 0;
  uint32_t idle_acc = 0;
  uint32_t busy_start;
  uint32_t idle_start;
  
  busy_start = get_time32();
  
  asm volatile ("" ::: "memory");
  cop_send_blocks();
  asm volatile ("" ::: "memory");
  
  idle_start = get_time32();
  busy_acc_s += (idle_start - busy_start);
  
  asm volatile ("" ::: "memory");  
  cop_busy_wait();
  asm volatile ("" ::: "memory");
  
  busy_start = get_time32();
  #ifdef TIME_IDLE
    idle_acc += (busy_start - idle_start);
  #endif
  
  asm volatile ("" ::: "memory");
  cop_retrieve_hash();
  asm volatile ("" ::: "memory");
  
  idle_start = get_time32();
  busy_acc_r += (idle_start - busy_start);
  
  asm volatile ("" ::: "memory");
  cop_busy_wait();
  asm volatile ("" ::: "memory");
  
  #ifdef TIME_IDLE
    busy_start = get_time32();
    idle_acc += (busy_start - idle_start);
  #endif
  
  asm volatile ("" ::: "memory");
  *busy_time_s = busy_acc_s;
  *busy_time_r = busy_acc_r;
  *idle_time = idle_acc;
}
