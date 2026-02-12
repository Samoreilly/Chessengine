cmake_minimum_required(VERSION 4.1)
project(ChessEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -march=native -DNDEBUG")
set(CMAKE_BUILD_TYPE Release)

add_executable(
  ChessEngine core/main.cpp 
  core/engine/Shell.cpp
  core/board/Piece.cpp
  core/board/Board.cpp
  core/board/Check.cpp
  core/board/Generate.cpp
  core/board/GenerateCheck.cpp
  core/engine/Evaluation.cpp
  core/engine/Search.cpp
)
