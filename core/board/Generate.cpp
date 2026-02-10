
#include "Generate.h"
#include "Piece.h"
#include "Board.h"
#include <vector>
#include <iostream>
#include "../Utils.h"

/*
Theres to be a generate() function that will serve as the main entry point
the generate() will loop through the board and return the current players turn moves
*/



std::vector<Gen> Generate::generate(bool white) {


    if(white) {
        return generateWhite();
    }else{
        return generateBlack();
    }   
}

std::vector<Gen> Generate::generateWhite() {
    
    int pieceCounter = 0;

    for(int i = 0;i < 8;i++) {
        for(int j = 0;j < 8;j++) {
            int idx = i * 8 + j;

            if(board.at(idx) > 0) {
                directGen(idx);
                pieceCounter++;
            }

            //no more pieces to scan for
            if(pieceCounter == totalPieces) {
                return moves;
            }

        }
    }

    return moves;
}

void Generate::directGen(int idx) {

    PieceType pieceType = static_cast<PieceType>(abs(board.at(idx)));

    switch(pieceType) {

        case PieceType::BISHOP:
            generateBishopMoves(idx);
            break;

        case PieceType::PAWN:
            generatePawnMoves(idx);
        
        case PieceType::KNIGHT:
            generateKnightMoves(idx);
            break;

        case PieceType::KING:
            generateKingMoves(idx);
            break;

        case PieceType::ROOK:
            generateRookMoves(idx);
            break;

        case PieceType::QUEEN:
            generateQueenMoves(idx);
            break;

        default: std::cout << "No directed generation in directGen()";

    }
}

void Generate::generateBishopMoves(int idx) {
    Gen gen;
    gen.from = idx;
    gen.piece = board.at(idx);
    
    int fromRow = idx / 8;
    int fromCol = idx % 8;

    for(auto dir : bishop) {
        int stepRow = dir[0];
        int stepCol = dir[1];
        
        int r = fromRow + stepRow;
        int c = fromCol + stepCol;

        while(r >= 0 && r < 8 && c >= 0 && c < 8) {
            int curr = r * 8 + c;

            if(board.at(curr) == 0) {
                gen.to = curr;
                gen.pieceTaken = 0;
                
                moves.push_back(gen);

            }else if(isOpponent(b, idx, curr)) {
                gen.to = curr;
                gen.pieceTaken = board.at(curr);
                
                moves.push_back(gen);
                break;

            }else {
                break;
            }

            r += stepRow;
            c += stepCol;

        }
    }
}


