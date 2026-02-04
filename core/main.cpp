#include "Utils.h"
#include "board/Board.h"
#include "engine/Shell.h"
#include <iostream>

int main() {

    std::cout << "\nInitializing board..\n\n\n";

    int x = 10/8;
    int y = 58/8;

    int modX = 10 % 8;
    int modY = 58 % 8;

    std::cout << x << y << "\n\n" ; 

    std::cout << modX << modY << "\n\n";

    Shell shell;
    
    return shell.run();
    

}

