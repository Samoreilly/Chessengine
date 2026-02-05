#include "Board.h"
#include "Piece.h"
#include <cstdlib>

//This one
//  0  1  2  3  4  5  6  7
//  8  9 10 11 12 13 14 15
// 16 17 18 19 20 21 22 23
// 24 25 26 27 28 29 30 31
// 32 33 34 35 36 37 38 39
// 40 41 42 43 44 45 46 47
// 48 49 50 51 52 53 54 55
// 56 57 58 59 60 61 62 63

// 56 57 58 59 60 61 62 63
// 48 49 50 51 52 53 54 55
// 40 41 42 43 44 45 46 47
// 32 33 34 35 36 37 38 39
// 24 25 26 27 28 29 30 31
// 16 17 18 19 20 21 22 23
//  8  9 10 11 12 13 14 15
//  0  1  2  3  4  5  6  7


//en passant and double moves
bool Piece::pawnMove(int from, int to) {

    
    
    bool white = (board.at(from) > 0);
    bool enpassant = (from / 8 == 1);

    int step = white ? 8 : -8;

    LastMove& lm = b.getLastMove();
    
    if(from % 8 == to % 8) {
        if(!white) std::cout << "Black piece";

        if(abs(to - from) <= 16) {
            
            for(int i = from;i < to - from; i += step) {
                   
                if(board.at(i) == 0) {                    
                    
                    std::cout << "Invalid move";
                    return false;
                }
            }
            
            board.at(from) = 0;
            board.at(to) = white ? 1 : -1;
            lm.from = from;
            lm.to = to;
            lm.piece = white ? 1 : -1;
            return true;            
        }
    
    //check for en passant, left or right column and also if previous move was 2 squares
    }else if(from % 8 - 1 == to % 8 || from % 8 + 1 == to % 8) {
        std::cout << "Entering en passant logic";

        int step = board.at(from) > 0 ? -8 : 8; //if white check the piece below for en passant 

        //then the piece below must be a pawn and the position of it must equal lm.to
        //check if piece is opposite of current color 1 == -1, last piece moved was the piece were taking and it moved 2 squares
        //'to' must be unoccupied
        if(std::abs((from % 8) - (to % 8)) == 1 && board.at(to) == 0 && board.at(to + step) == -board.at(from) && to + step == lm.to && abs(lm.to - lm.from) == 16) {
            std::cout << "Entering inner if statement en passant logic";

            board.at(to + step) = 0;
            board.at(to) = board.at(from);
            board.at(from) = 0;
            
            std::cout << "En passant";
            return true;
        }
    }


    return false;

}

//int from = 10, to = 50
bool Piece::rookMove(int from, int to) {

    int sameColF = from % 8, sameColT = to % 8;
    int sameRowF = from / 8, sameRowT = to / 8; 

    if((sameColT != sameColF) && (sameRowT != sameRowF)) return false;
 
    int step;
    //if same col move up/down 8, if same row right/left

    if(sameColT == sameColF) {
        step = (from > to) ? -8 : 8;      
    }else {
        step = (from > to) ? -1 : 1;
    }

    int curr = from + step;

    while(curr != to) {

        if(board.at(curr) != 0) return false;
        
        if(step == 1 || step == - 1) {
            if(curr / 8 != from / 8) return false; // if the index went up onto another row
        }

        curr += step;
    }

    std::cout << "Moved pieced";

    return true;
}

bool Piece::knightMove(int from, int to) {


    return false;
}

bool Piece::bishopMove(int from, int to) {


    return false;
}

bool Piece::queenMove(int from, int to) {


    return false;
}

bool Piece::kingMove(int from, int to) {

    if((from % 8 != to % 8) && (from / 2 != to / 2)) {
        std::cout << "Invalid move";
        return false;
    } 

    if(from % 8 == to % 8) {
        if(abs(to - from) > 8) {
            std::cout << "King can only move 1 square at a time";
            return false;
        }

        if(board.at(to) == 0) {
            board.at(to) = board.at(from);
            board.at(from) = 0;
    
            return true;
        
        }

    }else {

        if(abs(to - from) > 1) {
            std::cout << "King can only move 1 square at a time";
            return false;
        }

        if(board.at(to) == 0) {
            board.at(to) = board.at(from);
            board.at(from) = 0;
    
            return true;
        
        }

    }

    std::cout << "Square is occupied";

    return false;
}

bool Piece::emptyMove(int from, int to) {
    
    return false;
}
