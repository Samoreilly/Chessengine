
#include "Evaluation.h"
#include <cmath>

static inline bool onBoard(int r, int c) {
    return r >= 0 && r < 8 && c >= 0 && c < 8;
}

int Evaluation::materialValue(PieceType type) {
    switch (type) {
        case PieceType::PAWN:   return 100;
        case PieceType::KNIGHT: return 320;
        case PieceType::BISHOP: return 330;
        case PieceType::ROOK:   return 500;
        case PieceType::QUEEN:  return 900;
        case PieceType::KING:   return 0;
        default: return 0;
    }
}

// --------------------------------------------------------
// Is square `idx` attacked by a piece of color `byWhite`?
// --------------------------------------------------------
bool Evaluation::isAttacked(const std::array<int8_t, 64>& board, int idx, bool byWhite) {
    int r = idx / 8;
    int c = idx % 8;

    // 1. Pawn attacks
    if (byWhite) {
        for (int dc : {-1, 1}) {
            if (onBoard(r - 1, c + dc)) {
                int p = board[(r - 1) * 8 + (c + dc)];
                if (p > 0 && std::abs(p) == (int)PieceType::PAWN) return true;
            }
        }
    } else {
        for (int dc : {-1, 1}) {
            if (onBoard(r + 1, c + dc)) {
                int p = board[(r + 1) * 8 + (c + dc)];
                if (p < 0 && std::abs(p) == (int)PieceType::PAWN) return true;
            }
        }
    }

    // 2. Knight attacks
    static const int kDr[] = {-2, -1, 1, 2, 2, 1, -1, -2};
    static const int kDc[] = {1, 2, 2, 1, -1, -2, -2, -1};
    for (int k = 0; k < 8; k++) {
        int nr = r + kDr[k], nc = c + kDc[k];
        if (onBoard(nr, nc)) {
            int p = board[nr * 8 + nc];
            if (byWhite && p > 0 && std::abs(p) == (int)PieceType::KNIGHT) return true;
            if (!byWhite && p < 0 && std::abs(p) == (int)PieceType::KNIGHT) return true;
        }
    }

    // 3. Rook/Queen (straight lines)
    static const int sDr[] = {-1, 1, 0, 0};
    static const int sDc[] = {0, 0, -1, 1};
    for (int k = 0; k < 4; k++) {
        for (int d = 1; d < 8; d++) {
            int nr = r + sDr[k] * d, nc = c + sDc[k] * d;
            if (!onBoard(nr, nc)) break;
            int p = board[nr * 8 + nc];
            if (p != 0) {
                int t = std::abs(p);
                if (t == (int)PieceType::ROOK || t == (int)PieceType::QUEEN) {
                    if (byWhite && p > 0) return true;
                    if (!byWhite && p < 0) return true;
                }
                break;
            }
        }
    }

    // 4. Bishop/Queen (diagonals)
    static const int bDr[] = {-1, -1, 1, 1};
    static const int bDc[] = {-1, 1, -1, 1};
    for (int k = 0; k < 4; k++) {
        for (int d = 1; d < 8; d++) {
            int nr = r + bDr[k] * d, nc = c + bDc[k] * d;
            if (!onBoard(nr, nc)) break;
            int p = board[nr * 8 + nc];
            if (p != 0) {
                int t = std::abs(p);
                if (t == (int)PieceType::BISHOP || t == (int)PieceType::QUEEN) {
                    if (byWhite && p > 0) return true;
                    if (!byWhite && p < 0) return true;
                }
                break;
            }
        }
    }

    // 5. King attacks (adjacent)
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) continue;
            int nr = r + dr, nc = c + dc;
            if (onBoard(nr, nc)) {
                int p = board[nr * 8 + nc];
                if (std::abs(p) == (int)PieceType::KING) {
                    if (byWhite && p > 0) return true;
                    if (!byWhite && p < 0) return true;
                }
            }
        }
    }

    return false;
}

// --------------------------------------------------------
// Mobility: count pseudo-legal squares
// --------------------------------------------------------
int Evaluation::evaluateMobility(const std::array<int8_t, 64>& board, int idx, PieceType type, bool isWhite) {
    int r = idx / 8;
    int c = idx % 8;
    int moves = 0;

    if (type == PieceType::KNIGHT) {
        static const int kDr[] = {-2, -1, 1, 2, 2, 1, -1, -2};
        static const int kDc[] = {1, 2, 2, 1, -1, -2, -2, -1};
        for (int k = 0; k < 8; k++) {
            int nr = r + kDr[k], nc = c + kDc[k];
            if (onBoard(nr, nc)) {
                int p = board[nr * 8 + nc];
                if (p == 0 || (isWhite && p < 0) || (!isWhite && p > 0)) moves++;
            }
        }
    } else if (type == PieceType::BISHOP || type == PieceType::ROOK || type == PieceType::QUEEN) {
        static const int bDirs[4][2] = {{-1,-1},{-1,1},{1,-1},{1,1}};
        static const int rDirs[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};

        auto slide = [&](const int dirs[][2], int count) {
            for (int i = 0; i < count; i++) {
                for (int d = 1; d < 8; d++) {
                    int nr = r + dirs[i][0] * d, nc = c + dirs[i][1] * d;
                    if (!onBoard(nr, nc)) break;
                    int p = board[nr * 8 + nc];
                    if (p == 0) { moves++; }
                    else {
                        if ((isWhite && p < 0) || (!isWhite && p > 0)) moves++;
                        break;
                    }
                }
            }
        };

        if (type == PieceType::BISHOP) slide(bDirs, 4);
        else if (type == PieceType::ROOK) slide(rDirs, 4);
        else { slide(bDirs, 4); slide(rDirs, 4); }
    }

    return moves * 3;
}

// --------------------------------------------------------
// Pawn structure: isolated, doubled, passed, connected
// --------------------------------------------------------
int Evaluation::evaluatePawnStructure(const std::array<int8_t, 64>& board, int idx, bool isWhite) {
    int r = idx / 8;
    int c = idx % 8;
    int score = 0;
    int pawnVal = (int)PieceType::PAWN;

    // 1. Isolated pawn
    bool isolated = true;
    for (int dc : {-1, 1}) {
        int nc = c + dc;
        if (nc < 0 || nc >= 8) continue;
        for (int nr = 0; nr < 8; nr++) {
            int p = board[nr * 8 + nc];
            if ((isWhite && p > 0 && std::abs(p) == pawnVal) ||
                (!isWhite && p < 0 && std::abs(p) == pawnVal)) {
                isolated = false;
                break;
            }
        }
        if (!isolated) break;
    }
    if (isolated) score -= 15;

    // 2. Doubled pawn
    for (int nr = 0; nr < 8; nr++) {
        if (nr == r) continue;
        int p = board[nr * 8 + c];
        if ((isWhite && p > 0 && std::abs(p) == pawnVal) ||
            (!isWhite && p < 0 && std::abs(p) == pawnVal)) {
            score -= 10;
            break;
        }
    }

    // 3. Connected pawn — bonus for pawns supporting each other diagonally
    int supportRow = isWhite ? r - 1 : r + 1;
    if (supportRow >= 0 && supportRow < 8) {
        for (int dc : {-1, 1}) {
            int nc = c + dc;
            if (nc < 0 || nc >= 8) continue;
            int p = board[supportRow * 8 + nc];
            if ((isWhite && p > 0 && std::abs(p) == pawnVal) ||
                (!isWhite && p < 0 && std::abs(p) == pawnVal)) {
                score += 7;
            }
        }
    }

    // 4. Passed pawn
    bool passed = true;
    if (isWhite) {
        for (int nr = r + 1; nr < 8 && passed; nr++) {
            for (int dc = -1; dc <= 1; dc++) {
                int nc = c + dc;
                if (nc < 0 || nc >= 8) continue;
                int p = board[nr * 8 + nc];
                if (p < 0 && std::abs(p) == pawnVal) { passed = false; break; }
            }
        }
    } else {
        for (int nr = r - 1; nr >= 0 && passed; nr--) {
            for (int dc = -1; dc <= 1; dc++) {
                int nc = c + dc;
                if (nc < 0 || nc >= 8) continue;
                int p = board[nr * 8 + nc];
                if (p > 0 && std::abs(p) == pawnVal) { passed = false; break; }
            }
        }
    }
    if (passed) {
        int rank = isWhite ? r : (7 - r);
        // Quadratic bonus: more advanced = much more valuable
        score += rank * rank * 3;
    }

    return score;
}

// --------------------------------------------------------
// King safety: pawn shield + open file penalty
// --------------------------------------------------------
int Evaluation::evaluateKingSafety(const std::array<int8_t, 64>& board, int idx, bool isWhite) {
    int r = idx / 8;
    int c = idx % 8;
    int score = 0;
    int pawnVal = (int)PieceType::PAWN;

    // Pawn shield (row in front of king)
    int shieldRow = isWhite ? r + 1 : r - 1;
    if (shieldRow >= 0 && shieldRow < 8) {
        for (int dc = -1; dc <= 1; dc++) {
            int nc = c + dc;
            if (nc < 0 || nc >= 8) continue;
            int p = board[shieldRow * 8 + nc];
            if ((isWhite && p > 0 && std::abs(p) == pawnVal) ||
                (!isWhite && p < 0 && std::abs(p) == pawnVal)) {
                score += (dc == 0) ? 15 : 10;
            } else {
                // Penalty for missing shield pawn
                score -= 10;
            }
        }
    }

    // Open file near king penalty
    for (int dc = -1; dc <= 1; dc++) {
        int nc = c + dc;
        if (nc < 0 || nc >= 8) continue;
        bool hasFriendlyPawn = false;
        for (int nr = 0; nr < 8; nr++) {
            int p = board[nr * 8 + nc];
            if ((isWhite && p > 0 && std::abs(p) == pawnVal) ||
                (!isWhite && p < 0 && std::abs(p) == pawnVal)) {
                hasFriendlyPawn = true;
                break;
            }
        }
        if (!hasFriendlyPawn) score -= 15; // Open file near king = dangerous
    }

    return score;
}

// --------------------------------------------------------
// Threats: penalty for undefended pieces under attack
// --------------------------------------------------------
int Evaluation::evaluateThreats(const std::array<int8_t, 64>& board, int idx, PieceType type, bool isWhite) {
    if (type == PieceType::KING) return 0;

    if (isAttacked(board, idx, !isWhite)) {
        bool defended = isAttacked(board, idx, isWhite);

        if (!defended) {
            // Undefended and attacked: big penalty
            switch (type) {
                case PieceType::QUEEN:  return -60;
                case PieceType::ROOK:   return -40;
                case PieceType::BISHOP: return -25;
                case PieceType::KNIGHT: return -25;
                case PieceType::PAWN:   return -10;
                default: return 0;
            }
        } else {
            // Defended but attacked: smaller penalty
            switch (type) {
                case PieceType::QUEEN:  return -15;
                case PieceType::ROOK:   return -10;
                case PieceType::BISHOP: return -5;
                case PieceType::KNIGHT: return -5;
                default: return 0;
            }
        }
    }
    return 0;
}

// --------------------------------------------------------
// Rook on open/semi-open file
// --------------------------------------------------------
int Evaluation::evaluateRookFile(const std::array<int8_t, 64>& board, int idx, bool isWhite) {
    int c = idx % 8;
    int pawnVal = (int)PieceType::PAWN;

    bool friendlyPawn = false;
    bool enemyPawn = false;

    for (int r = 0; r < 8; r++) {
        int p = board[r * 8 + c];
        if (std::abs(p) == pawnVal) {
            if ((isWhite && p > 0) || (!isWhite && p < 0)) friendlyPawn = true;
            else enemyPawn = true;
        }
    }

    if (!friendlyPawn && !enemyPawn) return 25;  // Open file
    if (!friendlyPawn) return 15;                 // Semi-open file
    return 0;
}

// --------------------------------------------------------
// Bishop pair bonus
// --------------------------------------------------------
int Evaluation::evaluateBishopPair(const std::array<int8_t, 64>& board, bool isWhite) {
    int bishopCount = 0;
    int bishopVal = (int)PieceType::BISHOP;

    for (int i = 0; i < 64; i++) {
        int p = board[i];
        if (std::abs(p) == bishopVal) {
            if ((isWhite && p > 0) || (!isWhite && p < 0)) bishopCount++;
        }
    }

    return (bishopCount >= 2) ? 30 : 0;
}

// --------------------------------------------------------
// Center control: bonus for controlling e4, d4, e5, d5
// --------------------------------------------------------
int Evaluation::evaluateCenterControl(const std::array<int8_t, 64>& board, bool isWhite) {
    // Center squares: d4(27), e4(28), d5(35), e5(36)
    static const int center[] = {27, 28, 35, 36};
    int score = 0;

    for (int sq : center) {
        int p = board[sq];
        // Piece occupying center
        if (p != 0) {
            if ((isWhite && p > 0) || (!isWhite && p < 0)) {
                score += 10;
            }
        }
        // Pawn attacking center
        if (isAttacked(board, sq, isWhite)) {
            score += 5;
        }
    }

    return score;
}

// --------------------------------------------------------
// Per-square score
// --------------------------------------------------------
int Evaluation::getScore(const std::array<int8_t, 64>& board, int idx) {

    int piece = board[idx];
    if (piece == 0) return 0;

    bool isWhite = piece > 0;
    int row = idx / 8;
    int col = idx % 8;

    // PSTs are oriented top-down: row 0 = rank 8, row 7 = rank 1.
    int evalRow = isWhite ? 7 - row : row;

    PieceType type = static_cast<PieceType>(std::abs(piece));

    // 1. Material
    int material = materialValue(type);

    // 2. Positional from PST
    int positional = 0;
    switch (type) {
        case PieceType::PAWN:   positional = pawnEval[evalRow][col]; break;
        case PieceType::KNIGHT: positional = knightEval[evalRow][col]; break;
        case PieceType::BISHOP: positional = bishopEval[evalRow][col]; break;
        case PieceType::ROOK:   positional = rookEval[evalRow][col]; break;
        case PieceType::QUEEN:  positional = queenEval[evalRow][col]; break;
        case PieceType::KING:   positional = kingEval[evalRow][col]; break;
        default: break;
    }

    // 3. Mobility
    int mobility = 0;
    if (type == PieceType::KNIGHT || type == PieceType::BISHOP ||
        type == PieceType::ROOK   || type == PieceType::QUEEN) {
        mobility = evaluateMobility(board, idx, type, isWhite);
    }

    // 4. Pawn structure
    int structure = 0;
    if (type == PieceType::PAWN) {
        structure = evaluatePawnStructure(board, idx, isWhite);
    }

    // 5. King safety
    int kingSafety = 0;
    if (type == PieceType::KING) {
        kingSafety = evaluateKingSafety(board, idx, isWhite);
    }

    // 6. Threats
    int threats = evaluateThreats(board, idx, type, isWhite);

    // 7. Rook on open file
    int rookFile = 0;
    if (type == PieceType::ROOK) {
        rookFile = evaluateRookFile(board, idx, isWhite);
    }

    int total = material + positional + mobility + structure + kingSafety + threats + rookFile;

    return isWhite ? total : -total;
}

// --------------------------------------------------------
// Full board evaluation
// --------------------------------------------------------
int Evaluation::evaluation(const std::array<std::int8_t, 64>& board) {
    int score = 0;
    for (int i = 0; i < 64; ++i) {
        score += getScore(board, i);
    }

    // Global bonuses (computed once, not per piece)
    // Bishop pair
    score += evaluateBishopPair(board, true);
    score -= evaluateBishopPair(board, false);

    // Center control
    score += evaluateCenterControl(board, true);
    score -= evaluateCenterControl(board, false);

    // Tempo bonus: small advantage for side to move (applied in search)
    score += 10; // White gets a small tempo bonus in static eval

    return score;
}
