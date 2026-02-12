#include "Utils.h"
#include "board/Board.h"
#include "engine/Shell.h"
#include <iostream>

int main(int argc, char* argv[]) {

    bool api = false;
    if (argc > 1 && std::string(argv[1]) == "--api") {
        api = true;
    }

    if (!api) {
        std::cout << "\nInitializing board..\n\n\n";
    }

    Shell shell(api);
    
    return shell.run();
    

}

