cmake_minimum_required(VERSION 3.18)
project(chad_cheat)

include(FetchContent)
FetchContent_Declare(
    shadowhook
    GIT_REPOSITORY https://github.com/bytedance/android-inline-hook.git
    GIT_TAG main
)
FetchContent_MakeAvailable(shadowhook)

add_library(chad_cheat SHARED Main.cpp)
target_link_libraries(chad_cheat shadowhook log)
