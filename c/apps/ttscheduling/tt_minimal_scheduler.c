#include "tt_minimal_scheduler.h"

#ifdef WCET
__attribute__((noinline))
#endif
void init_minimal_tttask(MinimalTTTask *newTask, const unsigned long long period, const unsigned long long activation_time, long long (*func)(void *self))
{
  newTask->period = period;
  newTask->activation_time = activation_time;
  (newTask->func) = func;
  newTask->last_start_time = 0.0;
}

#ifdef WCET
__attribute__((noinline))
#endif
MinimalTTSchedule init_minimal_ttschedule(const unsigned long long hyperperiod, const uint16_t num_tasks, MinimalTTTask *tasks)
{
  MinimalTTSchedule newSchedule;
  newSchedule.hyperperiod = hyperperiod;
  newSchedule.num_tasks = num_tasks;
  newSchedule.tasks = tasks;
  return newSchedule;
}

#ifdef WCET
__attribute__((noinline))
#endif
uint32_t tt_minimal_schedule_loop(MinimalTTSchedule *schedule, const uint16_t noLoops, const bool infinite)
{
  uint32_t scheduleExecutedTasks = 0;
  register unsigned long long scheduleTime;
  schedule->scheduleTimeStart = get_cpu_usecs();
  _Pragma("loopbound min 1 max 1")
  while(scheduleTime < noLoops*schedule->hyperperiod || infinite){
      schedule->currentTime = get_cpu_usecs();
      scheduleTime = schedule->currentTime - schedule->scheduleTimeStart;
      scheduleExecutedTasks += tt_minimal_dispatcher((MinimalTTSchedule *)schedule, scheduleTime);
  }
  return scheduleExecutedTasks;
}

#ifdef WCET
__attribute__((noinline))
#endif
uint8_t tt_minimal_dispatcher(MinimalTTSchedule *schedule, const unsigned long long scheduleTime)
{
  uint8_t ans = 0;
  register unsigned i=0;
  _Pragma("loopbound min 1 max 1")
  for(i=0; i < schedule->num_tasks; i++){
      if(scheduleTime + WCET_DISPATCHER_US >= schedule->tasks[i].activation_time){
          schedule->tasks[i].func(&schedule->tasks[i]);
          schedule->tasks[i].activation_time += schedule->tasks[i].period;
          schedule->tasks[i].last_start_time = schedule->currentTime;
          return 1;
      }
  }
  return 0;
}

#ifdef WCET
__attribute__((noinline))
#endif
unsigned long long tt_minimal_wait(const unsigned long long timeout)
{
  unsigned long long elapsed=0;
  unsigned long long startTime;
  unsigned long long currentTime;
  startTime = get_cpu_usecs();
  _Pragma("loopbound min 1 max 1")
  while(elapsed < timeout){
      currentTime = get_cpu_usecs();
      elapsed = currentTime - startTime;
  }
  return elapsed;
}

#ifdef WCET
__attribute__((noinline))
#endif
void sort_asc_minimal_tttasks(MinimalTTTask *tasks, const unsigned short num_tasks)
{
  for (int i = 0; i < num_tasks; i++)                     
	{
		for (int j = 0; j < num_tasks; j++)             
		{
			if (tasks[j].period < tasks[i].period)                    
			{
				MinimalTTTask tmp = tasks[i];         
				tasks[i] = tasks[j];            
				tasks[j] = tmp;             
			}
		}
	}
}
