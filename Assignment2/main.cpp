#include<thread>
#include<unistd.h>
#include "./safe_queue.h"
#include <cmath>
#include <iostream>

#define EOS -1

bool is_prime(int x){
  if(x <= 3)
    return true;

  int limit = sqrt(x);
  for(int i = 2; i <= limit; i++){
    if(x % i == 0)
      return false;
  }

  return true;
}

int number_of_primes_up_to(int x){
  int count = 0;
  for(int i = 1; i <= x; i++){
    if(is_prime(i)){
      count++;
    }
  }
  return count;
}

void farm_worker(SafeQueue<int>* taskQueue, SafeQueue<int>* resultQueue){
  int v;
  while((v = taskQueue->safe_pop()) != EOS){
    int result = number_of_primes_up_to(v);
    resultQueue->safe_push(result);
  }
  resultQueue->safe_push(EOS);
}

void farm_emitter(SafeQueue<int>* taskQueue, int n_tasks, int n_workers){
  // insert tasks in queue
  for(int i = 0; i < n_tasks; i++){
    int x = std::rand() % 10000;
    taskQueue->safe_push(i); // TODO change
  }
  // send EOF to each worker
  for(int i = 0; i < n_workers; i++){
    taskQueue->safe_push(EOS);
  }
}

void farm_collector(SafeQueue<int>* resultQueue, int n_workers){
  int n_workers_done = 0;
  while(n_workers_done < n_workers){
    int v = resultQueue->safe_pop();
    if(v == EOS){
      n_workers_done++;
    } else {
      std::cout << v << " ";
    }
  }
  std::cout << std::endl;
}

int main(int argc, const char* argv[]){
  int n_tasks = atoi(argv[1]);
  int n_workers = atoi(argv[2]);

  SafeQueue<int> taskQueue;
  SafeQueue<int> resultQueue;
  std::vector<std::thread> worker_threads;

  // launch threads
  std::thread emitter_thread(farm_emitter, &taskQueue, n_tasks, n_workers);
  for(int i = 0; i < n_workers; i++){
    worker_threads.push_back(std::thread(farm_worker, &taskQueue, &resultQueue));
  }
  std::thread collector_thread(farm_collector, &resultQueue, n_workers);

  // wait threads termination
  emitter_thread.join();
  for (auto& t : worker_threads)
      t.join();
  collector_thread.join();


  return 0;
}
