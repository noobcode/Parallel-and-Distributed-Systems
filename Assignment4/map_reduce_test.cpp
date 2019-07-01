#include "./map_reduce.h"
#include <iostream>



int main(int argc, char* argv[]){
  MapReduce mr(5,3);
  mr.print_results();
  return 0;
}
