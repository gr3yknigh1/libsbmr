cmake_minimum_required(VERSION 3.25)
project(
    breakout
    VERSION 0.0.1
    LANGUAGES CXX
)

add_executable(
    ${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/src/Main.cpp
)

