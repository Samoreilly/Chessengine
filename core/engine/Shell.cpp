#include "Shell.h"
#include "../Utils.h"
#include <iostream>
#include "../board/Piece.h"
#include "../board/Check.h"
#include "../board/Board.h"
#include "../board/Generate.h"
#include "../engine/Search.h"

int Shell::run() {
    const int depth = 4;

    for (;;) {
        bool turn = b.getTurn(); // true = White, false = Black

        // Check for checkmate/stalemate before asking for a move
        std::vector<Gen> legalMoves;
        std::vector<Gen> allMoves = g.generate(b.getBoard(), turn);
        for (auto move : allMoves) {
            std::optional<std::array<int8_t, 64>> result = g.makeMove(b.getBoard(), turn, move);
            if (result.has_value()) {
                legalMoves.push_back(move);
            }
        }

        if (legalMoves.empty()) {
            b.printBoard();
            if (c.isCheck(turn)) {
                std::cout << (turn ? "Black" : "White") << " wins by checkmate!\n";
            } else {
                std::cout << "Stalemate! The game is a draw.\n";
            }
            return 0;
        }

        // Engine evaluation
        int score = s.search(b.getBoard(), depth, turn);
        int absScore = turn ? score : -score;

        // Convert to percentages
        double whiteProb = toWinPercent(absScore);
        double blackProb = 100.0 - whiteProb;

        b.printBoard();
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "White: " << whiteProb << "%, Black: " << blackProb << "%\n";
        std::cout << "Evaluation: " << absScore << "\n\n";

        // Show top 3 lines for current player
        std::vector<ScoredMove> topMoves = s.getTopMoves(b.getBoard(), depth, turn, 3);
        std::cout << "Best lines for " << (turn ? "White" : "Black") << ":\n";
        for (int i = 0; i < (int)topMoves.size(); i++) {
            ScoredMove& sm = topMoves[i];
            
            std::cout << "  " << (i + 1) << ". ";
            for (int j = 0; j < (int)sm.line.size(); j++) {
                if (j > 0) std::cout << "  ";
                std::cout << indexToAlgebraic(sm.line[j].from) << "-" << indexToAlgebraic(sm.line[j].to);
            }
            std::cout << "  (eval: " << sm.score << ")\n";
        }
        std::cout << "\n";

        // Ask current player for move
        std::cout << (turn ? "White" : "Black") << " enter your move: ";
        std::string move;
        std::cin >> move;

        if (!handleMove(move)) {
            std::cout << "Invalid move\n";
            continue;
        }

        // Check if the move left the player in check
        if (c.isCheck(turn)) { 
            std::cout << "Move leaves you in check!\n";
            c.undoMove();
            continue;
        }

        // Advance to next turn
        b.nextTurn();
    }

    return 0;
}


bool Shell::handleMove(std::string& move) {

    std::pair<std::string, std::string> pair = getCoord(move); 

    std::string from = pair.first;
    std::string to = pair.second;
    
    int fromIndex = getIndex(from);
    int toIndex = getIndex(to);
    
    if(fromIndex < 0 || fromIndex > 63 || toIndex < 0 || toIndex > 63 
        || (fromIndex == toIndex)) return false;
    
    //if piece that is being moved is the opposite of whos turn it is
    if((board.at(fromIndex) > 0 && b.isBlackTurn()) || (board.at(fromIndex) < 0 && b.isWhiteTurn())) {
        std::cout << "Not your turn";
        return false;
    }


    PieceType piece = p.getPieceType(abs(board.at(fromIndex)));

    switch(piece) {
    
        case PieceType::PAWN:
            return p.pawnMove(fromIndex, toIndex);

        case PieceType::ROOK:
            return p.rookMove(fromIndex, toIndex);

        case PieceType::KNIGHT:
            return p.knightMove(fromIndex, toIndex);

        case PieceType::BISHOP:
            return p.bishopMove(fromIndex, toIndex);

        case PieceType::QUEEN:
            return p.queenMove(fromIndex, toIndex);

        case PieceType::KING:
            return p.kingMove(fromIndex, toIndex);

        default: p.emptyMove(fromIndex, toIndex);

    };

    
    return false;

}


