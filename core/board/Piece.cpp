#include "Board.h"
#include "Piece.h"

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

bool Piece::pawnMove(int from, int to) {

    return false;

}

//int from = 10, to = 50
bool Piece::rookMove(int from, int to) {

    int sameColF = from % 8, sameColT = to % 8;
    int sameRowF = from / 8, sameRowT = to / 8; 

    if((sameColT != sameColF) && (sameRowT != sameRowF)) return false;

    //if same col move up/down 8, if same row right/left 1
    
    int step;

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


    return false;
}

bool Piece::emptyMove(int from, int to) {
    
    return false;
}
