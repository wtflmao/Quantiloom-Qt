cmake -B build-llvm -G "Visual Studio 18 2026" -T ClangCL -A x64 -DCMAKE_PREFIX_PATH="C:\\Qt\\6.10.1\\llvm-mingw_64"

#cmake --build build --config Debug -j
cmake --build build-llvm --config Release -j