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

// SHA-256 hash
uint32_t hash[HASH_WORDS] DATA_ALIGNMENT;

// SHA-256 constants
const uint32_t k[] = {
  0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
  0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
  0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
  0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
  0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
  0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
  0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
  0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

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

#define LROT(op, r)     (((op) << (r)) | ((op) >> (32 - (r))))
#define RROT(op, r)     (((op) >> (r)) | ((op) << (32 - (r))))
#define CH(a,b,c)       (((a) & (b)) ^ (~(a) & (c)))
#define MA(a,b,c)       (((a) & (b)) ^ ((a) & (c)) ^ ((b) & (c)))
#define SIGMA_0(a)      (RROT((a), 2) ^ RROT((a), 13) ^ (RROT((a), 22)))
#define SIGMA_1(a)      (RROT((a), 6) ^ RROT((a), 11) ^ (RROT((a), 25)))
#define SSIGMA_0(a)     (RROT((a), 7) ^ RROT((a), 18) ^ ((a) >> 3))
#define SSIGMA_1(a)     (RROT((a), 17) ^ RROT((a), 19) ^ ((a) >> 10))

// reset SHA-256 seed
void reset()
{
  hash[0] = 0x6a09e667;
  hash[1] = 0xbb67ae85;
  hash[2] = 0x3c6ef372;
  hash[3] = 0xa54ff53a;
  hash[4] = 0x510e527f;
  hash[5] = 0x9b05688c;
  hash[6] = 0x1f83d9ab;
  hash[7] = 0x5be0cd19;
}

INLINE_PREFIX void transform(uint32_t block)
{
  uint32_t w[64];
  uint32_t h[8];
  
  memcpy(w, msg_buf_word + (block * BLOCK_WORDS), sizeof(uint32_t) * 16);

  for(uint32_t i = 16; i < 64; ++i)
  {
    w[i] = SSIGMA_1(w[i - 2]) + w[i - 7] + SSIGMA_0(w[i - 15]) + w[i - 16];
  }
  
  memcpy(h, hash, sizeof(uint32_t) * 8);

  for(uint32_t i = 0; i < 64; ++i)
  {
		uint32_t t1 = h[7] + SIGMA_1(h[4]) + CH(h[4],h[5],h[6]) + k[i] + w[i];
		uint32_t t2 = SIGMA_0(h[0]) + MA(h[0],h[1],h[2]);
		h[7] = h[6];
		h[6] = h[5];
		h[5] = h[4];
		h[4] = h[3] + t1;
		h[3] = h[2];
		h[2] = h[1];
		h[1] = h[0];
		h[0] = t1 + t2;
  }
  
  for(uint32_t i = 0; i < 8; ++i)
  {
    hash[i] += h[i];
  }
}

INLINE_PREFIX void compute_hash()
{
  for(uint32_t i = 0; i < msg_blocks; ++i)
  {
    transform(i);
  }
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
void benchmark_hash(uint32_t *busy_time)
{  
  // perform SHA-256 device reset
  reset();
  asm volatile ("" ::: "memory");
  
  uint32_t busy_acc = 0;
  uint32_t busy_start;
  uint32_t idle_start;
  
  asm volatile ("" ::: "memory");
  busy_start = get_time32();
  asm volatile ("" ::: "memory");
  compute_hash();  
  asm volatile ("" ::: "memory");
    
  idle_start = get_time32();
  busy_acc += (idle_start - busy_start);
  
  asm volatile ("" ::: "memory");
  *busy_time = busy_acc;
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
#define REPETITIONS (5)

const char *benchmark_strings[] = { "",
                                    "abc",
                                    "This is a message which barely fits into one data block",
                                    "This is a message which already requires two data blocks",
                                    "We need three data blocks as soon as the message is at least 120 characters long"
                                    " but also not longer than 183 characters"};

uint32_t benchmark_string_count = sizeof(benchmark_strings) / sizeof(benchmark_strings[0]);

int main(int argc, char **argv) {
  uint32_t busy_time;
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
      benchmark_hash(&busy_time);
      printf("    busy_time: %lu\n", busy_time);
    }
    fputs("  hash: ", stdout);
    print_hash(hash);
    puts("\n");
  }
  
  return 0;
}
