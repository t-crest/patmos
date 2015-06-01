/*
	A small example that demonstrates how to initialize a data segment
	from the boot ROM.

	Author: Wolfgang Puffitsch
	Copyright: DTU, BSD License
*/

#include <string.h>
#include <machine/spm.h>

struct ldinfo_t {
  volatile char *src_start;
  unsigned src_size;
  volatile char *dst_start;
  unsigned dst_size;
} __attribute__((packed));

#define ldinfo ((struct ldinfo_t *)(0x80000000))

static void load_data(void) {
  unsigned i;

  volatile char *src = ldinfo->src_start;
  unsigned src_size = ldinfo->src_size;
  volatile char *dst = ldinfo->dst_start;
  unsigned dst_size = ldinfo->dst_size;

  for (i = 0; i < src_size; i++) {
	*dst++ = *src++;
  }
  for ( ; i < dst_size; i++) {
	*dst++ = 0;
  }
}

int main() {

  load_data();

  const char *msg = "Hello, World!\n";
  unsigned i, k;

  volatile _SPM int *led_ptr = (volatile _SPM int *) 0xF0090000;
  volatile _SPM int *uart_ptr = (volatile _SPM int *) 0xF0080004;

  for (i = 0; i < strlen(msg); i++) {
	*uart_ptr = msg[i];
	*led_ptr = msg[i];
	for (k = 0; k < 1000000; k++) {
	  asm volatile("");
	}
  }
}
