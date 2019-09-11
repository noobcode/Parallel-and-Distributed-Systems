#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <math.h>
#include <cstdlib>
#include <fstream>
#include "./autonomic_farm.h"

template <typename Tin, typename Tout>
std::chrono::microseconds computeSequentialTime(std::vector<Tin> data, std::function<Tout(Tin)> f){
  std::chrono::microseconds sequential_time;
  std::vector<Tout> results(data.size());
  {
    utimer timer(&sequential_time);
    for(size_t i = 0; i < data.size(); i++)
      results[i] = f(data[i]);
  }
  return sequential_time;
}

float my_f(std::vector<int> v){
  float PI = 3.14159265;
  float sum = 0;

  size_t n = v.size();
  for(size_t i = 0; i < n; i++){
    sum += (int)pow(v[i], 2) % 1000 + sin(v[i]*2*PI) + cos(v[i]*2*PI) + sqrt(v[i]);
  }
  return sum;
}

// 0 1 1 2 3 5 8 13 21
int fibonacci(int n){
  if(n <= 1) return n;
  return fibonacci(n-1) + fibonacci(n-2);
}

// time_to_wait (microseconds)
int busywait(size_t time_to_wait){
  size_t elapsed_time = 0;
  std::chrono::system_clock::time_point tic, toc;
  tic = std::chrono::system_clock::now();
  while(elapsed_time < time_to_wait){
    toc = std::chrono::system_clock::now();
    elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(toc - tic).count();
  }
  return 0;
}

/*
generates a collection of length 3n. The collection is divided in 3 partitions of length n.
The elements of the i-th partition are vectors of length l_i, (for i = 1,2,3).
*/
std::vector<std::vector<int>> generateCollectionOfVectors(size_t n, size_t l1, size_t l2, size_t l3){
  std::vector<std::vector<int>> data;

  auto add_partition = [&data, n](size_t l){
    for(size_t i = 0; i < n; i++){
      auto v = std::vector<int>(l);
      std::generate(v.begin(), v.end(), std::rand);
      data.push_back(v);
    }
  };

  add_partition(l1);
  add_partition(l2);
  add_partition(l3);

  return data;
}

std::vector<int> generateCollection(size_t n, int f1, int f2, int f3){
  std::vector<int> data;

  auto add_partition = [&data, n](int f){
    for(size_t i = 0; i < n; i++){
      data.push_back(f);
    }
  };

  add_partition(f1);
  add_partition(f2);
  add_partition(f3);

  return data;
}

int main(int argc, const char* argv[]){
  if(argc != 10){
    std::cout << "usage:" << std::endl
              << argv[0] << " nw_init nw_max ts_goal concurrency_throttling alpha n l1 l2 l3"
              << std::endl;
    return -1;
  }

  // parse arguments
  size_t nw_init = atoi(argv[1]);
  size_t nw_max = atoi(argv[2]);
  std::chrono::microseconds service_time_goal(atoi(argv[3]));
  bool concurrency_throttling = (bool)atoi(argv[4]);
  float alpha = atof(argv[5]);
  size_t n = atoi(argv[6]);
  size_t l1 = atoi(argv[7]);
  size_t l2 = atoi(argv[8]);
  size_t l3 = atoi(argv[9]);

  //std::vector<std::vector<int>> data = generateCollectionOfVectors(n, l1, l2, l3);
  //AutonomicFarm<std::vector<int>, float> farm(nw_max, my_f, alpha, concurrency_throttling);

  std::vector<int> data = generateCollection(n, l1, l2, l3);
  //AutonomicFarm<int, int> farm(nw_max, fibonacci, alpha, concurrency_throttling);
  AutonomicFarm<int, int> farm(nw_max, busywait, alpha, concurrency_throttling);

////////////////////////////////////////////////////////////////////////////////
// TODO normal run with concurrency throttling
// - service time service time history
// - active workers history
// - elapsed time history (emitter, workers, collector)
////////////////////////////////////////////////////////////////////////////////
  farm.run_and_wait(data, nw_init, service_time_goal);
  farm.service_time_history_to_csv("service_time_trial");

  auto completion_time = farm.getCompletionTime().count();
  std::cout << "completion_time: " << completion_time << std::endl;

///////////////////////////////////////////////////////////////////////////////
// TODO experiment for scalability
// - completion time T(1)...T(nw_max) - (no concurrency throttling)
//////////////////////////////////////////////////////////////////////////////
  std::cout << "running experiment for scalability" << std::endl;

  std::ofstream myfile;
  myfile.open("Statistics/completion_time_vs_nw.csv");
  myfile << "nw,completion_time" << std::endl;
  for(size_t i = 1; i <= nw_max; i++){
    //AutonomicFarm<std::vector<int>, float> farm(i, my_f, alpha, false);
    //AutonomicFarm<int, int> farm(i, fibonacci, alpha, false);
    AutonomicFarm<int, int> farm(i, busywait, alpha, false);
    farm.run_and_wait(data, i, service_time_goal);
    auto completion_time = farm.getCompletionTime().count();
    myfile << i << "," << completion_time << std::endl;
  }
  myfile.close();

////////////////////////////////////////////////////////////////////////////////
// TODO experiment for speedup
// - compute sequential time
// - T(1)...T(n) computed above
// ////////////////////////////////////////////////////////////////////////////
  std::cout << "running experiment for speedup" << std::endl;
  auto sequential_time = computeSequentialTime<int, int>(data, busywait);
  //auto sequential_time = computeSequentialTime<int, int>(data, fibonacci);
  //auto sequential_time = computeSequentialTime<std::vector<int>, int>(data, my_f);

  std::ofstream myfile_2;
  myfile_2.open("Statistics/sequential_time.csv");
  myfile_2 << "sequential_time" << std::endl
           << sequential_time.count() << std::endl;
  myfile_2.close();
  std::cout << "sequential time: " << sequential_time.count() << std::endl;

////////////////////////////////////////////////////////////////////////////////
// TODO experiment for average service time error vs alpha
// - service time history for each alpha
// - compute mean relative error
///////////////////////////////////////////////////////////////////////////////
  std::cout << "running experiment for alpha" << std::endl;
  std::vector<float> alpha_values = {0.0, 0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.35,
                                     0.4, 0.45, 0.5, 0.55, 0.6, 0.65, 0.7,
                                   0.75, 0.8, 0.85, 0.9, 0.95, 1.0};
  std::ofstream myfile_3;
  myfile_3.open("Statistics/service_time_history_vs_alpha.csv");
  //myfile_3 << "#alpha,service_time_goal,service_time_history" << std::endl;
  for(size_t i=0; i < alpha_values.size(); i++){
    std::cout << "-- alpha: " << alpha_values[i] << std::endl;
    //AutonomicFarm<std::vector<int>, float> farm(nw_max, my_f, alpha_values[i], true);
    //AutonomicFarm<int, int> farm(nw_max, fibonacci, alpha_values[i], true);
    AutonomicFarm<int, int> farm(nw_max, busywait, alpha_values[i], true);

    farm.run_and_wait(data, nw_init, service_time_goal);

    auto service_time_history = farm.getServiceTimeHistory();
    myfile_3 << alpha_values[i] << ","
             << service_time_goal.count();
    for(size_t j=0; j< service_time_history.size(); j++)
      myfile_3 << "," << service_time_history[j].count();
    myfile_3 << std::endl;
  }
  myfile_3.close();

///////////////////////////////////////////////////////////////////////////////
// TODO experiment max speedup and scalability vs task size (fixed collection size)
// - iterate task size (all three partition same size)
//  - iterate nw workers
//  - save completion_time(nw)
///////////////////////////////////////////////////////////////////////////////
  std::cout << "running experiment for task size" << std::endl;
  std::vector<size_t> task_size = {250, 500, 1000, 2000, 4000, 8000, 16000, 32000};

// sequential time vs task size
  std::ofstream myfile4;
  myfile4.open("Statistics/task_size_vs_sequential_time.csv");
  myfile4 << "task_size,sequential_time" << std::endl;
  for(size_t i = 0; i < task_size.size(); i++){
    size_t size = task_size[i];
    std::cout << "-- task size: " << size << std::endl;
    auto data = generateCollection(n, size, size, size);
    auto sequential_time = computeSequentialTime<int, int>(data, busywait);
    myfile4 << size << "," << sequential_time.count() << std::endl;
  }
  myfile4.close();

// nw vs completion_time for each task size
  for(size_t j = 0; j < task_size.size(); j++){
    size_t size = task_size[j];
    std::cout << "-- task size: " << size << std::endl;
    auto data = generateCollection(n, size, size, size);

    std::ofstream myfile5;
    myfile5.open("Statistics/TaskSize/completion_time_vs_nw_task_size_" + std::to_string(size) + ".csv");
    myfile5 << "nw,completion_time" << std::endl;
    for(size_t i = 1; i <= nw_max; i++){
      AutonomicFarm<int, int> farm(i, busywait, alpha, false);

      farm.run_and_wait(data, i, service_time_goal);
      auto completion_time = farm.getCompletionTime().count();
      myfile5 << i << "," << completion_time << std::endl;
    }
    myfile5.close();
  }

////////////////////////////////////////////////////////////////////////////////
// TODO experiment max speedup and scalability vs collection size (fixed task size)
// - iterate collection size n (so total collection size is 3n)
//  - iterate nw workers
//  - save completion_time(nw)
////////////////////////////////////////////////////////////////////////////////
std::cout << "runnning experiment for collection size" << std::endl;
std::vector<size_t> collection_size = {100, 200, 400, 800, 1600, 3200, 6400};

// sequential time vs collection size
std::ofstream myfile6;
myfile6.open("Statistics/collection_size_vs_sequential_time.csv");
myfile6 << "collection_size,sequential_time" << std::endl;
for(size_t i = 0; i < collection_size.size(); i++){
  size_t size = collection_size[i];
  std::cout << "-- collection size: " << size << std::endl;
  auto data = generateCollection(size, l1, l2, l3);
  auto sequential_time = computeSequentialTime<int, int>(data, busywait);
  myfile6 << (3*size) << "," << sequential_time.count() << std::endl;
}
myfile6.close();

// nw vs completion_time for each collection size
for(size_t j = 0; j < collection_size.size(); j++){
  size_t size = collection_size[j];
  std::cout << "-- collection size: " << size << std::endl;
  auto data = generateCollection(size, l1, l2, l3);

  std::ofstream myfile7;
  myfile7.open("Statistics/CollSize/completion_time_vs_nw_coll_size_" + std::to_string(3*size) + ".csv");
  myfile7 << "nw,completion_time" << std::endl;
  for(size_t i = 1; i <= nw_max; i++){
    AutonomicFarm<int, int> farm(i, busywait, alpha, false);

    farm.run_and_wait(data, i, service_time_goal);
    auto completion_time = farm.getCompletionTime().count();
    myfile7 << i << "," << completion_time << std::endl;
  }
  myfile7.close();
}

  return 0;
}
