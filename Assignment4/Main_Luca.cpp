#include <unistd.h>
#include <iostream>
#include <utility>
#include <vector>
#include <type_traits>
#include <typeinfo>
#include "./map_reduce_Luca.h"

template <typename T>
std::pair<T, int> count_numbers_map(T item) {
    return std::make_pair(item, 1);
}

int main(int argc, char* argv[]) {
    int nw_mapper = atoi(argv[1]);
    int nw_reducer = atoi(argv[2]);
    //int type_data = atoi(argv[3]);


    std::cout << std::is_same<typeid("ciao").name(), typeid("").name()>::value << std::endl;

    //std::vector<int> data = {2, 3, 3, 4, 6, 2, 4, 6, 6, 5, 6, 1, 1, 1, 1, 1};
    std::vector<std::string> data = {"ciao", "mondo", "roma", "lazio", "italia", "lex", "ciao", "italia", "roma"};

    for (auto i : data)
        std::cout << i << " ";
    std::cout << std::endl;

    MapReduce<std::string> mr(nw_mapper, nw_reducer);
    mr.map_and_reduce(data, count_numbers_map<std::string>);
    mr.print_results();

    return 0;
}
