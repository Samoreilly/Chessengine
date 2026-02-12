#include "Shell.h"
#include "../Utils.h"
#include <iostream>
#include <chrono>
#include "../board/Piece.h"
#include "../board/Check.h"
#include "../board/Board.h"
#include "../board/Generate.h"
#include "../engine/Search.h"

int Shell::run() {
    const int depth = 6;

    if (apiMode) {
        // API Mode: Loop listening for commands on stdin
        // Output JSON to stdout
        
        // Initial "ready" signal if needed, or just wait.
        std::string cmd;
        while (std::cin >> cmd) {
            if (cmd == "isready") {
                std::cout << "{\"ready\": true}" << std::endl;
            } 
            else if (cmd == "newgame") {
                // Reset board
                b = Board(); // Re-assign default board
                // Reset any other state if needed
                std::cout << "{\"status\": \"new_game_started\"}" << std::endl;
            }
            else if (cmd == "move") {
                std::string moveStr;
                std::cin >> moveStr;
                // Parse move string (e.g., "e2e4" or "e2-e4")
                bool valid = handleMove(moveStr); // handleMove prints errors to stdout which might break JSON, we need to be careful.
                // handleMove currently prints to stdout. We should probably refactor handleMove to NOT print if apiMode is set, or capture it.
                // For now, let's assume valid moves.
                if (valid) {
                     b.nextTurn(); // Switch turn after successful move
                     std::cout << "{\"status\": \"move_ok\", \"turn\": " << (b.getTurn() ? "\"white\"" : "\"black\"") << "}" << std::endl;
                } else {
                     std::cout << "{\"status\": \"illegal_move\"}" << std::endl;
                }
            }
            else if (cmd == "search") {
                int d;
                std::cin >> d;
                int score = s.search(b.getBoard(), d, b.getTurn(), -2000000, 2000000);
                if (!b.getTurn()) score = -score; // Normalize to White-relative
                std::vector<ScoredMove> best = s.getTopMoves(b.getBoard(), d, b.getTurn(), 3);
                
                std::cout << "{";
                std::cout << "\"eval\": " << score << ", ";

                // bestmove (first move of best line)
                if (!best.empty() && !best[0].line.empty()) {
                    Gen& bestMove = best[0].line[0];
                    std::string mStr = indexToAlgebraic(bestMove.from) + indexToAlgebraic(bestMove.to);
                    std::cout << "\"bestmove\": \"" << mStr << "\", ";
                } else {
                    std::cout << "\"bestmove\": null, ";
                }

                // topMoves array
                std::cout << "\"topMoves\": [";
                for (size_t i = 0; i < best.size(); i++) {
                    if (i > 0) std::cout << ", ";
                    std::cout << "{";
                    std::cout << "\"score\": " << best[i].score << ", ";
                    std::cout << "\"line\": [";
                    for (size_t j = 0; j < best[i].line.size(); j++) {
                        if (j > 0) std::cout << ", ";
                        std::string mv = indexToAlgebraic(best[i].line[j].from) + indexToAlgebraic(best[i].line[j].to);
                        std::cout << "\"" << mv << "\"";
                    }
                    std::cout << "]}";
                }
                std::cout << "]";

                std::cout << "}" << std::endl;
            }
            else if (cmd == "quit") {
                break;
            }
        }
        return 0;
    }

    // ---- Mode selection ----
    std::cout << "\n";
    std::cout << "\033[1;36m  ╔═══════════════════════════╗\033[0m\n";
    std::cout << "\033[1;36m  ║     ♔  CHESS ENGINE  ♚    ║\033[0m\n";
    std::cout << "\033[1;36m  ╚═══════════════════════════╝\033[0m\n\n";
    std::cout << "  \033[1m1.\033[0m Analysis mode (two players)\n";
    std::cout << "  \033[1m2.\033[0m Play vs AI \033[2m(you are White)\033[0m\n";
    std::cout << "  \033[1m3.\033[0m Play vs AI \033[2m(you are Black)\033[0m\n";
    std::cout << "\n  \033[1mSelect ▸\033[0m ";

    std::string choice;
    std::cin >> choice;

    if (choice == "2") {
        mode = GameMode::VS_AI;
        humanIsWhite = true;
        std::cout << "\n  \033[32m✓ You play White. Engine plays Black.\033[0m\n";
    } else if (choice == "3") {
        mode = GameMode::VS_AI;
        humanIsWhite = false;
        std::cout << "\n  \033[32m✓ You play Black. Engine plays White.\033[0m\n";
    } else {
        mode = GameMode::ANALYSIS;
        std::cout << "\n  \033[32m✓ Analysis mode\033[0m\n";
    }

    // ---- Main game loop ----
    for (;;) {
        bool turn = b.getTurn(); // true = White, false = Black

        // Check for checkmate/stalemate
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

        // --- AI's turn ---
        if (mode == GameMode::VS_AI && turn != humanIsWhite) {
            std::cout << "\n  \033[2m⏳ Engine is thinking...\033[0m\n";

            if (makeAIMove(turn, depth)) {
                b.nextTurn();
            }
            continue;
        }

        // --- Human's turn (or analysis mode) ---

        // Engine evaluation (timed)
        auto t0 = std::chrono::steady_clock::now();

        int score = s.search(b.getBoard(), depth, turn, -2000000, 2000000);
        int absScore = turn ? score : -score;

        double whiteProb = toWinPercent(absScore);
        double blackProb = 100.0 - whiteProb;

        // Top moves
        std::vector<ScoredMove> topMoves = s.getTopMoves(b.getBoard(), depth, turn, 3);

        auto t1 = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double>(t1 - t0).count();

        b.printBoard();

        // ANSI formatting
        const char* BOLD  = "\033[1m";
        const char* DIM   = "\033[2m";
        const char* GREEN = "\033[32m";
        const char* CYAN  = "\033[36m";
        const char* YELLOW = "\033[33m";
        const char* RST   = "\033[0m";

        std::cout << std::fixed << std::setprecision(1);

        // Eval bar
        int barWidth = 20;
        int whiteBars = (int)(whiteProb / 100.0 * barWidth);
        std::cout << "  " << BOLD << "⚪ " << whiteProb << "%" << RST << " ";
        for (int i = 0; i < barWidth; i++) {
            if (i < whiteBars) std::cout << "\033[47m ";
            else std::cout << "\033[40m ";
        }
        std::cout << RST << " " << BOLD << blackProb << "% ⚫" << RST << "\n";

        std::cout << DIM << "  eval " << absScore << " · " << s.getNodesSearched() << " nodes · " << std::setprecision(2) << elapsed << "s" << RST << "\n\n";

        // Best lines
        std::cout << CYAN << "  ── Best lines for " << (turn ? "White" : "Black") << " ──" << RST << "\n";
        for (int i = 0; i < (int)topMoves.size(); i++) {
            ScoredMove& sm = topMoves[i];
            
            std::cout << "  " << YELLOW << (i + 1) << "." << RST << " ";
            for (int j = 0; j < (int)sm.line.size(); j++) {
                if (j > 0) std::cout << " ";
                // Detect castling notation
                Gen& m = sm.line[j];
                if (std::abs(m.piece) == 6 && std::abs(m.to - m.from) == 2) {
                    std::cout << (m.to > m.from ? "O-O" : "O-O-O");
                } else {
                    std::cout << indexToAlgebraic(m.from) << indexToAlgebraic(m.to);
                    if (m.promotion) std::cout << "=Q";
                }
            }
            std::cout << DIM << " (" << sm.score << ")" << RST << "\n";
        }
        std::cout << "\n";

        // Move prompt
        std::cout << BOLD << "  " << (turn ? "White" : "Black") << " ▸ " << RST;
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

        b.nextTurn();
    }

    return 0;
}

// --------------------------------------------------------
// AI makes the best move automatically
// --------------------------------------------------------
bool Shell::makeAIMove(bool turn, int depth) {
    auto t0 = std::chrono::steady_clock::now();

    std::vector<ScoredMove> topMoves = s.getTopMoves(b.getBoard(), depth, turn, 1);

    auto t1 = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(t1 - t0).count();

    if (topMoves.empty()) return false;

    Gen& bestMove = topMoves[0].line[0];

    // Apply the move to the real board
    std::string fromStr = indexToAlgebraic(bestMove.from);
    std::string toStr = indexToAlgebraic(bestMove.to);
    std::string moveStr = fromStr + "-" + toStr;

    if (!handleMove(moveStr)) {
        // Fallback: directly apply to board
        board.at(bestMove.to) = board.at(bestMove.from);
        board.at(bestMove.from) = 0;
        if (bestMove.promotion) {
            board.at(bestMove.to) = turn ? 5 : -5; // Queen
        }
    }

    // Check if move puts opponent in check
    if (c.isCheck(turn)) {
        c.undoMove();
        // Direct board manipulation as fallback
        board.at(bestMove.to) = board.at(bestMove.from);
        board.at(bestMove.from) = 0;
        if (bestMove.promotion) {
            board.at(bestMove.to) = turn ? 5 : -5;
        }
    }

    // Display the board and what the engine played
    double whiteProb = toWinPercent(topMoves[0].score);
    double blackProb = 100.0 - whiteProb;

    const char* BOLD  = "\033[1m";
    const char* DIM   = "\033[2m";
    const char* CYAN  = "\033[36m";
    const char* MAGENTA = "\033[35m";
    const char* RST   = "\033[0m";

    b.printBoard();

    std::cout << std::fixed << std::setprecision(1);

    // Eval bar
    int barWidth = 20;
    int whiteBars = (int)(whiteProb / 100.0 * barWidth);
    std::cout << "  " << BOLD << "⚪ " << whiteProb << "%" << RST << " ";
    for (int i = 0; i < barWidth; i++) {
        if (i < whiteBars) std::cout << "\033[47m ";
        else std::cout << "\033[40m ";
    }
    std::cout << RST << " " << BOLD << blackProb << "% ⚫" << RST << "\n";

    std::cout << DIM << "  eval " << topMoves[0].score << " · " << s.getNodesSearched() << " nodes · " << std::setprecision(2) << elapsed << "s" << RST << "\n\n";

    // Engine move display
    std::cout << MAGENTA << "  ▸ Engine plays: " << RST << BOLD;
    if (std::abs(bestMove.piece) == 6 && std::abs(bestMove.to - bestMove.from) == 2) {
        std::cout << (bestMove.to > bestMove.from ? "O-O" : "O-O-O");
    } else {
        std::cout << fromStr << toStr;
        if (bestMove.promotion) std::cout << "=Q";
    }
    std::cout << RST << "\n";

    // Show the engine's planned line
    if (topMoves[0].line.size() > 1) {
        std::cout << DIM << "  line: ";
        for (int j = 0; j < (int)topMoves[0].line.size(); j++) {
            if (j > 0) std::cout << " ";
            Gen& m = topMoves[0].line[j];
            if (std::abs(m.piece) == 6 && std::abs(m.to - m.from) == 2) {
                std::cout << (m.to > m.from ? "O-O" : "O-O-O");
            } else {
                std::cout << indexToAlgebraic(m.from) << indexToAlgebraic(m.to);
                if (m.promotion) std::cout << "=Q";
            }
        }
        std::cout << RST << "\n";
    }
    std::cout << "\n";

    return true;
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
        if (apiMode) {
            std::cout << "{\"status\": \"error\", \"message\": \"not_your_turn\"}" << std::endl;
        } else {
            std::cout << "Not your turn";
        }
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


