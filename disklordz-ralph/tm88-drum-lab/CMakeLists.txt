cmake_minimum_required(VERSION 3.22)
project(DiskLordzTm88DrumLab VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(DL_THREE_LAYER_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/../factory/drum-architecture/Source")
add_library(DiskLordzTm88Core INTERFACE)
target_include_directories(DiskLordzTm88Core INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/Source"
    "${DL_THREE_LAYER_SOURCE}")

add_executable(Tm88M1Renderer Tools/render_milestone1.cpp)
target_link_libraries(Tm88M1Renderer PRIVATE DiskLordzTm88Core)

include(CTest)
if(BUILD_TESTING)
    add_executable(Tm88Milestone1Tests Tests/milestone1_test.cpp)
    target_link_libraries(Tm88Milestone1Tests PRIVATE DiskLordzTm88Core)
    add_test(NAME Tm88Milestone1Tests COMMAND Tm88Milestone1Tests)
endif()

option(DISKLORDZ_TM88_BUILD_JUCE "Build TM88 JUCE VST3/Standalone (requires JUCE)" OFF)
if(DISKLORDZ_TM88_BUILD_JUCE)
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.4
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
    juce_add_plugin(DiskLordzTm88DrumLab
        COMPANY_NAME "DiskLordz Audio"
        BUNDLE_ID "com.disklordzaudio.tm88drumlab"
        PLUGIN_MANUFACTURER_CODE DskT
        PLUGIN_CODE Tm88
        FORMATS VST3 Standalone
        IS_SYNTH TRUE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        PRODUCT_NAME "DiskLordz TM88 Drum Lab")
    target_sources(DiskLordzTm88DrumLab PRIVATE Source/Tm88PluginProcessor.cpp)
    target_link_libraries(DiskLordzTm88DrumLab PRIVATE DiskLordzTm88Core
        juce::juce_audio_utils juce::juce_recommended_config_flags)
    target_compile_definitions(DiskLordzTm88DrumLab PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
endif()