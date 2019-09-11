#ifndef __AUTONOMIC_FARM_H__
#define __AUTONOMIC_FARM_H__

#include "./farm_emitter.h"
#include "./farm_worker.h"
#include "./farm_collector.h"
#include "./farm_manager.h"
#include "./safe_queue.h"
#include "./farm_utility.h"
#include "./utimer.h"

#include <iostream>
#include <chrono>
#include <iostream>
#include <fstream>

template <typename Tin, typename Tout> class AutonomicFarm{
//class AutonomicFarm{
private:
  unsigned int max_nw; // maximum number of workers
  bool concurrency_throttling;
  SafeQueue<int>* workers_requests; // unilateral channel from Workers to Emitter
  std::vector<SafeQueue<Task<Tin>*>*>* task_queues; // unilateral channels from Emitter to each Worker
  SafeQueue<Task<Tout>*>* workers_result; // queue where workers push results
  SafeQueue<Tout>* output_stream;    // queue where collector pushes results
  SafeQueue<std::chrono::microseconds*>* latency_queue;

  FarmEmitter<Tin> emitter;
  std::vector<FarmWorker<Tin, Tout>*>* workers;
  FarmCollector<Tout> collector;
  FarmManager<Tin, Tout> manager;

  // TODO save statistics for later access
  std::chrono::microseconds completion_time;

public:
  // contructors
  AutonomicFarm(unsigned int max_nw,
                std::function<Tout(Tin)> f,
                float alpha,
                bool concurrency_throttling) : max_nw(max_nw),
                               concurrency_throttling(concurrency_throttling),
                               workers_requests(new SafeQueue<int>(max_nw)),
                               task_queues(new std::vector<SafeQueue<Task<Tin>*>*>(max_nw)),
                               workers_result(new SafeQueue<Task<Tout>*>),
                               output_stream(new SafeQueue<Tout>),
                               latency_queue(new SafeQueue<std::chrono::microseconds*>),
                               emitter(max_nw, workers_requests, task_queues),
                               workers(new std::vector<FarmWorker<Tin, Tout>*>(max_nw)),
                               collector(max_nw, workers_result, output_stream),
                               manager(alpha, max_nw, concurrency_throttling, workers, latency_queue)
  {
    // allocate task queues
    for(size_t i = 0; i < max_nw; i++)
      task_queues->at(i) = new SafeQueue<Task<Tin>*>(1);

    // initialize workers
    for(size_t i = 0; i < max_nw; i++)
      workers->at(i) = new FarmWorker<Tin, Tout>(i, concurrency_throttling,
            workers_requests, task_queues->at(i), workers_result, latency_queue, f, INACTIVE);
  };

  // methods
  void run(std::vector<Tin> tasks, unsigned int nw_initial, std::chrono::microseconds service_time_goal){
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

  void run_and_wait(std::vector<Tin> tasks, unsigned int nw_initial, std::chrono::microseconds service_time_goal)
  {
    {
      utimer timer(&completion_time);
      run(tasks, nw_initial, service_time_goal);
      wait();
    }
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

  std::vector<std::chrono::microseconds> getServiceTimeHistory(){
    auto service_time_history = manager.getServiceTimeHistory();
    return service_time_history;
  }

  void getActiveWorkersHistory(){
    std::cout << "=== active workers history ===" << std::endl;
    auto active_workers_history = manager.getActiveWorkersHistory();
    for(size_t i = 0; i < active_workers_history.size(); i++)
      std::cout << active_workers_history[i] << std::endl;
    std::cout << "==============================" << std::endl;
  }

  void service_time_history_to_csv(const std::string file_name){
    auto service_time_history = manager.getServiceTimeHistory();
    auto service_time_goal = manager.getServiceTimeGoal();
    auto active_workers_history = manager.getActiveWorkersHistory();

    auto emitter_elapsed_time_history = emitter.getElapsedTimeHistory();
    auto workers_elapsed_time_history = manager.getWorkersElapsedTimeHistory();
    auto collector_elapsed_time_history = collector.getElapsedTimeHistory();

    std::ofstream myfile;
    myfile.open("Statistics/" + file_name + ".csv");

    myfile << "max_nw,"
           << "active_workers_history,"
           << "service_time_history,"
           << "service_time_goal,"
           << "emitter_elapsed_time_history,"
           << "workers_elapsed_time_history,"
           << "collector_elapsed_time_history"
           << std::endl;

    for(size_t i = 0; i < service_time_history.size(); i++){
      myfile << max_nw << ","
             << active_workers_history[i] << ","
             << service_time_history[i].count() << ","
             << service_time_goal.count() << ","
             << emitter_elapsed_time_history[i].count() << ","
             << workers_elapsed_time_history[i].count() << ","
             << collector_elapsed_time_history[i].count()
             << std::endl;
    }
    myfile.close();
  }

  std::chrono::microseconds getCompletionTime(){
    return completion_time;
  }

  void printResults(){
    std::cout << "======= results =======" << std::endl;
    output_stream->empty_and_print();
    std::cout << "=======================" << std::endl;
  }



};


#endif // __AUTONOMIC_FARM_H__
