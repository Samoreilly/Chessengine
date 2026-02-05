#include "Utils.h"
#include "board/Board.h"
#include "engine/Shell.h"
#include <iostream>

int main() {

    std::cout << "\nInitializing board..\n\n\n";

    Shell shell;
    
    return shell.run();
    

}

