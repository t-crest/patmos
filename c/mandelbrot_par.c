/*
  A fixed-point parallel mandelbrot generator.

  Outputs a script for ed that results in an xpm image that is
  continuously updated during computations.

  Usage: mandelbrot_par | ed foo.xpm  

  TODO: This is a mock-up that uses Unix shared memory for
  communication between N processes.
  Current usage: for i in `seq 0 3`; do ./mandelbrot_par $i & done | ed foo.xpm

  Author: Wolfgang Puffitsch
  Copyright: DTU, BSD License
*/

#define CORES 4
#define SLAVES (CORES-1)

#define DELAY 0

int core_id;

#define ROWS  480
#define COLS  720

#define FRAC_BITS 16
#define FRAC_ONE  (1 << FRAC_BITS)

#define XSTART     (2*-FRAC_ONE)
#define XEND       (FRAC_ONE)
#define YSTART     (-FRAC_ONE)
#define YEND       (FRAC_ONE)
#define XSTEP_SIZE ((XEND-XSTART)/COLS)
#define YSTEP_SIZE ((YEND-YSTART)/ROWS)

#define MAX_SQUARE (16*FRAC_ONE)
#define MAX_ITER   64

#include <unistd.h>
#define WRITE(data,len) write(STDOUT_FILENO,data,len)

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

int main(int argc, char **argv);

static void master(void);
static void write_xpm_header(void);
static void itoa(char *buf, unsigned v);
static char to_color(int v) __attribute__((noinline));

static void slave(void);
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

/*//////////////////////////////////////////////////////////////////////////
// SHM handling
//////////////////////////////////////////////////////////////////////////*/

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/unistd.h>

#define SHM_MPB_KEY 0x4A730500

static void *shm_alloc(int key, int size, int perm) {

  int shm_id = shmget(key, size, IPC_CREAT | perm);
  /* get shared memory area */
  if (shm_id == -1) {
    fprintf(stderr, "error: cannot get shared memory: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  void *ptr = shmat(shm_id, NULL, 0);
  if (ptr == (void *)-1) {
    fprintf(stderr, "error: cannot attach shared memory: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  return ptr;
}

static void shm_free(int key, volatile void *ptr, int perm) {
  /* do not care about errors here */
  shmdt((void *)ptr);

  int shm_id = shmget(key, 0, perm);
  if (shm_id != -1) {
    shmctl(shm_id, IPC_RMID, NULL);
  }
}

/*/////////////////////////////////////////////////////////////////////////
// Definitions for communication structures
/////////////////////////////////////////////////////////////////////////*/

#define CMD_START (-3)
#define CMD_STOP  (-2)
#define CMD_NULL  (-1)

struct packet_t {
  volatile int yval;
  volatile int xstart;
  volatile int xend;
  volatile int xstep;
  volatile int cmd;
};

struct rowbuf_t {
  volatile char data[COLS];
  volatile int cmd;
};

struct mpb_t {
  struct packet_t packet;
  struct rowbuf_t row;
};

struct mpb_master_t {
  struct mpb_t slave[SLAVES];
};

struct mpb_master_t *shm_master_mpb;
struct mpb_t *shm_slave_mpb[SLAVES];

static void init_mpb() {
  unsigned i;
  shm_master_mpb = shm_alloc(SHM_MPB_KEY, sizeof(struct mpb_master_t), 0666);
  for (i = 0; i < SLAVES; i++) {
    shm_slave_mpb[i] = shm_alloc(SHM_MPB_KEY+1+i, sizeof(struct mpb_t), 0666);
  }
}

static void clean_mpb() {
  unsigned i;
  shm_free(SHM_MPB_KEY, shm_master_mpb, 0666);
  for (i = 0; i < SLAVES; i++) {
    shm_free(SHM_MPB_KEY+1+i, shm_slave_mpb[i], 0666);
  }
}

static void dma(volatile void *dst, volatile void *src, size_t len) {
  size_t i;
  volatile int *d = (volatile int *)dst;
  volatile int *s = (volatile int *)src;
  for (i = 0; i < (len+sizeof(int)-1)/sizeof(int); i++) {
    d[i] = s[i];
  }
}

/*/////////////////////////////////////////////////////////////////////////
// Main application
/////////////////////////////////////////////////////////////////////////*/

int main(int argc, char **argv) {

  core_id = strtol(argv[1], NULL, 0);

  init_mpb();

  if (core_id == 0) {
    master();
  } else {
    slave();
  }

  clean_mpb();

  return 0;
}

static void master(void) {
  unsigned i;

  write_xpm_header();

  /* clear response fields */
  for (i = 0; i < SLAVES; i++) {
    shm_master_mpb->slave[i].row.cmd = CMD_STOP;
  }
  /* send start packets */
  for (i = 0; i < SLAVES; i++) {
    shm_master_mpb->slave[i].packet.cmd = CMD_START;
    dma(&(shm_slave_mpb[i]->packet),
        &(shm_master_mpb->slave[i].packet),
        sizeof(struct packet_t));
  }
  /* wait for acknowledgement */
  for (i = 0; i < SLAVES; i++) {
    while (shm_master_mpb->slave[i].row.cmd != CMD_START) {
      /* spin until start reponse */
    }
  }
  /* clear response fields */
  for (i = 0; i < SLAVES; i++) {
    shm_master_mpb->slave[i].row.cmd = CMD_NULL;
  }

  int row[SLAVES];
  int received_row;
    
  /* send first packets to slaves */
  for (i = 0; i < SLAVES && i < ROWS; i++) {
    row[i] = i*(ROWS/SLAVES);   
    shm_master_mpb->slave[i].packet.cmd    = row[i];
    shm_master_mpb->slave[i].packet.yval   = YSTART + row[i]*YSTEP_SIZE;
    shm_master_mpb->slave[i].packet.xstart = XSTART;
    shm_master_mpb->slave[i].packet.xend   = XEND;
    shm_master_mpb->slave[i].packet.xstep  = XSTEP_SIZE;      
    dma(&(shm_slave_mpb[i]->packet),
        &(shm_master_mpb->slave[i].packet),
        sizeof(struct packet_t));

    row[i]++;
  }
  /* receive rows and send next packets */
  for (received_row = 0; received_row < ROWS; ) {
    for (i = 0; i < SLAVES; i++) {
      if (shm_master_mpb->slave[i].row.cmd != CMD_NULL) {
        received_row++;

        static char buf[10];
        itoa(buf, shm_master_mpb->slave[i].row.cmd);
        WRITE(buf, 10);

        /* write out data */
        WRITE(" 68c\n\"", 6);
        WRITE((char *)shm_master_mpb->slave[i].row.data, COLS);
        WRITE("\",\n.\nw\n", 7);

        /* clear response field */
        shm_master_mpb->slave[i].row.cmd = CMD_NULL;

        /* send next packet */
        if (row[i] < (i+1)*(ROWS/SLAVES)) {
          shm_master_mpb->slave[i].packet.cmd    = row[i];
          shm_master_mpb->slave[i].packet.yval   = YSTART + row[i]*YSTEP_SIZE;
          shm_master_mpb->slave[i].packet.xstart = XSTART;
          shm_master_mpb->slave[i].packet.xend   = XEND;
          shm_master_mpb->slave[i].packet.xstep  = XSTEP_SIZE;    
          dma(&(shm_slave_mpb[i]->packet),
              &(shm_master_mpb->slave[i].packet),
              sizeof(struct packet_t));

          row[i]++;
        }
      }
    }
  }

  /* send stop packets */
  for (i = 0; i < SLAVES; i++) {
    shm_master_mpb->slave[i].packet.cmd = CMD_STOP;
    dma(&(shm_slave_mpb[i]->packet),
        &(shm_master_mpb->slave[i].packet),
        sizeof(struct packet_t));
  }
}

static void write_xpm_header(void) {
  int i;
  WRITE("1,$d\n0a\n", 8);
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
  for (i = 0; i < ROWS; i++) {
    WRITE("\"", 1);
    unsigned k;
    for (k = 0; k < COLS; k++) {
      WRITE(" ", 1);      
    }
    WRITE("\",\n", 3);
  }
  WRITE("};\n.\nw\n", 7);
}

static void itoa(char *buf, unsigned v) {
  int i;
  for (i = 9; i >= 0; --i) {
    buf[i] = v == 0 && i != 9 ? ' ' : '0' + v % 10;
    v /= 10;
  }
}

static char to_color(int v) {
  return v & 2 ? (v & 1 ? 'F' : 'A') : (v & 1 ? '5' : '0');
}

static void slave(void) {
  int slave_id = core_id-1;

  while (shm_slave_mpb[slave_id]->packet.cmd != CMD_START) {
    /* spin until start packet arrives */
  }
  /* respond to start packet */
  shm_slave_mpb[slave_id]->row.cmd = CMD_START;
  dma(&(shm_master_mpb->slave[slave_id].row),
      &(shm_slave_mpb[slave_id]->row),
      sizeof(struct rowbuf_t)); 

  /* loop while new packets arrive */
  for (;;) {
    while (shm_slave_mpb[slave_id]->packet.cmd <= CMD_NULL
           && shm_slave_mpb[slave_id]->packet.cmd != CMD_STOP) {
      /* spin until packet arrives */
    }
    /* terminate if stop packet arrives */
    if (shm_slave_mpb[slave_id]->packet.cmd == CMD_STOP) {
      break;
    }

    /* copy packet to local memory */
    struct packet_t p = shm_slave_mpb[slave_id]->packet;
    /* clear command */
    shm_slave_mpb[slave_id]->packet.cmd = CMD_NULL;
    /* compute pixels for one row */
    int x, idx;
    for (idx = 0, x = p.xstart; x < p.xend; idx++, x += p.xstep) {
      int val = do_iter(x, p.yval, MAX_SQUARE, MAX_ITER);
      shm_slave_mpb[slave_id]->row.data[idx] = map[val & 0x3f];
    }
    /* send row */
    shm_slave_mpb[slave_id]->row.cmd = p.cmd;
    dma(&(shm_master_mpb->slave[slave_id].row),
        &(shm_slave_mpb[slave_id]->row),
        sizeof(struct rowbuf_t));
  }
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
