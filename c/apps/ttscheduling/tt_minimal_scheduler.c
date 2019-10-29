#include "tt_minimal_scheduler.h"

#ifdef WCET
__attribute__((noinline))
#endif
void init_minimal_tttask(MinimalTTTask *newTask, const unsigned long long period, const unsigned long long activation_time, long long (*func)(const void *self))
{
  newTask->period = period;
  newTask->activation_time = activation_time;
  (newTask->func) = func;
  newTask->last_start_time = 0.0;
}

#ifdef WCET
__attribute__((noinline))
#endif
MinimalTTSchedule init_minimal_ttschedule(const unsigned long long hyperperiod, const uint16_t num_tasks, MinimalTTTask *tasks, unsigned long long (*get_time)(void))
{
  MinimalTTSchedule newSchedule;
  newSchedule.hyper_period = hyperperiod;
  newSchedule.num_tasks = num_tasks;
  newSchedule.tasks = tasks;
  newSchedule.get_time = get_time;
  newSchedule.start_time = 0;
  return newSchedule;
}

#ifdef WCET
__attribute__((noinline))
#endif
uint32_t tt_minimal_schedule_loop(MinimalTTSchedule *schedule, const uint16_t noLoops, const bool infinite)
{
  uint32_t scheduleExecutedTasks = 0;
  register unsigned long long schedule_time;
  schedule->start_time = schedule->get_time();
  _Pragma("loopbound min 1 max 1")
  while(infinite || schedule_time < noLoops*schedule->hyper_period){
      schedule_time = schedule->get_time() - schedule->start_time;
      scheduleExecutedTasks += tt_minimal_dispatcher((MinimalTTSchedule *)schedule, schedule_time);
  }
  return scheduleExecutedTasks;
}

#ifdef WCET
__attribute__((noinline))
#endif
uint8_t tt_minimal_dispatcher(MinimalTTSchedule *schedule, const unsigned long long schedule_time)
{
  uint8_t ans = 0;
  long jitter = 0;
  register unsigned i=0;
  _Pragma("loopbound min 1 max 1")
  for(i=0; i < schedule->num_tasks; i++){
      if(schedule_time + WCET_DISPATCHER_US*schedule->num_tasks >= schedule->tasks[i].activation_time){
          schedule->tasks[i].func(&schedule->tasks[i]);
          schedule->tasks[i].activation_time += schedule->tasks[i].period;
          schedule->tasks[i].last_start_time = schedule_time + schedule->start_time;
          return 1;
      }
  }
  return 0;
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
