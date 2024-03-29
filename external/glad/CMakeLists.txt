cmake_minimum_required(VERSION 3.25)
project(glad C)

add_library(
    ${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/src/glad.c
)

target_include_directories(
    ${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/include
)
