cmake_minimum_required(VERSION 3.20.0)

project("Reduced C Compiler (C++)"
    VERSION 0.1
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

add_executable(rcc
    src/main.cpp
)