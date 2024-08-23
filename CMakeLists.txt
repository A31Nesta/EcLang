cmake_minimum_required(VERSION 3.5.0)
project(meatgrinder VERSION 0.1.0 LANGUAGES C CXX)

add_library(${PROJECT_NAME} STATIC
    src/auiistrings.cpp
    src/instructionstrings.cpp
    src/stringutils.cpp
    src/meatgrinder.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE
)

target_include_directories(${PROJECT_NAME} PRIVATE
    include
    src
)