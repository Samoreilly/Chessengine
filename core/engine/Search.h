#pragma once

#include <array>
#include <cstdint>
#include <vector>
#include <string>
#include <chrono>
#include "../board/Board.h"
#include "../board/Generate.h"
#include "../engine/Evaluation.h"

struct ScoredMove {
    std::vector<Gen> line;
    int score;
};

// Killer moves: non-capture moves that caused beta cutoffs
static const int MAX_DEPTH = 64;

class Search {

    Board& b;
    Generate& g;

    // Killer moves: 2 per ply (non-captures that caused cutoffs)
    Gen killers[MAX_DEPTH][2];

    // History heuristic: indexed by [side][from][to]
    int history[2][64][64];

    // Search statistics
    long long nodesSearched;

    // MVV-LVA + killer + history move ordering
    void orderMoves(std::vector<Gen>& moves, int ply, bool white);

    // Quiescence search: resolve captures at leaf nodes
    int quiesce(std::array<int8_t, 64> board, bool white, int alpha, int beta, Evaluation& eval);

    // Internal search with ply tracking
    int alphabeta(std::array<int8_t, 64> board, int depth, int ply, bool white, int alpha, int beta, std::vector<Gen>* pv, Evaluation& eval);

    // Store a killer move
    void storeKiller(int ply, const Gen& move);

    // Check if move is a killer
    bool isKiller(int ply, const Gen& move) const;

public:
    Search(Board& b, Generate& g) : b(b), g(g), nodesSearched(0) {
        clearHistory();
    }

    void clearHistory() {
        for (int s = 0; s < 2; s++)
            for (int f = 0; f < 64; f++)
                for (int t = 0; t < 64; t++)
                    history[s][f][t] = 0;
        for (int d = 0; d < MAX_DEPTH; d++) {
            killers[d][0] = {0, 0, 0, 0, false};
            killers[d][1] = {0, 0, 0, 0, false};
        }
    }

    // Main entry: iterative deepening search
    int search(std::array<int8_t, 64> board, int depth, bool white, int alpha, int beta);

    // PV search for top-move display
    int searchPV(std::array<int8_t, 64> board, int depth, bool white, int alpha, int beta, std::vector<Gen>& pv);

    // Returns top N moves sorted best-first
    std::vector<ScoredMove> getTopMoves(std::array<int8_t, 64> board, int depth, bool white, int topN);

    long long getNodesSearched() const { return nodesSearched; }
};
