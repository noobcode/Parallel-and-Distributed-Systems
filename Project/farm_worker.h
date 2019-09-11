#ifndef __FARM_WORKER_H__
#define __FARM_WORKER_H__

#include <vector>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "./safe_queue.h"
#include "./farm_utility.h"
#include "./utimer.h"

enum WorkerStatus {INACTIVE, ACTIVE};

template <typename Tin, typename Tout> class FarmWorker{
private:
  unsigned int worker_id;
  bool concurrency_throttling;
  SafeQueue<int>* workers_requests; // unilateral channel from Workers to Emitter
  SafeQueue<Task<Tin>*>* task_queue; // unilateral channels from Emitter to each Worker
  SafeQueue<Task<Tout>*>* output_stream; // where all workers put the results
  SafeQueue<std::chrono::microseconds*>* latency_queue; // emitter to manager
  std::function<Tout(Tin)> f;
  WorkerStatus status_worker;

  std::thread* worker_thread;
  std::mutex* status_mutex;
  std::condition_variable* status_condition;

public:
  // constructor
  FarmWorker(unsigned int worker_id,
             bool concurrency_throttling,
             SafeQueue<int>* workers_requests,
             SafeQueue<Task<Tin>*>* task_queue,
             SafeQueue<Task<Tout>*>* output_stream,
             SafeQueue<std::chrono::microseconds*>* latency_queue,
             std::function<Tout(Tin)> f,
             WorkerStatus status_worker): worker_id(worker_id),
                                          concurrency_throttling(concurrency_throttling),
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
    std::chrono::microseconds elapsed_time;

    while(true){
      {
        std::unique_lock<std::mutex> lock(*status_mutex);
        status_condition->wait(lock, [=]{return status_worker == ACTIVE;});   // if holds, it goes through
      }

      tic = std::chrono::system_clock::now();
      // worker tells emitter that is ready
      workers_requests->safePush(worker_id);
      // worker waits for task
      Task<Tin>* task = task_queue->safePop();
      if(task->isEOS()) break;

      auto result = f(task->getData());

      // send result
      output_stream->safePush(new Task<Tout>(result));

      toc = std::chrono::system_clock::now();
      std::chrono::duration<double> elapsed = toc - tic;
      elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(elapsed);


      if(concurrency_throttling)
        latency_queue->safePush(new std::chrono::microseconds(elapsed_time));
    }
    sendEOS();
  }

  void run(){
    worker_thread = new std::thread(&FarmWorker::body, this);
  }

  void join(){
    worker_thread->join();
  }

  void sendEOS(){
    if(concurrency_throttling)
      latency_queue->safePush(new std::chrono::microseconds(-1));
    output_stream->safePush(Task<Tout>::EOS());
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
