#include "Board.h"

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
    std::array<int8_t, 64>& board;
    std::vector<Gen> moves;
   
    std::vector<Gen> generateWhite();
    std::vector<Gen> generateBlack();

    int bishop[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};

    //total pieces per color - using this to exit early
    const int totalPieces = 16;
public:

    Generate(Board& b) : b(b), board(b.getBoard()) {}

    void clearGen() {
        moves.clear();
    }

    std::vector<Gen> generate(bool white);
    void directGen(int piece);

    void generateRookMoves(int pos);
    void generateBishopMoves(int pos);
    void generateKnightMoves(int pos);
    void generateQueenMoves(int pos);
    void generatePawnMoves(int pos);
    void generateKingMoves(int pos);
    
};
