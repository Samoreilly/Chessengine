#include "GenerateCheck.h"

bool GenerateCheck::isCheck(const std::array<int8_t, 64>& board, bool turn) {
    int kingPos = turn ? wKingPos(board) : bKingPos(board);
    return scanRookQueen(board, kingPos) ||
           scanDiagonal(board, kingPos) ||
           scanKnight(board, kingPos) ||
           scanPawn(board, kingPos);
}

bool GenerateCheck::scanRookQueen(const std::array<int8_t, 64>& board, int kingPos) {
    int fromRow = kingPos / 8;
    int fromCol = kingPos % 8;

    for(auto dir : dirs) {
        int r = fromRow + dir[0];
        int c = fromCol + dir[1];

        while(r >= 0 && r < 8 && c >= 0 && c < 8) {
            int idx = r*8 + c;
            int piece = board[idx];

            if(piece != 0) {
                if(isOpponent(board, kingPos, idx) &&
                   (abs(piece) == static_cast<int>(PieceType::ROOK) ||
                    abs(piece) == static_cast<int>(PieceType::QUEEN))) {
                    return true;
                }
                break; // blocked
            }
            r += dir[0]; c += dir[1];
        }
    }
    return false;
}

bool GenerateCheck::scanDiagonal(const std::array<int8_t, 64>& board, int kingPos) {
    int fromRow = kingPos / 8;
    int fromCol = kingPos % 8;

    for(auto dir : diag) {
        int r = fromRow + dir[0];
        int c = fromCol + dir[1];

        while(r >= 0 && r < 8 && c >= 0 && c < 8) {
            int idx = r*8 + c;
            int piece = board[idx];

            if(piece != 0) {
                if(isOpponent(board, kingPos, idx) &&
                   (abs(piece) == static_cast<int>(PieceType::BISHOP) ||
                    abs(piece) == static_cast<int>(PieceType::QUEEN))) {
                    return true;
                }
                break;
            }
            r += dir[0]; c += dir[1];
        }
    }
    return false;
}

bool GenerateCheck::scanKnight(const std::array<int8_t, 64>& board, int kingPos) {
    int fromRow = kingPos / 8;
    int fromCol = kingPos % 8;

    for(auto dir : knight) {
        int r = fromRow + dir[0];
        int c = fromCol + dir[1];

        if(r >=0 && r <8 && c >=0 && c <8) {
            int idx = r*8 + c;
            int piece = board[idx];
            if(piece != 0 && isOpponent(board, kingPos, idx) &&
               abs(piece) == static_cast<int>(PieceType::KNIGHT)) {
                return true;
            }
        }
    }
    return false;
}

bool GenerateCheck::scanPawn(const std::array<int8_t, 64>& board, int kingPos) {
    int row = kingPos / 8;
    int col = kingPos % 8;
    bool kingIsWhite = board[kingPos] > 0;

    int pawnRow = kingIsWhite ? row + 1 : row - 1;
    for(int dc : {-1,1}) {
        int r = pawnRow;
        int c = col + dc;
        if(r >=0 && r <8 && c >=0 && c <8) {
            int idx = r*8 + c;
            int p = board[idx];
            int pawn = static_cast<int>(PieceType::PAWN);
            if(p == (kingIsWhite ? -pawn : pawn)) return true;
        }
    }
    return false;
}

// Dummy implementations — you will need real logic to find king positions
int GenerateCheck::wKingPos(const std::array<int8_t, 64>& board) { /* ... */ return 60; }
int GenerateCheck::bKingPos(const std::array<int8_t, 64>& board) { /* ... */ return 4; }
bool GenerateCheck::isOpponent(const std::array<int8_t, 64>& board, int kingPos, int targetPos) {
    return (board[kingPos] * board[targetPos]) < 0;
}

