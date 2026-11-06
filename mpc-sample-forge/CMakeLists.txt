cmake_minimum_required(VERSION 3.22)
project(MPCSampleForge VERSION 1.2.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows: link the C++ runtime statically so the .exe runs without installing
# the Visual C++ Redistributable.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# ---------------------------------------------------------------------------
# JUCE: use a local checkout if JUCE_DIR is given, otherwise download it.
#   cmake -B build -DJUCE_DIR=/path/to/JUCE
# ---------------------------------------------------------------------------
set(JUCE_DIR "" CACHE PATH "Path to an existing JUCE checkout (optional)")

if(JUCE_DIR)
    add_subdirectory(${JUCE_DIR} ${CMAKE_BINARY_DIR}/JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        8.0.4
        GIT_SHALLOW    ON)
    FetchContent_MakeAvailable(JUCE)
endif()

set(FORGE_ENGINE_SOURCES
    Source/Theory.cpp
    Source/Styles.cpp
    Source/SamplePlanner.cpp
    Source/OfflineRenderer.cpp
    Source/PluginHost.cpp
    Source/BatchJob.cpp
    Source/CommandLine.cpp
    Source/BuiltInSynth.cpp
    Source/Effects.cpp
    Source/Diagnostics.cpp
    Source/Analysis.cpp
    Source/ProgressionLibrary.cpp
    Source/PresetSweep.cpp
    Source/RenderQueue.cpp)

set(FORGE_DEFINITIONS
    FORGE_VERSION_STRING="${PROJECT_VERSION}"
    JUCE_PLUGINHOST_VST3=1
    JUCE_PLUGINHOST_AU=0
    JUCE_PLUGINHOST_LADSPA=0
    JUCE_PLUGINHOST_LV2=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0)

# ---------------------------------------------------------------------------
# The app (GUI; also accepts --cli on macOS / Linux)
# ---------------------------------------------------------------------------
juce_add_gui_app(MPCSampleForge
    PRODUCT_NAME "MPC Sample Forge"
    COMPANY_NAME "MPCSampleForge"
    VERSION ${PROJECT_VERSION})

target_sources(MPCSampleForge PRIVATE ${FORGE_ENGINE_SOURCES} Source/Main.cpp Source/MainComponent.cpp Source/DiagnosticsPanel.cpp Source/ReviewPanel.cpp Source/StyleEditor.cpp Source/PresetSweepPanel.cpp Source/QueuePanel.cpp)

target_compile_definitions(MPCSampleForge PRIVATE ${FORGE_DEFINITIONS}
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:MPCSampleForge,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:MPCSampleForge,JUCE_VERSION>")

target_link_libraries(MPCSampleForge
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_audio_formats
        juce::juce_audio_devices
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# ---------------------------------------------------------------------------
# mpcforge: command-line tool for automation / scripting (all platforms)
# ---------------------------------------------------------------------------
juce_add_console_app(mpcforge
    PRODUCT_NAME "mpcforge"
    COMPANY_NAME "MPCSampleForge"
    VERSION ${PROJECT_VERSION})

target_sources(mpcforge PRIVATE ${FORGE_ENGINE_SOURCES} Source/CliMain.cpp)
target_compile_definitions(mpcforge PRIVATE ${FORGE_DEFINITIONS} JUCE_MODAL_LOOPS_PERMITTED=1)

target_link_libraries(mpcforge
    PRIVATE
        juce::juce_audio_processors
        juce::juce_audio_formats
        juce::juce_audio_devices
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)

# ---------------------------------------------------------------------------
# Optional: a tiny VST3 instrument (the built-in FM keys) for testing the
# VST3 hosting path without third-party plugins.
#   cmake -B build -DFORGE_BUILD_TEST_PLUGIN=ON
# ---------------------------------------------------------------------------
option(FORGE_BUILD_TEST_PLUGIN "Build the ForgeTestKeys VST3 used for testing" OFF)

if(FORGE_BUILD_TEST_PLUGIN)
    juce_add_plugin(ForgeTestKeys
        PRODUCT_NAME "Forge Test Keys"
        COMPANY_NAME "MPCSampleForge"
        IS_SYNTH TRUE
        NEEDS_MIDI_INPUT TRUE
        PLUGIN_MANUFACTURER_CODE Mpcf
        PLUGIN_CODE Fkys
        FORMATS VST3
        COPY_PLUGIN_AFTER_BUILD FALSE)

    target_sources(ForgeTestKeys PRIVATE Tests/TestPlugin.cpp Source/BuiltInSynth.cpp)
    target_compile_definitions(ForgeTestKeys PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
    target_include_directories(ForgeTestKeys PRIVATE Source)
    target_link_libraries(ForgeTestKeys PRIVATE juce::juce_audio_utils PUBLIC juce::juce_recommended_config_flags)

    # 12 presets built to exercise the preset sweep's checks (duplicates, silence, arps, FX ...)
    juce_add_plugin(ForgeTestSynth
        PRODUCT_NAME "ForgeTestSynth"
        COMPANY_NAME "MPCSampleForge"
        IS_SYNTH TRUE
        NEEDS_MIDI_INPUT TRUE
        PLUGIN_MANUFACTURER_CODE Mpcf
        PLUGIN_CODE Fswp
        FORMATS VST3
        COPY_PLUGIN_AFTER_BUILD FALSE)
    target_sources(ForgeTestSynth PRIVATE Tests/SweepTestPlugin.cpp)
    target_compile_definitions(ForgeTestSynth PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
    target_link_libraries(ForgeTestSynth PRIVATE juce::juce_audio_processors PUBLIC juce::juce_recommended_config_flags)
endif()
