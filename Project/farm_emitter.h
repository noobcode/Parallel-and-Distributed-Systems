#ifndef __FARM_EMITTER_H__
#define __FARM_EMITTER_H__

#include <vector>
#include <iostream>
#include <thread>
#include "./safe_queue.h"
#include "./farm_utility.h"

//template <class T>
class FarmEmitter{
private:
  unsigned int max_nw;
  SafeQueue<int>* workers_requests; // unilateral channel from Workers to Emitter
  std::vector<SafeQueue<Task*>*>* task_queues; // unilateral channels from Emitter to each Worker
  std::thread* emitter_thread;

public:
  // constructors
  FarmEmitter(int max_nw,
              SafeQueue<int>* workers_requests,
              std::vector<SafeQueue<Task*>*>* task_queues) : max_nw(max_nw),
                                                           workers_requests(workers_requests),
                                                           task_queues(task_queues) {};

  // methods
  void emitTasks(std::vector<int> data_stream){ // TODO template
    for(size_t i=0; i < data_stream.size(); i++){
      // receive data from input stream
      Task* task = new Task(data_stream[i]);
      // receive disponibility from a worker
      int worker_id = this->workers_requests->safePop();
      this->task_queues->at(worker_id)->safePush(task); // TODO task_queues is a vector of one-slot safe_queue
    }
  }

  void sendEOS(){
    for(size_t i = 0; i < this->max_nw; i++)
      this->task_queues->at(i)->safePush(Task::EOS());
  }

  void body(std::vector<int> data){
    emitTasks(data);
    sendEOS();
  }

  void run(std::vector<int> data){
    emitter_thread = new std::thread(&FarmEmitter::body, this, data);
  }

  void join(){
    emitter_thread->join();
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
