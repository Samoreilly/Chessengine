
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



std::vector<Gen> Generate::generate(std::array<int8_t, 64> board, bool white) {
    clearGen(); 
    return white ? generateWhite(board) : generateBlack(board); 
}

std::vector<Gen> Generate::generateWhite(std::array<int8_t, 64> board) {
    
    int pieceCounter = 0;

    for(int i = 0;i < 8;i++) {
        for(int j = 0;j < 8;j++) {
            int idx = i * 8 + j;

            if(board.at(idx) > 0) {
                directGen(board, idx);
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

std::vector<Gen> Generate::generateBlack(std::array<int8_t, 64> board) {
    
    int pieceCounter = 0;

    for(int i = 7;i >= 0;i--) {
        for(int j = 7; j >= 0;j--) {
            int idx = i * 8 + j;
            
            if(board.at(idx) < 0) {
                directGen(board, idx);
                pieceCounter++;
            }

            if(pieceCounter == totalPieces) {
                return moves;
            }

        }
    }
    
    return moves;
}

void Generate::directGen(std::array<int8_t, 64> board, int idx) {

    PieceType pieceType = static_cast<PieceType>(abs(board.at(idx)));

    switch(pieceType) {

        case PieceType::BISHOP:
            generateBishopMoves(board, idx);
            break;

        case PieceType::PAWN:
            generatePawnMoves(board, idx);
            break;

        case PieceType::KNIGHT:
            generateKnightMoves(board, idx);
            break;

        case PieceType::KING:
            generateKingMoves(board, idx);
            break;

        case PieceType::ROOK:
            generateRookMoves(board, idx);
            break;

        case PieceType::QUEEN:
            generateQueenMoves(board, idx);
            break;

        default: std::cout << "No directed generation in directGen()";

    }
}

void Generate::generateBishopMoves(std::array<int8_t, 64> board, int idx) {
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

void Generate::generatePawnMoves(std::array<int8_t, 64> board, int idx) {

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

void Generate::generateKingMoves(std::array<int8_t, 64> board, int idx) {

    int row = idx / 8;
    int col = idx % 8;

    for(int dr : {-1,0,1}) {
        for(int dc : {-1,0,1}) {
            if(dr == 0 && dc == 0) continue;

            int newRow = row + dr;
            int newCol = col + dc;

            if(newRow < 0 || newRow >= 8 || newCol < 0 || newCol >= 8) continue;

            int toIdx = newRow * 8 + newCol;

            if(board.at(toIdx) == 0 || isOpponent(b, idx, toIdx)) {
                Gen king;
                king.from = idx;
                king.to = toIdx;
                king.piece = board.at(idx);
                king.pieceTaken = board.at(toIdx) == 0 ? 0 : board.at(toIdx);
                
                moves.push_back(king);
            }
        }
    }

}

void Generate::generateRookMoves(std::array<int8_t, 64> board, int idx) {
   
    static const int rookDirs[4][2] = {{-1, 0}, {0, -1}, {1, 0}, {0, 1}};

    int row = idx / 8;
    int col = idx % 8;

    for(auto dir : rookDirs){
        int r = row + dir[0];
        int c = col + dir[1];


        while(r >= 0 && r < 8 && c >= 0 && c < 8) {
            int newIdx = r * 8 + c;


            if (board.at(newIdx) == 0) {
                Gen rook;
                rook.from = idx;
                rook.to = newIdx;
                rook.piece = board.at(idx);
                rook.pieceTaken = 0;
                
                moves.push_back(rook);
            
            }else if (isOpponent(b, idx, newIdx)) {
                // capture
                Gen rook;
                rook.from = idx;
                rook.to = newIdx;
                rook.piece = board.at(idx);
                rook.pieceTaken = board.at(newIdx);
            
                moves.push_back(rook);
                break;
            
            }else {

                break;
            }

            r += dir[0];
            c += dir[1];
        }

    }

}

void Generate::generateKnightMoves(std::array<int8_t, 64> board, int idx) {
     
    int row = idx / 8;
    int col = idx % 8;

    for(auto dir : b.knight) {
        int r = row + dir[0];
        int c = col + dir[1];

        if (r < 0 || r >= 8 || c < 0 || c >= 8) continue;
        
        int newIdx = r * 8 + c;
 
        if(board.at(newIdx) == 0 || isOpponent(b, idx, newIdx)) {
            Gen knight;
            knight.from = idx;
            knight.to = newIdx;
            knight.piece = board.at(idx);
            knight.pieceTaken = board.at(newIdx) == 0 ? 0 : board.at(newIdx);
            
            moves.push_back(knight);
        }

    }

}

void Generate::generateQueenMoves(std::array<int8_t, 64> board, int idx) {

    static const int queenDirs[8][2] = {
        {-1,  0},
        { 1,  0},
        { 0, -1},
        { 0,  1},
        {-1, -1},
        {-1,  1},
        { 1, -1},
        { 1,  1}
    };
 
    int row = idx / 8;
    int col = idx % 8;

    for(auto dir : queenDirs) {
        int r = row + dir[0];
        int c = col + dir[1];
        
        while(r >= 0 && r < 8 && c >= 0 && c < 8) {
            int newIdx = r * 8 + c;

            if(isOpponent(b, idx, newIdx)) {
                Gen queen;
                queen.from = idx;
                queen.to = newIdx;
                queen.piece = board.at(idx);
                queen.pieceTaken = board.at(newIdx);
                
                moves.push_back(queen);
                break;
            
            }else if(board.at(newIdx) == 0) {
                Gen queen;
                queen.from = idx;
                queen.to = newIdx;
                queen.piece = board.at(idx);
                queen.pieceTaken = 0;
                
                moves.push_back(queen);

            }else {
                break;
            }

            r += dir[0];
            c += dir[1];

        }


    }
}














