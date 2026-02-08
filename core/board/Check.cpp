
#include "Check.h"
#include "../Utils.h"
#include "Board.h"
#include "Piece.h"

int dirs[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
int diag[4][2] = {{-1, 1}, {-1, -1}, {1, 1}, {1, -1}};
bool Check::isCheck() {

    int kingPos = b.wKingPos();
    std::cout << "KINGS POSITION" << kingPos;


    return scanRookQueen(kingPos) || scanDiagonal(kingPos);




}



bool Check::whiteCheck() {
 
    int8_t whiteKing = b.wKingPos();


    //check up, down, left, right
    


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
