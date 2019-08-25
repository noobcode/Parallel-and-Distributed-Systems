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

  farm.run_and_wait(tasks, nw_initial, service_time_goal);
  farm.printResults();

  return 0;
}
