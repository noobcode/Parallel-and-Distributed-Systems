// @Authors Alessi Carlo, Corbucci Luca

#include <unistd.h>
#include <iostream>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <vector>
#include "./MapReduce.h"


template <typename T>
std::pair<T, int> count_numbers_map(T item) {
    return std::make_pair(item, 1);
}

template <typename T>
void setup(std::vector<T> data, int nw_mapper, int nw_reducer) {
    MapReduce<T> mr(nw_mapper, nw_reducer);
    mr.map_and_reduce(data, count_numbers_map<T>);
    mr.print_results();
}

int main(int argc, char* argv[]) {
    std::vector<std::string> dataString = {"ciao", "mondo", "roma", "lazio", "italia", "lex", "ciao", "italia", "roma"};
    std::vector<double> dataDouble = {4.0, 5.1, 6.0, 2, 0, 5.1, 4.0, 6.0, 5.1};
    std::vector<int> dataInt = {12, 3, 4, 5, 3, 4, 12, 16, 2, 3, 4, 6, 12, 20, 3, 5, 20, 16, 12};

    // for (auto i : data)
    //     std::cout << i << " ";
    // std::cout << std::endl;

    // First test with strings
    std::cout << "Test with strings" << std::endl;
    setup(dataString, atoi(argv[1]), atoi(argv[2]));

    // Second test with integers
    std::cout << "Test with integers" << std::endl;
    setup(dataInt, atoi(argv[1]), atoi(argv[2]));

    // Third test with doubles
    std::cout << "Test with doubles" << std::endl;

    setup(dataDouble, atoi(argv[1]), atoi(argv[2]));

    return 0;
}
