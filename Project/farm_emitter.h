#ifndef __FARM_EMITTER_H__
#define __FARM_EMITTER_H__

#include <vector>
#include <iostream>
#include <thread>
#include "./safe_queue.h"
#include "./task.h"

//template <class T>
class FarmEmitter{
private:
  unsigned int max_nw;
  SafeQueue<int>* workers_requests; // unilateral channel from Workers to Emitter
  std::vector<SafeQueue<Task*>*>* task_queues; // unilateral channels from Emitter to each Worker
  std::thread* emitter_thread;

public:
  // constructors
  //FarmEmitter() = delete;
  FarmEmitter(int max_nw,
              SafeQueue<int>* workers_requests,
              std::vector<SafeQueue<Task*>*>* task_queues) : max_nw(max_nw),
                                                           workers_requests(workers_requests),
                                                           task_queues(task_queues)
  {
    std::cout << "creating emitter..." << std::endl;
  };

  // methods
  static void emitTasks(std::vector<int> data_stream, SafeQueue<int>* workers_requests, std::vector<SafeQueue<Task*>*>* task_queues){ // TODO template
    for(size_t i=0; i < data_stream.size(); i++){
      // receive data from input stream
      Task* task = new Task(data_stream[i]);
      //std::cout << task << std::endl;
      // receive disponibility from a worker
      int worker_id = workers_requests->safePop();
      // send data to worker
      // (dopo controlla anche che il worker sia disponibile, nw attivi potrebbe essere cambiato,
      // in caso continua a poppare e controllare finche non trovi uno disponibile e attivo)
      task_queues->at(worker_id)->safePush(task); // TODO task_queues is a vector of one-slot safe_queue
    }
  }

  static void sendEOS(unsigned int max_nw, std::vector<SafeQueue<Task*>*>* task_queues){
    for(size_t i = 0; i < max_nw; i++)
      task_queues->at(i)->safePush(Task::EOS());
  }

  static void emitterJob(std::vector<int> data, unsigned int max_nw, SafeQueue<int>* workers_requests, std::vector<SafeQueue<Task*>*>* task_queues){
    FarmEmitter::emitTasks(data, workers_requests, task_queues);
    FarmEmitter::sendEOS(max_nw, task_queues);
  }

  void run(std::vector<int> data){
    emitter_thread = new std::thread(FarmEmitter::emitterJob, data, max_nw, workers_requests, task_queues);
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
