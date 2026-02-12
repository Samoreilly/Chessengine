#include "Board.h"
#include "Piece.h"
#include <cstdlib>
#include "../Utils.h"

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

        if(abs(to - from) <= 16) {
            
            for(int i = from + step;i != to; i += step) {
                   
                if(board.at(i) != 0) {                    
                    
                    return false;
                }
            }
            
            
            lm.from = from;
            lm.to = to;
            lm.piece = board.at(from);
            lm.pieceTaken = board.at(to);

            board.at(from) = 0;
            board.at(to) = white ? 1 : -1;

            return true;            
        }
    
    //check for en passant or diagonal capture, left or right column and also if previous move was 2 squares
    }else if(abs(from % 8 - to % 8) == 1) {

        int step = board.at(from) > 0 ? -8 : 8; //if white check the piece below for en passant 
        
        //diagonal capture
        if(board.at(to) != 0 && ((board.at(from) < 0 && board.at(to) > 0) || (board.at(from) > 0 && board.at(to) < 0))) {
            
            
            lm.from = from;
            lm.to = to;
            lm.piece = board.at(from);
            lm.pieceTaken = board.at(to);

            board.at(to) = board.at(from);
            board.at(from) = 0;

            return true;
        }

        //then the piece below must be a pawn and the position of it must equal lm.to
        //check if piece is opposite of current color 1 == -1, last piece moved was the piece were taking and it moved 2 squares
        //'to' must be unoccupied
        if(board.at(to) == 0 && board.at(to + step) == -board.at(from) && to + step == lm.to && abs(lm.to - lm.from) == 16) {

            lm.from = from;
            lm.to = to;
            lm.piece = board.at(from);
            lm.pieceTaken = board.at(to);

            board.at(to + step) = 0;
            board.at(to) = board.at(from);
            board.at(from) = 0;

            
            return true;
        }
    }


    return false;

}

//int from = 10, to = 50
bool Piece::rookMove(int from, int to) {

    LastMove& lm = b.getLastMove();

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


    if(board.at(from) * board.at(to) <= 0) {

        lm.from = from;
        lm.to = to;
        lm.piece = board.at(from);
        lm.pieceTaken = board.at(to);

        board.at(to) = board.at(from);
        board.at(from) = 0;

        b.revokeCastling(from);
        return true;                
    }

    return false;
}

// 56 57 58 59 60 61 62 63
// 48 49 50 51 52 53 54 55
// 40 41 42 43 44 45 46 47
// 32 33 34 35 36 37 38 39
// 24 25 26 27 28 29 30 31
// 16 17 18 19 20 21 22 23
//  8  9 10 11 12 13 14 15
//  0  1  2  3  4  5  6  7


bool Piece::knightMove(int from, int to) {
    
    LastMove& lm = b.getLastMove();

    int colDiff = abs(from % 8 - to % 8);
    int rowDiff = abs(from / 8 - to / 8);
    
    
    bool isValid = (rowDiff == 2 && colDiff == 1)
        || (rowDiff == 1 && colDiff == 2);


    if(isValid && (isOpponent(b, from, to) || board.at(to) == 0)) {
            
        lm.from = from;
        lm.to = to;
        lm.piece = board.at(from);
        lm.pieceTaken = board.at(to);

        board.at(to) = board.at(from);
        board.at(from) = 0;

        return true;
    }



    return false;
}

// 56 57 58 59 60 61 62 63
// 48 49 50 51 52 53 54 55
// 40 41 42 43 44 45 46 47
// 32 33 34 35 36 37 38 39
// 24 25 26 27 28 29 30 31
// 16 17 18 19 20 21 22 23
//  8  9 10 11 12 13 14 15
//  0  1  2  3  4  5  6  7

//up-left and down right +-7
//up-right and down left +-9

bool Piece::bishopMove(int from, int to) {

    LastMove& lm = b.getLastMove();

    int fromRow = from / 8;
    int fromCol = from % 8;
    int toRow   = to / 8;
    int toCol   = to % 8;

    int dr = toRow - fromRow;
    int dc = toCol - fromCol;

    //checks if the rows and cols between from and to are equal
    //as they must be
    if (abs(dr) != abs(dc)) {
        return false;
    }

    
    int stepRow = (dr > 0) ? 1 : -1;
    int stepCol = (dc > 0) ? 1 : -1;

    int r = fromRow + stepRow;
    int c = fromCol + stepCol;

    while (r != toRow || c != toCol) {
        int idx = r * 8 + c;

        if (board.at(idx) != 0) {
            return false;
        }

        r += stepRow;
        c += stepCol;
    }

    if (board.at(to) != 0 && !isOpponent(b, from, to)) {
        return false;
    }

    lm.from = from;
    lm.to = to;
    lm.piece = board.at(from);
    lm.pieceTaken = board.at(to);

    board.at(to) = board.at(from);
    board.at(from) = 0;
    return true;
}


bool Piece::queenMove(int from, int to) {

    LastMove& lm = b.getLastMove();

    int colDiff = abs(from % 8 - to % 8);
    int rowDiff = abs(from / 8 - to / 8);
    
    int fromRow = from / 8;
    int fromCol = from % 8;
    int toRow   = to / 8;
    int toCol   = to % 8;

    if((from % 8 != to % 8 && from / 8 != to / 8 && colDiff != rowDiff)) {
        return false;
    }

    int dr = toRow - fromRow;
    int dc = toCol - fromCol;

    
    int stepRow = (dr == 0) ? 0 : (dr > 0 ? 1 : -1);
    int stepCol = (dc == 0) ? 0 : (dc > 0 ? 1 : -1);
    
    int r = fromRow + stepRow;
    int c = fromCol + stepCol;

    while(r != toRow || c != toCol) {
        int idx = r * 8 + c;

        if(board.at(idx) != 0) {
            return false;
        }
        
        r += stepRow;
        c += stepCol;

    }

    if (board.at(to) != 0 && !isOpponent(b, from, to)) {
        return false;
    }

    lm.from = from;
    lm.to = to;
    lm.piece = board.at(from);
    lm.pieceTaken = board.at(to);

    board.at(to) = board.at(from);
    board.at(from) = 0;
    return true;
}

bool Piece::kingMove(int from, int to) {

    LastMove& lm = b.getLastMove();

    int rankDiff = abs(from / 8 - to / 8);
    int fileDiff = abs(from % 8 - to % 8);

    // --- Castling: king moves 2 squares horizontally ---
    if (rankDiff == 0 && fileDiff == 2) {
        bool isWhite = board.at(from) > 0;

        if (to > from) {
            // Kingside
            if (!b.canCastleKingSide(isWhite)) {
                return false;
            }
            int rookFrom = from + 3;
            int rookTo = from + 1;
            // Check squares empty
            if (board.at(from + 1) != 0 || board.at(from + 2) != 0) {
                return false;
            }
            // Move king and rook
            lm.from = from; lm.to = to;
            lm.piece = board.at(from); lm.pieceTaken = 0;

            board.at(to) = board.at(from);
            board.at(from) = 0;
            board.at(rookTo) = board.at(rookFrom);
            board.at(rookFrom) = 0;

            b.revokeCastling(from);
            return true;
        } else {
            // Queenside
            if (!b.canCastleQueenSide(isWhite)) {
                return false;
            }
            int rookFrom = from - 4;
            int rookTo = from - 1;
            // Check squares empty
            if (board.at(from - 1) != 0 || board.at(from - 2) != 0 || board.at(from - 3) != 0) {
                return false;
            }
            lm.from = from; lm.to = to;
            lm.piece = board.at(from); lm.pieceTaken = 0;

            board.at(to) = board.at(from);
            board.at(from) = 0;
            board.at(rookTo) = board.at(rookFrom);
            board.at(rookFrom) = 0;

            b.revokeCastling(from);
            return true;
        }
    }

    // --- Normal king move ---
    if(rankDiff > 1 || fileDiff > 1) {
        return false;
    }
    
    if(rankDiff == 0 && fileDiff == 0) {
        return false;
    }

    if(board.at(to) == 0 || isOpponent(b, from, to)) {

        lm.from = from;
        lm.to = to;
        lm.piece = board.at(from);
        lm.pieceTaken = board.at(to);

        board.at(to) = board.at(from);
        board.at(from) = 0;

        b.revokeCastling(from);
        return true;
    }

    return false;
}

bool Piece::emptyMove(int from, int to) {

    if(board.at(from) == 0) {
    }
    
    return false;
}
