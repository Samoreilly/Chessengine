

#pragma once

#include <iostream>
#include <array>
#include <cstdint>
#include <iomanip>
#include <optional>

//0 is empty square
//White Pawn - 1, Black Pawn - -1
//White Rook - 2, Black Rook - -2
//White Bishop - 3, Black Bishop - -3
//White Knight - 4, Black Knight - -4
//White Queen - 5, Black Queen - -5
//White King - 6, Black King - -6

struct LastMove {
    int to;
    int from;
    int piece;
    std::optional<int8_t> pieceTaken;
};

class Board {

    //flat 1d array representing 2d array
    //white - 16 ints, black - last 16 ints
    //Positives is white, Negatives is black
    
    std::array<int8_t, 64> board;
    LastMove lastMove;
    int whiteKing {4};
    int blackKing {60};

    // Castling rights
    bool whiteKingSide = true;
    bool whiteQueenSide = true;
    bool blackKingSide = true;
    bool blackQueenSide = true;

public:

     
    //state for white/black turn
    bool white;

    //used to generate moves and for check scanning
    int dirs[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    int diag[4][2] = {{-1, 1}, {-1, -1}, {1, 1}, {1, -1}};
    int knight[8][2] = {{2, 1}, {2, -1}, {-2, 1}, {-2, -1}, {1, 2}, {1, -2}, {-1, 2}, {-1, -2}};
    int pawn[2][2] = {{1, 1}, {1, -1}};


    Board() : lastMove{-1, -1, 0} {
    


board = {
    2,  4,  3,  5,  6,  3,  4,  2,   // Rank 1: White back rank
    1,  1,  1,  1,  1,  1,  1,  1,   // Rank 2: White pawns
    0,  0,  0,  0,  0,  0,  0,  0,   // Rank 3
    0,  0,  0,  0,  0,  0,  0,  0,   // Rank 4
    0,  0,  0,  0,  0,  0,  0,  0,   // Rank 5
    0,  0,  0,  0,  0,  0,  0,  0,   // Rank 6
   -1, -1, -1, -1, -1, -1, -1, -1,   // Rank 7: Black pawns
   -2, -4, -3, -5, -6, -3, -4, -2    // Rank 8: Black back rank
};



        white = true;

    }

    void nextTurn() {
        white = !white;
    }

    void setTurn(bool& turn) {
        white = turn;
    }

    bool& getTurn() {
        return white;
    }

    bool isWhiteTurn() {
        return white;
    }

    bool isBlackTurn() {
        return !white;
    }

    void uKingPos(int8_t pos, bool white) {
        if(white)whiteKing = pos;
        else blackKing = pos;
    }

    int8_t wKingPos() {
        return whiteKing;
    }

    int bKingPos() {
        return blackKing;
    }

    std::array<int8_t, 64>& getBoard() {
        return board;
    }

    LastMove& getLastMove() {
        return lastMove;
    }

    // Castling rights accessors
    bool canCastleKingSide(bool isWhite) const { return isWhite ? whiteKingSide : blackKingSide; }
    bool canCastleQueenSide(bool isWhite) const { return isWhite ? whiteQueenSide : blackQueenSide; }

    // Call when king or rook moves/captured
    void revokeCastling(int idx) {
        if (idx == 4) { whiteKingSide = false; whiteQueenSide = false; }       // e1 king
        if (idx == 0) whiteQueenSide = false;   // a1 rook
        if (idx == 7) whiteKingSide = false;    // h1 rook
        if (idx == 60) { blackKingSide = false; blackQueenSide = false; }      // e8 king
        if (idx == 56) blackQueenSide = false;  // a8 rook
        if (idx == 63) blackKingSide = false;   // h8 rook
    }

    void printBoard() {

        // ANSI colors (Classic Walnut Theme - High Contrast)
        const char* RESET    = "\033[0m";
        const char* LIGHT_SQ = "\033[48;5;180m";  // Light Tan
        const char* DARK_SQ  = "\033[48;5;94m";   // Rich Walnut Brown
        const char* WHITE_PC = "\033[38;5;231;1m"; // Bold Pure White
        const char* BLACK_PC = "\033[38;5;232;1m"; // Bold Solid Black

        // We use the solid-filled Unicode icons for both sides
        // because they are much more visible and "bigger" in terminals.
        auto pieceChar = [](int8_t p) -> const char* {
            switch (std::abs(p)) {
                case  1: return "♟";
                case  2: return "♜";
                case  3: return "♝"; // Bishop
                case  4: return "♞"; // Knight
                case  5: return "♛";
                case  6: return "♚";
                default: return " ";
            }
        };
        
        // Helper to robustly get the icon
        auto getPieceIcon = [&](int8_t p) -> const char* {
            return pieceChar(p);
        };

        std::cout << "\n";
        // Top border (restored to 7 chars wide)
        std::cout << "      ┌───────┬───────┬───────┬───────┬───────┬───────┬───────┬───────┐\n";

        for (int r = 7; r >= 0; --r) {
            for (int line = 0; line < 3; ++line) {
                if (line == 1) {
                    std::cout << "  " << (r + 1) << "   │";
                } else {
                    std::cout << "      │";
                }

                for (int c = 0; c < 8; ++c) {
                    int index = r * 8 + c;
                    int8_t p = board.at(index);
                    bool lightSquare = (r + c) % 2 == 1;
                    const char* bg = lightSquare ? LIGHT_SQ : DARK_SQ;
                    const char* fg = (p > 0) ? WHITE_PC : BLACK_PC;

                    if (line == 1 && p != 0) {
                        // Center line with the solid piece icon (restored centering)
                        std::cout << bg << fg << "   " << getPieceIcon(p) << "   " << RESET << "│";
                    } else {
                        // Padding lines
                        std::cout << bg << "       " << RESET << "│";
                    }
                }
                std::cout << "\n";
            }

            if (r > 0) {
                std::cout << "      ├───────┼───────┼───────┼───────┼───────┼───────┼───────┼───────┤\n";
            }
        }

        // Bottom border
        std::cout << "      └───────┴───────┴───────┴───────┴───────┴───────┴───────┴───────┘\n";
        std::cout << "          a       b       c       d       e       f       g       h\n\n";
    }
    

};
