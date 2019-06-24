#include <iostream>
#include <ff/ff.hpp>
#include <ff/pipeline.hpp>
#include <ff/farm.hpp>
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

struct firstStage: ff_node_t<int, int> {
    firstStage(const size_t n_tasks):n_tasks(n_tasks) {}
    int* svc(int*) {
        for(size_t i=0; i<n_tasks; ++i) {
            ff_send_out(new int(std::rand() % 10000));
        }
        return EOS;
    }
    const size_t n_tasks;
};

struct secondStage: ff_node_t<int, std::pair<int, int>> {
    std::pair<int, int>* svc(int * task) {
        int &t = *task;
        int result = number_of_primes_up_to(t);
        return new std::pair<int, int>(t, result);
    }
};

struct thirdStage: ff_node_t<std::pair<int,int>, int> {
    int* svc(std::pair<int,int>* task) {
        results.push_back(task);
        return GO_ON;
    }

    void svc_end() {
      std::cout << "collector ends" << std::endl;
      for(int i=0; i < results.size(); i++)
        std::cout << "(x, n_primes) = " << results[i]->first << ", " << results[i]->second << std::endl;
    }

    std::vector<std::pair<int, int>*> results;
};

int main(int argc, char *argv[]) {
    if (argc<3) {
        std::cerr << "use: " << argv[0]  << " nworkers stream-length\n";
        return -1;
    }
    const size_t nworkers = std::stol(argv[1]);
    firstStage  first(std::stol(argv[2]));
    thirdStage  third;

    srand(1); // seed

    std::vector<std::unique_ptr<ff_node> > W;
    for(size_t i=0;i<nworkers;++i) W.push_back(make_unique<secondStage>());
    ff_Farm<float> farm(std::move(W), first, third);

    ffTime(START_TIME);
    if (farm.run_and_wait_end()<0) {
        error("running farm");
        return -1;
    }
    ffTime(STOP_TIME);
    std::cout << "Time: " << ffTime(GET_TIME) << "\n";
    return 0;
}
