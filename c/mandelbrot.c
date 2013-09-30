/*
    A fixed-point mandelbrot generator that can be compiled into the
    boot ROM.

    Author: Wolfgang Puffitsch
    Copyright: DTU, BSD License
*/

#include <unistd.h>

#ifdef __patmos__
#include <machine/patmos.h>
#include <machine/spm.h>
#endif

int main() __attribute__((naked,used));
int fracmul(int x, int y); // __attribute__((noinline));
int do_iter(int cx, int cy, int max_square, int max_iter);

#define FRAC_BITS 16
#define FRAC_ONE  (1 << FRAC_BITS)
#define STEP_SIZE (FRAC_ONE/16)

#define MAX_SQUARE ((1 << 10)*FRAC_ONE)
#define MAX_ITER   (1 << 10)

#ifdef __patmos__
#define UART_STATUS *((volatile _SPM int *) 0xF0000800)
#define UART_DATA   *((volatile _SPM int *) 0xF0000804)

#define WRITE(data,len) do { \
  unsigned i; \
  for (i = 0; i < (len); i++) { \
    while ((UART_STATUS & 0x01) == 0); \
    UART_DATA = (data)[i]; \
  } \
} while(0)

#else /* __patmos__ */
#define WRITE(data,len) write(STDOUT_FILENO,data,len)
#endif /* __patmos__ */

const char *map = " .,:;/!lILFPERWM";

int main() {
  int x, y;
  for (y = -FRAC_ONE+STEP_SIZE; y < FRAC_ONE; y += STEP_SIZE) {
    for (x = -3*FRAC_ONE+STEP_SIZE; x < 2*FRAC_ONE; x += STEP_SIZE) {
      int val = do_iter(x, y, MAX_SQUARE, MAX_ITER);
      WRITE(&(map[val & 0xf]), 1);
    }
    WRITE("\n", 1);
  }
  return 0;
}

int fracmul(int x, int y) {
  return (long long)x*y >> FRAC_BITS;
}

int do_iter(int cx, int cy,
            int max_square, int max_iter) {
  int square = 0;
  int iter = 0;
  int x = 0;
  int y = 0;
  while (square <= max_square && iter < max_iter) {
    int xt = fracmul(x, x) - fracmul(y, y) + cx;
    int yt = 2*fracmul(x, y) + cy;
    x = xt;
    y = yt;
    iter++;
    square = fracmul(x, x) + fracmul(y, y);
  }

  return iter;
}
