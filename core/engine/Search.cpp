#include "Search.h"
#include "../board/Generate.h"

int Search::search(std::array<int8_t, 64> board, int max, bool white, int depth) {
    if(depth == 0) {
        //return evaluate() function
    }
    
    std::vector<Gen> gen = g.generate(board, white);

    for(auto ge : gen) {

        std::optional<std::array<int8_t,  64>> copy = g.makeMove(board, white, ge);
        
        if(!copy.has_value()) continue;
        //TODO: make move, that will return a copy and pass into recursive function
    
        max = std::max(max, search(copy.value(), max, !white, depth - 1));
        
    }

    return max;

}
