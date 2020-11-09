#include "tt_minimal_scheduler.h"

#ifdef WCET
__attribute__((noinline))
#endif
void init_minimal_tttask(MinimalTTTask *newTask, const schedtime_t period, schedtime_t *release_times, 
                        const schedtime_t nr_releases, void (*func)(const void *self))
{
  (newTask->func) = func;
  newTask->period = period;
  newTask->last_release_time = 0;
  newTask->delta_sum = 0;
  newTask->exec_count = 0;
  newTask->release_inst = 0;
  newTask->nr_releases = nr_releases;
  newTask->release_times = release_times;
}

#ifdef WCET
__attribute__((noinline))
#endif
MinimalTTSchedule init_minimal_ttschedule(const schedtime_t hyperperiod, const uint32_t num_tasks, MinimalTTTask *tasks, schedtime_t (*get_time)(void))
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
uint32_t tt_minimal_schedule_loop(MinimalTTSchedule *schedule, const uint32_t noLoops, const bool infinite)
{
  uint32_t scheduleExecutedTasks = 0;
  schedtime_t start_time = schedule->get_time();
  schedtime_t schedule_time = (schedule->get_time() - start_time);
  #pragma loopbound min 1 max 1
  while(infinite || schedule_time < noLoops*schedule->hyper_period) 
  {
      scheduleExecutedTasks +=
        tt_minimal_dispatcher((MinimalTTSchedule *)schedule, schedule_time);
      schedule_time = schedule->get_time() - start_time;
  };
  return scheduleExecutedTasks;
}

#ifdef WCET
__attribute__((noinline))
#endif
uint8_t tt_minimal_dispatcher(MinimalTTSchedule *schedule, const schedtime_t schedule_time)
{
  uint8_t ans = 0;
  #pragma loopbound min 1 max 1
  for(unsigned i=0; i < schedule->num_tasks; i++)
  {
      if(schedule_time + (uint64_t)(WCET_DISPATCHER * CLKS_TO_US) >= schedule->tasks[i].release_times[schedule->tasks[i].release_inst])
      {
          schedule->tasks[i].func(&schedule->tasks[i]);
          schedule->tasks[i].release_times[schedule->tasks[i].release_inst] += schedule->hyper_period;
          schedule->tasks[i].release_inst = (schedule->tasks[i].release_inst + 1) % schedule->tasks[i].nr_releases;
				  schedule->tasks[i].delta_sum += schedule->tasks[i].last_release_time == 0 ? schedule->tasks[i].period == 0 :
                                          (schedule_time - schedule->tasks[i].last_release_time);
          schedule->tasks[i].last_release_time = schedule_time;
          schedule->tasks[i].exec_count++;
          ans = 1;
          break;
      }
  }
  return ans;
}

#ifdef WCET
__attribute__((noinline))
#endif
void sort_asc_minimal_tttasks(MinimalTTTask *tasks, const uint32_t num_tasks)
{
  #pragma loopbound min 1 max 1
  for (int i = 0; i < num_tasks; i++)                     
	{
    #pragma loopbound min 1 max 1
		for (int j = 0; j < num_tasks; j++)             
		{
			if (tasks[j].period > tasks[i].period)                    
			{
				MinimalTTTask tmp = tasks[i];         
				tasks[i] = tasks[j];            
				tasks[j] = tmp;             
			}
		}
	}
}
