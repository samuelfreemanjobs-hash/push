# Built via starter-cpp: cmake ... -DDLZ_AUTORESEARCH=ON

set(DLZ_STARTER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../starter-cpp")

juce_add_console_app(HeadlessRender PRODUCT_NAME "HeadlessRender")

target_sources(HeadlessRender PRIVATE Source/HeadlessMain.cpp Source/VoiceDSP.h)

target_compile_definitions(HeadlessRender
    PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)

target_link_libraries(HeadlessRender
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_formats
        juce::juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)

# ==============================================================================
# VoiceDSP preview VST (OpenMontage screen capture + pluginval strictness 8)
# ==============================================================================
option(DLZ_AUTORESEARCH_VST "Build DrumSynth Autoresearch VST3 + Standalone" ON)

if(DLZ_AUTORESEARCH_VST)
    juce_add_plugin(DrumSynthAutoresearch
        COMPANY_NAME "DiskLordz"
        IS_SYNTH TRUE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS TRUE
        PLUGIN_MANUFACTURER_CODE Dloz
        PLUGIN_CODE Dsar
        FORMATS VST3 Standalone
        PRODUCT_NAME "DrumSynth Autoresearch")

    set(DLZ_LICENSE_SOURCES
        ${DLZ_STARTER_DIR}/Source/LicenseVerifier.cpp
        ${DLZ_STARTER_DIR}/Source/LicenseVerifier.h
        ${DLZ_STARTER_DIR}/Modules/License/crypto/monocypher.c)

    target_sources(DrumSynthAutoresearch
        PRIVATE
            plugin/Source/PluginProcessor.cpp
            plugin/Source/PluginProcessor.h
            plugin/Source/PluginEditor.cpp
            plugin/Source/PluginEditor.h
            Source/VoiceDSP.h
            ${DLZ_LICENSE_SOURCES})

    set_source_files_properties(
        ${DLZ_STARTER_DIR}/Modules/License/crypto/monocypher.c
        PROPERTIES LANGUAGE C)

    target_include_directories(DrumSynthAutoresearch
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/plugin/Source
            ${CMAKE_CURRENT_SOURCE_DIR}/Source
            ${DLZ_STARTER_DIR}/Source
            ${DLZ_STARTER_DIR}/Modules/License)

    target_compile_definitions(DrumSynthAutoresearch
        PUBLIC
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0
            JUCE_VST3_CAN_REPLACE_VST2=0
        PRIVATE
            DLZ_LICENSE_ED25519=1)

    target_link_libraries(DrumSynthAutoresearch
        PRIVATE
            juce::juce_audio_utils
            juce::juce_dsp
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags)
endif()