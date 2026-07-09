cmake_minimum_required(VERSION 3.28)
project(v_lang LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB SOURCE_FILES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(v_lang main.cpp ${SOURCE_FILES})

target_include_directories(v_lang PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
