
#include "Evaluation.h"
#include <cmath>

// Helper to check board boundaries
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
        case PieceType::KING:   return 0; // Both sides have a king, don't count material
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
    // White pawns move UP (row increases). A white pawn at (r-1, c±1) attacks (r,c).
    // Black pawns move DOWN (row decreases). A black pawn at (r+1, c±1) attacks (r,c).
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

    // 5. King attacks (adjacent squares)
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
// Mobility: count pseudo-legal squares a piece can reach
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
        // Build direction list based on piece type
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
        else { slide(bDirs, 4); slide(rDirs, 4); } // Queen
    }

    // 3 centipawns per legal square — mild bonus
    return moves * 3;
}

// --------------------------------------------------------
// Pawn structure: isolated, doubled, passed
// --------------------------------------------------------
int Evaluation::evaluatePawnStructure(const std::array<int8_t, 64>& board, int idx, bool isWhite) {
    int r = idx / 8;
    int c = idx % 8;
    int score = 0;
    int pawnVal = (int)PieceType::PAWN;

    // 1. Isolated pawn — no friendly pawn on adjacent files
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

    // 2. Doubled pawn — another friendly pawn on same file
    for (int nr = 0; nr < 8; nr++) {
        if (nr == r) continue;
        int p = board[nr * 8 + c];
        if ((isWhite && p > 0 && std::abs(p) == pawnVal) ||
            (!isWhite && p < 0 && std::abs(p) == pawnVal)) {
            score -= 10;
            break;
        }
    }

    // 3. Passed pawn — no enemy pawn ahead on same or adjacent files
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
        int rank = isWhite ? r : (7 - r); // how far advanced (0=back, 7=promotion)
        score += rank * 10;
    }

    return score;
}

// --------------------------------------------------------
// King safety: pawn shield in front of king
// --------------------------------------------------------
int Evaluation::evaluateKingSafety(const std::array<int8_t, 64>& board, int idx, bool isWhite) {
    int r = idx / 8;
    int c = idx % 8;
    int score = 0;
    int pawnVal = (int)PieceType::PAWN;

    int shieldRow = isWhite ? r + 1 : r - 1;
    if (shieldRow < 0 || shieldRow >= 8) return 0;

    for (int dc = -1; dc <= 1; dc++) {
        int nc = c + dc;
        if (nc < 0 || nc >= 8) continue;
        int p = board[shieldRow * 8 + nc];
        if ((isWhite && p > 0 && std::abs(p) == pawnVal) ||
            (!isWhite && p < 0 && std::abs(p) == pawnVal)) {
            score += (dc == 0) ? 15 : 10; // center pawn worth more
        }
    }

    return score;
}

// --------------------------------------------------------
// Threats: penalty if a piece is attacked by the enemy
// --------------------------------------------------------
int Evaluation::evaluateThreats(const std::array<int8_t, 64>& board, int idx, PieceType type, bool isWhite) {
    if (type == PieceType::PAWN || type == PieceType::KING) return 0;

    if (isAttacked(board, idx, !isWhite)) {
        switch (type) {
            case PieceType::QUEEN:  return -40;
            case PieceType::ROOK:   return -25;
            case PieceType::BISHOP: return -15;
            case PieceType::KNIGHT: return -15;
            default: return 0;
        }
    }
    return 0;
}

// --------------------------------------------------------
// Per-square score (material + position + strategy)
// Positive = good for White, negative = good for Black
// --------------------------------------------------------
int Evaluation::getScore(const std::array<int8_t, 64>& board, int idx) {

    int piece = board[idx];
    if (piece == 0) return 0;

    bool isWhite = piece > 0;
    int row = idx / 8;
    int col = idx % 8;

    // Piece-square tables are oriented White = bottom (row 0).
    // For Black, mirror the row.
    int evalRow = isWhite ? row : 7 - row;

    PieceType type = static_cast<PieceType>(std::abs(piece));

    // 1. Positional score from tables
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

    // 2. Material
    int material = materialValue(type);

    // 3. Mobility (knights, bishops, rooks, queens only)
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

    int total = material + positional + mobility + structure + kingSafety + threats;

    return isWhite ? total : -total;
}

// --------------------------------------------------------
// Full board evaluation (positive = White advantage)
// --------------------------------------------------------
int Evaluation::evaluation(const std::array<std::int8_t, 64>& board) {
    int score = 0;
    for (int i = 0; i < 64; ++i) {
        score += getScore(board, i);
    }
    return score;
}
