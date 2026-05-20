/*
   Multicore demo with pthreads and a mutex-protected shared state.

   Each core pulls jobs from a shared queue, computes a deterministic
   workload, and contributes to a shared checksum under a lock.
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include <machine/patmos.h>

#define JOB_COUNT 64
#define WORK_UNITS 256

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static int next_job = 0;
static int finished_workers = 0;
static unsigned long long shared_sum = 0;

static unsigned long long compute_job_value(int job_id) {
  unsigned long long job_sum = 0;

  for (int i = 0; i < WORK_UNITS; ++i) {
    job_sum += (unsigned long long) (job_id + 1) * (unsigned long long) (i + 1);
  }

  return job_sum;
}

static void *worker(void *arg) {
  (void) arg;

  int core_id = get_cpuid();
  int local_jobs = 0;
  unsigned long long local_sum = 0;

  for (;;) {
    int job_id;

    pthread_mutex_lock(&lock);
    if (next_job < JOB_COUNT) {
      job_id = next_job++;
    } else {
      job_id = -1;
    }
    pthread_mutex_unlock(&lock);

    if (job_id < 0) {
      break;
    }

    local_sum += compute_job_value(job_id);
    ++local_jobs;
  }

  pthread_mutex_lock(&lock);
  shared_sum += local_sum;
  ++finished_workers;
  printf("core %d finished %d jobs, local sum=%llu\n",
         core_id, local_jobs, local_sum);
  pthread_mutex_unlock(&lock);

  return NULL;
}

int main(void) {
  int cpucnt = get_cpucnt();
  pthread_t *threads = malloc(sizeof(pthread_t) * cpucnt);
  unsigned long long expected_sum = 0;

  if (threads == NULL) {
    printf("Unable to allocate thread table\n");
    return 1;
  }

  printf("Starting %d cores with a mutex-protected work queue\n", cpucnt);

  for (int i = 1; i < cpucnt; ++i) {
    int retval = pthread_create(&threads[i], NULL, worker, NULL);
    if (retval != 0) {
      printf("Unable to start core thread %d, error code %d\n", i, retval);
      free(threads);
      return retval;
    }
  }

  worker(NULL);

  for (int i = 1; i < cpucnt; ++i) {
    void *dummy;
    int retval = pthread_join(threads[i], &dummy);
    if (retval != 0) {
      printf("Unable to join core thread %d, error code %d\n", i, retval);
      free(threads);
      return retval;
    }
  }

  free(threads);

  for (int job_id = 0; job_id < JOB_COUNT; ++job_id) {
    expected_sum += compute_job_value(job_id);
  }

  printf("Expected jobs=%d, completed workers=%d\n",
         JOB_COUNT, finished_workers);
  printf("Expected sum=%llu, actual sum=%llu\n", expected_sum, shared_sum);

  return (shared_sum == expected_sum && finished_workers == cpucnt) ? 0 : 1;
}