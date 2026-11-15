cmake_minimum_required(VERSION 3.22)

project(AudioFactoryHarness VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(DLZ_USE_MONOREPO_JUCE "Use starter-cpp/externals/JUCE (recommended in push repo)" ON)

set(DLZ_STARTER_JUCE "${CMAKE_CURRENT_SOURCE_DIR}/../starter-cpp/externals/JUCE")
# starter-cpp lives under vst/, one level up from factory/
set(DLZ_VST_STARTER_JUCE "${CMAKE_CURRENT_SOURCE_DIR}/../../vst/starter-cpp/externals/JUCE")

if(DLZ_USE_MONOREPO_JUCE AND EXISTS "${DLZ_VST_STARTER_JUCE}/CMakeLists.txt")
    set(DLZ_JUCE_ROOT "${DLZ_VST_STARTER_JUCE}")
elseif(DLZ_USE_MONOREPO_JUCE AND EXISTS "${DLZ_STARTER_JUCE}/CMakeLists.txt")
    set(DLZ_JUCE_ROOT "${DLZ_STARTER_JUCE}")
else()
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.6
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
    set(DLZ_JUCE_ROOT "")
endif()

if(DLZ_JUCE_ROOT)
    add_subdirectory("${DLZ_JUCE_ROOT}" "${CMAKE_BINARY_DIR}/JUCE")
endif()

juce_add_console_app(dsp_harness PRODUCT_NAME "dsp_harness")

juce_generate_juce_header(dsp_harness)

target_sources(dsp_harness
    PRIVATE
        dsp_harness/dsp_harness.cpp
        src/dsp/VintageSaturator.h)

target_include_directories(dsp_harness PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_link_libraries(dsp_harness
    PRIVATE
        juce::juce_audio_formats
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)

if(MSVC)
    target_compile_options(dsp_harness PRIVATE /W4 /permissive-)
endif()