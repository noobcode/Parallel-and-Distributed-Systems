#ifndef __AUTONOMIC_FARM_H__
#define __AUTONOMIC_FARM_H__

#include "./farm_emitter.h"
#include "./farm_worker.h"
#include "./farm_collector.h"
#include <iostream>
#include "./safe_queue.h"
#include "./task.h"

enum WorkerStatus {inactive, active};

//template <class T> class AutonomicFarm
class AutonomicFarm{
private:
  unsigned int max_nw; // maximum number of workers
  //std::atomic<size_t> nw;     // current number of active workers
  SafeQueue<int>* workers_requests; // unilateral channel from Workers to Emitter
  std::vector<SafeQueue<Task*>*>* task_queues; // unilateral channels from Emitter to each Worker
  SafeQueue<Task*>* workers_result; // queue where workers push results
  SafeQueue<int>* output_stream;    // queue where collector pushs results

  FarmEmitter emitter;
  std::vector<FarmWorker> workers;
  FarmCollector collector;
  //FarmManager manager;

  //TaskSlot worker_tasks; // queue of size one, item i is the slot for worker i
  //WorkerStatus worker_status[];
  //std::atomic<size_t> last_active_worker;
  //unsigned int service_time_goal;
  // TODO save statistics for later access

public:
  // contructors
  AutonomicFarm(unsigned int max_nw,
                std::function<int(int)> f) : max_nw(max_nw),
                                       workers_requests(new SafeQueue<int>(max_nw)),
                                       task_queues(new std::vector<SafeQueue<Task*>*>(max_nw)),
                                       workers_result(new SafeQueue<Task*>),
                                       output_stream(new SafeQueue<int>),
                                       emitter(max_nw, this->workers_requests, this->task_queues),
                                       workers(max_nw, FarmWorker(-1, this->workers_requests, nullptr, workers_result, f)),
                                       collector(max_nw, this->workers_result, this->output_stream)
  {
    // allocate task queues
    for(size_t i = 0; i < max_nw; i++)
      task_queues->at(i) = new SafeQueue<Task*>(1);

    // assign an ID and a task queue to each worker
    for(size_t i = 0; i < max_nw; i++){
      workers[i].setWorkerId(i);
      workers[i].setTaskQueue(task_queues->at(i));
    }

    std::cout << "farm created" << std::endl;
  };

  // methods
  void run_and_wait(std::vector<int> tasks){
    std::cout << "farm executing..." << std::endl;
    emitter.run(tasks);
    for(size_t i = 0; i < max_nw; i++)
      workers[i].run();
    collector.run();
    //manager.run();

    // join threads
    emitter.join();
    for(size_t i = 0; i < max_nw; i++){
      workers[i].join();
    }
    collector.join();

  }

  void print_af(){
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
      workers[i].printWorker();
    collector.printCollector();
  }

  void printResults(){
    std::cout << "results:" << std::endl;
    output_stream->empty_and_print();
  }
};

#endif // __AUTONOMIC_FARM_H__