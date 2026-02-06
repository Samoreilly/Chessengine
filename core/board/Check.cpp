
#include "Check.h"
#include "../Utils.h"
#include "Board.h"
#include "Piece.h"

int dirs[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

bool Check::isCheck() {

    std::cout << "KINGS POSITION" << b.wKingPos();
    return scanRookQueen(b.wKingPos());



}



bool Check::whiteCheck() {
 
    int8_t whiteKing = b.wKingPos();


    //check up, down, left, right
    


}

//4 cardinal directions
bool Check::scanRookQueen(int8_t kingPos) {
    
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
