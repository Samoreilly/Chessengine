
#pragma once

#include <array>
#include <cstdint>
#include <vector>
#include <string>
#include "../board/Board.h"
#include "../board/Generate.h"

struct ScoredMove {
    std::vector<Gen> line;  // full PV: sequence of moves (depth moves long)
    int score;              // absolute score (positive = White advantage)
};

class Search {

    Board& b;
    Generate& g;

public:
    Search(Board& b, Generate& g) : b(b), g(g) {}

    // Main search entry (NegaMax)
    int search(std::array<int8_t, 64> board, int depth, bool white);

    // Search that also collects the principal variation (best line)
    int searchPV(std::array<int8_t, 64> board, int depth, bool white, std::vector<Gen>& pv);

    // Returns top N moves sorted best-first for the given side
    std::vector<ScoredMove> getTopMoves(std::array<int8_t, 64> board, int depth, bool white, int topN);
};

