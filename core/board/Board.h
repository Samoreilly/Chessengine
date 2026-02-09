

#pragma once

#include <iostream>
#include <array>
#include <cstdint>
#include <iomanip>

//0 is empty square
//White Pawn - 1, Black Pawn - -1
//White Rook - 2, Black Rook - -2
//White Bishop - 3, Black Bishop - -3
//White Knight - 4, Black Knight - -4
//White Queen - 5, Black Queen - -5
//White King - 6, Black King - -6

struct LastMove {
    int to;
    int from;
    int piece;
};

class Board {

    //flat 1d array representing 2d array
    //white - 16 ints, black - last 16 ints
    //Positives is white, Negatives is black
    
    std::array<int8_t, 64> board;
    LastMove lastMove;
    int whiteKing {4};
    int blackKing {60};
    
    //state for white/black turn
    bool white;

public:

    Board() : lastMove{-1, -1, 0} {
    
        board =  {  2, 4, 3, 5, 6, 3, 4, 2,
                    1, 1, 1, 1, 1, 1, 1, 1,
                    0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0,
                    -1, -1, -1, -1, -1, -1, -1, -1,
                    -2, -4, -3, -5, -6, -3, -4, -2, 
        };

        white = true;

    }

    void setTurn(bool turn) {
        white = turn;
    }

    bool isWhiteTurn() {
        return white;
    }

    bool isBlackTurn() {
        return !white;
    }

    void uKingPos(int8_t pos, bool white) {
        if(white)whiteKing = pos;
        else blackKing = pos;
    }

    int8_t wKingPos() {
        return whiteKing;
    }

    int bKingPos() {
        return blackKing;
    }

    std::array<int8_t, 64>& getBoard() {
        return board;
    }

    LastMove& getLastMove() {
        return lastMove;
    }

    void printBoard() {
        std::cout << "\n\n   ";
        
        for(int i = 1; i <= 8;i++) {
            std::cout << std::setw(3) << i << " ";
        }
        
        std::cout << "\n   ";

        for(int i = 1; i <= 8;i++) {
            std::cout << std::setw(3) << "---" << " ";
        }
        
        std::cout << "\n\n";
        for(int r = 7; r >= 0; --r){
                
            std::cout << 8 - r << "| ";
            for(int c = 0; c < 8; ++c) {  
                int index = r * 8 + c;
                std::cout << std::setw(3) << static_cast<int>(board.at(index)) << " ";
            
            }
            std::cout << "\n\n";
        }
    }
    

};
