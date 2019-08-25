#include <iostream>
#include <vector>
#include <chrono>
#include "./autonomic_farm.h"

int my_f(int x){
  return x;
}

int main(int argc, const char* argv[]){
  std::vector<int> tasks = {97, 98, 99, 100, 101, 102};

  size_t max_nw = 4;
  std::chrono::milliseconds service_time_goal(1000);
  unsigned int nw_initial = 2;

  AutonomicFarm farm(max_nw, my_f);
  //farm.printFarm();

  farm.run_and_wait(tasks, nw_initial, service_time_goal);
  farm.printResults();

  return 0;
}
