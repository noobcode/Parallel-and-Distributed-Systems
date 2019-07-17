// @Authors Alessi Carlo, Corbucci Luca

#include <unistd.h>
#include <iostream>
#include <utility>
#include <vector>

#include <ff/ff.hpp>
#include <ff/pipeline.hpp>
#include <ff/all2all.hpp>
#include <ff/farm.hpp>

using namespace ff;

template <typename T>
std::pair<T, int> count_numbers_map(T item) {
    return std::make_pair(item, 1);
}

template <typename T>
int findItem(std::vector<std::pair<T, int>> local_results, T item) {
    for (int i = 0; i < local_results.size(); i++)
        if (local_results[i].first == item)
            return i;
    return -1;
}

/*
    Left worker of the a2a building block. 
    Here we compute the map function.
*/
template <typename T>
struct map_worker : ff_monode_t<std::pair<T, int>> {
    map_worker(std::function<std::pair<T, int>(T)> f_map,
               std::vector<T> data, int startIdx, int endIdx) : f_map(f_map), data(data), startIdx(startIdx), endIdx(endIdx) {}

    std::pair<T, int> *svc(std::pair<T, int> *) {
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

        const int nw_reducer = this->get_num_outchannels();
        for (auto &item : temp_results) {
            auto channel = hasher(item.first) % nw_reducer;
            this->ff_send_out_to(&item, channel);
        }
        return this->EOS;
    }

    // params
    std::function<std::pair<T, int>(T)> f_map;
    std::vector<T> data;
    int startIdx;
    int endIdx;
    std::vector<std::pair<T, int>> temp_results;
    std::hash<T> hasher;
};

/*
    Right worker of the a2a building block. 
    Here we compute the reduce function.
    This function store a vector of partial results, when the computation
    is over we access this vector to print the final result of the map reduce.
*/
template <typename T>
struct reduce_worker : ff_minode_t<std::pair<T, int>> {
    std::pair<T, int> *svc(std::pair<T, int> *in) {
        std::pair<T, int> &pair = *in;
        int index = findItem(temp_results, pair.first);
        if (index == -1)
            temp_results.push_back(pair);
        else
            temp_results[index].second += pair.second;
        // I have no more tasks to send out, give me another input task (if any)
        return this->GO_ON;
    }

    std::vector<std::pair<T, int>> temp_results;
};

/*
    Function that start the map reduce.
    We created this function because we need to use the general
    type T and we can't use it in the main function. 
*/
template <typename T>
void setup(std::vector<T> data, int nw_reducer, int nw_mapper) {
    std::vector<ff_node *> map_workers;
    std::vector<ff_node *> reduce_workers;

    int dim = data.size() / nw_mapper;

    ffTime(START_TIME);

    // create mappers
    for (int i = 0; i < nw_mapper; i++) {
        int startIdx = i * dim;
        int endIdx = i == nw_mapper - 1 ? data.size() - 1 : (i + 1) * dim - 1;
        map_workers.push_back(new map_worker<T>(count_numbers_map<T>, data, startIdx, endIdx));
    }

    // create reducers
    for (int i = 0; i < nw_reducer; i++)
        reduce_workers.push_back(new reduce_worker<T>());

    // Create all to all
    ff_a2a a2a;
    a2a.add_firstset(map_workers);
    a2a.add_secondset(reduce_workers);

    if (a2a.run_and_wait_end() < 0)
        error("Error running pipe");

    // getting results from reducers
    std::vector<std::pair<T, int>> results;
    for (int i = 0; i < nw_reducer; i++) {
        reduce_worker<T> *r = reinterpret_cast<reduce_worker<T> *>(reduce_workers[i]);
        if (r->temp_results.size())
            results.insert(std::upper_bound(results.begin(), results.end(),
                                            r->temp_results[0]),
                           r->temp_results.begin(), r->temp_results.end());
    }
    ffTime(STOP_TIME);

    // printing results:
    for (auto pair : results) {
        std::cout << pair.first << ", " << pair.second << std::endl;
    }
    std::cout << "Time: " << ffTime(GET_TIME) << " (ms)\n";
    std::cout << "A2A Time: " << a2a.ffTime() << " (ms)\n";
}

int main(int argc, char *argv[]) {
    std::vector<std::string> dataString = {"ciao", "mondo", "roma", "lazio", "italia", "lex", "ciao", "italia", "roma"};
    std::vector<int> dataInt = {12, 3, 4, 5, 3, 4, 12, 16, 2, 3, 4, 6, 12, 20, 3, 5, 20, 16, 12};

    // for (auto i : data)
    //     std::cout << i << " ";
    // std::cout << std::endl;

    std::cout << "First test, with strings" << std::endl;
    setup(dataString, atoi(argv[1]), atoi(argv[2]));
    std::cout << "" << std::endl;
    std::cout << "Second test, with integers" << std::endl;
    setup(dataInt, atoi(argv[1]), atoi(argv[2]));

    return 0;
}

