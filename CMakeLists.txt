cmake_minimum_required(VERSION 3.26)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

project(IntTitan LANGUAGES CXX)
add_executable(IntTitan main.cpp)
target_include_directories(IntTitan PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})