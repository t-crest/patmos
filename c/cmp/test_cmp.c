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
#include <machine/rtc.h>
#include "libnoc/noc.h"
#include "bootloader/cmpboot.h"
#include "libmp/mp.h"
#include "libcorethread/corethread.h"

#define MINADDR (512*1024)
#define TEST_START ((volatile _UNCACHED unsigned int *) MINADDR + 0)
#define TEST_START_CACHED ((unsigned int *) MINADDR + 0)


#define ABORT_IF_FAIL(X,Y) if (X){puts(Y); abort();}

int main_mem_size = 0;
int core_count = 0;
//int core_com[64];

void prefix(int size, char* buf)  __attribute__((section(".text.spm"))){
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

#define MEM_AREA_TEST(addr,size)\
  int i,tmp; \
  for(i = 0; i < size; i++){ \
    tmp = *(addr+i); \
    *(addr+i) = 0; \
    if(*(addr+i) != 0){ \
      *(addr+i) = tmp; \
      return -1; \
    } \
    *(addr+i) = i; \
    if(*(addr+i) != i){ \
      *(addr+i) = tmp; \
      return -1; \
    } \
    *(addr+i) = tmp; \
  } \
  return 0;

int mem_area_test_uncached(volatile _UNCACHED unsigned int * mem_addr,int mem_size) {
  MEM_AREA_TEST(mem_addr,mem_size);
}

int mem_area_test_spm(volatile int _SPM * mem_addr,int mem_size) {
  MEM_AREA_TEST(mem_addr,mem_size);
}

#define TEST_MEM_SIZE(addr)\
  int init = *(addr); \
  int tmp; \
  *(addr) = 0xFFEEDDCC; \
  int i = 2; \
  int j = 0; \
  for(j = 0; j < 28; j++) { \
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
  *(addr) = init; \
  return -1;
  

int test_mem_size_cached(unsigned int * mem_addr){
  TEST_MEM_SIZE(mem_addr);
}

int test_mem_size_uncached(volatile _UNCACHED unsigned int * mem_addr){
  TEST_MEM_SIZE(mem_addr);
}

int test_mem_size_spm(volatile int _SPM * mem_addr){
  TEST_MEM_SIZE(mem_addr);
}

int print_noc_info(){
  printf("NoC schedule generated for %d cores\n",NOC_CORES);
  printf("NoC schedule contains %d timeslots\n",NOC_TIMESLOTS);
  return NOC_CORES;
}

int print_processor_info() {
  //puts("CPU info:");
  printf("CPU ID: %d\n",get_cpuid());
  int platform_cores = get_cpucnt();
  printf("Number of cores: %d\n",platform_cores);
  //printf("Operating frequency: %d MHz\n",(get_cpu_freq()) >> 20);
  printf("Operating frequency: %d MHz\n",(get_cpu_freq())/1000000);
  int i = 0;
  int cores = 1;
  for(i = 1; i < get_cpucnt(); i++){
    if(boot_info->slave[i].status != STATUS_NULL){
      cores++;
    }
  }
  printf("Number of cores booted: %d\n",cores);
  int noc_cores = print_noc_info();
  ABORT_IF_FAIL(platform_cores!=noc_cores,"An incorrect noc schedule is used");
  ABORT_IF_FAIL(platform_cores!=cores,"Not all cores booted");
  return cores;
}

int main_mem_test() {
  printf("Testing MAINMEM...");
  ABORT_IF_FAIL(mem_area_test_uncached(TEST_START,0x1000)<0,"FAIL");
  printf("OK\n");
  printf("Testing MAINMEM size: ");
  int size = 0;
  int cached_size = 0;
  size = test_mem_size_uncached(TEST_START);
  ABORT_IF_FAIL(size<0,"Size could not be retrieved");
  char buf[11];
  prefix(size,buf);
  puts(buf);
  return size;
}

void com_spm_test() {
  printf("Testing COM SPM...");
  ABORT_IF_FAIL(mem_area_test_spm(NOC_SPM_BASE,0x8000)<0,"FAIL 0x8000");
  ABORT_IF_FAIL(mem_area_test_spm(NOC_SPM_BASE,0x8001)<0,"FAIL 0x8001");
  ABORT_IF_FAIL(mem_area_test_spm(NOC_SPM_BASE,0x80001)<0,"FAIL 0x80001");
  printf("OK\n");
  printf("Testing COM SPM size: "); 
  fflush(stdout);
  int size = 0;
  size = test_mem_size_spm(NOC_SPM_BASE);
  ABORT_IF_FAIL(size<0,"Size could not be retrieved");
  char buf[11];
  prefix(size,buf);
  puts(buf);
  return;
}

void noc_test_master() {

}

void noc_test_slave() {

}

void mem_load_test() {
  int size = (main_mem_size-MINADDR)/get_cpucnt(); 
  volatile _UNCACHED unsigned int *addr = TEST_START + get_cpuid()*size;
  for(unsigned int start_time = get_cpu_usecs(); get_cpu_usecs() - start_time < 2000 ;) {
    ABORT_IF_FAIL(mem_area_test_uncached(addr,size)<0,"FAIL");
  }
}

void slave_tester (void* arg) {
  noc_test_slave();
    //mem_load_test();
  int ret = 0;
  corethread_exit(&ret);
  return;
}

void print_cpuinfo() {
  printf("get_cpufeat(): %08x\n", get_cpufeat());
  char buf[12];
  int size;
  size = get_extmem_size();
  prefix(size,buf);
  printf("get_extmem_size(): %s\n", buf);
  printf("get_extmem_conf(): %08x\n", get_extmem_conf());
  size = get_icache_size();
  prefix(size,buf);
  printf("get_icache_size(): %s\n", buf);
  printf("get_icache_conf(): %08x\n", get_icache_conf());
  size = get_dcache_size();
  prefix(size,buf);
  printf("get_dcache_size(): %s\n", buf);
  printf("get_dcache_conf(): %08x\n", get_dcache_conf());
  size = get_scache_size();
  prefix(size,buf);
  printf("get_scache_size(): %s\n", buf);
  printf("get_scache_conf(): %08x\n", get_scache_conf());
  size = get_ispm_size();
  prefix(size,buf);
  printf("get_ispm_size(): %s\n", buf);
  size = get_dspm_size();
  prefix(size,buf);
  printf("get_dspm_size(): %s\n", buf);
  size = get_bootspm_size();
  prefix(size,buf);
  printf("get_bootspm_size(): %s\n", buf);

}

int main() {
  print_cpuinfo();
  inval_dcache();
  core_count = print_processor_info();
  com_spm_test();
  main_mem_size = main_mem_test();
  noc_test_master();
  int param = 0;
  for(int i = 0; i < get_cpucnt(); i++) {
    if (i != NOC_MASTER) {
      corethread_t ct = (corethread_t) i;
      if(corethread_create(&ct,&slave_tester,(void*)param) != 0){
        printf("Corethread %d not created\n",i);
      }
    }
  }
  printf("Performing main mem load test...");
  fflush(stdout);
  mem_load_test();
  printf("OK\n");
  int* ret;
  for (int i = 0; i < get_cpucnt(); ++i) {
    if (i != NOC_MASTER) {
      corethread_join((corethread_t)i,(void**)&ret);
    }
  }
  printf("Joined with other cores\n");
  return 0;
}

