#include "board/Board.h"

#include <iostream>

int main() {

    std::cout << "\nInitializing board..\n\n\n";

    Board b;

    auto& board = b.getBoard();
        
    b.printBoard();
    

}

