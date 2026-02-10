
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

void Generate::generatePawnMoves(int idx) {

    // used to generate en passant
    LastMove& lm = b.getLastMove();
     
    bool white = board.at(idx) > 0; 
    bool canMoveTwo {false};
    int row = idx / 8;

    int step = white ? 1 : -1;
    
    // starting rank check for double move
    if(white && row == 1) {
        canMoveTwo = true;
    } else if(!white && row == 6) {
        canMoveTwo = true;
    }

    int r = row + step;
    int c = idx % 8;

    // single forward and diagonal captures
    for(int i : {-1, 0, 1}) {
        int col = c + i;
        int curr = r * 8 + col;

        if(r >= 0 && r < 8 && col >= 0 && col < 8) {

            if(col == c) {
                // forward move
                if(board.at(curr) == 0) {
                    Gen forward;
                    forward.from = idx;
                    forward.to = curr;
                    forward.piece = board.at(idx);
                    forward.pieceTaken = 0;
                    moves.push_back(forward);
                }

            } else if(isOpponent(b, idx, curr)) {
                // diagonal capture
                Gen capture;
                capture.from = idx;
                capture.to = curr;
                capture.piece = board.at(idx);
                capture.pieceTaken = board.at(curr);
                moves.push_back(capture);
            }
        }
    }

    // --- EN PASSANT LOGIC ---
    int lmToRow = lm.to / 8;
    int lmCol = lm.to % 8;
    int pawnCol = c;

    if(white && row == 4) { // white pawn on 5th rank
        if(lmToRow == 4 && abs(lmCol - pawnCol) == 1 &&
           static_cast<PieceType>(abs(board.at(lm.to))) == PieceType::PAWN &&
           isOpponent(b, idx, lm.to)) {

            Gen enp;
            enp.from = idx;
            enp.to = lm.to - 8;      // empty square behind black pawn
            enp.piece = board.at(idx);
            enp.pieceTaken = board.at(lm.to); // captured pawn
            moves.push_back(enp);
        }
    } else if(!white && row == 3) { // black pawn on 4th rank
        if(lmToRow == 3 && abs(lmCol - pawnCol) == 1 &&
           static_cast<PieceType>(abs(board.at(lm.to))) == PieceType::PAWN &&
           isOpponent(b, idx, lm.to)) {

            Gen enp;
            enp.from = idx;
            enp.to = lm.to + 8;      // empty square behind white pawn
            enp.piece = board.at(idx);
            enp.pieceTaken = board.at(lm.to); // captured pawn
            moves.push_back(enp);
        }
    }

    // double pawn move
    int nextIdx = (row + step) * 8 + c;
    int nextNextIdx = (row + step + step) * 8 + c;
    
    if(canMoveTwo && nextIdx >= 0 && nextNextIdx >= 0 &&
       nextIdx < 64 && nextNextIdx < 64 &&
       board.at(nextIdx) == 0 && board.at(nextNextIdx) == 0) {

        Gen doublePush;
        doublePush.from = idx;
        doublePush.to = nextNextIdx;
        doublePush.piece = board.at(idx);
        doublePush.pieceTaken = 0;
        moves.push_back(doublePush);
    }
}

