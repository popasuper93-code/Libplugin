cmake_minimum_required(VERSION 3.18)
project(chad_menu)

add_compile_options(-Wno-error)

include(FetchContent)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.91.1
)
FetchContent_MakeAvailable(imgui)

add_library(chad_menu SHARED Main.cpp)

target_include_directories(chad_menu PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_sources(chad_menu PRIVATE
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_link_libraries(chad_menu log android EGL GLESv3)
