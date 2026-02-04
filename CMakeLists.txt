cmake_minimum_required(VERSION 4.1)
project(ChessEngine)

set(CMAKE_CXX_STANDARD 20)

add_executable(
  ChessEngine core/main.cpp 
  core/engine/Shell.cpp
)
