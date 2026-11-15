cmake_minimum_required(VERSION 3.22)
project(DiskLordzMaybachDrumLab VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(DL_THREE_LAYER_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/../factory/drum-architecture/Source")
add_library(DiskLordzMaybachCore INTERFACE)
target_include_directories(DiskLordzMaybachCore INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/Source"
    "${DL_THREE_LAYER_SOURCE}")

add_executable(MaybachM1Renderer Tools/render_milestone1.cpp)
target_link_libraries(MaybachM1Renderer PRIVATE DiskLordzMaybachCore)

include(CTest)
if(BUILD_TESTING)
    add_executable(MaybachMilestone1Tests Tests/milestone1_test.cpp)
    target_link_libraries(MaybachMilestone1Tests PRIVATE DiskLordzMaybachCore)
    add_test(NAME MaybachMilestone1Tests COMMAND MaybachMilestone1Tests)
endif()

option(DISKLORDZ_MAYBACH_BUILD_JUCE "Build MAYBACH DRUM LAB PRO JUCE VST3/Standalone (requires JUCE)" OFF)
if(DISKLORDZ_MAYBACH_BUILD_JUCE)
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.4
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
    juce_add_plugin(DiskLordzMaybachDrumLab
        COMPANY_NAME "DiskLordz Audio"
        BUNDLE_ID "com.disklordzaudio.maybachdrumlab"
        PLUGIN_MANUFACTURER_CODE DskM
        PLUGIN_CODE Mybh
        FORMATS VST3 Standalone
        IS_SYNTH TRUE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        PRODUCT_NAME "Maybach Drum Lab PRO")
    target_sources(DiskLordzMaybachDrumLab PRIVATE Source/MaybachPluginProcessor.cpp)
    target_link_libraries(DiskLordzMaybachDrumLab PRIVATE DiskLordzMaybachCore
        juce::juce_audio_utils juce::juce_recommended_config_flags)
    target_compile_definitions(DiskLordzMaybachDrumLab PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
endif()