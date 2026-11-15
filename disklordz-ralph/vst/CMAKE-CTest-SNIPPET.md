# Catch2 v3 + CTest + TrapCoreDSP

Integrated in **`starter-cpp/CMakeLists.txt`** (copied to VST project root on bootstrap).

```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64 -DDLZ_BUILD_TESTS=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

| Target | Role |
|--------|------|
| `TrapCoreDSP` | `Modules/TrapCoreDSP/*.cpp` |
| `TrapCoreDSPTests` | `Tests/Test808Glide.cpp`, `Tests/TestSoftClipper.cpp` |
| `DetroitPhonkTrapMaker` | Links `TrapCoreDSP` |

Catch2 repo: `https://github.com/catchorg/Catch2.git` tag **v3.7.1** (not bare `github.com`).