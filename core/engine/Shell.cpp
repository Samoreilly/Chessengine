#include "Shell.h"
#include "../Utils.h"
#include <iostream>
#include "../board/Piece.h"

int Shell::run() {


    for(;;) {
        
        b.printBoard();

        std::cout << "Enter a move\n";

        std::string move;
        std::cin >> move;
        
        if(!handleMove(move)) {
            std::cout << "Invalid move";
            continue;
        }

        
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

    
    PieceType piece = p.getPieceType(board.at(fromIndex));

    switch(piece) {
    
        case PieceType::PAWN:
            p.pawnMove(fromIndex, toIndex);
            break;

        case PieceType::ROOK:
            p.rookMove(fromIndex, toIndex);

        case PieceType::KNIGHT:
            p.knightMove(fromIndex, toIndex);

        case PieceType::BISHOP:
            p.bishopMove(fromIndex, toIndex);

        case PieceType::QUEEN:
            p.queenMove(fromIndex, toIndex);

        case PieceType::KING:
            p.kingMove(fromIndex, toIndex);

        default: p.emptyMove(fromIndex, toIndex);

    };

    
    return false;

}
