//#pragma once

//The following is the #include header guard.
#ifndef __FARM_EMITTER_H__
#define __FARM_EMITTER_H__

#include <vector>
#include <iostream>
#include "./safe_queue.h"

//template <class T>
class FarmEmitter{
private:
  int max_nw;
  SafeQueue<int> workers_requests; // unilateral channel from Workers to Emitter
  std::vector<SafeQueue<int>> task_queues; // unilateral channels from Emitter to each Worker

public:
  // constructors
  FarmEmitter() = delete;
  FarmEmitter(int max_nw, SafeQueue<int> workers_requests,
              std::vector<SafeQueue<int>> task_queues) : max_nw(max_nw),
                                                          workers_requests(workers_requests),
                                                          task_queues(task_queues){
    std::cout << "Emitter created." << std::endl;
    // inizialmente vuota, dim max = max_nw
    std::cout << "size and max size of W->E queue...\n";
    std::cout << workers_requests->safeSize() << std::endl << workers_requests->maxSize() << std::endl;

    //std::cout << "max size of E->W queue...\n";
    //for(int i=0; i < task_queues->size(); i++)
    //  std::cout << (*task_queues)[i].maxSize() << std::endl;
  };

  // methods
  /*
  void emitTasks(std::vector<int> tasks){ // TODO template
    for(unsigned int i=0; i < tasks.size(); i++){
      // receive data from input stream
      int task = tasks[i];
      // receive disponibility from a worker
      int worker_id = workers_requests->safePop();
      // send data to worker
      // (dopo controlla anche che il worker sia disponibile, nw attivi potrebbe essere cambiato,
      // in caso continua a poppare e controllare finche non trovi uno disponibile e attivo)
      (*task_queues)[worker_id].safePush(task); // TODO task_queues is a vector of one-slot safe_queue
    }
  }
  */

  void end(){
    // invia EOS
  }

};
#endif // __FARM_EMITTER_H__
