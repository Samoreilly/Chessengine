
#include "Check.h"
#include "../Utils.h"
#include "Board.h"
#include "Piece.h"
#include "Board.h"

bool Check::isCheck(bool turn) {

    int kingPos = turn ? b.wKingPos() : b.bKingPos();

    return scanRookQueen(kingPos) || scanDiagonal(kingPos) || scanKnight(kingPos);
}


//4 cardinal directions
bool Check::scanRookQueen(int kingPos) {
    
    int fromRow = kingPos / 8;
    int fromCol = kingPos % 8;

    for(auto dir : dirs) {
        
        int rowStep = dir[0];
        int colStep = dir[1];

        int r = fromRow + rowStep;
        int c = fromCol + colStep;

        while(r >= 0 && r < 8 && c >= 0 && c < 8) {

            int idx = r * 8 + c;
            int piece = board.at(idx);     

            if (board.at(idx) != 0) {
                PieceType type = static_cast<PieceType>(abs(piece)); 
                
                if (isOpponent(b, kingPos, idx) &&
                    (type == PieceType::ROOK || type == PieceType::QUEEN)) {
                    return true;
                }
                //not opponent
                break;
            }

            r += rowStep;
            c += colStep;
        }

    } 

    return false;
}


bool Check::scanDiagonal(int kingPos) {
    
    int fromRow = kingPos / 8;
    int fromCol = kingPos % 8;

    for(auto dir : diag) {
        
        int rowStep = dir[0];
        int colStep = dir[1];

        int r = fromRow + rowStep;
        int c = fromCol + colStep;

        while(r >= 0 && r < 8 && c >= 0 && c < 8) {

            int idx = r * 8 + c;
            int piece = board.at(idx);     

            if (board.at(idx) != 0) {
                PieceType type = static_cast<PieceType>(abs(piece)); 
                
                if (isOpponent(b, kingPos, idx) &&
                    (type == PieceType::BISHOP || type == PieceType::QUEEN)) {
                    return true;
                }
                //not opponent
                break;
            }

            r += rowStep;
            c += colStep;
        }

    } 

    return false;
}

bool Check::scanKnight(int kingPos) {

    int fromRow = kingPos / 8;
    int fromCol = kingPos % 8;

    for(auto dir : knight) {
        
        int rowStep = dir[0];
        int colStep = dir[1];

        int r = fromRow + rowStep;
        int c = fromCol + colStep;

        while(r >= 0 && r < 8 && c >= 0 && c < 8) {

            int idx = r * 8 + c;
            int piece = board.at(idx);     

            if (board.at(idx) != 0) {
                PieceType type = static_cast<PieceType>(abs(piece)); 
                
                if (isOpponent(b, kingPos, idx) &&
                    (type == PieceType::KNIGHT)) {
                    return true;
                }
                //not opponent
                break;
            }

            r += rowStep;
            c += colStep;
        }

    } 

    return false;
}

void Check::undoMove() {

    LastMove& lm = b.getLastMove();

    int8_t toPiece = board.at(lm.to);

    board.at(lm.to) = board.at(lm.from);
    board.at(lm.from) = toPiece;

    std::cout << "You cannot move into check";
}
