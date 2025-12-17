cmake_minimum_required(VERSION 3.21)

project(hlvm VERSION 0.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(spdlog REQUIRED)
find_package(GTest REQUIRED)

add_library(hlvm STATIC)
target_sources(hlvm
    PRIVATE
        grammar.cpp
        ast/ast.cpp
        ast/node.cpp
        ast/decl.cpp
        lex/lexer.cpp
        parser/parser.cpp
)
target_include_directories(hlvm
    PUBLIC
        ${CMAKE_SOURCE_DIR}
)

add_executable(example)
target_sources(example
    PRIVATE
        example.cpp
)
target_link_libraries(example
    PRIVATE
        hlvm
        spdlog::spdlog
        gtest::gtest
)
target_link_libraries(example
    PRIVATE
        stdc++exp
)
