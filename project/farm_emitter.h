#ifndef __FARM_EMITTER_H__
#define __FARM_EMITTER_H__

#include <vector>
#include <iostream>
#include <thread>
#include "./safe_queue.h"

//template <class T>
class FarmEmitter{
private:
  unsigned int max_nw;
  SafeQueue<int>* workers_requests; // unilateral channel from Workers to Emitter
  std::vector<SafeQueue<int>*>* task_queues; // unilateral channels from Emitter to each Worker
  std::thread emitter_thread;

public:
  // constructors
  //FarmEmitter() = delete;
  FarmEmitter(int max_nw,
              SafeQueue<int>* workers_requests,
              std::vector<SafeQueue<int>*>* task_queues) : max_nw(max_nw),
                                                           workers_requests(workers_requests),
                                                           task_queues(task_queues)
  {
    std::cout << "creating emitter..." << std::endl;
  };

  // methods
  static void emitTasks(std::vector<int> tasks, SafeQueue<int>* workers_requests){ // TODO template
    for(unsigned int i=0; i < tasks.size(); i++){
      // receive data from input stream
      int task = tasks[i];
      std::cout << task << std::endl;
      // receive disponibility from a worker
      //int worker_id = workers_requests->safePop();
      // send data to worker
      // (dopo controlla anche che il worker sia disponibile, nw attivi potrebbe essere cambiato,
      // in caso continua a poppare e controllare finche non trovi uno disponibile e attivo)
      //(*task_queues)[worker_id].safePush(task); // TODO task_queues is a vector of one-slot safe_queue
    }
  }

  static void sendEOS(){
    std::cout << "send EOS" << std::endl;
    // invia EOS
  }

  static void emitterJob(std::vector<int> tasks, SafeQueue<int>* workers_requests){
    FarmEmitter::emitTasks(tasks, workers_requests);
    FarmEmitter::sendEOS();
  }

  void run(std::vector<int> tasks){
    std::cout << "run emitter..." << std::endl;
    emitter_thread = std::thread(FarmEmitter::emitterJob, tasks, workers_requests);
  }

  void join(){
    std::cout << "joining emitter..." << std::endl;
    emitter_thread.join();
  }

  void printEmitter(){
    std::cout << "===== EMITTER ====" << std::endl;
    std::cout << "max_nw:" << max_nw << std::endl;


    std::cout << "farm workers_requests: " << workers_requests << " - " << workers_requests->maxSize() << std::endl;


    // inizialmente vuota, dim max = max_nw
    std::cout << "size and max size of W->E queue...";
    std::cout << workers_requests->safeSize() << std::endl << workers_requests->maxSize() << std::endl;

    std::cout << "task_queues size: " << task_queues->size() << std::endl;
    std::cout << "size and max size of E->W queue...\n";
    for(size_t i=0; i < task_queues->size(); i++)
      std::cout << (*task_queues)[i]->safeSize() << " - " << (*task_queues)[i]->maxSize() << std::endl;
  }

};
#endif // __FARM_EMITTER_H__
