#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
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

int maxInUnsortedArray(std::vector<int> v){
  int max = v[0];
  for(size_t i = 1; i < v.size(); i++){
    if(v[i] > max)
      max = v[i];
  }
  return max;
}

std::vector<std::vector<int>> generateCollectionOfVectors(size_t n, size_t l1, size_t l2, size_t l3){
  std::vector<std::vector<int>> data(n);
  for(size_t i = 0; i < n; i++){
    auto v = std::vector<int>(l1);
    std::generate(v.begin(), v.end(), std::rand);
    data.at(i) = v;
  }
  return data;
}


int main(int argc, const char* argv[]){
  //std::vector<int> tasks = {100, 100, 100, 100, 100, 500, 500, 500, 1000, 1000, 50, 50, 50};
  //std::vector<std::string> data = {"casa", "casa", "casa", "casa", "casa",
  //                                "poltronaccia", "poltronaccia", "poltronaccia", "poltronaccia",
  //                                "tempodiservizio",  "tempodiservizio", "tempodiservizio", "tempodiservizio"};

   std::vector<std::vector<int>> data = generateCollectionOfVectors(50, 1000, 500, 200);

  size_t max_nw = 8;
  std::chrono::microseconds service_time_goal(100);
  unsigned int nw_initial = 2;

  //AutonomicFarm<std::string, int> farm(max_nw, len);
  AutonomicFarm<std::vector<int>, int> farm(max_nw, maxInUnsortedArray);

  //farm.printFarm();

  farm.run_and_wait(data, nw_initial, service_time_goal);
  farm.stampCompletionTime();
  farm.printResults();
  //farm.getServiceTimeHistory();
  //farm.getActiveWorkersHistory();
  farm.service_time_history_to_csv("service_time_trial");

  return 0;
}
