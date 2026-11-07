cmake_minimum_required(VERSION 3.18)
project(chad_cheat)

# Заставляем CMake использовать правильную архитектуру
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Указываем, где лежат заголовки Dobby
include_directories(Dobby/include)

# Отключаем сборку лишнего для Dobby
set(DOBBY_GENERATE_SHARED OFF CACHE BOOL "" FORCE)
set(DOBBY_DEBUG OFF CACHE BOOL "" FORCE)

add_library(chad_cheat SHARED Main.cpp)
add_subdirectory(Dobby)

target_link_libraries(chad_cheat dobby log)
