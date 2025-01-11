cmake_minimum_required(VERSION 3.20.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(eclang VERSION 0.1.0 LANGUAGES C CXX)

# TODO: Set to off later lmao
option(ENABLE_LOGGING "Enables logging to stdout for debugging purposes. Should be off unless you're debugging the library" ON)

add_library(${PROJECT_NAME} STATIC
    src/util/globalConfig.cpp
    src/util/stringUtils.cpp
    src/attribute.cpp
    src/class.cpp
    src/object.cpp
    src/language.cpp
    src/eclangClass.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE
)

# Define ECLANG_DEBUG
if(ENABLE_LOGGING)
    add_compile_definitions(ECLANG_DEBUG)
endif(ENABLE_LOGGING)


target_include_directories(${PROJECT_NAME} PRIVATE
    include
    src
)


# Test EcLang Executable
project(eclang-test VERSION 0.1.0 LANGUAGES C CXX)

add_executable(${PROJECT_NAME}
    src/util/globalConfig.cpp
    src/util/stringUtils.cpp
    src/attribute.cpp
    src/class.cpp
    src/object.cpp
    src/language.cpp
    src/eclangClass.cpp

    testEntry.cpp
)

# Define ECLANG_DEBUG
if(ENABLE_LOGGING)
    add_compile_definitions(ECLANG_DEBUG)
endif(ENABLE_LOGGING)

target_include_directories(${PROJECT_NAME} PRIVATE
    include
    src
)