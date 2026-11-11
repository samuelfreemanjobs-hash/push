cmake_minimum_required(VERSION 3.22)
project(PhonkFactory VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(PHONK_BUILD_PLUGIN "Build the JUCE VST3 + Standalone targets (needs JUCE, downloaded automatically)" ON)
option(PHONK_BUILD_CLI "Build the kitgen command line tool (no JUCE needed)" ON)

# ---- JUCE-free DSP core: kit synthesis, pattern generator, loop renderer, WAV export ----
add_library(PhonkCore STATIC Source/PhonkDSP.cpp)
target_include_directories(PhonkCore PUBLIC Source)

if(PHONK_BUILD_CLI)
    add_executable(kitgen Tools/kitgen.cpp)
    target_link_libraries(kitgen PRIVATE PhonkCore)
endif()

if(PHONK_BUILD_PLUGIN)
    include(FetchContent)
    set(FETCHCONTENT_QUIET OFF)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.9
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)

    set(PHONK_FORMATS VST3 Standalone)
    if(APPLE)
        list(APPEND PHONK_FORMATS AU)
    endif()

    juce_add_plugin(PhonkFactory
        COMPANY_NAME "Phonk Factory"
        PRODUCT_NAME "Phonk Factory"
        BUNDLE_ID com.phonkfactory.phonkfactory
        PLUGIN_MANUFACTURER_CODE Pnkf
        PLUGIN_CODE Pkfc
        IS_SYNTH TRUE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD FALSE
        VST3_CATEGORIES Instrument Drum
        FORMATS ${PHONK_FORMATS})

    target_sources(PhonkFactory PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp)

    target_compile_definitions(PhonkFactory PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0)

    target_link_libraries(PhonkFactory
        PRIVATE
            PhonkCore
            juce::juce_audio_utils
            juce::juce_audio_processors
            juce::juce_gui_basics
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags)

    # headless regression test of the processor (sequencer timing, host sync, export, editor)
    juce_add_console_app(PhonkSelfTest PRODUCT_NAME "PhonkSelfTest")
    target_sources(PhonkSelfTest PRIVATE
        Tools/selftest.cpp
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp)
    target_include_directories(PhonkSelfTest PRIVATE Source)
    target_compile_definitions(PhonkSelfTest PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_MODAL_LOOPS_PERMITTED=1)
    target_link_libraries(PhonkSelfTest
        PRIVATE
            PhonkCore
            juce::juce_audio_utils
            juce::juce_audio_processors
            juce::juce_gui_basics
        PUBLIC
            juce::juce_recommended_config_flags)
endif()
