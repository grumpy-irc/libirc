cmake_minimum_required(VERSION 3.0)
project(libirc)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_subdirectory("libirc")
add_subdirectory("libircclient")
