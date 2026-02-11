#pragma once

#include "Board.h"
#include <cstdint>
#include <vector>

struct Gen {
    int from;
    int to;
    int piece;
    int pieceTaken; // 0 = no piece taken
    bool promotion;
};

class Generate {
 
    Board& b;

    std::vector<Gen> moves;
   
    std::vector<Gen> generateWhite(std::array<int8_t, 64> board);
    std::vector<Gen> generateBlack(std::array<int8_t, 64> board);

    int bishop[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};

    //total pieces per color - using this to exit early
    const int totalPieces = 16;
public:

    Generate(Board& b) : b(b) {}
    Generate();
    void clearGen() {
        moves.clear();
    }

    std::vector<Gen> generate(std::array<int8_t, 64> board, bool white);
    void directGen(std::array<int8_t, 64> board, int piece);

    void generateRookMoves(std::array<int8_t, 64> board, int pos);
    void generateBishopMoves(std::array<int8_t, 64> board, int pos);
    void generateKnightMoves(std::array<int8_t, 64> board, int pos);
    void generateQueenMoves(std::array<int8_t, 64> board, int pos);
    void generatePawnMoves(std::array<int8_t, 64> board, int pos);
    void generateKingMoves(std::array<int8_t, 64> board, int pos);
    

    std::array<int8_t, 64> makeMove(std::array<int8_t, 64> board, Gen& gen) {
        //TODO:
    }

};
