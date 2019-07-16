#include <unistd.h>
#include <iostream>
#include <utility>
#include <vector>
#include "../MapReduce.h"

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

// R-Worker
template <typename T>
struct reduce_worker : ff_minode_t<std::pair<T, int>> {
    std::pair<std::string, int> *svc(std::pair<std::string, int> *in) {
        try {
            std::pair<std::string, int> &pair = *in;
            int index = findItem(temp_results, pair.first);
            if (index == -1)
                temp_results.push_back(pair);
            else
                temp_results[index].second += pair.second;
        } catch (MyException &e) {
            return this->EOS;
        }

        return this->GO_ON;
    }

    void
    svc_end() {
        // push to global results
        for (auto i : temp_results) {
            std::cout << i.first << "," << i.second << std::endl;
            //this->ff_send_out_to()
        }
        std::cout << "EOS received\n";
    }

    std::vector<std::pair<T, int>> temp_results;
};

int main(int argc, char *argv[]) {
    int nw_mapper = atoi(argv[1]);
    int nw_reducer = atoi(argv[2]);

    std::vector<std::string> data = {"ciao", "mondo", "roma", "lazio", "italia", "lex", "ciao", "italia", "roma"};

    for (auto i : data)
        std::cout << i << " ";
    std::cout << std::endl;

    std::vector<ff_node *> map_workers;
    std::vector<ff_node *> reduce_workers;

    int dim = data.size() / nw_mapper;

    // create mappers
    for (int i = 0; i < nw_mapper; i++) {
        int startIdx = i * dim;
        int endIdx = i == nw_mapper - 1 ? data.size() - 1 : (i + 1) * dim - 1;
        map_workers.push_back(new map_worker<std::string>(count_numbers_map<std::string>, data, startIdx, endIdx));
    }

    // create reducers
    for (int i = 0; i < nw_reducer; i++)
        reduce_workers.push_back(new reduce_worker<std::string>());

    ff_a2a a2a;
    a2a.add_firstset(map_workers, 0, true);
    a2a.add_secondset(reduce_workers);

    if (a2a.run_and_wait_end() < 0)
        error("running a2a");

    //farm_map()
    //farm_reduce()
    //pipe(farm_map, farm_reduce)

    return 0;
}
