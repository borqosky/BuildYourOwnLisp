cmake_minimum_required(VERSION 3.2)
project(byol)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=c99")
add_executable(byol ${SOURCE_FILES} prompt.c)
target_link_libraries(byol -ledit)
