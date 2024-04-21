cmake_minimum_required(VERSION 3.25)
project(
    libsbmr
    VERSION 0.0.1
    LANGUAGES CXX
)

add_executable(
    ${PROJECT_NAME}
    WIN32   # Required for entry point in WinAPI
    ${PROJECT_SOURCE_DIR}/src/Main.cpp
    ${PROJECT_SOURCE_DIR}/src/String.cpp
    ${PROJECT_SOURCE_DIR}/src/BMR.cpp
)

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE ${PROJECT_SOURCE_DIR}/src
)
