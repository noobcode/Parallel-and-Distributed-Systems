#include <thread>
#include <iostream>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <utility>
#include "./safe_queue.h"

SafeQueue<std::pair<int, int>> results;

int findItem(std::vector<std::pair<int, int>> local_results, int item){
  for(int i=0; i<local_results.size();i++){
    if(local_results[i].first == item){
      return i;
    }
  }
  return -1;
}

void count_numbers_map(std::vector<int> data, int startIdx, int endIdx){
  std::vector<std::pair<int, int>> local_results;

  for(int i = startIdx; i <= endIdx; i++){
    int item = data[i];
    int index = findItem(local_results, item);
    if(index==-1){
      local_results.push_back(std::make_pair(item,1));
    }
    else{
      local_results[index].second++;
    }
  }

  // push local results to reducers queue
  for(auto i: local_results)
    results.safe_push(i);
}

/*
void count_numbers_reduce(SafeQueue* queue){
  while(true){
    auto pair = queue.safe_pop();
    // check if pair is dummy pair
    // do something
    // push to result std::vector
  }
}
*/

int main(int argc, char* argv[]){

  int nw = atoi(argv[1]);
  std::vector<std::thread> mappers(nw);

  std::vector<int> data = {3, 3, 4, 6, 2, 4, 6, 6, 5};

  int dim = data.size() / nw;
  // start workers
  for(int i = 0; i < nw; i++){
    int startIdx = i * dim;
    int endIdx = i == nw-1 ? data.size()-1 : (i+1) * dim-1;
    mappers[i] = std::thread(count_numbers_map, data, startIdx,  endIdx);
  }

  // join
  for(int i=0;i<nw;i++)
    mappers[i].join();

  //std::vector<std::pair<int, int>> data_mapped = count_numbers_map(data, 0, data.size()-1);

  while(!results.isEmpty()){
    auto item = results.safe_pop();
    std::cout << item.first << "," << item.second << std::endl;
  }


  return 0;
}
