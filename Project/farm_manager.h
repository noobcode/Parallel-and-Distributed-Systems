#ifndef __FARM_MANAGER_H__
#define __FARM_MANAGER_H__

#include <chrono>
#include <thread>
#include <cmath>
#include "./farm_worker.h"

class FarmManager{
private:
  float alpha;
  unsigned int max_nw;
  int last_active_worker; // index of the last active worker (-1 means all workers are disactive)
  std::vector<FarmWorker*>* workers;
  SafeQueue<std::chrono::microseconds*>* latency_queue;
  std::thread* manager_thread;

  // store statistics
  std::chrono::microseconds service_time_goal;
  std::vector<unsigned int> active_workers_history;
  std::vector<std::chrono::microseconds> service_time_history;
  std::vector<std::chrono::microseconds> workers_elapsed_time_history;

public:
  FarmManager(float alpha,
              unsigned int max_nw,
              std::vector<FarmWorker*>* workers,
              SafeQueue<std::chrono::microseconds*>* latency_queue) : alpha(alpha),
                                                                      max_nw(max_nw),
                                                                      last_active_worker(-1),
                                                                      workers(workers),
                                                                      latency_queue(latency_queue)
                                                                      {};

  void body(unsigned int nw_initial){
    unsigned int nw_new;
    unsigned int EOS_counter = 0;
    std::chrono::microseconds moving_avg_latency(0); // exponentially weighted moving average of latencies
    std::chrono::microseconds actual_service_time; // TODO o semplicemente fare tempo consegna_i - tempo_consegna_i-1

    // notify nw_initial workers
    for(size_t i = 0; i < nw_initial; i++)
      activateWorker();

    while(EOS_counter < max_nw){
      std::chrono::microseconds* latency_worker = latency_queue->safePop();

      if(latency_worker->count() == -1){
        EOS_counter++;
        activateAllWorkers(); // activate all workers if not already active
        continue; // go pop another execution time or EOS
      }

      auto tmp = alpha * *latency_worker + (1 - alpha) * moving_avg_latency;
      moving_avg_latency = std::chrono::duration_cast<std::chrono::microseconds>(tmp);
      actual_service_time = moving_avg_latency/(last_active_worker+1);

      std::cout << "ric_latenza/" << latency_worker->count() << " "
                << "expo_avg_latency/" << moving_avg_latency.count() << " "
                << "actual_service_time/" << actual_service_time.count() << " ";

      if(!EOS_counter){
        // don't need to update parallelism degree once tasks are finished
        nw_new = computeNumberOfRequiredWorkers(moving_avg_latency, service_time_goal);
        updateParallelismDegree(nw_new);
        std::cout<< "nw_new/" << nw_new << " LAW/" << last_active_worker << std::endl;
      }

      // store statistics
      service_time_history.push_back(actual_service_time);
      active_workers_history.push_back(last_active_worker+1);
      workers_elapsed_time_history.push_back(*latency_worker);
    }
  }

  void run(unsigned int nw_initial, std::chrono::microseconds service_time_goal){
    this->service_time_goal = service_time_goal;
    manager_thread = new std::thread(&FarmManager::body, this, nw_initial);
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
    for(size_t i = last_active_worker+1; i < max_nw; i++){
      activateWorker();
    }
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

  unsigned int computeNumberOfRequiredWorkers(std::chrono::microseconds moving_avg_latency,
                                      std::chrono::microseconds service_time_goal){
    float a = (float) moving_avg_latency.count();
    float b = (float) service_time_goal.count();
    unsigned int nw_new = round(a/b);

    nw_new = std::max((unsigned int) 1, std::min(nw_new, max_nw));
    return nw_new;
  }

  std::vector<std::chrono::microseconds> getServiceTimeHistory(){
    return service_time_history;
  }

  std::vector<unsigned int> getActiveWorkersHistory(){
    return active_workers_history;
  }

  std::chrono::microseconds getServiceTimeGoal(){
    return service_time_goal;
  }

  std::vector<std::chrono::microseconds> getWorkersElapsedTimeHistory(){
    return workers_elapsed_time_history;
  }

  void printManager(){
    std::cout << "==== MANAGER ====" << std::endl;
    std::cout << "max_nw:" << this->max_nw << std::endl;
  }

};

#endif // __FARM_MANAGER_H__
