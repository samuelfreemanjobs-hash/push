add_library(TrapCoreDSP STATIC
    Trap808Voice.cpp
    Trap808Voice.h
    TrapSamplerSound.cpp
    TrapSamplerSound.h
    SoftClipper.cpp
    SoftClipper.h)

target_include_directories(TrapCoreDSP PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(TrapCoreDSP
    PUBLIC
        juce::juce_audio_basics
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_dsp)