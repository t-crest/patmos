/* LZSS decoder, based on implementation by Haruhiko Okumura
 *
 * Modified to use a 1k ring buffer (instead of the original 4k), and
 * 6 bits to encode the length (instead of the original 4 bits) to
 * compensate for the fewer index bits. Also modified to return one
 * byte at a time instead of decoding in one pass.
 *
 * Adapted by Wolfgang Puffitsch 
 */

/***************************************************************
	4/6/1989 Haruhiko Okumura
	Use, distribute, and modify this program freely.
	Please send me your improved versions.
		PC-VAN		SCIENCE
		NIFTY-Serve	PAF01022
		CompuServe	74050,1022
**************************************************************/

#include "boot.h"

#ifdef COMPRESSION

#define INDEX_BITS 10                      /* bits to encode index */
#define LENGTH_BITS (16-INDEX_BITS)        /* bits to encode length */
#define THRESHOLD 2 /* encode string into position and length if
                       match_length is greater than this */

#define N (1 << INDEX_BITS)                /* size of ring buffer */
#define F ((1 << LENGTH_BITS) + THRESHOLD) /* lookahead buffer size */

static unsigned char text_buf[N];
static unsigned int write_pos, read_pos;
static unsigned int flags;

void decompress_init(void)
{
  for (write_pos = 0; write_pos < N - F; write_pos++) {
    text_buf[write_pos] = ' ';
    asm volatile(""); // prevent inferring memset
  }
  read_pos = write_pos;
  flags = 0;
}

int decompress_get_byte(void)
{
  int c;

  if (read_pos == write_pos) {
    flags >>= 1;
    if ((flags & 0x100) == 0) {
      c = uart_read();
      flags = c | 0xFF00;  // uses higher byte cleverly to count eight
    }   
    if (flags & 1) {
      c = uart_read();
      text_buf[write_pos++] = c;
      write_pos &= (N - 1);
    } else {
      int i = uart_read();
      int j = uart_read();
      i |= (j & (0xff << LENGTH_BITS) & 0xff) << (8 - LENGTH_BITS);
      j  = (j & ((1 << LENGTH_BITS) - 1)) + THRESHOLD;
      for (int k = i; k <= i + j; k++) {
        c = text_buf[k & (N - 1)];
        text_buf[write_pos++] = c;
        write_pos &= (N - 1);
      }
    }
  }

  c = text_buf[read_pos++];
  read_pos &= (N - 1);
  return c;
}

#endif /* COMPRESSION */
