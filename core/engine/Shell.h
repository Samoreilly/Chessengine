#pragma once

#include "../board/Board.h"
#include "../board/Piece.h"
#include "../board/Check.h"
#include "../board/Generate.h"
#include "../engine/Search.h"

enum class GameMode { ANALYSIS, VS_AI };

class Shell {

    Board b;
    Piece p{b};
    Check c{b};
    Generate g{b};
    Search s{b, g};

    // Game mode
    GameMode mode = GameMode::ANALYSIS;
    bool humanIsWhite = true; // which side the human plays in AI mode

    // AI makes its move automatically
    bool makeAIMove(bool turn, int depth);

public:

    Shell(bool api = false) : apiMode(api) {}

    bool apiMode = false;

    std::array<int8_t, 64>& board = b.getBoard();

    int run();
    bool handleMove(std::string& move);

};
