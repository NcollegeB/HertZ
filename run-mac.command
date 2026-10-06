#!/bin/bash
set -euo pipefail

# Double-click to build and open HertZ with the same preset used by VS Code.
# --build-only builds the app without opening it.
mode="${1:---run}"
case "$mode" in
    --run|--build-only) ;;
    --help|-h)
        printf 'Usage: %s [--run|--build-only]\n' "$0"
        exit 0
        ;;
    *) printf 'Unknown option: %s (use --help)\n' "$mode" >&2; exit 1 ;;
esac
[[ $# -le 1 ]] || { printf 'Use at most one option.\n' >&2; exit 1; }
[[ "$(uname -s)" == "Darwin" ]] || { printf 'This launcher requires macOS.\n' >&2; exit 1; }

project_dir="$(cd "$(dirname "$0")" && pwd -P)"
build_dir="$project_dir/build/macos-debug"
app="$build_dir/HertZ_artefacts/Debug/Standalone/HertZ.app"

# Finder does not inherit Homebrew's PATH. Respect the selected Apple tools.
export PATH="/opt/homebrew/bin:/usr/local/bin:/Applications/CMake.app/Contents/bin:$PATH"
for tool in cmake make xcrun; do
    command -v "$tool" >/dev/null 2>&1 || {
        printf 'Missing %s. Install CMake and Apple Command Line Tools.\n' "$tool" >&2
        exit 1
    }
done

xcrun --sdk macosx --find clang++ >/dev/null
xcrun --sdk macosx --show-sdk-path >/dev/null

printf 'Preparing Hertz for this Mac...\n'
cd "$project_dir"
cmake --preset macos-debug

cmake --build --preset macos-debug --target HertZ_Standalone
[[ -x "$app/Contents/MacOS/HertZ" ]] || { printf 'Hertz executable was not created.\n' >&2; exit 1; }
printf '\nBuilt: %s\n' "$app"
if [[ "$mode" == --run ]]; then
    # Start this newly built executable even if an older copy is still open.
    open -n "$app"
fi
