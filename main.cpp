#include <iostream>
#include <string>

#include "map_with_get_very_frequent_lib/map_get_fresh_top_k.h"

using namespace std;

int main(int argc, char **argv) {
    MapGetFreshTopK<> map;
    std::string key = "key_HELLO";
    std::string val = "value_WORLD!";

    std::cout << "Welcome to main.cpp!" << std::endl;

    map.set(key, val);
    std::cout << "Successful set pair (key, value): (" << key << ", " << "val" << ")" << std::endl;

    if (map.get(key) == val) {
        std::cout << "Successful get(key): " << val << std::endl;
    } else {
        std::cout << "Unsuccessful get(key)" << std::endl;
    }


    if (map.get_very_frequent()[0] == key) {
        std::cout << "Successful get_top_k: " << key << std::endl;
    } else {
        std::cout << "Unsuccessful get_top_k" << std::endl;
    }

    do {
        cout << '\n' << "Press Enter to continue...";
    } while (cin.get() != '\n');

    return 0;
}