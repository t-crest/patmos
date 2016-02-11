/*
   Copyright 2016 Technical University of Denmark, DTU Compute. 
   All rights reserved.
   
   This file is part of the time-predictable VLIW processor Patmos.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

      1. Redistributions of source code must retain the above copyright notice,
         this list of conditions and the following disclaimer.

      2. Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
   NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   The views and conclusions contained in the software and documentation are
   those of the authors and should not be interpreted as representing official
   policies, either expressed or implied, of the copyright holder.
 */

/*
 * Patong, a single-player clone of Pong for Patmos.
 * Showcases interrupt handling and the use of terminal escape codes (for xterm).
 * Works best when echo is turned off.
 * 
 * Authors: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 */

#include <stdlib.h>
#include <stdio.h>

#include <machine/exceptions.h>
#include <machine/rtc.h>

// definitions for terminal escape codes
#define ESC       "\x1b"
#define CSI       ESC"["

#define GOTO(X,Y) CSI #X ";" #Y "H"

#define CLEARALL  CSI"2J"

#define NORMAL    CSI"0m"
#define BOLD      CSI"1m"
#define INV       CSI"7m"
#define INVIS     CSI"8m"
#define HIDDEN    CSI"0;8m"

// the assumed size of the screen
#define XDIM 80
#define YDIM 24

// works for the simulator with -G0
/* #define PERIOD 4000 */
/* #define SPEEDUP 6 */

// works for the hardware
#define PERIOD 20000 // 50 FPS
#define SPEEDUP 1

// variables to keep track of the state of the game
int xdir = 48*SPEEDUP;
int ydir = 24*SPEEDUP;

int xpos = (XDIM/2) << 8;
int ypos = (YDIM/2) << 8;

int lpos = YDIM/2;

int score = 0;

int pause = 0;
int over = 0;

// switch to alternate screen buffer
void init_screen(void) {
  // use alternate screen
  fputs(CSI"?1049h", stdout);
  // hide cursor
  fputs(CSI"?25l", stdout);
  fflush(stdout);
}

// print a welcome screen
void intro_screen(void) {
  fputs(CLEARALL GOTO(1,1), stdout);

  fputs("Welcome to Patong!\n\n", stdout);
  fputs("Controls:\n", stdout);
  fputs("UP:    `e', <KEY1>\n", stdout);
  fputs("DOWN:  `d', <KEY0>\n", stdout);
  fputs("PAUSE: `p'\n", stdout);
  fputs("QUIT:  `q'\n", stdout);
  fputs("\nPress any key to continue...", stdout);
  fflush(stdout);

  int c = fgetc(stdin);
}

// switch back to normal screen buffer
void cleanup_screen(void) {
  // reset graphic attributes
  fputs(NORMAL, stdout);
  // use normal screen again
  fputs(CSI"?1049l", stdout);
  // show cursor again
  fputs(CSI"?25h", stdout);
  fflush(stdout);
}

// prepare screen for actual game play
void start_screen(void) {
  // try to hide echoed input
  fputs(HIDDEN, stdout);
  fputs(CLEARALL, stdout);
  fflush(stdout);
}

// update the state of the game and draw screen
void update_screen(void) {
  // variables to remember state and only send a diff
  static int old_xpos = 0;
  static int old_ypos = 0;
  static int old_lpos = YDIM/2 - 5;
  static int old_score = -1;

  // nothing to do when being paused
  if (pause) {
    return;
  }

  // ball went out of bounds
  if ((xpos >> 8) <= 1) {
    if (!over) {
      fprintf(stdout, NORMAL BOLD GOTO(%d, %d) "GAME OVER", YDIM/2, XDIM/2-3);
      fprintf(stdout, NORMAL GOTO(%d, %d) "press `q' to quit", YDIM/2+2, XDIM/2-7);
      fputs(HIDDEN, stdout);
      fflush(stdout);
      over = 1;
      return;
    }
  }

  // update position of ball
  xpos += xdir;
  ypos += ydir;

  // integer positions
  int xpos_int = xpos >> 8;
  int ypos_int = ypos >> 8;

  // reflect ball
  if (((xpos + 0x80) >> 8) <= 2) {
    int diff = ypos_int - lpos;
    // did we hit the paddle?
    if (diff >= -2 && diff <= 2) {
      xdir = -xdir + 8*SPEEDUP;
      ydir += diff * 8*SPEEDUP;
      score++;
      // make sure ball is in front of paddle
      if (xpos_int < 2) {
        xpos = 2 << 8;
        xpos_int = 2;
      }
    } else {
      // avoid underflows
      if (xpos_int < 0) {
        xpos_int = 0;
      }
    }
  } else if (((xpos + 0x80) >> 8) > XDIM) {
    xdir = -xdir;
    // add a little disturbance to the Y-direction
    int r = rand() % 8*SPEEDUP;
    if (r > 4*SPEEDUP) { r -= 8*SPEEDUP; }
    ydir += r;
    // avoid overflows
    if (xpos_int > XDIM) {
      xpos_int = XDIM;
    }
  }
  // slightly softer bouncing in Y-direction
  if (((ypos + 0x40) >> 8) <= 1 || ((ypos + 0xc0) >> 8) > YDIM) {
    ydir = -ydir;
  }

  // sometimes things go wrong when echo is turned on, so we should
  // draw from scratch from time to time
  static int refresh_count = 0;
  refresh_count++;
  if (refresh_count > 500/SPEEDUP) {
    fputs(CLEARALL, stdout);
    old_lpos = lpos < YDIM/2 ? lpos + 5 : lpos - 5;
    old_score = -1;
    refresh_count = 0;
  }

  // print ball
  if (old_ypos == ypos_int && old_xpos == xpos_int) {
    // same as old
  } else {
    fprintf(stdout, NORMAL GOTO(%d,%d) " ", old_ypos, old_xpos);
    fprintf(stdout, GOTO(%d,%d) "o", ypos_int, xpos_int);
  }

  // print paddle
  int ldiff = old_lpos - lpos;
  if (ldiff < 0) {
    fputs(NORMAL, stdout);
    for (int i = old_lpos-2; i < old_lpos-2-ldiff; i++) {
      fprintf(stdout, GOTO(%d,1) " ", i);
    }
    fputs(INV, stdout);
    for (int i = lpos+2+ldiff+1; i <= lpos+2; i++) {
      fprintf(stdout, GOTO(%d,1) " ", i);
    } 
  } else if (ldiff > 0) {
    fputs(NORMAL, stdout);
    for (int i = old_lpos+2-ldiff+1; i <= old_lpos+2; i++) {
      fprintf(stdout, GOTO(%d,1) " ", i);
    }
    fputs(INV, stdout);
    for (int i = lpos-2; i < lpos-2+ldiff; i++) {
      fprintf(stdout, GOTO(%d,1) " ", i);
    }
  }

  // print score
  if (old_score != score || old_ypos <= 1 || ypos_int <= 1) {
    fprintf(stdout, NORMAL INV GOTO(1,%d) " %3d ", XDIM/2-1, score);    
  }

  // remember old state
  old_xpos = xpos_int;
  old_ypos = ypos_int;
  old_lpos = lpos;
  old_score = score;

  // try to hide echoed input
  fputs(HIDDEN, stdout);
  fflush(stdout);
}

unsigned long long next_time;

// update positions and draw screen periodically
void timer_intr(void) __attribute__((naked));
void timer_intr(void) {
  exc_prologue();

  update_screen();
  if (!over) {
    next_time += PERIOD;
    arm_usec_timer(next_time);
  }

  exc_epilogue();
}

// move paddle down
void down(void) {
  if (!pause && lpos < 22) {
    lpos++;
  }
}

// interrupt to move paddle down, triggered by a key
void down_intr(void) __attribute__((naked));
void down_intr(void) {
  exc_prologue();
  down();
  exc_epilogue();
}

// move paddle up
void up(void) {
  if (!pause && lpos > 3) {
    lpos--;
  }
}

// interrupt to move paddle up, triggered by a key
void up_intr(void) __attribute__((naked));
void up_intr(void) {
  exc_prologue();
  up();
  exc_epilogue();
}

int main(void) {
  setvbuf(stdin,NULL,_IONBF,0);

  init_screen();
  intro_screen();

  // register interrupt handlers
  exc_register(17, &timer_intr);
  exc_register(18, &down_intr);
  exc_register(19, &up_intr);

  // unmask interrupts
  intr_unmask_all();
  // clear pending flags
  intr_clear_all_pending();
  // enable interrupts
  intr_enable();

  // arm timer
  next_time = get_cpu_usecs() + PERIOD;
  arm_usec_timer(next_time);

  start_screen();

  // handle inputs
  for (;;) {    
    int c = fgetc(stdin);
    switch (c) {
    case 'e': up(); break;
    case 'd': down(); break;
    case 'p': pause = !pause; break;
    case 'q': cleanup_screen(); exit(0); break;
    }
  }

  return 0;
}
