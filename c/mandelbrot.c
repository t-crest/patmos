/*
    A fixed-point mandelbrot generator that can be compiled into the
    boot ROM.

    Author: Wolfgang Puffitsch
    Copyright: DTU, BSD License
*/

#define DELAY 50

#define ROWS  240
#define COLS  320

#define FRAC_BITS 16
#define FRAC_ONE  (1 << FRAC_BITS)

#define XSTART     (2*-FRAC_ONE)
#define XEND       (FRAC_ONE)
#define YSTART     (-FRAC_ONE)
#define YEND       (FRAC_ONE)
#define XSTEP_SIZE ((XEND-XSTART+COLS-1)/COLS)
#define YSTEP_SIZE ((YEND-YSTART+ROWS-1)/ROWS)

#define MAX_SQUARE (16*FRAC_ONE)
#define MAX_ITER   64

#ifdef __patmos__

#include <machine/patmos.h>
#include <machine/spm.h>

#define UART_STATUS *((volatile _SPM int *) 0xF0000800)
#define UART_DATA   *((volatile _SPM int *) 0xF0000804)
static void write(const char *msg, int len) __attribute__((noinline));
#define WRITE(data,len) write(data,len)

#else /* __patmos__ */

#include <unistd.h>
#define WRITE(data,len) write(STDOUT_FILENO,data,len)

#endif /* __patmos__ */

#include <string.h>

#ifdef BOOTROM
extern int _stack_cache_base, _shadow_stack_base;
int main(int argc, char **argv);
void _start(void) __attribute__((naked,used));

void _start(void) {
  // setup stack frame and stack cache.
  asm volatile ("mov $r29 = %0;" // initialize shadow stack pointer"
				"mts $ss  = %1;" // initialize the stack cache's spill pointer"
				"mts $st  = %1;" // initialize the stack cache's top pointer"
				"li $r30 = %2;" // initialize return base"
				: : "r" (&_shadow_stack_base),
				    "r" (&_stack_cache_base),
				    "i" (&_start));
  // call main()
  main(0, NULL);
  // freeze
  for(;;);
}
#endif

static void write_xpm_header(void);
static char to_color(int v) __attribute__((noinline));
static int do_iter(int cx, int cy, unsigned int max_square, int max_iter);
static int fracmul(int x, int y) __attribute__((noinline));

#define STRFY(X) STR(X)
#define STR(X) #X

static const char *map = 
  " .',:;-~+=<>/\\!(){}[]it1lfIJ?L7VThYZCdbUOQGD2S5XFHKP9NAE38&RBWM#";
static const char *hdr = 
  "/* XPM */\n"
  "static char *mandel[] = {\n"
  "\"" STRFY(COLS) " " STRFY(ROWS) " 64 1\",\n";

int main(int argc, char **argv) {
  write_xpm_header();

  int x, y;
  for (y = YSTART; y < YEND; y += YSTEP_SIZE) {
    WRITE("\"", 1);
    for (x = XSTART; x < XEND; x += XSTEP_SIZE) {
      int val = do_iter(x, y, MAX_SQUARE, MAX_ITER);
      WRITE(&(map[val & 0x3f]), 1);
    }
    WRITE("\",\n", 3);
  }
  WRITE("};\n", 3);

  return 0;
}

static void write_xpm_header(void) {
  int i;
  WRITE(hdr, strlen(hdr));
  for (i = 0; i < 64; i++) {
    static char buf [15];
    buf[0]  = '"';
    buf[1]  = map[i];
    buf[2]  = ' ';
    buf[3]  = 'c';
    buf[4]  = ' ';
    buf[5]  = '#';
    char r = to_color((i >> 4) & 3);
    buf[6]  = r;
    buf[7]  = r;
    char g = to_color((i >> 0) & 3);
    buf[8]  = g;
    buf[9]  = g;
    char b = to_color((i >> 2) & 3);
    buf[10] = b;
    buf[11] = b;
    buf[12] = '"';
    buf[13] = ',';
    buf[14] = '\n';
    WRITE(buf, 15);
  }
}

static char to_color(int v) {
  return v & 2 ? (v & 1 ? 'F' : 'A') : (v & 1 ? '5' : '0');
}

static int do_iter(int cx, int cy,
                   unsigned int max_square, int max_iter) {
  unsigned int square = 0;
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

static int fracmul(int x, int y) {
  int i;
  for (i = 0; i < DELAY; i++) {
    asm volatile ("");
  }
  return (long long)x*y >> FRAC_BITS;
}

#ifdef __patmos__
static void write(const char* msg, int len) {
  unsigned i;
  for (i = 0; i < len; i++) {
    while ((UART_STATUS & 0x01) == 0);
    UART_DATA = msg[i];
  }
}
#endif
