#include "Shell.h"
#include "../Utils.h"
#include <iostream>
#include "../board/Piece.h"
#include "../board/Check.h"
#include "../board/Board.h"
#include "../board/Generate.h"

int Shell::run() {


    for(;;) {
        
        bool& turn = b.getTurn();

        b.printBoard();

        std::cout << "Enter a move\n";

        std::string move;
        std::cin >> move;
        std::cout << "\n\n\n";

        //std::cout << "MOVE->" << move << "\n\n";

        if(!handleMove(move)) {
            std::cout << "Invalid move\n";
            continue;
        }
        
        if(c.isCheck(turn)) {
            std::cout << "Check\n";
            c.undoMove();
            continue;
        }
        
        std::vector<Gen> gen = g.generate(turn);
        
        for (auto& g : gen) {
            std::cout << "From: " << g.from
                    << " To: " << g.to
                    << " Piece: " << g.piece
                    << " Taken: " << g.pieceTaken
                    << std::endl;
        }

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
    
    //std::cout << "===\n";
    //std::cout << "PIECE-> " << static_cast<int>(piece) << "\n";
    //std::cout << "===\n";

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


