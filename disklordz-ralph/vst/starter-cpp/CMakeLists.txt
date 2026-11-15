cmake_minimum_required(VERSION 3.22)
project(DetroitPhonkTrapMaker VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(externals/JUCE)

# ==============================================================================
# 1. Shared DRY core DSP library (TrapCoreDSP)
# ==============================================================================
add_subdirectory(Modules/TrapCoreDSP)

# ==============================================================================
# 2. Headless DSP unit tests (Catch2 v3 + CTest)
# ==============================================================================
option(DLZ_BUILD_TESTS "Catch2 + CTest for TrapCoreDSP" ON)

if(DLZ_BUILD_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Tests/Test808Glide.cpp")
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.7.1)
    FetchContent_MakeAvailable(Catch2)

    enable_testing()

    add_executable(TrapCoreDSPTests
        Tests/Test808Glide.cpp
        Tests/TestSoftClipper.cpp)

    target_include_directories(TrapCoreDSPTests PRIVATE Modules/TrapCoreDSP)

    target_link_libraries(TrapCoreDSPTests
        PRIVATE
            TrapCoreDSP
            Catch2::Catch2WithMain
            juce::juce_audio_basics)

    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
    include(Catch)
    catch_discover_tests(TrapCoreDSPTests)
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

option(DLZ_COPY_VST3_TO_PROGRAMFILES "Copy VST3 to Common Files after build" OFF)
option(DLZ_LICENSE_ED25519 "Ed25519 offline license verify (Core+; requires libsodium)" OFF)

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

target_include_directories(DetroitPhonkTrapMaker PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(DetroitPhonkTrapMaker
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/LicenseVerifier.cpp
        Source/LicenseVerifier.h
        Modules/License/crypto/monocypher.c)

set_source_files_properties(
    Modules/License/crypto/monocypher.c
    PROPERTIES LANGUAGE C)

target_compile_definitions(DetroitPhonkTrapMaker
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=1
    PRIVATE
        $<$<BOOL:${DLZ_LICENSE_ED25519}>:DLZ_LICENSE_ED25519=1>)

target_link_libraries(DetroitPhonkTrapMaker
    PRIVATE
        TrapCoreDSP
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# ==============================================================================
# 3. Optional C++ autoresearch headless render (VoiceDSPSandbox.h — agent sandbox)
# ==============================================================================
option(DLZ_AUTORESEARCH "Build DiskLordzHeadlessRender for evaluate_vst.py" OFF)
if(DLZ_AUTORESEARCH)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../autoresearch-cpp ${CMAKE_BINARY_DIR}/autoresearch-cpp)
endif()