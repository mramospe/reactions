#
# Example of a CMake project depending on Reactions
#
cmake_minimum_required(VERSION 2.8)
project(ReactionsTest)
set(CMAKE_CXX_STANDARD 17)
find_package(Reactions REQUIRED)
message("-- Found Reactions version ${Reactions_VERSION}")
message("-- Including directories ${Reactions_INCLUDE_DIR}")
add_executable(main main.cpp)
target_include_directories(main PUBLIC ${Reactions_INCLUDE_DIR})
