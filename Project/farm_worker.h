#ifndef __FARM_WORKER_H__
#define __FARM_WORKER_H__

#include <vector>
#include <iostream>
#include <thread>
#include "./safe_queue.h"
#include "./task.h"

class FarmWorker{
private:
  unsigned int worker_id;
  SafeQueue<int>* workers_requests; // unilateral channel from Workers to Emitter
  SafeQueue<Task*>* task_queue; // unilateral channels from Emitter to each Worker
  SafeQueue<Task*>* output_stream; // where all workers put the results
  std::function<int(int)> f;
  std::thread* worker_thread;

public:
  // constructor
  FarmWorker(unsigned int worker_id,
             SafeQueue<int>* workers_requests,
             SafeQueue<Task*>* task_queue,
             SafeQueue<Task*>* output_stream,
             std::function<int(int)> f): worker_id(worker_id),
                                         workers_requests(workers_requests),
                                         task_queue(task_queue),
                                         output_stream(output_stream),
                                         f(f)
  {
    std::cout << "creating worker..." << std::endl;
  };


  void run(){
    worker_thread = new std::thread(body, f, worker_id, workers_requests, task_queue, output_stream);
  }

  static void body(std::function<int(int)> f,
                   unsigned int worker_id,
                   SafeQueue<int>* workers_requests,
                   SafeQueue<Task*>* task_queue,
                   SafeQueue<Task*>* output_stream){
    while(true){
      // worker tells emitter that is ready
      workers_requests->safePush(worker_id);
      // worker waits for task
      Task* task = task_queue->safePop();
      if(task->isEOS()) break;

      auto result = f(task->getData());
      output_stream->safePush(new Task(result));
    }
    output_stream->safePush(Task::EOS());
  }

  void join(){
    worker_thread->join();
  }

void setWorkerId(unsigned int id){
  worker_id = id;
}

void setTaskQueue(SafeQueue<Task*>* queue){
  task_queue = queue;
}

void printWorker(){
  std::cout << "==== WORKER " << worker_id << " ====" << std::endl;

  std::cout << "(Workers->Emitter) workers_requests: address/"
            << workers_requests << " - size/"
            << workers_requests->safeSize() <<  " - max_size/"
            << workers_requests->maxSize() << std::endl;

  std::cout << "(Emitter->Worker) task_queue: address/"
            << task_queue << " - size/"
            << task_queue->safeSize() << "- max_size/"
            << task_queue->maxSize() << std::endl;

  std::cout << "(Workers->Collector) output_stream: address/" << output_stream
            << " - size/" << output_stream->safeSize()
            << " - max_size/" << output_stream->maxSize() << std::endl;
}


};










#endif // __FARM_WORKER_H__
