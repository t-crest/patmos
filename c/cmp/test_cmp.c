/*
	Author: Rasmus Bo Soerensen (rasmus@rbscloud.dk)
	Copyright: DTU, BSD License
*/
const int NOC_MASTER = 0;
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <machine/patmos.h>
#include <machine/exceptions.h>
#include "libnoc/noc.h"
#include "bootloader/cmpboot.h"

#define MINADDR (512*1024)
#define TEST_START ((volatile _UNCACHED unsigned int *) MINADDR + 0)


#define ABORT_IF_FAIL(X,Y) if (X<0){puts(Y); abort();}

//int core_com[64];

void prefix(int size, char* buf){
	int pref = 0;
	while (size > 1024){
		size = size >> 10;
		pref++;
	}
	char* format;
	switch(pref){
		case 0:
			format = "%d B";
			break;
		case 1:
			format = "%d KB";
			break;
		case 2:
			format = "%d MB";
			break;
		case 3:
			format = "%d GB";
			break;
	}
	snprintf(buf,11,format,size);
	return;
}

int mem_area_test_uncached(volatile _UNCACHED unsigned int * mem_addr,int mem_size) {
	int i,tmp;
	for(i = 0; i < mem_size; i++){
    tmp = *(mem_addr+i); // Store the temporary data
		*(mem_addr+i) = 0;
		if(*(mem_addr+i) != 0){
      *(mem_addr+i) = tmp; // Restore the temporary data
      return -1;
    }
		*(mem_addr+i) = i;
		if(*(mem_addr+i) != i){
      *(mem_addr+i) = tmp; // Restore the temporary data
      return -1;
    }
		*(mem_addr+i) = tmp; // Restore the temporary data
	}
	return 0;
}

int mem_area_test_spm(volatile int _SPM * mem_addr,int mem_size) {
	int i,tmp;
	for(i = 0; i < mem_size; i++){
    tmp = *(mem_addr+i); // Store the temporary data
		*(mem_addr+i) = 0;
		if(*(mem_addr+i) != 0){
      *(mem_addr+i) = tmp; // Restore the temporary data
      return -1;
    }
		*(mem_addr+i) = i;
		if(*(mem_addr+i) != i){
      *(mem_addr+i) = tmp; // Restore the temporary data
      return -1;
    }
    *(mem_addr+i) = tmp; // Restore the temporary data
	}
	return 0;
}

#define MEM_TEST(addr)\
  int init = *(addr); \
  int tmp; \
  *(addr) = 0xFFEEDDCC; \
  int i = 2; \
  int j = 0; \
  for(j = 0; j < 32; j++) { \
    tmp = *(addr+i); \
    *(addr+i) = 0; \
    if (*(addr) == 0) { \
      *(addr+i) = tmp; \
      *(addr) = init; \
      return i*4; \
    } \
    i = i << 1; \
    if (*(addr) != 0xFFEEDDCC){ \
      *(addr+i) = tmp; \
      *(addr) = init; \
      return -1; \
    } \
    *(addr+i) = tmp; \
  } \
  *(addr) = init;
  

int test_mem_size_uncached(volatile _UNCACHED unsigned int * mem_addr){
  MEM_TEST(mem_addr);
  return -1;
}

int test_mem_size_spm(volatile int _SPM * mem_addr){
  MEM_TEST(mem_addr);
	return -1;
}

void print_processor_info() {
  //puts("CPU info:");
  printf("CPU ID: %d\n",get_cpuid());
  //printf("Operating frequency: %d MHz\n",(get_cpu_freq()) >> 20);
  printf("Operating frequency: %d MHz\n",(get_cpu_freq())/1000000);
  int i = 0;
  int cores = 1;
  for(i = 1; i < MAX_CORES; i++){
    if(boot_info->slave[i].status != STATUS_NULL){
      cores++;
    }
  }
  printf("Number of cores booted: %d\n",cores);
  return;
}

void print_noc_info(){
  printf("NoC scheduler generated for %d cores\n",NOC_CORES);
  printf("NoC scheduler contains %d timeslots\n",NOC_TIMESLOTS);
}

void main_mem_test() {
	fputs("Testing MAINMEM...",stdout);
	ABORT_IF_FAIL(mem_area_test_uncached(TEST_START,0x1000),"FAIL");
	puts("OK");
	fputs("Testing MAINMEM size: ",stdout);
	int size = 0;
	size = test_mem_size_uncached(TEST_START);
	ABORT_IF_FAIL(size,"Size could not be retrieved");
	char buf[11];
	prefix(size,buf);
	puts(buf);
	return;
}

void com_spm_test() {
	fputs("Testing COM SPM...",stdout);
	ABORT_IF_FAIL(mem_area_test_spm(NOC_SPM_BASE,0x8000),"FAIL 0x8000");
	ABORT_IF_FAIL(mem_area_test_spm(NOC_SPM_BASE,0x8001),"FAIL 0x8001");
	ABORT_IF_FAIL(mem_area_test_spm(NOC_SPM_BASE,0x80001),"FAIL 0x80001");
	puts("OK");
	fputs("Testing COM SPM size: ",stdout);	
	fflush(stdout);
	int size = 0;
	size = test_mem_size_spm(NOC_SPM_BASE);
	ABORT_IF_FAIL(size,"Size could not be retrieved");
	char buf[11];
	prefix(size,buf);
	puts(buf);
	return;
}

int main() {
  if (get_cpuid() == 0) {
    print_processor_info();
    print_noc_info();
	com_spm_test();
	main_mem_test();
    return 0;

  } else {
    // other cores do idle loop
    //for (;;) { }
    return 0;
  }
  return -1;
}

