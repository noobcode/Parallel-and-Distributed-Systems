//#pragma once

//The following is the #include header guard.
#ifndef __AUTONOMIC_FARM_H__
#define __AUTONOMIC_FARM_H__

#include "./farm_emitter.h"
#include <iostream>
#include "./safe_queue.h"

enum WorkerStatus {inactive, active};

//template <class T> class AutonomicFarm
class AutonomicFarm{
private:
  unsigned int max_nw; // maximum number of workers
  //std::atomic<size_t> nw;     // current number of active workers

  FarmEmitter emitter;
  //FarmCollector collector;
  //FarmWorker workers[]
  //FarmManager manager;

  //TaskSlot worker_tasks; // queue of size one, item i is the slot for worker i
  //WorkerStatus worker_status[];
  //std::atomic<size_t> last_active_worker;
  SafeQueue<int> workers_requests; // unilateral channel from Workers to Emitter
  std::vector<SafeQueue<int>> task_queues; // unilateral channels from Emitter to each Worker
  unsigned int service_time_goal;
  // TODO save statistics for later access

public:
  // contructors
  AutonomicFarm(unsigned int max_nw) :
  max_nw(max_nw), workers_requests(max_nw), task_queues(max_nw),
  emitter(max_nw, workers_requests, task_queues) {};

  // methods
  //void run_and_wait(){ // TODO return results?
    //emitter.run(tasks);
    //startWorkers();
    //collector.run();
    //manager.run();
  //}

  void print_af(){
    std::cout << "max_nw:" << max_nw << std::endl;

    // inizialmente vuota, dim max = max_nw
    std::cout << "size and max size of W->E queue...\n";
    std::cout << workers_requests.safeSize() << std::endl << workers_requests.maxSize() << std::endl;

    std::cout << "max size of E->W queue...\n";
    for(int i=0; i < task_queues.size(); i++)
      std::cout << task_queues[i].maxSize() << std::endl;
  }
};

#endif // __AUTONOMIC_FARM_H__
