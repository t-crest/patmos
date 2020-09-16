#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/spm.h>
#include <machine/rtc.h>
#include "threads.h"

/* The only purpose of this file is to do 
 * some configuration if required before
 * calling the actual rosace main function
 */

int main(){
  LED = 0x100;
  uint64_t tsimu=300*200;
  return run_rosace(tsimu);
}

