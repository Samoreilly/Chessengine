#pragma once

#include <utility>
#include <string>
#include <iostream>

inline std::pair<std::string, std::string> getCoord(std::string& str) {
    
    std::pair<std::string, std::string> pair;
    //a8-b7
    std::string_view from(str.data(), str.length() - 3);
    std::string_view to(str.data() + 3, str.length());
    
    pair.first = std::string(from);
    pair.second = std::string(to);
    
    return pair;

}

//converts for example: "a8" to an index in the 1d flat array;
inline int8_t getIndex(std::string str) {

    int col = str[0] - 'a';
    int row = str[1] - '1';

    int index = row * 8 + col;
    
    //std::cout << "INDEX->" << index;

    return index;
}



