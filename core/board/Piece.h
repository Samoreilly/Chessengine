#pragma once

#include <cstdlib>

//Since each individual piece is mapped to a number e.g. 1 to pawn.
//PLACEHOLDER to avoid piece - 1 index

enum class PieceType {EMPTY, PAWN, ROOK, BISHOP, KNIGHT, QUEEN, KING};

class Piece {

public:

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

};
