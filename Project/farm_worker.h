#ifndef __FARM_WORKER_H__
#define __FARM_WORKER_H__

#include <vector>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "./safe_queue.h"
#include "./farm_utility.h"

enum WorkerStatus {INACTIVE, ACTIVE};

class FarmWorker{
private:
  unsigned int worker_id;
  SafeQueue<int>* workers_requests; // unilateral channel from Workers to Emitter
  SafeQueue<Task*>* task_queue; // unilateral channels from Emitter to each Worker
  SafeQueue<Task*>* output_stream; // where all workers put the results
  SafeQueue<std::chrono::microseconds*>* latency_queue;
  std::function<int(int)> f;
  WorkerStatus status_worker;

  std::thread* worker_thread;
  std::mutex* status_mutex;
  std::condition_variable* status_condition;

public:
  // constructor
  FarmWorker(unsigned int worker_id,
             SafeQueue<int>* workers_requests,
             SafeQueue<Task*>* task_queue,
             SafeQueue<Task*>* output_stream,
             SafeQueue<std::chrono::microseconds*>* latency_queue,
             std::function<int(int)> f,
             WorkerStatus status_worker): worker_id(worker_id),
                                          workers_requests(workers_requests),
                                          task_queue(task_queue),
                                          output_stream(output_stream),
                                          latency_queue(latency_queue),
                                          f(f),
                                          status_worker(status_worker)
  {
    status_mutex = new std::mutex();
    status_condition = new std::condition_variable();
  };

  void body(){
    std::chrono::system_clock::time_point tic, toc;
    while(true){
      {
        std::unique_lock<std::mutex> lock(*status_mutex);
        status_condition->wait(lock, [=]{return status_worker == ACTIVE;});   // if holds, it goes through
      }
      // worker tells emitter that is ready
      workers_requests->safePush(worker_id);
      // worker waits for task
      Task* task = task_queue->safePop();
      if(task->isEOS()) break;

      tic = std::chrono::system_clock::now();
      auto result = f(task->getData());
      toc = std::chrono::system_clock::now();
      auto elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(toc - tic);

      output_stream->safePush(new Task(result));
      latency_queue->safePush(new std::chrono::microseconds(elapsed_time));
    }
    latency_queue->safePush(new std::chrono::microseconds(-1));
    output_stream->safePush(Task::EOS());
  }

  void run(){
    worker_thread = new std::thread(&FarmWorker::body, this);
  }

  void join(){
    worker_thread->join();
  }

  void activate(){
    std::unique_lock<std::mutex> lock(*status_mutex);
    status_worker = ACTIVE;
    status_condition->notify_one();
  }

  void disactivate(){
    std::unique_lock<std::mutex> lock(*status_mutex);
    status_worker = INACTIVE;
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
