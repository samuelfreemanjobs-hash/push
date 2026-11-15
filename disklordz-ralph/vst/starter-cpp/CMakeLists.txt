cmake_minimum_required(VERSION 3.22)
project(DetroitPhonkTrapMaker VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE submodule from bootstrap: externals/JUCE
add_subdirectory(externals/JUCE)

option(DLZ_BUILD_TESTS "Catch2 TrapCoreDSP tests" ON)
if(DLZ_BUILD_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
  enable_testing()
  add_subdirectory(trap-core-dsp)
  add_subdirectory(tests)
endif()

# PDB in Release for REA / Ghidra (MSVC)
if(MSVC)
  add_compile_options($<$<CONFIG:Release>:/Zi> $<$<CONFIG:RelWithDebInfo>:/Zi>)
  add_link_options(
    $<$<CONFIG:Release>:/DEBUG>
    $<$<CONFIG:Release>:/OPT:REF>
    $<$<CONFIG:Release>:/OPT:ICF>
    $<$<CONFIG:RelWithDebInfo>:/DEBUG>)
endif()

# Default OFF — no Program Files permission errors (DiskLordz Ralph rule).
# Optional: cmake -B build -DDLZ_COPY_VST3_TO_PROGRAMFILES=ON (run terminal as Admin)
option(DLZ_COPY_VST3_TO_PROGRAMFILES "Copy VST3 to Common Files after build" OFF)

juce_add_plugin(DetroitPhonkTrapMaker
    COMPANY_NAME "DiskLordz"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD ${DLZ_COPY_VST3_TO_PROGRAMFILES}
    PLUGIN_MANUFACTURER_CODE Dloz
    PLUGIN_CODE Dptm
    FORMATS VST3 Standalone
    PRODUCT_NAME "Detroit Phonk Trap Maker")

target_sources(DetroitPhonkTrapMaker
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h)

target_compile_definitions(DetroitPhonkTrapMaker
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=1)

if(TARGET TrapCoreDSP)
  target_link_libraries(DetroitPhonkTrapMaker PRIVATE TrapCoreDSP)
endif()

target_link_libraries(DetroitPhonkTrapMaker
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# Offline / Ralph verify (add Source/cli/DrumTestCLI.cpp when DLZ-VST-002)
# juce_add_console_app(DrumTestCLI ...)