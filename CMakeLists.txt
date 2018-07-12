cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
project(wav-riff LANGUAGES CXX)

include_directories(include)

add_executable(wavFileTest
        src/wavFileTest.cpp src/WavData.cpp src/Chunk.cpp)

target_link_libraries(wavFileTest)
