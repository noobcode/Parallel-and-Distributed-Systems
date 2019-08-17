#include <iostream>
#include <vector>
#include "./autonomic_farm.h"

int main(int argc, const char* argv[]){
  std::vector<int> tasks = {98, 99, 100};

  AutonomicFarm farm(4);
  farm.print_af();
  //farm.run_and_wait(tasks);

  return 0;
}
