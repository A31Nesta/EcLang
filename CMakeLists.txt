cmake_minimum_required(VERSION 3.20.0)
project(eclang VERSION 0.1.0 LANGUAGES C CXX)

add_library(${PROJECT_NAME} STATIC
    src/base/config/globalConfig.cpp
    src/base/class.cpp
    src/base/languageDescriptor.cpp
    src/base/eclangBase.cpp
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
    src/base/config/globalConfig.cpp
    src/base/class.cpp
    src/base/languageDescriptor.cpp
    src/base/eclangBase.cpp

    testEntry.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    include
    src
)