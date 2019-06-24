#include <iostream>
#include <ff/ff.hpp>
#include <ff/parallel_for.hpp>
using namespace ff;

bool is_prime(int x){
  if(x <= 3)
    return true;

  int limit = sqrt(x);
  for(int i = 2; i <= limit; i++){
    if(x % i == 0)
      return false;
  }

  return true;
}

int number_of_primes_up_to(int x){
  int count = 0;
  for(int i = 1; i <= x; i++){
    if(is_prime(i)){
      count++;
    }
  }
  return count;
}

int main(int argc, char* argv[]){
  if (argc < 4) {
    std::cerr << "use: " << argv[0]  << " nworkers stream-length chunksize\n";
    return -1;
  }

  const size_t nworkers = std::stol(argv[1]);
  const size_t n = std::stol(argv[2]);
  const size_t chunksize = std::stol(argv[3]);

  srand(1); // seed
  std::vector<int> tasks;
  for(int i = 0; i<n; i++)
    tasks.push_back(std::rand() % 10000);

  ParallelFor pf(nworkers);
  // parallel_for(first-index, last-index, stepsize, chunksize, bodyFunction, nworkers);

  ffTime(START_TIME);
  pf.parallel_for(0, n, 1, chunksize,
    [tasks](int i){
      int result = number_of_primes_up_to(tasks[i]);
      std::cout << "(x, n_primes) = " << tasks[i] << ", " << result << std::endl;
      return;}, nworkers);
  ffTime(STOP_TIME);
  std::cout << "Time: " << ffTime(GET_TIME) << "\n";

  return 0;
}
