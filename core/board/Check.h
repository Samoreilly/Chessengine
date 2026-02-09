#pragma once

#include "Board.h"

//handles checkmate and check
class Check {

    Board& b;
    std::array<int8_t, 64>& board;

public:

    Check(Board& b) : b(b), board(b.getBoard()) {}
    
    bool isCheck();
    bool isCheckMate();

    bool whiteCheck();
    bool blackCheck();


    bool scanRookQueen(int kingPos);
    bool scanDiagonal(int kingPos);
    bool scanKnight(int kingPos);

};
