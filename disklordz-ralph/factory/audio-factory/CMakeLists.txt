cmake_minimum_required(VERSION 3.22)
project(AudioFactoryHarness LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(dsp_harness dsp_harness/main.cpp)
target_include_directories(dsp_harness PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/dsp_harness")

if(MSVC)
    target_compile_options(dsp_harness PRIVATE /W4 /permissive-)
endif()