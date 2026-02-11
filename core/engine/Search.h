#pragma once

#include "../board/Board.h"
#include "../board/Generate.h"

//call generator() then loop through move vector
//apply each move to a copy of the board
//recursively call search until depth
class Search {

    Generate g{};

public:

    Search() {}

    //Search(Board b) : b(b), board(b.getBoard()) {}

    int search(std::array<int8_t, 64> board, int max, bool white, int depth);
    


};
