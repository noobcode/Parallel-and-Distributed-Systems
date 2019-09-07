#ifndef __FARM_EMITTER_H__
#define __FARM_EMITTER_H__

#include <vector>
#include <iostream>
#include <thread>
#include "./safe_queue.h"
#include "./farm_utility.h"
#include <chrono>
#include "utimer.h"

template <typename T> class FarmEmitter{
private:
  unsigned int max_nw;
  SafeQueue<int>* workers_requests; // unilateral channel from Workers to Emitter
  std::vector<SafeQueue<Task<T>*>*>* task_queues; // unilateral channels from Emitter to each Worker
  std::thread* emitter_thread;

  std::vector<std::chrono::microseconds> elapsed_time_history;

public:
  // constructors
  FarmEmitter(int max_nw,
              SafeQueue<int>* workers_requests,
              std::vector<SafeQueue<Task<T>*>*>* task_queues) : max_nw(max_nw),
                                                           workers_requests(workers_requests),
                                                           task_queues(task_queues) {};

  // methods
  void emitTasks(std::vector<T> data_stream){
    std::chrono::system_clock::time_point tic, toc;
    std::chrono::microseconds elapsed_time;

    size_t n = data_stream.size();
    elapsed_time_history.resize(n);

    for(size_t i = 0; i < n; i++){
      tic = std::chrono::system_clock::now();

      // receive data from input stream
      Task<T>* task = new Task<T>(data_stream[i]);
      // receive disponibility from a worker
      int worker_id = workers_requests->safePop();
      // send task to worker
      task_queues->at(worker_id)->safePush(task);

      toc = std::chrono::system_clock::now();
      std::chrono::duration<double> elapsed = toc - tic;
      elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(elapsed);
      // update internal statistics
      elapsed_time_history.at(i) = elapsed_time;
    }
  }

  void sendEOS(){
    for(size_t i = 0; i < max_nw; i++)
      task_queues->at(i)->safePush(Task<T>::EOS());
  }

  void body(std::vector<T> data){
    emitTasks(data);
    sendEOS();
  }

  void run(std::vector<T> data){
    emitter_thread = new std::thread(&FarmEmitter::body, this, data);
  }

  void join(){
    emitter_thread->join();
  }

  std::vector<std::chrono::microseconds> getElapsedTimeHistory(){
    return elapsed_time_history;
  }

  void printEmitter(){
    std::cout << "==== EMITTER ====" << std::endl;
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
  }

};
#endif // __FARM_EMITTER_H__
