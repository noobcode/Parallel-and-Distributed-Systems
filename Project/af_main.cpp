#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <math.h>
#include "./autonomic_farm.h"

int my_f(int x){
  long int sum = 0;
  for(int i = 0; i < 100*x; i++)
    sum += i*i + i;
  for(int i = 0; i < 100*x; i++)
    sum -= i*i;
  return sum;
}

int len(std::string x){
  return x.length();
}

float maxInUnsortedArray(std::vector<int> v){
  float PI = 3.14159265;
  float sum = 0;
  for(size_t i = 0; i < v.size(); i++){
    sum += (int)pow(v[i], 2) % 1000 + sin(v[i]*2*PI) + cos(v[i]*2*PI) + sqrt(v[i]);
  }
  return sum;
}

std::vector<std::vector<int>> generateCollectionOfVectors(size_t n, size_t l1, size_t l2, size_t l3){
  std::vector<std::vector<int>> data;

  for(size_t i = 0; i < n; i++){
    auto v = std::vector<int>(l1);
    std::generate(v.begin(), v.end(), std::rand);
    data.push_back(v);
  }

  for(size_t i = 0; i < n; i++){
    auto v = std::vector<int>(l2);
    std::generate(v.begin(), v.end(), std::rand);
    data.push_back(v);
  }

  for(size_t i = 0; i < n; i++){
    auto v = std::vector<int>(l3);
    std::generate(v.begin(), v.end(), std::rand);
    data.push_back(v);
  }

  return data;
}


int main(int argc, const char* argv[]){
  //std::vector<int> tasks = {100, 100, 100, 100, 100, 500, 500, 500, 1000, 1000, 50, 50, 50};
  //std::vector<std::string> data = {"casa", "casa", "casa", "casa", "casa",
  //                                "poltronaccia", "poltronaccia", "poltronaccia", "poltronaccia",
  //                                "tempodiservizio",  "tempodiservizio", "tempodiservizio", "tempodiservizio"};

   std::vector<std::vector<int>> data = generateCollectionOfVectors(100, 1000, 8000, 4000);

  size_t max_nw = 8;
  std::chrono::microseconds service_time_goal(100);
  unsigned int nw_initial = 2;

  //AutonomicFarm<std::string, int> farm(max_nw, len);
  AutonomicFarm<std::vector<int>, float> farm(max_nw, maxInUnsortedArray);

  //farm.printFarm();

  farm.run_and_wait(data, nw_initial, service_time_goal);
  farm.stampCompletionTime();
  farm.printResults();
  //farm.getServiceTimeHistory();
  //farm.getActiveWorkersHistory();
  farm.service_time_history_to_csv("service_time_trial");

  return 0;
}
