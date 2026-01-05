cmake -B build-msvc -G "Visual Studio 18 2026" -A x64 -DQUANTILOOM_BUILD_TESTS=OFF -DCMAKE_PREFIX_PATH="C:\\Qt\\6.10.1\\msvc2022_64"

#cmake --build build --config Debug -j
cmake --build build-msvc --config Release -j