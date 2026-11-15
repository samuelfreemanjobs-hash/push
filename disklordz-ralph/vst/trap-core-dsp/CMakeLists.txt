cmake_minimum_required(VERSION 3.22)

# Header-first DSP; .cpp voice files land in Source/ as you implement DLZ-VST-*.
add_library(TrapCoreDSP INTERFACE)
target_include_directories(TrapCoreDSP
    INTERFACE
        ${CMAKE_SOURCE_DIR}/Source
        ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(TrapCoreDSP
    INTERFACE
        juce::juce_audio_basics
        juce::juce_audio_processors
        juce::juce_dsp)