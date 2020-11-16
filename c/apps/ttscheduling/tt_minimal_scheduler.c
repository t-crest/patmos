#include "tt_minimal_scheduler.h"

#ifdef WCET
__attribute__((noinline))
#endif
void init_minimal_tttask(MinimalTTTask *newTask, const uint16_t id, const schedtime_t period, schedtime_t *release_times, 
                        const schedtime_t nr_releases, void (*func)(const void *self))
{
  (newTask->func) = func;
  newTask->id = id;
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
  MinimalTTSchedule newSchedule = {
    .hyper_period = hyperperiod,
    .task_count = num_tasks,
    .get_time = get_time,
    .start_time = 0,
    .head = NULL,
    .tail = NULL
  };
  return newSchedule;
}

#ifdef WCET
__attribute__((noinline))
#endif
// A utility function to create a new task node. 
MinimalTTTaskNode* create_tttasknode(MinimalTTTask task) 
{ 
    MinimalTTTaskNode* temp = (MinimalTTTaskNode*)malloc(sizeof(MinimalTTTaskNode)); 
    temp->task = task; 
    temp->next = NULL; 
    return temp; 
} 

#ifdef WCET
__attribute__((noinline))
#endif
void ttschedule_enqueue(MinimalTTSchedule* schedule, MinimalTTTask task)
{
  // Create a new LL node 
  MinimalTTTaskNode* temp = create_tttasknode(task); 
  
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
void ttschedule_sortedinsert_period(MinimalTTSchedule *schedule, MinimalTTTaskNode* new_node)
{
  MinimalTTTaskNode ** head_ref = &schedule->head;
  MinimalTTTaskNode* current; 
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
void ttschedule_sortedinsert_release(MinimalTTSchedule *schedule, MinimalTTTaskNode* new_node)
{
  MinimalTTTaskNode ** head_ref = &schedule->head;
  MinimalTTTaskNode* current; 
  /* Special case for the head end */
  if (*head_ref == NULL || (*head_ref)->task.release_times[(*head_ref)->task.release_inst] >= new_node->task.release_times[new_node->task.release_inst]) 
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
              current->next->task.release_times[current->next->task.release_inst] < new_node->task.release_times[new_node->task.release_inst]) 
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
MinimalTTTaskNode* ttschedule_dequeue(MinimalTTSchedule* schedule)
{
  // If queue is empty, return NULL. 
  if (schedule->head == NULL) 
      return NULL; 

  // Store previous head and move head one node ahead 
  MinimalTTTaskNode* dequeued_tasknode = schedule->head; 
  
  schedule->head = schedule->head->next; 

  // If head becomes NULL, then change tail also as NULL 
  if (schedule->head == NULL) 
      schedule->tail = NULL; 
  
  schedule->task_count--;
  return dequeued_tasknode;
}

#ifdef WCET
__attribute__((noinline))
#endif
uint32_t tt_minimal_schedule_loop(MinimalTTSchedule *schedule, const uint32_t noLoops, const bool infinite)
{
  uint32_t scheduleExecutedTasks = 0;
  schedtime_t start_time = schedule->get_time();
  schedtime_t current_time = (schedule->get_time() - start_time);
  #pragma loopbound min 1 max 1
  while (infinite || current_time < noLoops*schedule->hyper_period) 
  {
    scheduleExecutedTasks += tt_minimal_dispatcher((MinimalTTSchedule *)schedule, current_time);
    current_time = (schedtime_t) (schedule->get_time() - start_time);
  }
  return scheduleExecutedTasks;
}

#ifdef WCET
__attribute__((noinline))
#endif
uint8_t tt_minimal_dispatcher(MinimalTTSchedule *schedule, const schedtime_t current_time)
{
  MinimalTTTaskNode* node_itr = schedule->head;
  #pragma loopbound min 1 max 1
  while(node_itr != NULL)
  {
    #ifdef DEBUG
    printf("@ %llu ? task_%d  w/ rt = %llu              ", 
          current_time, node_itr->task.id, node_itr->task.release_times[node_itr->task.release_inst]);
    print_ttschedule(schedule->head);
    #endif
    if(current_time >= node_itr->task.release_times[node_itr->task.release_inst])
    {
      // Pop & Execute
      node_itr->task.func(&node_itr->task);
      node_itr->task.release_times[node_itr->task.release_inst] += schedule->hyper_period;
      node_itr->task.release_inst = (node_itr->task.release_inst + 1) % node_itr->task.nr_releases;
      node_itr->task.delta_sum += node_itr->task.last_release_time == 0 ? node_itr->task.period : 
                                  (current_time - node_itr->task.last_release_time);
      node_itr->task.last_release_time = current_time;
      node_itr->task.exec_count++;      
      // Assign new index
      MinimalTTTaskNode* exec_task = ttschedule_dequeue(schedule);
      ttschedule_sortedinsert_release(schedule, exec_task);
      return 1;
    }
    node_itr = node_itr->next;
  }
  return 0;
}

// This function prints contents of the schedule
void print_ttschedule(MinimalTTTaskNode* n) 
{ 
  printf("|");
    while (n != NULL) { 
        printf("t_%d--> ", n->task.id); 
        n = n->next; 
    }
  printf("NULL|\n");
} 