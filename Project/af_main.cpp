#include <iostream>
#include <vector>
#include <chrono>
#include "./autonomic_farm.h"

int my_f(int x){
  long int sum = 0;
  for(int i = 0; i < 100*x; i++)
    sum += i*i + i;
  for(int i = 0; i < 100*x; i++)
    sum -= i*i;
  return sum;
}

int main(int argc, const char* argv[]){
  std::vector<int> tasks = {100, 100, 100, 100, 100, 500, 500, 1000, 1000, 50, 50, 50};

  size_t max_nw = 8;
  std::chrono::microseconds service_time_goal(100);
  unsigned int nw_initial = 2;

  AutonomicFarm farm(max_nw, my_f);
  //farm.printFarm();

  //farm.run_and_wait(tasks, nw_initial, service_time_goal);
  //farm.printResults();

  int accumulator = 0;
  float alpha = 0.9;
  std::vector<int> ts = {100, 100, 100, 100, 200, 200, 200, 200, 400, 400};
  std::cout << "new_value - running_avg" << std::endl;
  for(int i = 0; i < 10; i++){
    accumulator = (alpha * ts[i]) + (1.0 - alpha) * accumulator;
    std::cout << ts[i] << " " << accumulator << std::endl;
  }


  return 0;
}
