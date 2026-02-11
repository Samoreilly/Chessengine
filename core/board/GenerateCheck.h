#pragma once

#include <array>
#include <cstdint>
#include "Piece.h"
#include "Board.h"


// just used for check when generating moves

class GenerateCheck {
public:
    // Returns true if the king of color `turn` is in check
    bool isCheck(const std::array<int8_t, 64>& board, bool turn);

private:
    bool scanRookQueen(const std::array<int8_t, 64>& board, int kingPos);
    bool scanDiagonal(const std::array<int8_t, 64>& board, int kingPos);
    bool scanKnight(const std::array<int8_t, 64>& board, int kingPos);
    bool scanPawn(const std::array<int8_t, 64>& board, int kingPos);

    bool isOpponent(const std::array<int8_t, 64>& board, int kingPos, int targetPos);

    // Directions
    const int dirs[4][2]    = {{-1,0},{1,0},{0,-1},{0,1}};
    const int diag[4][2]    = {{-1,-1},{-1,1},{1,-1},{1,1}};
    const int knight[8][2]  = {{-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}};

    int wKingPos(const std::array<int8_t, 64>& board);
    int bKingPos(const std::array<int8_t, 64>& board);
};

