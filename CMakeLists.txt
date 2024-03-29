cmake_minimum_required(VERSION 3.25)
project(
    breakout
    VERSION 0.0.1
    LANGUAGES CXX
)

list(
    APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake
)

add_executable(
    ${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/src/main.c
    ${PROJECT_SOURCE_DIR}/src/gl.c
    ${PROJECT_SOURCE_DIR}/src/io.c
    ${PROJECT_SOURCE_DIR}/src/shader.c
)

set(
    EXTERNAL_DIR 
    ${PROJECT_SOURCE_DIR}/external 
    CACHE 
    PATH "External dependencies directory."
)
