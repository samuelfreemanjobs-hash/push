include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.7.1)
FetchContent_MakeAvailable(Catch2)

add_executable(TrapCoreDSP_tests
    test_pitch_glide.cpp)

target_link_libraries(TrapCoreDSP_tests PRIVATE Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(TrapCoreDSP_tests)