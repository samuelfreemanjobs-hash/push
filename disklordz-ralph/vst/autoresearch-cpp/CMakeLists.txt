# Headless kick renderer for C++ autoresearch — built from starter-cpp with -DDLZ_AUTORESEARCH=ON

cmake_minimum_required(VERSION 3.22)

add_executable(DiskLordzHeadlessRender HeadlessMain.cpp)
target_include_directories(DiskLordzHeadlessRender PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(DiskLordzHeadlessRender PRIVATE cxx_std_20)

target_link_libraries(DiskLordzHeadlessRender
    PRIVATE
        juce::juce_audio_formats
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)