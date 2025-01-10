cmake_minimum_required(VERSION 3.20.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(eclang VERSION 0.1.0 LANGUAGES C CXX)


add_library(${PROJECT_NAME} STATIC
    src/config/globalConfig.cpp
    src/vectors.cpp
    src/attribute.cpp
    src/class.cpp
    src/language.cpp
    src/eclangClass.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE
)

target_include_directories(${PROJECT_NAME} PRIVATE
    include
    src
)


# Test EcLang Executable
project(eclang-test VERSION 0.1.0 LANGUAGES C CXX)

add_executable(${PROJECT_NAME}
    src/config/globalConfig.cpp
    src/vectors.cpp
    src/attribute.cpp
    src/class.cpp
    src/language.cpp
    src/eclangClass.cpp

    testEntry.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    include
    src
)