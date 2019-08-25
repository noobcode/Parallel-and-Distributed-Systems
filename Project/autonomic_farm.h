#ifndef __AUTONOMIC_FARM_H__
#define __AUTONOMIC_FARM_H__

#include "./farm_emitter.h"
#include "./farm_worker.h"
#include "./farm_collector.h"
#include "./farm_manager.h"
#include "./safe_queue.h"
#include "./farm_utility.h"

#include <iostream>
#include <chrono>

//template <class T> class AutonomicFarm
class AutonomicFarm{
private:
  unsigned int max_nw; // maximum number of workers

  SafeQueue<int>* workers_requests; // unilateral channel from Workers to Emitter
  std::vector<SafeQueue<Task*>*>* task_queues; // unilateral channels from Emitter to each Worker
  SafeQueue<Task*>* workers_result; // queue where workers push results
  SafeQueue<int>* output_stream;    // queue where collector pushs results
  SafeQueue<std::chrono::milliseconds*>* latency_queue;

  FarmEmitter emitter;
  std::vector<FarmWorker*>* workers;
  FarmCollector collector;
  FarmManager manager;

  // TODO save statistics for later access

public:
  // contructors
  AutonomicFarm(unsigned int max_nw,
                std::function<int(int)> f) : max_nw(max_nw),
                                       workers_requests(new SafeQueue<int>(max_nw)),
                                       task_queues(new std::vector<SafeQueue<Task*>*>(max_nw)),
                                       workers_result(new SafeQueue<Task*>),
                                       output_stream(new SafeQueue<int>),
                                       latency_queue(new SafeQueue<std::chrono::milliseconds*>),
                                       emitter(max_nw, workers_requests, task_queues),
                                       workers(new std::vector<FarmWorker*>(max_nw)),
                                       collector(max_nw, workers_result, output_stream),
                                       manager(max_nw, workers, latency_queue)
  {
    // TODO unify for loops
    for(size_t i = 0; i < max_nw; i++)
      workers->at(i) = new FarmWorker(-1, workers_requests, nullptr, workers_result, latency_queue, f, INACTIVE);

    // allocate task queues
    for(size_t i = 0; i < max_nw; i++)
      task_queues->at(i) = new SafeQueue<Task*>(1);

    // assign an ID and a task queue to each worker
    for(size_t i = 0; i < max_nw; i++){
      workers->at(i)->setWorkerId(i);
      workers->at(i)->setTaskQueue(task_queues->at(i));
    }
  };

  // methods

  void run(std::vector<int> tasks, unsigned int nw_initial, std::chrono::milliseconds service_time_goal){
    emitter.run(tasks);
    for(size_t i = 0; i < max_nw; i++)
      workers->at(i)->run();
    collector.run();
    manager.run(nw_initial, service_time_goal);
  }

  void wait(){
    // join threads
    emitter.join();
    for(size_t i = 0; i < max_nw; i++){
      workers->at(i)->join();
    }
    collector.join();
    manager.join();
  }

  void run_and_wait(std::vector<int> tasks, unsigned int nw_initial, std::chrono::milliseconds service_time_goal){
    run(tasks, nw_initial, service_time_goal);
    wait();
  }

  void printFarm(){
    std::cout << "==== FARM ====" << std::endl;
    std::cout << "max_nw:" << max_nw << std::endl;

    std::cout << "(Workers->Emitter) workers_requests: address/"
              << workers_requests << " - size/"
              << workers_requests->safeSize() <<  " - max_size/"
              << workers_requests->maxSize() << std::endl;

    std::cout << "(Emitter->Workers) task_queues: address/"
              << task_queues << " - size/"
              << task_queues->size() << std::endl;
    for(size_t i=0; i < task_queues->size(); i++)
      std::cout << (*task_queues)[i] << " - "
                << (*task_queues)[i]->safeSize() << " - "
                << (*task_queues)[i]->maxSize() << std::endl;

    // collector
    std::cout << "(Workers->Collector) input_stream: address/" << workers_result
              << " - size/" << workers_result->safeSize()
              << " - max_size/" << workers_result->maxSize() << std::endl;

    std::cout << "(results) output_stream: address/" << output_stream
              << " - size/" << output_stream->safeSize()
              << " - max_size/" << output_stream->maxSize() << std::endl;

    emitter.printEmitter();
    for(size_t i = 0; i < max_nw; i++)
      workers->at(i)->printWorker();
    collector.printCollector();
    manager.printManager();
  }

  void printResults(){
    std::cout << "results:" << std::endl;
    output_stream->empty_and_print();
  }
};


#endif // __AUTONOMIC_FARM_H__
