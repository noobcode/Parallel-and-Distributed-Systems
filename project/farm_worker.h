#ifndef __FARM_WORKER_H__
#define __FARM_WORKER_H__

#include <vector>
#include <iostream>
#include <thread>
#include "./safe_queue.h"

class FarmWorker{
private:
  SafeQueue<int>* workers_requests; // unilateral channel from Workers to Emitter
  std::vector<SafeQueue<int>*>* task_queues; // unilateral channels from Emitter to each Worker
  std::thread worker_thread;

public:
  // constructor
  FarmWorker(SafeQueue<int>* workers_requests,
             std::vector<SafeQueue<int>*>* task_queues): workers_requests(workers_requests),
                                                         task_queues(task_queues){};

  
};










#endif // __FARM_WORKER_H__
