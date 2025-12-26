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
        ast/node-visitor.cpp
        ast/recursive-node-visitor.cpp
        ast/decl.cpp
        ast/expr.cpp
        ast/stmt.cpp
        lex/lexer.cpp
        parser/parser.cpp
        semantic/semantic-analyzer.cpp
        semantic/intepreter.cpp
)
target_link_libraries(hlvm
    PUBLIC
        stdc++exp
    PRIVATE
        spdlog::spdlog
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

add_executable(grammar-test)
target_sources(grammar-test
    PRIVATE
        test/grammar.cpp
)
target_link_libraries(grammar-test
    PRIVATE
        hlvm
        gtest::gtest
)

add_executable(automaton-test)
target_sources(automaton-test
    PRIVATE
        test/automaton-test.cpp
)
target_link_libraries(automaton-test
    PRIVATE
        hlvm
)
