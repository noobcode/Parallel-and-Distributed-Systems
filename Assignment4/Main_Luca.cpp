#include <unistd.h>
#include <iostream>
#include <utility>
#include <vector>
#include "./map_reduce_Luca.h"

//SafeQueue<std::pair<int, int>> global_results;

int findItem(std::vector<std::pair<int, int>> local_results, int item) {
    for (int i = 0; i < local_results.size(); i++) {
        if (local_results[i].first == item) {
            return i;
        }
    }
    return -1;
}

void increment_or_insert_item(std::vector<std::pair<int, int>> local_results, int item) {
    int index = findItem(local_results, item);
    if (index == -1) {
        local_results.push_back(std::make_pair(item, 1));
    } else {
        local_results[index].second++;
    }
}

template<typename T>
std::pair<T,int> count_numbers_map(T item){
	return std::make_pair(item, 1);
}

  //std::pair<int, int> count_numbers_map(int item, std::vector<SafeQueue<std::pair<int, int>>*> results) {

    //std::vector<std::pair<int, int>> local_results;
    //for (int i = startIdx; i <= endIdx; i++) {
        //int item = data[i];
        //increment_or_insert_item(local_results, item);
        //int index = findItem(results, item);
        //return std::make_pair(item, 1);
        //if (index == -1) {
          //  results.push_back(std::make_pair(item, 1));
      //  } else {
        //    results[index].second++;
        //}
    //}

    // push local results to reducers queue
    //int nw_reducers = results.size();
    //for (auto i : local_results)
    //    results[i.first % nw_reducers]->safe_push(i);
//}

std::vector<std::pair<int, int>> count_numbers_reduce(SafeQueue<std::pair<int, int>>* queue) {
    std::vector<std::pair<int, int>> local_results;

    while (true) {
        auto pair = queue->safe_pop();
        if (pair.first == -1 && pair.second == -1)
            break;

        //increment_or_insert_item(local_results, pair.first);
        int index = findItem(local_results, pair.first);
        if (index == -1) {
            local_results.push_back(std::make_pair(pair.first, 1));
        } else {
            local_results[index].second++;
        }
    }

    return local_results;
}

int main(int argc, char* argv[]) {
    int nw_mapper = atoi(argv[1]);
    int nw_reducer = atoi(argv[2]);

    std::vector<int> data = {2, 3, 3, 4, 6, 2, 4, 6, 6, 5, 6, 1, 1, 1, 1, 1};
    for(auto i:data)
      std::cout << i << " ";
    std::cout << std::endl;

    MapReduce<int> mr(nw_mapper, nw_reducer);
    mr.map_and_reduce(data, count_numbers_map<int>);
    mr.print_results();

    //std::vector<std::thread> mappers(nw_mapper);
    //std::vector<std::thread> reducers(nw_reducer);

    //std::vector<SafeQueue<std::pair<int, int>>*> results(nw_reducer);
    //for(int i = 0; i < nw_reducer; i++)
    //  results[i] = new SafeQueue<std::pair<int, int>>;

    //int dim = data.size() / nw_mapper;
    // start mappers
    //for(int i = 0; i < nw_mapper; i++){
    //int startIdx = i * dim;
    //int endIdx = i == nw_mapper-1 ? data.size()-1 : (i+1) * dim-1;
    //  mappers[i] = std::thread(count_numbers_map, data, startIdx,  endIdx, results);
    //}

    //start reducers
    //for(int i = 0; i < nw_reducer; i++)
    //  reducers[i] = std::thread(count_numbers_reduce, results[i]);

    // send termination signal to reducers
    //for(int i = 0; i < nw_reducer; i++)
    //  results[i]->safe_push(std::make_pair(-1,-1));

    // join mappers and reducers
    //for(int i=0;i<nw_mapper;i++)
    //  mappers[i].join();
    //for(int i=0;i<nw_reducer;i++)
    //  reducers[i].join();

    //std::vector<std::pair<int, int>> data_mapped = count_numbers_map(data, 0, data.size()-1);

    //while(!global_results.isEmpty()){
    //  auto item = global_results.safe_pop();
    //  std::cout << item.first << "," << item.second << std::endl;
    //}

    return 0;
}
