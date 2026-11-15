cmake_minimum_required(VERSION 3.22)
project(DiskLordzGoodMoneyDrumLab VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(DL_THREE_LAYER_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/../factory/drum-architecture/Source")
add_library(DiskLordzGoodMoneyCore INTERFACE)
target_include_directories(DiskLordzGoodMoneyCore INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/Source"
    "${DL_THREE_LAYER_SOURCE}")

add_executable(GoodMoneyM1Renderer Tools/render_milestone1.cpp)
target_link_libraries(GoodMoneyM1Renderer PRIVATE DiskLordzGoodMoneyCore)

include(CTest)
if(BUILD_TESTING)
    add_executable(GoodMoneyMilestone1Tests Tests/milestone1_test.cpp)
    target_link_libraries(GoodMoneyMilestone1Tests PRIVATE DiskLordzGoodMoneyCore)
    add_test(NAME GoodMoneyMilestone1Tests COMMAND GoodMoneyMilestone1Tests)
endif()

option(DISKLORDZ_GOODMONEY_BUILD_JUCE "Build GOOD MONEY DRUM LAB PRO JUCE VST3/Standalone" OFF)
if(DISKLORDZ_GOODMONEY_BUILD_JUCE)
    set(DLZ_JUCE "${CMAKE_CURRENT_SOURCE_DIR}/../vst/starter-cpp/externals/JUCE")
    if(EXISTS "${DLZ_JUCE}/CMakeLists.txt")
        add_subdirectory("${DLZ_JUCE}" "${CMAKE_BINARY_DIR}/JUCE")
    else()
        include(FetchContent)
        FetchContent_Declare(JUCE
            GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
            GIT_TAG 8.0.6
            GIT_SHALLOW TRUE)
        FetchContent_MakeAvailable(JUCE)
    endif()
    juce_add_plugin(DiskLordzGoodMoneyDrumLab
        COMPANY_NAME "DiskLordz Audio"
        BUNDLE_ID "com.disklordzaudio.goodmoneydrumlab"
        PLUGIN_MANUFACTURER_CODE DskG
        PLUGIN_CODE GdmL
        FORMATS VST3 Standalone
        IS_SYNTH TRUE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        PRODUCT_NAME "Good Money Drum Lab PRO")
    target_sources(DiskLordzGoodMoneyDrumLab PRIVATE Source/GoodMoneyPluginProcessor.cpp)
    target_link_libraries(DiskLordzGoodMoneyDrumLab PRIVATE DiskLordzGoodMoneyCore
        juce::juce_audio_utils juce::juce_recommended_config_flags)
    target_compile_definitions(DiskLordzGoodMoneyDrumLab PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
endif()