#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <machine/patmos.h>
#include "libmp/mp.h"
#include "libcorethread/corethread.h"

/*************************/
/*  Timer utils          */
/*************************/

static unsigned long long hidden_time = 0;

//#define TIMER_START		{hidden_time = get_cpu_usecs();}
#define TIMER_START   {hidden_time = get_cpu_cycles();}

//#define TIMER_STOP		{hidden_time = get_cpu_usecs() - hidden_time;}
#define TIMER_STOP    {hidden_time = get_cpu_cycles() - hidden_time;}

#define TIMER_ELAPSED	hidden_time

#define my_two_printf(...) if (get_cpuid() == NOC_MASTER) { \
								printf(__VA_ARGS__); \
							}

/*************************/
/*  Cache flush policy   */
/*************************/
/* flush = 0 No cache flushing
 * flush = 1 between message sizes
 * flush = 2 between repeates
 * flush = 3 all the time except between iterations
 * flush = 4 between iterations
 * flush = 5 all the time except between repeates
 * flush = 6 all the time except between sizes
 * flush = 7 all the time
 */
int flush = 0;
#define FLUSH_BETWEEN_SIZES 1<<0
#define FLUSH_BETWEEN_REPEATS 1<<1
#define FLUSH_BETWEEN_ITERATIONS 1<<2

int iterations = 100;
//int iterations = 2;
//int sizes[15] = {4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536};
//int sizes[1] = {4};
//int num_sizes = sizeof(sizes)/sizeof(sizes[0]);
int repeat_count = 1;

qpd_t m2s;
qpd_t s2m;

communicator_t comm;
communicator_t comm_world;

#define BUFFER_SIZE 64
int sizes[1] = {BUFFER_SIZE};
int num_sizes = sizeof(sizes)/sizeof(sizes[0]);

#define SLAVE_CORE 4
int cores[] = {0,SLAVE_CORE};
int cores_world[] = {0,1,2,3,4,5,6,7,8};
//coreid_t cores_world[] = {0,1};

/*************************/
/*  Benchmarks           */
/*************************/

int calibrate_cache_flush(int cnt) {
  int tmp = 1;
  int i;
  TIMER_START;
  for (i=0;i<cnt;i++) {
    if (flush & FLUSH_BETWEEN_ITERATIONS){
    inval_dcache();
    inval_mcache();
    }
  }
  TIMER_STOP;
  tmp = TIMER_ELAPSED;
  return tmp;
}

void mp_send_size(qpd_t* qpd_ptr, int bytes) {
	int k = 0;
	while(k < bytes) {
      int chunk = 0;
      if ( bytes-k >= qpd_ptr->buf_size) {
          // If the remaining data is more than the size of the buffer
          chunk = qpd_ptr->buf_size;   
      } else {
          // The remaining data all fits in a buffer
          chunk = bytes-k;
      }
      // Copy the chunk of data to the write buffer
    //  for (int j = 0; j < chunk; ++j) {
    //      *((volatile char _SPM *)m2s.write_buf+j) = send_data[k+j];
    //  }
      // Send the chunk of data
      mp_send(qpd_ptr);
      DEBUGGER("Message sent\n");
      k += chunk;
  	}
}

void mp_recv_size(qpd_t* qpd_ptr, int bytes) {
	int k = 0;
    while(k < bytes) {
      mp_recv(qpd_ptr);
      
      int chunk = 0;
      if ( bytes-k >= qpd_ptr->buf_size) {
          // If the remaining data is more than the size of the buffer
          chunk = qpd_ptr->buf_size;   
      } else {
          // The remaining data all fits in a buffer
          chunk = bytes-k;
      }
      // Copy the chunk of data to the write buffer
    //  for (int j = 0; j < chunk; ++j) {
    //      *((volatile char _SPM *)qpd_ptr.write_buf+j) = send_data[k+j];
    //  }
      mp_ack(qpd_ptr);
      k += chunk;
  	}
}

/* ALso known as the gap time by Berkeley, 
   the time to launch a message in the network's buffers. */

int latency_master(int cnt, int bytes) {
  int i;
  unsigned long long total = 0;

  TIMER_START;
  for (i=0; i<cnt; i++) {
    if (flush & FLUSH_BETWEEN_ITERATIONS) {
      inval_dcache();
      inval_mcache();   
    }
    mp_send_size(&m2s,bytes);
  }
  TIMER_STOP;
  mp_recv(&s2m);
  mp_ack(&s2m);
  total = TIMER_ELAPSED;
  total -= calibrate_cache_flush(cnt);
  return(total/cnt);   
    
}

int latency_slave(int cnt, int bytes) {
  int i;

  for (i=0; i<cnt; i++) {
    if (flush & FLUSH_BETWEEN_ITERATIONS) {
      inval_dcache();
      inval_mcache();
    }
    mp_recv_size(&m2s,bytes);
  }
  mp_send(&s2m); 
  return 0;
}

/* This might be your more familiar definition of latency...*/

int roundtrip_master(int cnt, int bytes) {
  int i;
  unsigned long long total = 0;

  TIMER_START;
  for (i=0; i<cnt; i++) {
    if (flush & FLUSH_BETWEEN_ITERATIONS)
      {
      inval_dcache();
      inval_mcache();
    }
    mp_send_size(&m2s, bytes);
    mp_recv_size(&s2m, bytes);
  }
  TIMER_STOP;
  total = TIMER_ELAPSED;
  total -= calibrate_cache_flush(cnt);
  return(((unsigned long long)cnt*1000000) / total); /* Transactions/sec */
}

int roundtrip_slave(int cnt, int bytes) {
  int i;

  for (i=0; i<cnt; i++) {
    if (flush & FLUSH_BETWEEN_ITERATIONS)
      {
      inval_dcache();
      inval_mcache();
    }
    mp_recv_size(&m2s, bytes);
    mp_send_size(&s2m, bytes);
  }
  return 0;
}


int bandwidth_master(int cnt, int bytes)
{
  int i;
  unsigned long long total = 0;
      TIMER_START;
      for (i=0; i<cnt; i++)
    {
        if (flush & FLUSH_BETWEEN_ITERATIONS)
      {
      inval_dcache();
      inval_mcache();
    }
        mp_send_size(&m2s, bytes);
    }
      mp_recv(&s2m);
      mp_ack(&s2m);
      TIMER_STOP;
      total = TIMER_ELAPSED;
      total -= calibrate_cache_flush(cnt);
      return(((unsigned long long)cnt*bytes*1000000)/(total*1024)); /* KB/sec */
}

int bandwidth_slave(int cnt, int bytes)
{
  int i;

      for (i=0; i<cnt; i++)
    {
        if (flush & FLUSH_BETWEEN_ITERATIONS)
      {
      inval_dcache();
      inval_mcache();
    }
        mp_recv_size(&m2s, bytes);
    }
      mp_send(&s2m);
      return 0;
}

///*  New bidirectional bandwidth test */
//
//int bibandwidth_master(int cnt, int bytes) {
//  int i;
//  long long int total = 0;
//  MPI_Request requestarray[2];
//  MPI_Status  statusarray[2];
//
//    TIMER_START;
//    for (i=0; i<cnt; i++)
//    {
//      if (flush & FLUSH_BETWEEN_ITERATIONS)
//        {
//      inval_dcache();
//      inval_mcache();
//    }
//      mp_irecv(dest_rank, 2, destbuf, bytes, &requestarray[1]);
//      mp_isend(dest_rank, 1, sendbuf, bytes, &requestarray[0]);
//      MPI_Waitall(2, requestarray, statusarray);
//    } 
//    TIMER_STOP;
//    total = TIMER_ELAPSED;
//    total -= calibrate_cache_flush(cnt);
//    return((2.0*(int)cnt*(int)bytes)/(total*1.0E-6*1024.0)); /* KB/sec */
//}
//
//
//int bibandwidth_slave(int cnt, int bytes) {
//  int i;
//  MPI_Request requestarray[2];
//  MPI_Status  statusarray[2];
//
//    /* This is the last process */
//    for (i=0; i<cnt; i++)
//    {
//      if (flush & FLUSH_BETWEEN_ITERATIONS)
//        {
//      inval_dcache();
//      inval_mcache();
//    }
//      mp_irecv(source_rank, 1, destbuf, bytes, &requestarray[0]);
//      mp_isend(source_rank, 2, sendbuf, bytes, &requestarray[1]);
//      MPI_Waitall(2, requestarray, statusarray);
//    }
//    return 0;
//}

int broadcast_master(int cnt, int bytes)
{
  int i;
  unsigned long long total = 0;

  TIMER_START;
  for (i=0; i<cnt; i++) {
    if (flush & FLUSH_BETWEEN_ITERATIONS) {
      inval_dcache();
      inval_mcache();
    }
    mp_broadcast(&comm_world, 1);
  }
  TIMER_STOP;
  total = TIMER_ELAPSED;
  total -= calibrate_cache_flush(cnt);
  return(((unsigned long long)cnt*bytes*1000000)/(total*1024.0)); /* KB/sec */
}


int broadcast_slave(int cnt, int bytes)
{
  int i;

  for (i=0; i<cnt; i++) {
    if (flush & FLUSH_BETWEEN_ITERATIONS) {
      inval_dcache();
      inval_mcache();
    }
    mp_broadcast(&comm_world, 1);
  }
  return 0;

}

int barrier_master(int cnt, int bytes)
{
  int i;
  int total = 0;

  TIMER_START;
  for (i=0; i<cnt; i++) {
    if (flush & FLUSH_BETWEEN_ITERATIONS) {
      inval_dcache();
      //inval_mcache();
    }
    mp_barrier(&comm_world);
  }
  TIMER_STOP;
  total = TIMER_ELAPSED;
  total -= calibrate_cache_flush(cnt);
  return(total/cnt); /* usec */
}


int barrier_slave(int cnt, int bytes)
{
  int i;

  for (i=0; i<cnt; i++) {
    if (flush & FLUSH_BETWEEN_ITERATIONS) {
      inval_dcache();
      //inval_mcache();
    }
    mp_barrier(&comm_world);
  }
  return 0;

}


void loop(void* arg) {
  int (*test)(int, int) = (int (*)(int, int))arg;
  int i, j;
  int res;
  //communicator_t* loc_com = &comm;
  communicator_t* loc_com = &comm_world;
  //if (test == barrier_master || test == barrier_slave ||
  //    test == broadcast_master || test == broadcast_slave) {
  //  loc_com = &comm_world;
  //}
  //communicator_t stack_com;
  //stack_com.barrier_set = loc_com->barrier_set;
  //for(int i = 0; i < loc_com->count; i++) {
  //  stack_com.addr[i] = loc_com->addr[i];
  //}
  //stack_com.count = loc_com->count;
  //stack_com.msg_size = loc_com->msg_size;
  DEBUGGER("Initial test run\n");
  /* Allow routing/cache setup ahead of time */
  test(1,1024);
  DEBUGGER("Initial test run done\n");
  for( i=0; i < num_sizes; i++){
    if (flush & FLUSH_BETWEEN_SIZES)
      {
      inval_dcache();
      inval_mcache();
    }
    for( j=1; j <= repeat_count; j++){
      if (flush & FLUSH_BETWEEN_REPEATS) {
        inval_dcache();
        inval_mcache();
      }
      mp_barrier(loc_com);
      //mp_barrier(&stack_com);
      res = test(iterations, sizes[i]);
      my_two_printf("%u\t%i\n",sizes[i],res);
      mp_barrier(loc_com);
      //mp_barrier(&stack_com);
    }
  }

	inval_dcache();
	inval_mcache();

  if(get_cpuid() != 0){
    int ret = 0;
    corethread_exit(&ret);
  }
  return;

}

/********************/
/*  main            */
/********************/


int main() {
  int slave1 = 1;
  int slave2 = 2;
  int slave3 = 3;
  int slave4 = 4;
  int slave5 = 5;
  int slave6 = 6;
  int slave7 = 7;
  int slave8 = 8;

  if (!mp_chan_init(&m2s,
      get_cpuid(),
      slave4,
      BUFFER_SIZE,
      2)) {
      abort();
  }
  if (!mp_chan_init(&s2m,
      slave4,
      get_cpuid(),
      BUFFER_SIZE,
      2)) {
      abort();
  }
  if (!mp_communicator_init(&comm,
      2,
      cores,
      0)) {
    abort();
  }
  if (!mp_communicator_init(&comm_world,
      sizeof(cores_world)/sizeof(cores_world[0]),
      cores_world,
      BUFFER_SIZE)) {
    abort();
  }

  int* ret;
  /* set up the run */
  //corethread_create(slave,&loop,(void*)roundtrip_slave);

  /* run appropriate test */
//  // TEST_LATENCY
//  puts("Latency (usecs)");
//  corethread_create(slave4,&loop,(void*)latency_slave);
//  loop(latency_master);
//  corethread_join(slave4,(void**)&ret);
//
//  // TEST_ROUNDTRIP
//  puts("Roundtrip (Transactions/sec)");
//  corethread_create(slave4,&loop,(void*)roundtrip_slave);
//  loop(roundtrip_master);
//  corethread_join(slave4,(void**)&ret);
//
//  // TEST_BANDWIDTH
//  puts("Bandwidth (KB/sec)");
//  corethread_create(slave4,&loop,(void*)bandwidth_slave);
//  loop(bandwidth_master);
//  corethread_join(slave4,(void**)&ret);

//  // TEST_BIBANDWIDTH
//  puts("Bibandwidth");
//  corethread_create(slave4,&loop,(void*)bibandwidth_slave);
//  loop(bibandwidth_master);
//  corethread_join(slave4,(void**)&ret);
//
//  // TEST_REDUCE
//  puts("Reduce");
//  corethread_create(slave4,&loop,(void*)reduce_slave);
//  loop(reduce_master);
//  corethread_join(slave4,(void**)&ret);
//
//  // TEST_ALLREDUCE
//  puts("Allreduce");
//  corethread_create(slave4,&loop,(void*)allreduce_slave);
//  loop(allreduce_master);
//  corethread_join(slave4,(void**)&ret);
//
//  // TEST_ALLTOALL
//  puts("Alltoall");
//  corethread_create(slave4,&loop,(void*)alltoall_slave);
//  loop(alltoall_master);
//  corethread_join(slave4,(void**)&ret);

  /////////////////////////////////////////////////////////////////////////////
  // TEST_BARRIER
  /////////////////////////////////////////////////////////////////////////////
  puts("Barrier (usecs)");
  for(int i = 0; i < sizeof(cores_world)/sizeof(cores_world[0]); i++) {
    if (i != NOC_MASTER) {
      if(corethread_create(cores_world[i],&loop,(void*)barrier_slave) != 0){
        printf("Corethread %d not created\n",i);
      }
    }
  }
  loop(barrier_master);
  //puts("Master finished");
  for (int i = 0; i < sizeof(cores_world)/sizeof(cores_world[0]); ++i) {
    if (i != NOC_MASTER) {
      corethread_join(cores_world[i],(void**)&ret);
      //printf("Slave %d joined\n",i);
    }
  }



//  /////////////////////////////////////////////////////////////////////////////
//  // TEST_BROADCAST
//  /////////////////////////////////////////////////////////////////////////////
//  puts("Broadcast (KB/sec)");
//  for(int i = 0; i < sizeof(cores_world)/sizeof(cores_world[0]); i++) {
//    if (i != NOC_MASTER) {
//      if(corethread_create(cores_world[i],&loop,(void*)broadcast_slave) != 0){
//        printf("Corethread %d not created\n",i);
//      }
//    }
//  }
//  loop(broadcast_master);
//  //puts("Master finished");
//  for (int i = 0; i < sizeof(cores_world)/sizeof(cores_world[0]); ++i) {
//    if (i != NOC_MASTER) {
//      corethread_join(cores_world[i],(void**)&ret);
//      //printf("Slave %d joined\n",i);
//    }
//  }

  exit(0);
}
