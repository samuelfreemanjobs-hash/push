# Built via starter-cpp: cmake ... -DDLZ_AUTORESEARCH=ON

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