#include <thread>
#include <iostream>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <utility>
#include "./safe_queue.h"
#include <unistd.h>

SafeQueue<std::pair<int, int>> global_results;

int findItem(std::vector<std::pair<int, int>> local_results, int item){
  for(int i=0; i<local_results.size();i++){
    if(local_results[i].first == item){
      return i;
    }
  }
  return -1;
}

void increment_or_insert_item(std::vector<std::pair<int, int>> local_results, int item){
  int index = findItem(local_results, item);
  if(index==-1){
    local_results.push_back(std::make_pair(item,1));
  }
  else{
    local_results[index].second++;
  }
}

void count_numbers_map(std::vector<int> data, int startIdx, int endIdx,
  std::vector<SafeQueue<std::pair<int, int>>*> results){

  std::vector<std::pair<int, int>> local_results;

  for(int i = startIdx; i <= endIdx; i++){
    int item = data[i];
    //increment_or_insert_item(local_results, item);
    int index = findItem(local_results, item);
    if(index==-1){
      local_results.push_back(std::make_pair(item,1));
    }
    else{
      local_results[index].second++;
    }
  }


  // push local results to reducers queue
  int nw_reducers = results.size();
  for(auto i: local_results)
    results[i.first%nw_reducers]->safe_push(i);
}

void count_numbers_reduce(SafeQueue<std::pair<int,int>>* queue){
  std::vector<std::pair<int, int>> local_results;

  while(true){
    auto pair = queue->safe_pop();
    if (pair.first == -1 && pair.second == -1)
      break;

    //increment_or_insert_item(local_results, pair.first);
    int index = findItem(local_results, pair.first);
    if(index==-1){
      local_results.push_back(std::make_pair(pair.first,1));
    }
    else{
      local_results[index].second++;
    }
  }

  // push to global results
  for(auto i: local_results)
  //  std::cout << local_results.size() << i.first << std::endl;
    global_results.safe_push(i);
}


int main(int argc, char* argv[]){
  int nw_mapper = atoi(argv[1]);
  int nw_reducer = atoi(argv[2]);

  std::vector<std::thread> mappers(nw_mapper);
  std::vector<std::thread> reducers(nw_reducer);

  std::vector<SafeQueue<std::pair<int, int>>*> results(nw_reducer);
  for(int i = 0; i < nw_reducer; i++)
    results[i] = new SafeQueue<std::pair<int, int>>;

  std::vector<int> data = {3, 3, 4, 6, 2, 4, 6, 6, 5};

  int dim = data.size() / nw_mapper;
  // start mappers
  for(int i = 0; i < nw_mapper; i++){
    int startIdx = i * dim;
    int endIdx = i == nw_mapper-1 ? data.size()-1 : (i+1) * dim-1;
    mappers[i] = std::thread(count_numbers_map, data, startIdx,  endIdx, results);
  }



  //start reducers
  for(int i = 0; i < nw_reducer; i++)
    reducers[i] = std::thread(count_numbers_reduce, results[i]);

  // send termination signal to reducers
  for(int i = 0; i < nw_reducer; i++)
    results[i]->safe_push(std::make_pair(-1,-1));

  // join mappers and reducers
  for(int i=0;i<nw_mapper;i++)
    mappers[i].join();
  for(int i=0;i<nw_reducer;i++)
    reducers[i].join();

  //std::vector<std::pair<int, int>> data_mapped = count_numbers_map(data, 0, data.size()-1);

  while(!global_results.isEmpty()){
    auto item = global_results.safe_pop();
    std::cout << item.first << "," << item.second << std::endl;
  }


  return 0;
}
