#pragma once

#include "../board/Board.h"

class Shell {

    Board& board;

public:

    Shell(Board& board) : board(board) {}
    
    int run();

};
