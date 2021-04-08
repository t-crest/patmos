#include <machine/patmos.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#define HASH_WORDS (8)                                        // 8 words per hash
#define BLOCK_WORDS (16)                                      // 16 words per data block
#define TERMINATION_WORDS (2)                                 // 2 words required for length termination

#define HASH_BYTES (HASH_WORDS * sizeof(int32_t))
#define BLOCK_BYTES (BLOCK_WORDS * sizeof(int32_t))
#define PADDING_BYTES (1)                                     // at least 1 byte required for padding (0x80)
#define TERMINATION_BYTES (TERMINATION_WORDS * sizeof(int32_t))

#define MAX_MSG_WORDS (8 * BLOCK_WORDS)
#define MAX_MSG_BYTES (MAX_MSG_WORDS * sizeof(int32_t))

// define SHA-256 IO-device pointer
_iodev_ptr_t sha_ptr = (_iodev_ptr_t)0xf00b0000;
_iodev_ptr_t sha_ptr_write = (_iodev_ptr_t)0xf00b0080;
_iodev_ptr_t sha_ptr_read = (_iodev_ptr_t)0xf00b0040;

// SHA-256 message state
char msg_buf[MAX_MSG_BYTES] = { 0x00 };
uint32_t *msg_buf_word = (uint32_t *)msg_buf;
uint32_t msg_blocks = 0;
uint32_t msg_block_cursor = 0;

// Copy message into message state and pad
// Returns true on success
bool pad_message(char *str, int32_t len)
{
  if (len < 0)
    len = strlen(str);
    
  if (len > MAX_MSG_BYTES - PADDING_BYTES - TERMINATION_BYTES)
    return false;

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
  // all length-termination words except for the last one are 0 because we only allow int for the length
  memset(msg_buf + newlen - TERMINATION_BYTES, 0x00, TERMINATION_BYTES - sizeof(int64_t));
  int64_t bit_len = len << 3;
  memcpy(msg_buf + newlen - sizeof(int64_t), &bit_len, sizeof(int64_t));
  msg_blocks = newlen / BLOCK_BYTES;
  
  // new state is now stored in msg_buf and msg_blocks
  return true;
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
bool send_next_block()
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
void retrieve_hash(uint32_t *hash)
{
  for(int32_t i = 0; i < HASH_WORDS; ++i)
  {
    hash[i] = *(sha_ptr_read + i);
  }
}

// attempts to retrieve hash
// returns true if successful
bool try_retrieve_hash(uint32_t *hash)
{
  if(*sha_ptr == 0)
  {
    retrieve_hash(hash);
    return true;
  }
  return false;
}

// waits until the computation is completed
void busy_wait()
{
  while(*sha_ptr);
}

/*-------------------------------------------------------------------------------------------------------------------*/
int main(int argc, char **argv) {
  // set message
  pad_message("abc", -1);
  
  // perform SHA-256 device reset
  reset();
  
  // send blocks
  while(!send_next_block())
  {
    busy_wait();
  }
  
  // hash array
  uint32_t hash[HASH_WORDS];
  busy_wait();
  retrieve_hash(hash);
  
  // print the hash
  char *hash_bytes = (char *)hash;
  for(int32_t i = 0; i < HASH_BYTES; ++i)
  {
    printf("%.2hhx ", hash_bytes[i]);
  }
  fflush(stdout);
  
  return 0;
}
