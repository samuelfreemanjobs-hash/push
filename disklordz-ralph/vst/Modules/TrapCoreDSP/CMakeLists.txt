cmake_minimum_required(VERSION 3.22)

add_library(TrapCoreDSP STATIC TrapCoreDSP.cpp TrapCoreDSP.h)

target_include_directories(TrapCoreDSP
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/Source)

target_link_libraries(TrapCoreDSP
    PUBLIC
        juce::juce_audio_basics
        juce::juce_audio_processors
        juce::juce_dsp)