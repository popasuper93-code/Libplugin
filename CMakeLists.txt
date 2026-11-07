cmake_minimum_required(VERSION 3.18)
project(chad_cheat)

add_library(chad_cheat SHARED Main.cpp)

# Подключаем Dobby (он будет скачан в папку Dobby)
add_subdirectory(Dobby)
target_link_libraries(chad_cheat dobby log)
