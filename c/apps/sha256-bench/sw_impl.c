#include "benchmark.h"

#define LROT(op, r)     (((op) << (r)) | ((op) >> (32 - (r))))
#define RROT(op, r)     (((op) >> (r)) | ((op) << (32 - (r))))
#define CH(a,b,c)       (((a) & (b)) ^ (~(a) & (c)))
#define MA(a,b,c)       (((a) & (b)) ^ ((a) & (c)) ^ ((b) & (c)))
#define SIGMA_0(a)      (RROT((a), 2) ^ RROT((a), 13) ^ (RROT((a), 22)))
#define SIGMA_1(a)      (RROT((a), 6) ^ RROT((a), 11) ^ (RROT((a), 25)))
#define SSIGMA_0(a)     (RROT((a), 7) ^ RROT((a), 18) ^ ((a) >> 3))
#define SSIGMA_1(a)     (RROT((a), 17) ^ RROT((a), 19) ^ ((a) >> 10))

// reset hash state
void sw_reset(void)
{
  hash_buf_word[0] = 0x6a09e667;
  hash_buf_word[1] = 0xbb67ae85;
  hash_buf_word[2] = 0x3c6ef372;
  hash_buf_word[3] = 0xa54ff53a;
  hash_buf_word[4] = 0x510e527f;
  hash_buf_word[5] = 0x9b05688c;
  hash_buf_word[6] = 0x1f83d9ab;
  hash_buf_word[7] = 0x5be0cd19;
}

// sha-256 constants
static const uint32_t k[] = {
  0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
  0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
  0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
  0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
  0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
  0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
  0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
  0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

// compute hash for a single block
INLINE_PREFIX void sw_transform(uint32_t block)
{
  uint32_t w[64];
  uint32_t h[8];
  
  memcpy(w, msg_buf_word + (block * BLOCK_WORDS), sizeof(uint32_t) * 16);

  for(uint32_t i = 16; i < 64; ++i)
  {
    w[i] = SSIGMA_1(w[i - 2]) + w[i - 7] + SSIGMA_0(w[i - 15]) + w[i - 16];
  }
  
  memcpy(h, hash_buf_word, sizeof(uint32_t) * 8);

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
    hash_buf_word[i] += h[i];
  }
}

// compute hash for a all blocks
INLINE_PREFIX void sw_compute_hash()
{
  for(uint32_t i = 0; i < msg_blocks; ++i)
  {
    sw_transform(i);
  }
}

void benchmark(uint32_t *busy_time)
{
  sw_reset();
  asm volatile ("" ::: "memory");
  
  uint32_t busy_acc = 0;
  uint32_t busy_start;
  uint32_t idle_start;
  
  asm volatile ("" ::: "memory");
  busy_start = get_time32();
  asm volatile ("" ::: "memory");
  sw_compute_hash();  
  asm volatile ("" ::: "memory");
    
  idle_start = get_time32();
  busy_acc += (idle_start - busy_start);
  
  asm volatile ("" ::: "memory");
  *busy_time = busy_acc;
}
