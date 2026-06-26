cmake_minimum_required(VERSION 3.28)
project(v_lang LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(v_lang
    main.cpp
    src/Lexer.cpp
    src/Parser.cpp
    src/CodeGeneration.cpp
    src/StaticCheck.hpp
)

target_include_directories(v_lang PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
