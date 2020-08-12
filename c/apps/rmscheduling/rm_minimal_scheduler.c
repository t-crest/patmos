#include "rm_minimal_scheduler.h"

#ifdef WCET
__attribute__((noinline))
#endif
void init_minimal_rmtask(MinimalRMTask *new_task, const uint16_t id, const schedtime_t period, const schedtime_t deadline, const uint32_t wcet, const schedtime_t earliest_release, void (*func)(const void *self))
{
  (new_task->func) = func;
  new_task->id = id;
  new_task->state = READY;
  new_task->period = period;
  new_task->deadline = deadline;
  new_task->wcet = wcet;
  new_task->release_time = earliest_release;
  new_task->last_release_time = 0;
  new_task->delta_sum = 0;
  new_task->exec_count = 0;
  new_task->overruns = 0;
}

#ifdef WCET
__attribute__((noinline))
#endif
// A utility function to create a new task node. 
MinimalRMTaskNode* create_rmtasknode(MinimalRMTask task) 
{ 
    MinimalRMTaskNode* temp = (MinimalRMTaskNode*)malloc(sizeof(MinimalRMTaskNode)); 
    temp->task = task; 
    temp->next = NULL; 
    return temp; 
} 

#ifdef WCET
__attribute__((noinline))
#endif
// A utility function to create an empty schedule 
MinimalRMSchedule init_minimal_rmschedule(const schedtime_t hyperperiod, const uint32_t num_tasks, schedtime_t (*get_time)(void))
{ 
    MinimalRMSchedule schedule = {
      .hyper_period = hyperperiod,
      .task_count = 0,
      .get_time = get_time,
      .start_time = 0,
      .head = NULL,
      .tail = NULL
    }; 
    return schedule; 
}

#ifdef WCET
__attribute__((noinline))
#endif
// The function to add a key task to schedule 
void rmschedule_enqueue(MinimalRMSchedule* schedule, MinimalRMTask task) 
{ 
    // Create a new LL node 
    MinimalRMTaskNode* temp = create_rmtasknode(task); 
    
    schedule->task_count++;

    // If queue is empty, then new task is head and tail both 
    if (schedule->tail == NULL) { 
        schedule->head = schedule->tail = temp; 
        return; 
    } 
  
    // Add the new task at the end of queue and change tail 
    schedule->tail->next = temp; 
    schedule->tail = temp; 
}


#ifdef WCET
__attribute__((noinline))
#endif
// The function to add a key task to schedule sorted by period 
void rmschedule_sortedinsert(MinimalRMSchedule *schedule, MinimalRMTaskNode* new_node) 
{ 
    MinimalRMTaskNode ** head_ref = &schedule->head;
    MinimalRMTaskNode* current; 
    /* Special case for the head end */
    if (*head_ref == NULL || (*head_ref)->task.period >= new_node->task.period) 
    { 
        new_node->next = *head_ref; 
        *head_ref = new_node; 
    } 
    else
    { 
        /* Locate the node before the point of insertion */
        current = *head_ref; 
        #pragma loopbound min 1 max 1
        while (current->next!=NULL && 
               current->next->task.period < new_node->task.period) 
        { 
            current = current->next; 
        } 
        new_node->next = current->next; 
        current->next = new_node; 
    } 
    schedule->task_count++;
} 

  
#ifdef WCET
__attribute__((noinline))
#endif
// Function to remove a key from given queue schedule 
MinimalRMTaskNode* rmschedule_dequeue(MinimalRMSchedule* schedule) 
{ 
    // If queue is empty, return NULL. 
    if (schedule->head == NULL) 
        return NULL; 
  
    // Store previous head and move head one node ahead 
    MinimalRMTaskNode* dequeued_tasknode = schedule->head; 
    
    schedule->head = schedule->head->next; 
  
    // If head becomes NULL, then change tail also as NULL 
    if (schedule->head == NULL) 
        schedule->tail = NULL; 
    
    schedule->task_count--;
    return dequeued_tasknode;
} 


// #ifdef WCET
// __attribute__((noinline))
// #endif
// void minimal_rm_scheduler(MinimalRMSchedule *schedule)
// {
//   schedtime_t current_time = (schedtime_t) (schedule->get_time() - schedule->start_time);
//   MinimalRMTaskNode* node_itr = schedule->head;
//   #pragma 
//   while(node_itr != NULL)
//   { 
//     #ifdef DEBUG
//     printf("? Check task_%d @ %llu w/ rt = %llu\t\t\t\t", node_itr->task.id, current_time, node_itr->task.release_time);
//     print_rmschedule(schedule->head);
//     #endif
//     if(current_time >= node_itr->task.release_time )
//     {
//       // Pop & Execute
//       node_itr->task.state = ELECTED;
//       node_itr->task.release_time = current_time + node_itr->task.period;
//       node_itr->task.delta_sum += node_itr->task.last_release_time == 0 ? 0 : (current_time - node_itr->task.last_release_time);
//       node_itr->task.last_release_time = current_time;
//       node_itr->task.exec_count++;
//       node_itr->task.func(&node_itr->task);
//       node_itr->task.state = READY;
//       break;
//     }
//     node_itr = node_itr->next;
//   }
// }


#ifdef WCET
__attribute__((noinline))
#endif
uint8_t minimal_rm_scheduler(MinimalRMSchedule *schedule)
{
  schedtime_t current_time = (schedtime_t) (schedule->get_time() - schedule->start_time);
  MinimalRMTaskNode* node_itr = schedule->head;
  #pragma loopbound min 1 max 1
  while(node_itr != NULL)
  { 
    #ifdef DEBUG
    printf("? Check task_%d @ %llu w/ rt = %llu\t\t\t\t", node_itr->task.id, current_time, node_itr->task.release_time);
    print_rmschedule(schedule->head);
    #endif
    if(current_time >= node_itr->task.release_time )
    {
      // Pop & Execute
      node_itr->task.state = ELECTED;
      node_itr->task.release_time = current_time + node_itr->task.period;
      node_itr->task.func(&node_itr->task);
      node_itr->task.delta_sum += node_itr->task.last_release_time == 0 ? node_itr->task.period : (current_time - node_itr->task.last_release_time);
      node_itr->task.last_release_time = current_time;
      node_itr->task.exec_count++;
      node_itr->task.overruns += (schedule->get_time() - schedule->start_time) > (node_itr->task.exec_count * node_itr->task.deadline) ? 1 : 0;
      node_itr->task.state = READY;
      // Assign new index
      MinimalRMTaskNode* exec_task = rmschedule_dequeue(schedule);
      rmschedule_sortedinsert(schedule, exec_task);
      return 1;
    }
    node_itr = node_itr->next;
  }
  return 0;
}


schedtime_t calc_hyperperiod(const MinimalRMSchedule *schedule, const uint32_t step)
{
  schedtime_t lcm = 0;
  MinimalRMTaskNode* node_itr = schedule->head;
  while(node_itr != NULL)
  {
    if(node_itr->task.period > lcm)
    {
      lcm = node_itr->task.period;
    }
    node_itr = node_itr->next;
  }
  int found_lcm = 0;
  while(1)
  {
    node_itr = schedule->head;
    while(node_itr != NULL){
      found_lcm = lcm % node_itr->task.period == 0 ? found_lcm+1 : 0;
      #ifdef DEBUG
      printf("LCM Check %llu %% %llu == %lu\n", lcm, node_itr->task.period, found_lcm);
      #endif
      node_itr = node_itr->next;
    }
    if(found_lcm == schedule->task_count)
    {
      break;
    }
    lcm += step;
  }
  return lcm;
}


#ifdef WCET
__attribute__((noinline))
#endif
void sort_period_rmtasks(MinimalRMTask tasks[], const uint32_t tasks_count)
{
  #pragma loopbound min 1 max 1
  for (int i = 0; i < tasks_count; i++)                     
	{
    #pragma loopbound min 1 max 1
		for (int j = 0; j < tasks_count; j++)             
		{
			if (tasks[j].period > tasks[i].period)                    
			{
				MinimalRMTask tmp = tasks[i];         
				tasks[i] = tasks[j];            
				tasks[j] = tmp;             
			}
		}
	}
}

// This function prints contents of the schedule
void print_rmschedule(MinimalRMTaskNode* n) 
{ 
  printf("|");
    while (n != NULL) { 
        printf("t_%d--> ", n->task.id); 
        n = n->next; 
    }
  printf("|\n");
} 