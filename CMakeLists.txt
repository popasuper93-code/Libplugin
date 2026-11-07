cmake_minimum_required(VERSION 3.18)
project(chad_cheat)

add_library(chad_cheat SHARED Main.cpp)

target_link_libraries(chad_cheat log)
