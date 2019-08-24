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
  std::chrono::milliseconds service_time_goal;
  std::thread* manager_thread;

public:
  FarmManager(unsigned int max_nw,
              std::vector<FarmWorker*>* workers) : max_nw(max_nw),
                                                   last_active_worker(-1),
                                                   workers(workers) {};

  static void body(unsigned int nw_initial,
                   std::chrono::milliseconds service_time_goal){
    // inizio
    //   imposta service_time_goal
    //   devo attivare i thread che stanno aspettando sulla variabile di condizione
    //   quanti ne attivo? solo nw_initial, cambia last_active_worker
    //
    //   durante
    //   while(i task non sono finiti, me lo dice il collector)
    //    ricevo tempo di esecuzione dello worker/task
    //    applico la politica che mi dice il nuovo valore di nw, nw_new
    //    poi faccio un ciclo for in cui chiamo activate() o disactive() tot volte,
  }

  void run(){
    manager_thread = new std::thread(body, ...);
  }

  void join(){
    manager_thread->join();
  }

  void activate_worker(){
    if(last_active_worker < max_nw - 1){
      last_active_worker++;
      workers->at(last_active_worker)->activate();
    }
  }

  void disactivate_worker(){
    if(last_active_worker > -1){
      workers->at(last_active_worker)->disactivate();
      last_active_worker--;
    }
  }

};

#endif // __FARM_MANAGER_H__
