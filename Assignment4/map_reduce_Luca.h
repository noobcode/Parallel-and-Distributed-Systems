#include <unistd.h>
#include <functional>
#include <iostream>
#include <utility>
#include <vector>
#include "./safe_queue.h"


template <class T>
class MapReduce {
   private:
    int nw_mapper;                                              // number of threads for the map
    int nw_reducer;                                             // number of threads for the reducer
    SafeQueue<std::pair<int, int>> global_results;              // final results - combination of local results
    std::vector<SafeQueue<std::pair<T, int>> *> local_results;  // results of each reducer
    std::vector<std::thread> mappers;
    std::vector<std::thread> reducers;

   public:
    MapReduce(int nw_mapper, int nw_reducer) : nw_mapper(nw_mapper),
                                               nw_reducer(nw_reducer),
                                               mappers(nw_mapper),
                                               reducers(nw_reducer),
                                               local_results(nw_reducer) {
        for (int i = 0; i < nw_reducer; i++)
            local_results[i] = new SafeQueue<std::pair<int, int>>;
    };

    int findItem(std::vector<std::pair<T, int>> local_results, int item) {
        for (int i = 0; i < local_results.size(); i++)
            if (local_results[i].first == item)
                return i;
        return -1;
    }

    //private:
    //std::pair<T,int>
    void map(std::function<std::pair<T, int>(T)> f_map,
             std::vector<int> data, int startIdx, int endIdx) {
        std::vector<std::pair<T, int>> temp_results;

        for (int i = startIdx; i <= endIdx; i++) {
            T item = data[i];
            auto temp_res = f_map(item);
            int index = findItem(temp_results, temp_res.first);
            if (index == -1) {
                temp_results.push_back(temp_res);
            } else {
                // local reduce
                temp_results[index].second++;
            }
        }

        //push local results to reducers queue
        int nw_reducers = local_results.size();
        for (auto i : temp_results)
            local_results[i.first % nw_reducers]->safe_push(i);
    }

    void reduce(SafeQueue<std::pair<int, int>> *queue) {
        std::vector<std::pair<T, int>> temp_results;

        while (true) {
            auto pair = queue->safe_pop();
            if (pair.first == -1 && pair.second == -1)
                break;

            int index = this->findItem(temp_results, pair.first);
            if (index == -1)
                temp_results.push_back(pair);
            else
                temp_results[index].second += pair.second;
        }

        // push to global results
        for (auto i : temp_results) {
            //std::cout << local_results.size() << " " << i.first << " " << i.second << std::endl;
            global_results.safe_push(i);
        }
    }

    void map_and_reduce(std::vector<int> data,
                        std::function<std::pair<T, int>(int)> f_map) {
        int dim = data.size() / nw_mapper;

        // start mappers
        for (int i = 0; i < nw_mapper; i++) {
            int startIdx = i * dim;
            int endIdx = i == nw_mapper - 1 ? data.size() - 1 : (i + 1) * dim - 1;
            mappers[i] = std::thread([=] { map(f_map, data, startIdx, endIdx); });
        }

        //start reducers
        for (int i = 0; i < nw_reducer; i++)
            reducers[i] = std::thread([=] { reduce(local_results[i]); });

        // join mappers and reducers
        for (int i = 0; i < nw_mapper; i++)
            mappers[i].join();

        // send termination signal to reducers
        for (int i = 0; i < nw_reducer; i++)
            local_results[i]->safe_push(std::make_pair(-1, -1));

        // join reducers
        for (int i = 0; i < nw_reducer; i++)
            reducers[i].join();
    }

    void print_results() {
        // std::cout << nw_mapper << std::endl;
        // std::cout << nw_reducer << std::endl;
        // std::cout << global_results.safe_size() << std::endl;
        // std::cout << local_results.size() << std::endl;
        // std::cout << mappers.size() << std::endl;
        // std::cout << reducers.size() << std::endl;

        while (!global_results.isEmpty()) {
            auto item = global_results.safe_pop();
            std::cout << item.first << "," << item.second << std::endl;
        }
    }
};
