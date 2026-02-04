#include "board/Board.h"
#include "engine/Shell.h"
#include <iostream>

int main() {

    std::cout << "\nInitializing board..\n\n\n";

    Board b;

    auto board = b.getBoard(); 

    Shell shell(b);
    
    return shell.run();
    

}

