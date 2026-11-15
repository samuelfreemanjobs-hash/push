cmake_minimum_required(VERSION 3.22)

# DrumKit Pro — JUCE 8 VST3 + WebView2 + AutoGen vst_rd_swarm.py (R&D template, not ship path)
project(DrumKitPlugin VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(DLZ_USE_MONOREPO_JUCE "Use starter-cpp/externals/JUCE (recommended in push repo)" ON)
option(DLZ_COPY_VST3_AFTER_BUILD "Copy VST3 to Program Files after build" OFF)
option(DLZ_WEBVIEW_STATIC "Static WebView2 link (no extra runtime DLLs)" ON)

set(DLZ_STARTER_JUCE "${CMAKE_CURRENT_SOURCE_DIR}/../../starter-cpp/externals/JUCE")

if(DLZ_USE_MONOREPO_JUCE AND EXISTS "${DLZ_STARTER_JUCE}/CMakeLists.txt")
    add_subdirectory("${DLZ_STARTER_JUCE}" "${CMAKE_BINARY_DIR}/JUCE")
else()
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.6
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

juce_add_plugin(DrumKitPlugin
    COMPANY_NAME "DiskLordz"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD ${DLZ_COPY_VST3_AFTER_BUILD}
    PLUGIN_MANUFACTURER_CODE Dloz
    PLUGIN_CODE DkPr
    FORMATS VST3 Standalone
    PRODUCT_NAME "DrumKit Pro"
    NEEDS_WEBVIEW2 TRUE)

juce_generate_juce_header(DrumKitPlugin)

file(GLOB_RECURSE WEB_UI_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/ui/*")
if(WEB_UI_FILES)
    juce_add_binary_data(WebUIAssets
        HEADER_NAME WebUIAssets.h
        NAMESPACE WebUIAssets
        SOURCES ${WEB_UI_FILES})
    target_link_libraries(DrumKitPlugin PRIVATE WebUIAssets)
    target_compile_definitions(DrumKitPlugin PRIVATE DLZ_HAS_EMBEDDED_UI=1)
endif()

target_sources(DrumKitPlugin
    PRIVATE
        src/PluginProcessor.cpp
        src/PluginProcessor.h
        src/PluginEditor.cpp
        src/PluginEditor.h)

target_include_directories(DrumKitPlugin PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_compile_definitions(DrumKitPlugin
    PUBLIC
        JUCE_WEB_BROWSER=1
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
    PRIVATE
        $<$<BOOL:${DLZ_WEBVIEW_STATIC}>:JUCE_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING=1>)

if(MSVC)
    target_compile_options(DrumKitPlugin PRIVATE /W4 /permissive-)
endif()

target_link_libraries(DrumKitPlugin
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

if(NOT DEFINED ENV{PLUGINVAL_STRICTNESS})
    set(DLZ_PLUGINVAL_STRICTNESS "8")
else()
    set(DLZ_PLUGINVAL_STRICTNESS $ENV{PLUGINVAL_STRICTNESS})
endif()

set(_DLZ_PLUGINVAL_HINTS
    "${CMAKE_CURRENT_SOURCE_DIR}/../../tools/pluginval"
    "C:/Tools/pluginval")

find_program(PLUGINVAL_EXECUTABLE pluginval PATHS ${_DLZ_PLUGINVAL_HINTS} ENV PATH)

if(PLUGINVAL_EXECUTABLE)
    set(_VST3_PRODUCT_NAME "DrumKit Pro.vst3")
    add_custom_target(validate_vst3
        COMMAND "${PLUGINVAL_EXECUTABLE}"
            --strictness-level ${DLZ_PLUGINVAL_STRICTNESS}
            --validate "${CMAKE_BINARY_DIR}/DrumKitPlugin_artefacts/$<CONFIG>/VST3/${_VST3_PRODUCT_NAME}"
        DEPENDS DrumKitPlugin_VST3
        COMMENT "pluginval strictness ${DLZ_PLUGINVAL_STRICTNESS}"
        VERBATIM)
endif()