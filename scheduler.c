/************************************************************************
 *
 * CSE130 Assignment 4
 *
 * Copyright (C) 2021-2022 David C. Harrison. All right reserved.
 *
 * You may not use, distribute, publish, or modify this code without 
 * the express written permission of the copyright holder.
 * 
 ************************************************************************/

/**
 * See scheduler.h for function details. All are callbacks; i.e. the simulator 
 * calls you when something interesting happens.
 */
#include <stdlib.h>
#include <stdio.h>
#include "simulator.h"
#include "scheduler.h"
#include "queue.h"
/**
 * Initalise a scheduler implemeting the requested ALGORITHM. QUANTUM is only
 * meaningful for ROUND_ROBIN.
 */


 // this is for remaining time
typedef struct __remain_t{
  unsigned int remain;
  unsigned int q_count ;
 // int flag ;   //if the thread hasn't runned yet, 
}remain_t ;

// this is the outer
typedef struct __time_t {
   thread_t *thread;
   unsigned int arrival ; 
   unsigned int wait ; 
   unsigned int started_waiting ;
   unsigned int finish ; 
   remain_t *remaining_time; 
  //  unsigned int run_start ; 
} times;

 void *ready_queue ;
 void *stats_queue ; 
 thread_t *running = NULL; 
 bool cpu = false; // if true : cpu is available, if false : cpu is unavailble
 
 int count = 0 ;   // used to set cpu to true for first thread initialized 
  unsigned int quan;
static bool inner_equalitor(void *time, void *thread) {
       
        return ((times*)time)->thread == (thread_t*)thread;
    }
enum algorithm algo ; 
void scheduler(enum algorithm algorithm, unsigned int quantum) {
 ready_queue  = queue_create();
 stats_queue = queue_create(); 
 

 // what is the type of scheduler?
  algo = algorithm;
  quan  = quantum; 

}

/**
 * Thread T is ready to be scheduled for the first time.
 */

// static int priority_comparator(void *a, void *b) {
//        return ((thread_t*)a)->priority - ((thread_t*)b)->priority;
// }   
static int priority_comparator(void *a, void *b) {
    return ((times*)a)->thread->priority - ((times*)b)->thread->priority;
  }
static int job_comparator(void *a, void *b) {
    return ((times*)a)->thread->length - ((times*)b)->thread->length;
  }

static int time_comparator(void *a, void *b) {
    return ((times*)a)->remaining_time->remain - ((times*)b)->remaining_time->remain; 
  }

void outer_iterator(void *outer) {
      // int run = sim_time() - ((times*)outer)->remaining_time->run_start;
      // printf("thread %d : %d\n",((times*)outer)->thread->tid, run);
      // ((times*)outer)->remaining_time->remain =  ((times*)outer)->thread->length - run ;
    }
void set_iterator (void *outer){
     ((times*)outer)->remaining_time->q_count = quan;  
}
void sys_exec(thread_t *t) {

 //printf("sys_exec: %d \n", t->tid);
 times *time = malloc(sizeof(times));
 remain_t *remain = malloc(sizeof(remain_t));
 time->remaining_time = remain; 
 
 time->thread = t; 
// printf("t->length: %d\n", t->length);
 // set arrival and started_waiting
 time->started_waiting = sim_time() ;
 time->arrival = sim_time();
 time->wait = 0;
 time->remaining_time->remain = time->thread->length;
 time->finish = 0; 
 //time->flag = 0 ; 
 queue_enqueue(ready_queue, time);
 
 queue_enqueue(stats_queue , time);
 
 count ++;
 
 if ( count == 1){
   cpu = true; 
   running = t;
 }

 // sort the ready list by priority lowest number of priority first 
 if (algo == NON_PREEMPTIVE_PRIORITY || algo == PREEMPTIVE_PRIORITY ){
   queue_sort(ready_queue, priority_comparator );
 } 
 // sort the ready list by Shortest Job shortest length first
 else if (algo == NON_PREEMPTIVE_SHORTEST_JOB_FIRST || algo == PREEMPTIVE_SHORTEST_JOB_FIRST ){
   queue_sort(ready_queue, job_comparator ); 
 }
 // sort the ready list by Remaining Time First
 else if (algo == NON_PREEMPTIVE_SHORTEST_REMAINING_TIME_FIRST || algo == PREEMPTIVE_SHORTEST_REMAINING_TIME_FIRST ){
    queue_iterate(ready_queue, outer_iterator);
   queue_sort(ready_queue, time_comparator ); 
 }
 else if ( algo == ROUND_ROBIN ){
    // set remaining_time->q_count to quan
    time->remaining_time->q_count = 0;

 }
}
//function to find times 
//static bool outer_equalitor(void *outer, void *inner){
//  return ((times*)outer) == (times*)inner;
//}
/**
 * Programmable clock interrupt handler. Call sim_time() to find out
 * what tick this is. Called after all calls to sys_exec() for this
 * tick have been made.
 */
void tick() { 
 // cpu is available -> ready list -> sim  
 // dispatch only in tck 
 // if preempt happens sort the ready list and chec
 if( algo == PREEMPTIVE_PRIORITY || algo == PREEMPTIVE_SHORTEST_JOB_FIRST || algo == PREEMPTIVE_SHORTEST_REMAINING_TIME_FIRST){
   if(queue_head(ready_queue)!= NULL){
    if( algo == PREEMPTIVE_PRIORITY ){  
    queue_sort(ready_queue, priority_comparator );
    }
    else if(  algo == PREEMPTIVE_SHORTEST_JOB_FIRST ){
     queue_sort(ready_queue, job_comparator );  
    }
    else if (algo == PREEMPTIVE_SHORTEST_REMAINING_TIME_FIRST){ // sort the ready list by Remaining Time First 
      queue_iterate(ready_queue, outer_iterator);
      queue_sort(ready_queue, time_comparator );  
    }
    
    // if running cpu doesn't has the highest priority
    times *t = queue_head(ready_queue);
 
    if (running != NULL && cpu == false ){
     if( algo == PREEMPTIVE_PRIORITY && t->thread->priority < running->priority ){   
   // if (cpu == false && t->thread->priority < running->priority){

       cpu = true;
       times *found = queue_find(stats_queue, inner_equalitor, running);
       if(found->remaining_time->remain != 0 ){
       queue_enqueue(ready_queue, found);
       found->started_waiting = sim_time()  ; 
      
       }
    }
    else if(algo ==  PREEMPTIVE_SHORTEST_JOB_FIRST  && t->thread->length < running->length ){
       cpu = true;
       times *found = queue_find(stats_queue, inner_equalitor, running);
       if(found->remaining_time->remain != 0 ){
       queue_enqueue(ready_queue, found);
       found->started_waiting = sim_time()  ; 
      
       } 
    }else if (algo ==  PREEMPTIVE_SHORTEST_REMAINING_TIME_FIRST ){
       times *found = queue_find(stats_queue, inner_equalitor, running);
       if(t->remaining_time->remain< found->remaining_time->remain ){
          cpu = true;
     
       if(found->remaining_time->remain != 0){
       queue_enqueue(ready_queue, found);
       found->started_waiting = sim_time()  ;  
       }
    }
    }

   }
 
 }
 }

 else if (algo == ROUND_ROBIN ){
    if( queue_head(stats_queue)!= NULL){
     if(running != NULL){
     times *found = queue_find(stats_queue, inner_equalitor, running); 
      if(queue_head(ready_queue)== NULL){
        if(found->remaining_time->q_count == quan){
          found->remaining_time->q_count = 0;
        }
      }
      else if(found->remaining_time->q_count == quan){
        found->remaining_time->q_count  = 0 ;
        found->started_waiting = sim_time();
        queue_enqueue(ready_queue, found);
        cpu = true; 
      }
    }
    }
  }

 if(cpu == true && queue_head(ready_queue)!= NULL  ){
   times *time = queue_dequeue(ready_queue);

  sim_dispatch(time->thread);


  running = time->thread; 
  //update waiting for sim_dispatch
 
  //time->remaining_time->run_start = sim_time();
  time->wait = time->wait +  (sim_time() - time->started_waiting);
  //}
  cpu = false; 
 } 

if (algo == ROUND_ROBIN ){
  if(running!= NULL){
    times *found = queue_find(stats_queue, inner_equalitor, running);
    found->remaining_time->q_count += 1; 

}
}
 // reamining time will be decreased by 1 
  if(running!= NULL){
    times *found = queue_find(stats_queue, inner_equalitor, running);
  if(found != NULL){
    found->remaining_time->remain -= 1; 
  } 
 }
 
 
}

/**
 * Thread T has completed execution and should never again be scheduled.
 */
void sys_exit(thread_t *t) {

  times *found = queue_find(stats_queue, inner_equalitor, t);
  // set finish 
  if(found != NULL){
    found->finish = sim_time();
  }
  
  cpu  = true;
  running  = NULL; 
  
}

/**
 * Thread T has requested a read operation and is now in an I/O wait queue.
 * When the read operation starts, io_starting(T) will be called, when the
 * read operation completes, io_complete(T) will be called.
 */
void sys_read(thread_t *t) { 
 // printf("sys_read\n");
 // printf("thread tid : %d \n", t->tid);
    times *found = queue_find(stats_queue, inner_equalitor, t);
   if(found != NULL){
     // set started waiting
    found->started_waiting = sim_time() + 1;
   }
  cpu = true;
  running = NULL;
  //free(t); 
}

/**
 * Thread T has requested a write operation and is now in an I/O wait queue.
 * When the write operation starts, io_starting(T) will be called, when the
 * write operation completes, io_complete(T) will be called.
 */
void sys_write(thread_t *t) {
  //sim_dispatch(t);
 
  times *found = queue_find(stats_queue, inner_equalitor, t);
  if(found != NULL){
    // set started waiting
   found->started_waiting = sim_time() + 1; 
  }
  cpu = true; 
  running = NULL;
   //free(t); 
}

/**
 * An I/O operation requested by T has completed; T is now ready to be 
 * scheduled again.
 */
void io_complete(thread_t *t) {
  //sim_dispatch(t);

  
  times *found = queue_find(stats_queue, inner_equalitor, t);

  if(found != NULL){
  queue_enqueue(ready_queue, found);
   if (algo == NON_PREEMPTIVE_PRIORITY || algo == PREEMPTIVE_PRIORITY ){
   queue_sort(ready_queue, priority_comparator );
 } 
 else if (algo == NON_PREEMPTIVE_SHORTEST_JOB_FIRST || algo == PREEMPTIVE_SHORTEST_JOB_FIRST ){
   queue_sort(ready_queue, job_comparator ); 
 }
  // sort the ready list by Remaining Time First
 else if (algo == NON_PREEMPTIVE_SHORTEST_REMAINING_TIME_FIRST || algo == PREEMPTIVE_SHORTEST_REMAINING_TIME_FIRST ){
   queue_iterate(ready_queue, outer_iterator);
   queue_sort(ready_queue, time_comparator ); 
 }
 else if(algo == ROUND_ROBIN){
    found->remaining_time->q_count = 0;
 }
  //set started waiting as the tick time
  //times *found = queue_find(stats_queue, inner_equalitor, t);
 
    // set started waiting
  
  found->started_waiting = sim_time() + 1 ; 
  }

  
}

/**
 * An I/O operation requested by T is starting; T will not be ready for
 * scheduling until the operation completes.
 */
void io_starting(thread_t *t) {
  
   times *found = queue_find(stats_queue, inner_equalitor, t);
   if(found != NULL){

  
   found->wait =  found->wait +  (sim_time() - found->started_waiting );  
   
   }
}

/**
 * Return dynamically allocated stats for the scheduler simulation, see 
 * scheduler.h for details. Memory allocated by your code will be free'd
 * by the similulator. Do NOT return a pointer to a stack variable.
 */

stats_t *stats() {

  //int thread_count = 1; 
  stats_t *stats = malloc(sizeof(stats_t));
  stats->tstats = malloc(sizeof(stats_t)*count);
  int total_turn = 0 ; 
  int total_wait = 0; 
  //int u = 0; 
  // while(queue_head(stats_queue)!= NULL){
  //   queue_dequeue(stats_queue);
  //   u ++;
  //   printf("%d      \n", u);
  // }
  while(queue_head(stats_queue)!= NULL){
    times * t = queue_dequeue(stats_queue);

    stats->tstats[t->thread->tid - 1].tid = t->thread->tid; 
    stats->tstats[t->thread->tid - 1].turnaround_time =  t->finish -  t->arrival + 1;
   
    total_turn += ( t->finish -  t->arrival + 1);
    stats->tstats[ t->thread->tid - 1].waiting_time =  t->wait; 
    total_wait +=  t->wait ;
    //free(t->thread);
    free(t->remaining_time);
    free(t);
    
  }
 
  stats->thread_count = count; 
  stats->turnaround_time = total_turn / count;
  stats->waiting_time = total_wait/count; 

  
  return stats; 
}
