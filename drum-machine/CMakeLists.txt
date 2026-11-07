cmake_minimum_required(VERSION 3.22)

project(KitForge VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build universal binaries on macOS (Apple Silicon + Intel)
if(APPLE AND NOT CMAKE_OSX_ARCHITECTURES)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)
endif()

option(KITFORGE_BUILD_TESTS "Build the offline render test" ON)

# ------------------------------------------------------------------------------
# JUCE: use a local checkout if JUCE_DIR is given, otherwise download it.
#   cmake -B build -DJUCE_DIR=/path/to/JUCE
# ------------------------------------------------------------------------------
if(DEFINED JUCE_DIR)
    add_subdirectory(${JUCE_DIR} JUCE EXCLUDE_FROM_ALL)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        8.0.4
        GIT_SHALLOW    TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

# ------------------------------------------------------------------------------
# Plugin (VST3 + Standalone)
# ------------------------------------------------------------------------------
juce_add_plugin(KitForge
    COMPANY_NAME                "KitForge Audio"
    BUNDLE_ID                   com.kitforgeaudio.kitforge
    IS_SYNTH                    TRUE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     FALSE
    PLUGIN_MANUFACTURER_CODE    Kfrg
    PLUGIN_CODE                 Kfdm
    FORMATS                     VST3 Standalone
    PRODUCT_NAME                "KitForge"
    VST3_CATEGORIES             Instrument Drum)

set(KITFORGE_SOURCES
    Source/Params.cpp
    Source/DrumVoice.cpp
    Source/KitFactory.cpp
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/SequencerGrid.cpp)

target_sources(KitForge PRIVATE ${KITFORGE_SOURCES})

target_compile_definitions(KitForge PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(KitForge
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# ------------------------------------------------------------------------------
# Offline render test: plays the default kit + pattern for 4 bars, checks the
# output is sane and writes a WAV you can listen to.
# ------------------------------------------------------------------------------
if(KITFORGE_BUILD_TESTS)
    juce_add_console_app(KitForgeRenderTest PRODUCT_NAME "KitForgeRenderTest")
    target_sources(KitForgeRenderTest PRIVATE Tests/RenderTest.cpp ${KITFORGE_SOURCES})
    target_include_directories(KitForgeRenderTest PRIVATE Source)
    target_compile_definitions(KitForgeRenderTest PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        KITFORGE_NO_PLUGIN_ENTRY=1)
    target_link_libraries(KitForgeRenderTest
        PRIVATE
            juce::juce_audio_utils
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags)
endif()
