#include <iostream>
#include <vector>
#include "./autonomic_farm.h"

int main(int argc, const char* argv[]){

  AutonomicFarm farm(4);
  //farm.print_af();

  std::vector<int> tasks = {1, 2, 3};

  return 0;
}
