#pragma once

#include <iostream>
#include <array>
#include <cstdint>

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

    }

    std::array<int8_t, 64>& getBoard() {
        return board;
    }

    LastMove& getLastMove() {
        return lastMove;
    }

    void printBoard() {

        for(int r = 7; r >= 0; --r){
            for(int c = 0; c < 8; ++c) {
                
                int index = r * 8 + c;
                std::cout << static_cast<int>(board.at(index)) << " ";
            
            }
            std::cout << "\n";
        }
    }
    

};
