#include <iostream>
#include <vector>
#include "./autonomic_farm.h"

int my_f(int x){
  return x;
}

int main(int argc, const char* argv[]){
  std::vector<int> tasks = {98, 99, 100};

  size_t max_nw = 4;

  AutonomicFarm farm(max_nw, my_f);
  //farm.print_af();
  farm.run_and_wait(tasks);

  return 0;
}
