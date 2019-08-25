#ifndef __FARM_MANAGER_H__
#define __FARM_MANAGER_H__

#include <chrono>
#include <thread>
#include "./farm_worker.h"

class FarmManager{
private:
  unsigned int max_nw;
  int last_active_worker; // index of the last active worker (-1 means all workers are disactive)
  std::vector<FarmWorker*>* workers;
  SafeQueue<std::chrono::microseconds*>* latency_queue;
  std::thread* manager_thread;

public:
  FarmManager(unsigned int max_nw,
              std::vector<FarmWorker*>* workers,
              SafeQueue<std::chrono::microseconds*>* latency_queue) : max_nw(max_nw),
                                                                      last_active_worker(-1),
                                                                      workers(workers),
                                                                      latency_queue(latency_queue) {};

  void body(unsigned int nw_initial, std::chrono::microseconds service_time_goal){
    // notify nw_initial workers
    for(size_t i = 0; i < nw_initial; i++)
      this->activateWorker();

    unsigned int window_size = 2;
    unsigned int EOS_counter = 0;
    unsigned int window_counter = 0;
    std::chrono::microseconds window_latency(0); // sums up latencies over window
    std::chrono::microseconds running_avg_latency; // average latency over window
    // TODO also save std::chrono::microseconds actual_service_time

    while(EOS_counter < this->max_nw){
      std::chrono::microseconds* latency_worker = this->latency_queue->safePop();

      if(latency_worker->count() == -1){
        EOS_counter++;
        activateAllWorkers(); // when you receive the first EOS from a worker it means that the emiter is done
                              // TODO: activate all only once, may put an if inside the function or take care of it before
        continue; // go pop another execution time or EOS
      }

      window_counter++;
      window_latency += *latency_worker;
      running_avg_latency = window_latency/window_counter;

      std::cout << "finestra/" << window_counter << " "
                << "ric_latenza/" << latency_worker->count() << " "
                << "running_avg_latency/" << running_avg_latency.count() << " ";

      if(window_counter == window_size){
        window_latency = std::chrono::microseconds(0);
        window_counter = 0;
      }

      unsigned int tmp = running_avg_latency/service_time_goal;
      unsigned int one = 1;
      unsigned int nw_new = std::max(one, std::min(tmp, max_nw));
      updateParallelismDegree(nw_new);

      std::cout<< "nw_new/" << nw_new << " LAW/" << last_active_worker << std::endl;
    }
  }

  void run(unsigned int nw_initial, std::chrono::microseconds service_time_goal){
    manager_thread = new std::thread(&FarmManager::body, this, nw_initial, service_time_goal);
  }

  void join(){
    manager_thread->join();
  }

  void activateWorker(){
    if(last_active_worker < (int)max_nw - 1){
      last_active_worker++;
      workers->at(last_active_worker)->activate();
    }
  }

  void disactivateWorker(){
    if(last_active_worker > -1){
      workers->at(last_active_worker)->disactivate();
      last_active_worker--;
    }
  }

  void activateAllWorkers(){
    // activate all workers so that they can read EOS and terminate
    for(size_t i = last_active_worker+1; i < max_nw; i++)
      activateWorker();
  }

  void updateParallelismDegree(unsigned int nw_new){
    int nw_diff = nw_new - (last_active_worker+1);
    if(nw_diff > 0){
      for(int i = 0; i < nw_diff; i++)
        activateWorker();
    } else if (nw_diff < 0){
      for(int i = 0; i < abs(nw_diff); i++)
        disactivateWorker();
    }
  }

  void printManager(){
    std::cout << "==== MANAGER ====" << std::endl;
    std::cout << "max_nw:" << this->max_nw << std::endl;
  }

};

#endif // __FARM_MANAGER_H__
