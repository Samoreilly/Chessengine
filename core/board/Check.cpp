
#include "Check.h"
#include "../Utils.h"
#include "Board.h"
#include "Piece.h"
#include "Board.h"

bool Check::isCheck(bool turn) {

    // Find king by scanning — the tracked positions can become stale
    int kingPos = -1;
    int kingVal = turn ? 6 : -6;
    for (int i = 0; i < 64; i++) {
        if (board.at(i) == kingVal) { kingPos = i; break; }
    }
    if (kingPos < 0) return false;

    return scanRookQueen(kingPos) || scanDiagonal(kingPos) || scanKnight(kingPos) || scanPawn(kingPos);
}


//4 cardinal directions
bool Check::scanRookQueen(int kingPos) {
    
    int fromRow = kingPos / 8;
    int fromCol = kingPos % 8;

    for(auto dir : b.dirs) {
        
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

    for(auto dir : b.diag) {
        
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

    for(auto dir : b.knight) {
        
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

bool Check::scanPawn(int kingPos) {
   

    int row = kingPos / 8;
    int col = kingPos % 8;

    bool kingIsWhite = board.at(kingPos) > 0;

    // direction pawns attack FROM
    int pawnRow = kingIsWhite ? row + 1 : row - 1;

    for (int dc : {-1, 1}) {
        int r = pawnRow;
        int c = col + dc;

        if (r >= 0 && r < 8 && c >= 0 && c < 8) {
            int idx = r * 8 + c;
            int p = board.at(idx);

            int pawn = static_cast<int>(PieceType::PAWN);
            if (p == (kingIsWhite ? -pawn : pawn)) {
                return true;
            }

        }
    }

    return false;
}


void Check::undoMove() {

    LastMove& lm = b.getLastMove();

    int8_t toPiece = board.at(lm.to);


    board.at(lm.to) = lm.pieceTaken.has_value() ? lm.pieceTaken.value() : 0;
    board.at(lm.from) = toPiece;

}
