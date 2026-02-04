#pragma once

#include "../board/Board.h"
#include "../board/Piece.h"

class Shell {

    Board b;
    Piece p{b};

public:

    Shell() {}

    std::array<int8_t, 64>& board = b.getBoard();

    int run();
    bool handleMove(std::string& move);

};
