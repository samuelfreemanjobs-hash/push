cmake_minimum_required(VERSION 3.22)
project(DiskLordzCinematicDSP VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_library(DiskLordzDSP INTERFACE)
target_include_directories(DiskLordzDSP INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Source)
add_executable(CinematicRenderer Tools/render_main.cpp)
target_link_libraries(CinematicRenderer PRIVATE DiskLordzDSP)
include(CTest)
if(BUILD_TESTING)
    add_executable(CinematicTests Tests/dsp_test.cpp)
    target_link_libraries(CinematicTests PRIVATE DiskLordzDSP)
    add_test(NAME CinematicDSPTests COMMAND CinematicTests)
endif()
# Optional JUCE 8 plugin, opt-in because JUCE is not preinstalled on all machines.
option(DISKLORDZ_BUILD_JUCE_PLUGIN "Build JUCE VST3 and Standalone targets" OFF)
if(DISKLORDZ_BUILD_JUCE_PLUGIN)
    include(FetchContent)
    FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.4 GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
    juce_add_plugin(DiskLordzCinematic
        COMPANY_NAME "DiskLordz Audio"
        BUNDLE_ID "com.disklordzaudio.cinematic"
        PLUGIN_MANUFACTURER_CODE DskL
        PLUGIN_CODE CnDr
        FORMATS VST3 Standalone
        IS_SYNTH TRUE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        PRODUCT_NAME "DiskLordz Cinematic DSP")
    target_sources(DiskLordzCinematic PRIVATE Source/PluginProcessor.cpp)
    target_link_libraries(DiskLordzCinematic PRIVATE DiskLordzDSP juce::juce_audio_utils juce::juce_recommended_config_flags)
    target_compile_definitions(DiskLordzCinematic PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
endif()
