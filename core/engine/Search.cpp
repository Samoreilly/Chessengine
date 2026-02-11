#include "Search.h"
#include "../board/Generate.h"

void Search::search(std::array<int8_t, 64> board, bool white, int depth) {
    if(depth == 0) {
        //return evaluate() function
    }
    
    std::vector<Gen> gen = g.generate(board, white);

    for(auto ge : gen) {

        std::array<int8_t,  64> copy = g.makeMove(board, ge);
        //TODO: make move, that will return a copy and pass into recursive function
    
        search(copy, !white, depth - 1);

    }

}
