#include "benchmark.h"

/*----------------------------------------------SHA-256 INITIALIZATIONS-----------------------------------------------*/
char msg_buf[MAX_MSG_BYTES] DATA_ALIGNMENT = { 0x00 };
uint32_t *msg_buf_word = (uint32_t *)msg_buf;
uint32_t msg_blocks = 0;

HASH_MODIFIER char hash_buf[HASH_BYTES] DATA_ALIGNMENT = {};
HASH_MODIFIER uint32_t *hash_buf_word = (HASH_MODIFIER uint32_t *)hash_buf;

/*---------------------------------------------BENCHMARK HELPER FUNCTIONS---------------------------------------------*/
// copy message into message state and pad
// returns number of blocks on success and 0 on failure
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

// prints the current hash to stdout
void print_hash()
{
  for(int32_t i = 0; i < HASH_BYTES; ++i)
  {
    printf("%.2hhx ", hash_buf[i]);
  }
  fflush(stdout);
}

/*-----------------------------------------------BENCHMARK MAIN PROGRAM-----------------------------------------------*/
static const char * const benchmark_strings[] =
  { "",
    "abc",
    "This is a message which barely fits into one data block",
    "This is a message which already requires two data blocks",
    "We need three data blocks as soon as the message is at least 120 characters long but also not longer than 183"
    " characters"
  };

static const uint32_t benchmark_string_count = sizeof(benchmark_strings) / sizeof(benchmark_strings[0]);

int main(int argc, char **argv) {
  #ifdef BENCHMARK_SW
    uint32_t busy_time;
  #else
    uint32_t busy_time_s;
    uint32_t busy_time_r;
    uint32_t idle_time;
  #endif
  bool success;
  
  printf("starting benchmark for implementation: %s\n", BENCHMARK_NAME);
  for(uint32_t i = 0; i < benchmark_string_count; ++i)
  {
    const char * str = benchmark_strings[i];
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
      #ifdef BENCHMARK_SW
        benchmark(&busy_time);
        printf("    busy_time: %lu\n", busy_time);
      #else
        benchmark(&busy_time_s, &busy_time_r, &idle_time);
        printf("    busy_send_time: %lu\n"
               "    busy_recv_time: %lu\n"
               "    idle_time:      %lu\n", busy_time_s, busy_time_r, idle_time);
      #endif
    }
    fputs("  hash: ", stdout);
    print_hash();
    puts("\n");
  }
  
  return 0;
}
