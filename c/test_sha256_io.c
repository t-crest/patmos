#include <machine/patmos.h>
#include <stdio.h>

#define HASH_WORDS (8)
#define BLOCK_WORDS (16)
#define PADDING_WORDS (2)

#define HASH_BYTES (HASH_WORDS * sizeof(int))
#define BLOCK_BYTES (BLOCK_WORDS * sizeof(int))
#define PADDING_BYTES (PADDING_WORDS * sizeof(int))

int main(int argc, char **argv) {
  puts("Starting Test...");

  // define SHA-256 IO-device pointer
  _iodev_ptr_t sha_ptr = (_iodev_ptr_t)0xf00b0000;

  // define test string and pad
  char msg[BLOCK_BYTES] = { 0x00 };
  msg[0] = 0x80;

  // get word-based string  
  int *msg_words = (int *)msg;

  // reset SHA-256 seed
  *sha_ptr = 0;
 
  // transfer message block
  for(int i = 0; i < BLOCK_WORDS; ++i)
  {
    *(sha_ptr + 0x20 + i) = msg_words[i];
  }
  
  // start computation
  *sha_ptr = 1;
  
  // busy-wait for SHA-256 device to finish hashing
  while(*sha_ptr);
  
  // hash array
  int hash[HASH_WORDS];
  
  // read back the hash
  for(int i = 0; i < HASH_WORDS; ++i)
  {
    hash[i] = *(sha_ptr + 0x10 + i);
  }
  puts("Hash has been transferred to CPU...");
  
  // print the hash
  char *hash_bytes = (char *)hash;
  for(int i = 0; i < HASH_BYTES; ++i)
  {
    printf("%.2hhx ", hash_bytes[i]);
  }
  fflush(stdout);
  
  return 0;
}
