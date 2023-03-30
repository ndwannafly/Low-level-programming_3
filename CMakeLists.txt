cmake_minimum_required(VERSION 3.23)
project(lab3 C)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_C_STANDARD 99)

add_executable(lab3 server.c client.c)
