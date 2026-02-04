				=== Chess Engine ===

Board - Simple 1D or 2D array for pieces (avoiding complex bitmasking for now)
Minimax - Looks few moves ahead
Pruning - Can skip bad lines/branches

0 is empty square
White Pawn - 1, Black Pawn - -1
White Rook - 2, Black Rook - -2
White Bishop - 3, Black Bishop - -3
White Knight - 4, Black Knight - -4
White Queen - 5, Black Queen - -5
White King - 6, Black King - -6




				=== FLOW ===

Initialize Board and pieces
Give user a shell-esque interface in the terminal
A bar or percentage representing the favouribility of both players winning




# SHELL HIGH LEVEL OVERVIEW

- Continuous loop
- Take 2 user inputs, piece to move and destination e.g. b2 and b4 or I could
 take it in on one input like b2-b4

- If valid, move piece, print board + evaluation etc


