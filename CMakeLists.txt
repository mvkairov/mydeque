cmake_minimum_required(VERSION 3.20)
project(mydeque)

set(CMAKE_CXX_STANDARD 20)

include_directories(.)

add_executable(mydeque
        deque.cpp
        deque.h)
