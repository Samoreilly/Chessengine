#pragma once

#include "Board.h"
#include <cstdlib>

//Since each individual piece is mapped to a number e.g. 1 to pawn.
//PLACEHOLDER to avoid piece - 1 index

enum class PieceType {EMPTY, PAWN, ROOK, BISHOP, KNIGHT, QUEEN, KING};

class Piece {

public:

    Board& b;

    Piece(Board& b) : b(b) {}
    
    std::array<int8_t, 64>& board = b.getBoard();
    

    PieceType getPieceType(int piece) {
        int absPiece = std::abs(piece);
        return static_cast<PieceType>(piece); 
    }

    bool isWhite(int& piece) {
        return piece > 0;
    }

    bool isBlack(int& piece) {
        return piece < 0;
    }
    
    bool isEmpty(int& piece) {
        return piece == 0;
    }

    bool pawnMove(int from, int to);
    bool knightMove(int from, int to);
    bool bishopMove(int from, int to);
    bool rookMove(int from, int to);
    bool queenMove(int from, int to);
    bool kingMove(int from, int to);
    bool emptyMove(int from, int to);


};
