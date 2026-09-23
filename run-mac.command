#!/bin/bash
set -euo pipefail

# Double-click to build and open Hertz. VS Code uses this same script.
# --build-only builds the app without opening it; --test runs processor checks.
mode="${1:---run}"
case "$mode" in
    --run|--build-only|--test) ;;
    --help|-h)
        printf 'Usage: %s [--run|--build-only|--test]\n' "$0"
        exit 0
        ;;
    *) printf 'Unknown option: %s (use --help)\n' "$mode" >&2; exit 1 ;;
esac
[[ $# -le 1 ]] || { printf 'Use at most one option.\n' >&2; exit 1; }
[[ "$(uname -s)" == "Darwin" ]] || { printf 'This launcher requires macOS.\n' >&2; exit 1; }

project_dir="$(cd "$(dirname "$0")" && pwd -P)"
build_dir="$project_dir/build-vscode"
app="$build_dir/HertZ_artefacts/Debug/Standalone/HertZ.app"

# Finder does not inherit Homebrew's PATH. Use the working standalone Apple tools
# for this process, without changing the system's selected Xcode installation.
export PATH="/opt/homebrew/bin:/usr/local/bin:/Applications/CMake.app/Contents/bin:$PATH"
if [[ -x /Library/Developer/CommandLineTools/usr/bin/clang++ ]]; then
    export DEVELOPER_DIR=/Library/Developer/CommandLineTools
fi
for tool in cmake ninja xcrun; do
    command -v "$tool" >/dev/null 2>&1 || {
        printf 'Missing %s. Install CMake, Ninja, and Apple Command Line Tools.\n' "$tool" >&2
        exit 1
    }
done

sdk_path="$(xcrun --sdk macosx --show-sdk-path)"
c_compiler="$(xcrun --sdk macosx --find clang)"
cpp_compiler="$(xcrun --sdk macosx --find clang++)"

printf 'Preparing Hertz for this Mac...\n'
cmake -S "$project_dir" -B "$build_dir" -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    "-DCMAKE_C_COMPILER=$c_compiler" \
    "-DCMAKE_CXX_COMPILER=$cpp_compiler" \
    "-DCMAKE_MAKE_PROGRAM=$(command -v ninja)" \
    "-DCMAKE_OSX_SYSROOT=$sdk_path" \
    "-DCMAKE_OSX_ARCHITECTURES=$(uname -m)" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DJUCE_SOURCE_DIR:PATH= \
    -DBUILD_TESTING=ON

if [[ "$mode" == --test ]]; then
    cmake --build "$build_dir" --config Debug --parallel 2 --target HertZTests
    ctest --test-dir "$build_dir" -C Debug --output-on-failure --no-tests=error
    exit 0
fi

cmake --build "$build_dir" --config Debug --parallel 2 --target HertZ_Standalone
[[ -x "$app/Contents/MacOS/HertZ" ]] || { printf 'Hertz executable was not created.\n' >&2; exit 1; }
printf '\nBuilt: %s\n' "$app"
if [[ "$mode" == --run ]]; then
    # Start this newly built executable even if an older copy is still open.
    open -n "$app"
fi
