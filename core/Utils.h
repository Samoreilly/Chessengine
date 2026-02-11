#pragma once

#include <utility>
#include <string>
#include <cmath>
#include <iostream>
#include "board/Board.h"

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

inline bool isOpponent(Board& board, int from, int to) {
    return board.getBoard().at(from) * board.getBoard().at(to) < 0;
}

inline double toWinPercent(int eval) {
    return 100.0 / (1 + exp(-eval / 100.0));
}

// Converts board index (0-63) to algebraic notation e.g. 4 -> "e1"
inline std::string indexToAlgebraic(int idx) {
    char col = 'a' + (idx % 8);
    char row = '1' + (idx / 8);
    return std::string(1, col) + std::string(1, row);
}




