#include <iostream>
#include "./safe_queue.h"

template <class T> class MapReduce {
    private:
      int nw_mapper; // number of threads for the map
      int nw_reducer; // number of threads for the reducer
      SafeQueue<std::pair<T, T>> global_results;  // final results - combination of local results
      std::vector<SafeQueue<std::pair<T,T>>*> local_results; // results of each reducer
      std::vector<std::thread> mappers;
      std::vector<std::thread> reducers;

    public:
        MapReduce(int nw_mapper, int nw_reducer) : nw_mapper(nw_mapper),
                                                   nw_reducer(nw_reducer),
                                                   mappers(nw_mapper),
                                                   reducers(nw_reducer),
                                                   local_results(nw_reducer)
        {
          for(int i = 0; i < nw_reducer; i++)
            local_results[i] = new SafeQueue<std::pair<int, int>>;
        };

        void map_and_reduce(std::vector<T> data,
                            std::function f_map,
                            std::function f_reduce){
          int dim = data.size() / nw_mapper;

          // start mappers
          for(int i = 0; i < nw_mapper; i++){
            int startIdx = i * dim;
            int endIdx = i == nw_mapper-1 ? data.size()-1 : (i+1) * dim-1;
            mappers[i] = std::thread(f_map, data, startIdx,  endIdx); // , local_results);
          }

          //start reducers
          for(int i = 0; i < nw_reducer; i++)
            reducers[i] = std::thread(f_reduce, local_results[i]);

          // join mappers and reducers
          for(int i=0;i<nw_mapper;i++)
            mappers[i].join();

          // send termination signal to reducers
          for(int i = 0; i < nw_reducer; i++)
            local_results[i]->safe_push(std::make_pair(-1,-1));

          // join reducers
          for(int i=0;i<nw_reducer;i++)
            reducers[i].join();
        }

        void print_results(){
          std::cout << nw_mapper << std::endl;
          std::cout << nw_reducer << std::endl;
          std::cout << global_results.safe_size() << std::endl;
          std::cout << local_results.size() << std::endl;
          std::cout << mappers.size() << std::endl;
          std::cout << reducers.size() << std::endl;

          while(!global_results.isEmpty()){
            auto item = global_results.safe_pop();
            std::cout << item.first << "," << item.second << std::endl;
          }
        }

};
