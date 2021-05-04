#include <machine/patmos.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#define INLINE_PREFIX static inline                   // for inlining of benchmark-related leaf-functions
#define DATA_ALIGNMENT __attribute__((aligned(16)))   // for burst-length alignment of msg- and hash-buffers
#define TIME_IDLE                                     // enable timing of idle-periods (i.e. busy-waiting)

/*----------------------------------------DEFINITIONS FOR SHA-256 IO-DEVICE------------------------------------------*/
#define HASH_WORDS (8)                                        // 8 words per hash
#define BLOCK_WORDS (16)                                      // 16 words per data block
#define TERMINATION_WORDS (2)                                 // 2 words required for length termination

#define HASH_BYTES (HASH_WORDS * sizeof(uint32_t))
#define BLOCK_BYTES (BLOCK_WORDS * sizeof(uint32_t))
#define PADDING_BYTES (1)                                     // at least 1 byte required for padding (0x80)
#define TERMINATION_BYTES (TERMINATION_WORDS * sizeof(uint32_t))

#define MAX_MSG_WORDS (8 * BLOCK_WORDS)
#define MAX_MSG_BYTES (MAX_MSG_WORDS * sizeof(uint32_t))

// define SHA-256 IO-device pointer
_iodev_ptr_t sha_ptr = (_iodev_ptr_t)0xf00b0000;
_iodev_ptr_t sha_ptr_write = (_iodev_ptr_t)0xf00b0080;
_iodev_ptr_t sha_ptr_read = (_iodev_ptr_t)0xf00b0040;

// SHA-256 message state
char msg_buf[MAX_MSG_BYTES] DATA_ALIGNMENT = { 0x00 };
uint32_t *msg_buf_word = (uint32_t *)msg_buf;
uint32_t msg_blocks = 0;
uint32_t msg_block_cursor = 0;

// SHA-256 hash
uint32_t hash[HASH_WORDS] DATA_ALIGNMENT;

// Copy message into message state and pad
// Returns number of blocks on success and 0 on failure
uint32_t pad_message(const char *str, int32_t len)
{
  if (len < 0)
    len = strlen(str);
    
  if (len > MAX_MSG_BYTES - PADDING_BYTES - TERMINATION_BYTES)
    return 0;

  // copy the message and append padding byte
  memcpy(msg_buf, str, len);
  msg_buf[len] = 0x80;

  // zero-pad and append message length
  int32_t last_block_padding = BLOCK_BYTES - (len % BLOCK_BYTES);
  int32_t newlen = len + last_block_padding;
  if (last_block_padding < PADDING_BYTES + TERMINATION_BYTES)
  {
    // we must append another block
    memset(msg_buf + len + 1, 0x00, last_block_padding - PADDING_BYTES + BLOCK_BYTES - TERMINATION_BYTES);
    newlen += BLOCK_BYTES;
  }
  else
  {
    // we can simply terminate the last block
    memset(msg_buf + len + 1, 0x00, last_block_padding - PADDING_BYTES - TERMINATION_BYTES);
  }
  memset(msg_buf + newlen - TERMINATION_BYTES, 0x00, TERMINATION_BYTES - sizeof(int64_t));
  int64_t bit_len = ((int64_t)len) << 3; // multiply by 8 because we usually state the length in bits 
  memcpy(msg_buf + newlen - sizeof(int64_t), &bit_len, sizeof(int64_t));
  msg_blocks = newlen / BLOCK_BYTES;
  
  // new state is now stored in msg_buf and msg_blocks
  return msg_blocks;
}

// reset SHA-256 seed and transmission state
void reset()
{
  *sha_ptr = 0;           // reset seed
  msg_block_cursor = 0;   // reset block cursor
}

// send next block and start computation
// returns true when last block has been sent
// must only be called when prior computation is completed
INLINE_PREFIX bool send_next_block()
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
// must only be called when the computation is completed
INLINE_PREFIX void retrieve_hash(uint32_t *hash)
{
  for(int32_t i = 0; i < HASH_WORDS; ++i)
  {
    hash[i] = *(sha_ptr_read + i);
  }
}

// attempts to retrieve hash
// returns true if successful
INLINE_PREFIX bool try_retrieve_hash(uint32_t *hash)
{
  if(*sha_ptr == 0)
  {
    retrieve_hash(hash);
    return true;
  }
  return false;
}

// waits until the computation is completed
INLINE_PREFIX void busy_wait()
{
  while(*sha_ptr);
}

/*-----------------------------------------DEFINITIONS FOR TIMER IO-DEVICE-------------------------------------------*/
// define timer IO-device pointer
_iodev_ptr_t timer_ptr_high = (_iodev_ptr_t)(PATMOS_IO_TIMER);
_iodev_ptr_t timer_ptr_low = (_iodev_ptr_t)(PATMOS_IO_TIMER + 0x04);

INLINE_PREFIX uint32_t get_time32()
{
  return *timer_ptr_low;
}

// TODO: maybe replace this with some inline asm code (-O2 seems to generate correctly optimized code though)
INLINE_PREFIX uint64_t get_time64()
{
  uint64_t ret = *timer_ptr_low;
  ret |= ((uint64_t)*timer_ptr_high) << 32;
  return ret;
}

/*-------------------------------------------DEFINITIONS FOR BENCHMARKING--------------------------------------------*/
void benchmark_hash(uint32_t *hash, uint32_t *busy_time_s, uint32_t *busy_time_r, uint32_t *idle_time)
{  
  // perform SHA-256 device reset
  reset();
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
    done = send_next_block();
    asm volatile ("" ::: "memory");
    
    idle_start = get_time32();
    asm volatile ("" ::: "memory");
    busy_acc_s += (idle_start - busy_start);
    
    asm volatile ("" ::: "memory");
    busy_wait();
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
  retrieve_hash(hash);
  asm volatile ("" ::: "memory");
  
  idle_start = get_time32();
  busy_acc_r += (idle_start - busy_start);
  
  asm volatile ("" ::: "memory");
  *busy_time_s = busy_acc_s;
  *busy_time_r = busy_acc_r;
  *idle_time = idle_acc;
}

void print_hash(uint32_t *hash)
{
  char *hash_bytes = (char *)hash;
  for(int32_t i = 0; i < HASH_BYTES; ++i)
  {
    printf("%.2hhx ", hash_bytes[i]);
  }
  fflush(stdout);
}

/*---------------------------------------------------MAIN PROGRAM----------------------------------------------------*/
#define REPETITIONS (3)

const char *benchmark_strings[] = { "",
                                    "abc",
                                    "This is a message which barely fits into one data block",
                                    "This is a message which already requires two data blocks",
                                    "We need three data blocks as soon as the message is at least 120 characters long"
                                    " but also not longer than 183 characters"};

uint32_t benchmark_string_count = sizeof(benchmark_strings) / sizeof(benchmark_strings[0]);

int main(int argc, char **argv) {
  uint32_t busy_time_s;
  uint32_t busy_time_r;
  uint32_t idle_time;
  bool success;
  
  for(uint32_t i = 0; i < benchmark_string_count; ++i)
  {
    const char *str = benchmark_strings[i];
    uint32_t block_count = pad_message(str, -1);
    if(block_count == 0)
    {
      printf("could not process string \"%s\"\n\n", str);
      continue;
    }
    
    printf("benchmarking %ld blocks for string \"%s\"\n", block_count, str);
    for(uint32_t j = 0; j < REPETITIONS; ++j)
    {
      printf("  repetition %ld\n", j);
      benchmark_hash(hash, &busy_time_s, &busy_time_r, &idle_time);
      printf("    busy_send_time: %lu\n"
             "    busy_recv_time: %lu\n"
             "    idle_time:      %lu\n", busy_time_s, busy_time_r, idle_time);
    }
    fputs("\thash: ", stdout);
    print_hash(hash);
    puts("\n");
  }
  
  return 0;
}
