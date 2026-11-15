cmake_minimum_required(VERSION 3.22)
project(DiskLordzLugerDrumLab VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(DL_THREE_LAYER_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/../factory/drum-architecture/Source")
add_library(DiskLordzLugerCore INTERFACE)
target_include_directories(DiskLordzLugerCore INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/Source"
    "${DL_THREE_LAYER_SOURCE}")

add_executable(LugerM1Renderer Tools/render_milestone1.cpp)
target_link_libraries(LugerM1Renderer PRIVATE DiskLordzLugerCore)

include(CTest)
if(BUILD_TESTING)
    add_executable(LugerMilestone1Tests Tests/milestone1_test.cpp)
    target_link_libraries(LugerMilestone1Tests PRIVATE DiskLordzLugerCore)
    add_test(NAME LugerMilestone1Tests COMMAND LugerMilestone1Tests)
endif()

option(DISKLORDZ_LUGER_BUILD_JUCE "Build Luger JUCE VST3/Standalone (requires JUCE)" OFF)
if(DISKLORDZ_LUGER_BUILD_JUCE)
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.4
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
    juce_add_plugin(DiskLordzLugerDrumLab
        COMPANY_NAME "DiskLordz Audio"
        BUNDLE_ID "com.disklordzaudio.lugerdrumlab"
        PLUGIN_MANUFACTURER_CODE DskL
        PLUGIN_CODE LgDr
        FORMATS VST3 Standalone
        IS_SYNTH TRUE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        PRODUCT_NAME "DiskLordz Luger Drum Lab")
    target_sources(DiskLordzLugerDrumLab PRIVATE Source/LugerPluginProcessor.cpp)
    target_link_libraries(DiskLordzLugerDrumLab PRIVATE DiskLordzLugerCore
        juce::juce_audio_utils juce::juce_recommended_config_flags)
    target_compile_definitions(DiskLordzLugerDrumLab PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
endif()