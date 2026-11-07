cmake_minimum_required(VERSION 3.18)
project(chad_cheat)

# Скачиваем Dobby автоматически
include(FetchContent)
FetchContent_Declare(
    dobby
    GIT_REPOSITORY https://github.com/jmpews/Dobby.git
    GIT_TAG        master
)
# Отключаем сборку тестов Dobby, чтобы не тратить время
set(DOBBY_GENERATE_SHARED OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(dobby)

add_library(chad_cheat SHARED Main.cpp)

# Привязываем Dobby к нашему читу
target_link_libraries(chad_cheat dobby log)
