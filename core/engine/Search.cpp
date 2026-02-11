#include "Search.h"
#include "../board/Generate.h"
#include "../board/GenerateCheck.h"
#include "Evaluation.h"
#include <limits>
#include <optional>
#include <algorithm>

static const int CHECKMATE_SCORE = 100000;

int Search::search(std::array<int8_t, 64> board, int depth, bool white) {
    if (depth == 0) {
        Evaluation eval;
        int score = eval.evaluation(board);
        return white ? score : -score;
    }
    
    std::vector<Gen> moves = g.generate(board, white);

    int bestScore = -std::numeric_limits<int>::max();
    bool hasLegalMove = false;

    for (auto move : moves) {
        std::optional<std::array<int8_t, 64>> nextBoard = g.makeMove(board, white, move);
        if (!nextBoard.has_value()) continue;

        hasLegalMove = true;
        int score = -search(nextBoard.value(), depth - 1, !white);
        
        if (score > bestScore) {
            bestScore = score;
        }
    }

    if (!hasLegalMove) {
        GenerateCheck gc;
        if (gc.isCheck(board, white)) {
            return -CHECKMATE_SCORE + (4 - depth);
        } else {
            return 0;
        }
    }

    return bestScore;
}

// Search that also tracks the principal variation (best move sequence)
int Search::searchPV(std::array<int8_t, 64> board, int depth, bool white, std::vector<Gen>& pv) {
    pv.clear();

    if (depth == 0) {
        Evaluation eval;
        int score = eval.evaluation(board);
        return white ? score : -score;
    }
    
    std::vector<Gen> moves = g.generate(board, white);

    int bestScore = -std::numeric_limits<int>::max();
    bool hasLegalMove = false;

    for (auto move : moves) {
        std::optional<std::array<int8_t, 64>> nextBoard = g.makeMove(board, white, move);
        if (!nextBoard.has_value()) continue;

        hasLegalMove = true;

        std::vector<Gen> childPV;
        int score = -searchPV(nextBoard.value(), depth - 1, !white, childPV);
        
        if (score > bestScore) {
            bestScore = score;
            pv.clear();
            pv.push_back(move);
            pv.insert(pv.end(), childPV.begin(), childPV.end());
        }
    }

    if (!hasLegalMove) {
        GenerateCheck gc;
        if (gc.isCheck(board, white)) {
            return -CHECKMATE_SCORE + (4 - depth);
        } else {
            return 0;
        }
    }

    return bestScore;
}

std::vector<ScoredMove> Search::getTopMoves(std::array<int8_t, 64> board, int depth, bool white, int topN) {
    std::vector<ScoredMove> results;

    std::vector<Gen> moves = g.generate(board, white);

    for (auto move : moves) {
        std::optional<std::array<int8_t, 64>> nextBoard = g.makeMove(board, white, move);
        if (!nextBoard.has_value()) continue;

        // Get score + PV for the continuation after this move
        std::vector<Gen> childPV;
        int score = -searchPV(nextBoard.value(), depth - 1, !white, childPV);

        // Convert to absolute score (positive = White advantage)
        int absScore = white ? score : -score;

        ScoredMove sm;
        sm.score = absScore;
        // Build the full line: this move + the continuation
        sm.line.push_back(move);
        sm.line.insert(sm.line.end(), childPV.begin(), childPV.end());

        results.push_back(sm);
    }

    // Sort: best for the current player first
    std::sort(results.begin(), results.end(), [&](const ScoredMove& a, const ScoredMove& b) {
        if (white) return a.score > b.score;
        else return a.score < b.score;
    });

    if ((int)results.size() > topN) {
        results.resize(topN);
    }

    return results;
}
