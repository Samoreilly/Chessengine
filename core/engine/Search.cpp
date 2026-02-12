#include "Search.h"
#include "../board/Generate.h"
#include "../board/GenerateCheck.h"
#include "Evaluation.h"
#include <limits>
#include <optional>
#include <algorithm>
#include <cmath>
#include <iostream>

static const int CHECKMATE_SCORE = 100000;
static const int INF = 2000000;

// ----------------------------------------------------------
// Piece value for MVV-LVA ordering
// ----------------------------------------------------------
static int pieceOrderValue(int piece) {
    switch (std::abs(piece)) {
        case 1: return 100;   // pawn
        case 2: return 500;   // rook
        case 3: return 330;   // bishop
        case 4: return 320;   // knight
        case 5: return 900;   // queen
        case 6: return 20000; // king
        default: return 0;
    }
}

// ----------------------------------------------------------
// Killer move tracking
// ----------------------------------------------------------
void Search::storeKiller(int ply, const Gen& move) {
    if (ply >= MAX_DEPTH) return;
    // Don't store captures as killers
    if (move.pieceTaken != 0) return;
    // Shift: slot 1 = old slot 0, slot 0 = new killer
    if (killers[ply][0].from != move.from || killers[ply][0].to != move.to) {
        killers[ply][1] = killers[ply][0];
        killers[ply][0] = move;
    }
}

bool Search::isKiller(int ply, const Gen& move) const {
    if (ply >= MAX_DEPTH) return false;
    return (killers[ply][0].from == move.from && killers[ply][0].to == move.to) ||
           (killers[ply][1].from == move.from && killers[ply][1].to == move.to);
}

// ----------------------------------------------------------
// Move ordering: captures (MVV-LVA) > killers > history
// ----------------------------------------------------------
void Search::orderMoves(std::vector<Gen>& moves, int ply, bool white) {
    int side = white ? 0 : 1;

    std::sort(moves.begin(), moves.end(), [&](const Gen& a, const Gen& b) {
        // Score each move for ordering
        auto score = [&](const Gen& m) -> int {
            if (m.pieceTaken != 0) {
                // MVV-LVA: victim value * 10 - attacker value
                // Ensures captures are always first, sorted by best capture
                return 10000000 + pieceOrderValue(m.pieceTaken) * 10 - pieceOrderValue(m.piece);
            }
            if (isKiller(ply, m)) {
                return 5000000; // Below captures, above quiet moves
            }
            // History heuristic for quiet moves
            return history[side][m.from][m.to];
        };
        return score(a) > score(b);
    });
}

// ----------------------------------------------------------
// Quiescence search: keep searching captures until quiet
// ----------------------------------------------------------
int Search::quiesce(std::array<int8_t, 64> board, bool white, int alpha, int beta, Evaluation& eval) {
    nodesSearched++;

    int standPat = eval.evaluation(board);
    standPat = white ? standPat : -standPat;

    if (standPat >= beta) return beta;
    if (standPat > alpha) alpha = standPat;

    // Delta pruning: if even capturing a queen can't raise alpha, skip
    if (standPat + 1000 < alpha) return alpha;

    std::vector<Gen> moves = g.generate(board, white);
    orderMoves(moves, 0, white);

    for (auto& move : moves) {
        if (move.pieceTaken == 0) continue; // Only captures

        // SEE-like pruning: skip captures of higher value pieces by lower value ones
        // (don't search pawn captures queen if we're way behind)

        std::optional<std::array<int8_t, 64>> nextBoard = g.makeMove(board, white, move);
        if (!nextBoard.has_value()) continue;

        int score = -quiesce(nextBoard.value(), !white, -beta, -alpha, eval);

        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }

    return alpha;
}

// ----------------------------------------------------------
// Core alpha-beta with all pruning techniques
// ----------------------------------------------------------
int Search::alphabeta(std::array<int8_t, 64> board, int depth, int ply, bool white, int alpha, int beta, std::vector<Gen>* pv, Evaluation& eval) {
    if (pv) pv->clear();

    // Leaf node: quiescence search
    if (depth <= 0) {
        return quiesce(board, white, alpha, beta, eval);
    }

    nodesSearched++;

    GenerateCheck gc;
    bool inCheck = gc.isCheck(board, white);

    // Check extension: extend search by 1 ply when in check
    if (inCheck) depth++;

    // Reverse Futility Pruning
    if (!inCheck && depth <= 3 && ply > 0) {
        int evalScore = eval.evaluation(board);
        evalScore = white ? evalScore : -evalScore;
        
        int margin = 120 * depth;
        if (evalScore - margin >= beta) {
            return evalScore - margin; 
        }
    }

    // Null move pruning: skip our turn (only if not in check, has pieces)
    if (!inCheck && depth >= 3 && ply > 0) {
        // Search with reduced depth after passing
        int nullScore = -alphabeta(board, depth - 3, ply + 1, !white, -beta, -beta + 1, nullptr, eval);
        if (nullScore >= beta) return beta;
    }

    std::vector<Gen> moves = g.generate(board, white);
    orderMoves(moves, ply, white);

    int bestScore = -INF;
    bool hasLegalMove = false;
    int movesSearched = 0;

    for (auto& move : moves) {
        std::optional<std::array<int8_t, 64>> nextBoard = g.makeMove(board, white, move);
        if (!nextBoard.has_value()) continue;

        hasLegalMove = true;
        movesSearched++;

        int score;
        std::vector<Gen> childPV;

        // Late move reductions (LMR): reduce depth for late quiet moves
        if (movesSearched > 3 && depth >= 3 && !inCheck && move.pieceTaken == 0) {
            // Reduced search
            score = -alphabeta(nextBoard.value(), depth - 2, ply + 1, !white, -alpha - 1, -alpha, nullptr, eval);
            // If it improves alpha, re-search at full depth
            if (score > alpha) {
                score = -alphabeta(nextBoard.value(), depth - 1, ply + 1, !white, -beta, -alpha, pv ? &childPV : nullptr, eval);
            }
        } else {
            score = -alphabeta(nextBoard.value(), depth - 1, ply + 1, !white, -beta, -alpha, pv ? &childPV : nullptr, eval);
        }

        if (score > bestScore) bestScore = score;

        if (score >= beta) {
            // Beta cutoff
            storeKiller(ply, move);
            // Update history for quiet moves
            if (move.pieceTaken == 0) {
                int side = white ? 0 : 1;
                history[side][move.from][move.to] += depth * depth;
            }
            return beta;
        }

        if (score > alpha) {
            alpha = score;
            if (pv) {
                pv->clear();
                pv->push_back(move);
                pv->insert(pv->end(), childPV.begin(), childPV.end());
            }
        }
    }

    if (!hasLegalMove) {
        if (inCheck) {
            return -CHECKMATE_SCORE + ply; // Prefer faster checkmates
        }
        return 0; // Stalemate
    }

    return alpha;
}

// ----------------------------------------------------------
// Iterative deepening wrapper
// ----------------------------------------------------------
int Search::search(std::array<int8_t, 64> board, int depth, bool white, int alpha, int beta) {
    nodesSearched = 0;
    int score = 0;
    Evaluation eval;

    // Iterative deepening: search depth 1, 2, ... up to target
    for (int d = 1; d <= depth; d++) {
        score = alphabeta(board, d, 0, white, -INF, INF, nullptr, eval);
    }

    return score;
}

// ----------------------------------------------------------
// PV search for display
// ----------------------------------------------------------
int Search::searchPV(std::array<int8_t, 64> board, int depth, bool white, int alpha, int beta, std::vector<Gen>& pv) {
    nodesSearched = 0;
    int score = 0;
    Evaluation eval;

    // With iterative deepening, each iteration informs move ordering
    for (int d = 1; d <= depth; d++) {
        std::vector<Gen> iterPV;
        score = alphabeta(board, d, 0, white, -INF, INF, &iterPV, eval);
        if (d == depth) pv = iterPV; // Keep the last iteration's PV
    }

    return score;
}

// ----------------------------------------------------------
// Top N moves for display
// ----------------------------------------------------------
std::vector<ScoredMove> Search::getTopMoves(std::array<int8_t, 64> board, int depth, bool white, int topN) {
    std::vector<ScoredMove> results;
    std::vector<Gen> moves = g.generate(board, white);
    orderMoves(moves, 0, white);
    Evaluation eval;

    for (auto& move : moves) {
        std::optional<std::array<int8_t, 64>> nextBoard = g.makeMove(board, white, move);
        if (!nextBoard.has_value()) continue;

        std::vector<Gen> childPV;
        // Search directly at (depth - 1)
        int searchDepth = depth - 1 < 1 ? 1 : depth - 1;
        int score = -alphabeta(nextBoard.value(), searchDepth, 1, !white, -INF, INF, &childPV, eval);

        int absScore = white ? score : -score;

        ScoredMove sm;
        sm.score = absScore;
        sm.line.push_back(move);
        sm.line.insert(sm.line.end(), childPV.begin(), childPV.end());

        results.push_back(sm);
    }

    std::sort(results.begin(), results.end(), [&](const ScoredMove& a, const ScoredMove& b) {
        if (white) return a.score > b.score;
        else return a.score < b.score;
    });

    if ((int)results.size() > topN) {
        results.resize(topN);
    }

    return results;
}
