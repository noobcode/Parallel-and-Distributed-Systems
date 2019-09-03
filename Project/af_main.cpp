#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <math.h>
#include <cstdlib>
#include <fstream>
#include "./autonomic_farm.h"

float my_f(std::vector<int> v){
  float PI = 3.14159265;
  float sum = 0;
  for(size_t i = 0; i < v.size(); i++){
    sum += (int)pow(v[i], 2) % 1000 + sin(v[i]*2*PI) + cos(v[i]*2*PI) + sqrt(v[i]);
  }
  return sum;
}

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

  std::vector<std::vector<int>> data = generateCollectionOfVectors(n, l1, l2, l3);

  AutonomicFarm<std::vector<int>, float> farm(nw_max, my_f, alpha, concurrency_throttling);

  //farm.printFarm();

  farm.run_and_wait(data, nw_init, service_time_goal);
  auto completion_time = farm.getCompletionTime().count();
  std::cout << "completion_time: " << completion_time << std::endl;
  //farm.printResults();
  //farm.getServiceTimeHistory();
  //farm.getActiveWorkersHistory();
  farm.service_time_history_to_csv("service_time_trial");

  // experiment for scalability
  std::ofstream myfile;
  myfile.open("Statistics/completion_time_vs_nw.csv");
  myfile << "nw,completion_time" << std::endl;
  for(size_t i = 1; i <= nw_max; i++){
    AutonomicFarm<std::vector<int>, float> farm(i, my_f, alpha, false);
    farm.run_and_wait(data, i, service_time_goal);
    auto completion_time = farm.getCompletionTime().count();
    myfile << i << "," << completion_time << std::endl;
  }
  myfile.close();

  // experiment for speedup
  std::chrono::microseconds sequential_time;
  {
    utimer timer(&sequential_time);
    for(size_t i = 0; i < data.size(); i++)
      my_f(data[i]);
  }
  std::ofstream myfile_2;
  myfile_2.open("Statistics/sequential_time.csv");
  myfile_2 << "sequential_time" << std::endl
           << sequential_time.count() << std::endl;
  myfile_2.close();

  // experiment for average service time error vs alpha
  std::vector<float> alpha_values = {0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0};
  std::ofstream myfile_3;
  myfile_3.open("Statistics/service_time_history_vs_alpha.csv");
  myfile_3 << "alpha,service_time_goal,service_time_history" << std::endl;
  for(size_t i=0; i < alpha_values.size(); i++){
    AutonomicFarm<std::vector<int>, float> farm(nw_max, my_f, alpha_values[i], true);
    farm.run_and_wait(data, nw_init, service_time_goal);

    auto service_time_history = farm.getServiceTimeHistory();
    myfile_3 << alpha_values[i] << ","
             << service_time_goal.count() << ",[";
    for(size_t j=0; j< service_time_history.size(); j++)
      myfile_3 << service_time_history[j].count() << ",";
    myfile_3 << "]"  << std::endl;
  }
  myfile_3.close();

  return 0;
}
