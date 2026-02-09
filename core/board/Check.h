#pragma once

#include "Board.h"

//handles checkmate and check
class Check {

    Board& b;
    std::array<int8_t, 64>& board;

    int dirs[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    int diag[4][2] = {{-1, 1}, {-1, -1}, {1, 1}, {1, -1}};
    int knight[8][2] = {{2, 1}, {2, -1}, {-2, 1}, {-2, -1}, {1, 2}, {1, -2}, {-1, 2}, {-1, -2}};


public:

    Check(Board& b) : b(b), board(b.getBoard()) {}
    
    bool isCheck(bool turn);
    bool isCheckMate();

    bool whiteCheck();
    bool blackCheck();

    bool scanRookQueen(int kingPos);
    bool scanDiagonal(int kingPos);
    bool scanKnight(int kingPos);

    void undoMove();

};
