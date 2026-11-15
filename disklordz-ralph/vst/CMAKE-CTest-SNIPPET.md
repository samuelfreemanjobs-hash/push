# Catch2 + CTest — already in `starter-cpp/CMakeLists.txt`

Superpowers can run headless tests immediately after bootstrap:

```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64 -DDLZ_BUILD_TESTS=ON
cmake --build build --config Release --target TrapCoreDSP_tests --parallel
ctest --test-dir build -C Release --output-on-failure
```

## What CMake does

Root `CMakeLists.txt` (from `starter-cpp/`):

1. `add_subdirectory(Modules/TrapCoreDSP)` — static lib
2. `enable_testing()` + `add_subdirectory(tests)` when `DLZ_BUILD_TESTS=ON`
3. `tests/CMakeLists.txt` — `FetchContent` Catch2 v3.7.1, `catch_discover_tests`

## Extend TDD

Add `tests/test_*.cpp`; link JUCE only when testing real `renderNextBlock`:

```cmake
target_link_libraries(TrapCoreDSP_tests PRIVATE TrapCoreDSP juce::juce_audio_basics)
```