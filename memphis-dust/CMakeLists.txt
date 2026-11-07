cmake_minimum_required(VERSION 3.22)
project(MemphisDust VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS version")

# Point JUCE_DIR at a local JUCE checkout to skip the download:
#   cmake -B build -DJUCE_DIR=/path/to/JUCE
if(DEFINED JUCE_DIR)
    add_subdirectory(${JUCE_DIR} ${CMAKE_BINARY_DIR}/JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.4
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

juce_add_plugin(MemphisDust
    PRODUCT_NAME "Memphis Dust"
    COMPANY_NAME "Sam Freeman"
    BUNDLE_ID com.samfreeman.memphisdust
    PLUGIN_MANUFACTURER_CODE Smfr
    PLUGIN_CODE Mdst
    FORMATS VST3 AU Standalone
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE   # set TRUE to auto-install into your system plugin folder
    VST3_CATEGORIES Fx Distortion Dynamics
    AU_MAIN_TYPE kAudioUnitType_Effect)

target_sources(MemphisDust PRIVATE
    Source/Parameters.cpp
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_include_directories(MemphisDust PRIVATE Source)

target_compile_definitions(MemphisDust PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(MemphisDust
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# Pure-C++ DSP tests (no JUCE needed): cmake --build build --target dsp_tests && ./build/dsp_tests
option(MEMPHIS_DUST_TESTS "Build DSP unit tests" ON)
if(MEMPHIS_DUST_TESTS)
    add_executable(dsp_tests tests/DspTests.cpp)
    target_include_directories(dsp_tests PRIVATE Source)
endif()
