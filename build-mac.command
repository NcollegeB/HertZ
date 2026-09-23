#!/bin/bash
set -euo pipefail

# Usage: bash build-mac.command [path-to-an-existing-JUCE-checkout]
# Requires macOS 11+, Apple command line tools, and CMake 3.22 or newer.

fail() { printf 'Error: %s\n' "$*" >&2; exit 1; }

[[ "$(uname -s)" == "Darwin" ]] || fail "Run this script on a Mac."
[[ $# -le 1 ]] || fail "Usage: bash build-mac.command [path-to-JUCE]"

project_dir="$(cd "$(dirname "$0")" && pwd -P)"
build_dir="$project_dir/build-mac"
dist_dir="$project_dir/dist-mac"
archive="$dist_dir/HertZ-macOS-universal.zip"
juce_source="${1:-${HERTZ_JUCE_SOURCE_DIR:-}}"

# Include common CMake install locations when launched from Finder.
export PATH="/opt/homebrew/bin:/usr/local/bin:/Applications/CMake.app/Contents/bin:$PATH"
for tool in cmake ctest make xcrun ditto codesign; do
    command -v "$tool" >/dev/null 2>&1 || fail "Missing $tool. Install CMake from https://cmake.org/download/ and Apple command line tools with xcode-select --install, then retry."
done
xcrun --sdk macosx --find clang++ >/dev/null 2>&1 || fail "Apple's macOS compiler is unavailable. Run xcode-select --install, or select your installed Xcode developer directory."
sdk_path="$(xcrun --sdk macosx --show-sdk-path)"
[[ -d "$sdk_path" ]] || fail "The selected macOS SDK does not exist."
macos_major="$(sw_vers -productVersion | cut -d . -f 1)"
[[ "$macos_major" -ge 11 ]] || fail "Building and running these checks requires macOS 11 or later."

configure_args=(
    -S "$project_dir" -B "$build_dir" -G "Unix Makefiles"
    -DCMAKE_BUILD_TYPE=Release
    -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0
    "-DCMAKE_OSX_SYSROOT=$sdk_path"
    "-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64"
    -DBUILD_TESTING=ON
)
if [[ -n "$juce_source" ]]; then
    [[ -f "$juce_source/CMakeLists.txt" ]] || fail "The JUCE path must contain its CMakeLists.txt."
    juce_source="$(cd "$juce_source" && pwd -P)"
fi
# An empty value selects the pinned JUCE release in CMakeLists.txt.
configure_args+=("-DJUCE_SOURCE_DIR=$juce_source")

printf 'Configuring universal arm64/x86_64 Release build for macOS 11+...\n'
cmake "${configure_args[@]}"
cmake --build "$build_dir" --config Release --parallel 2 \
    --target HertZ_Standalone HertZ_VST3 HertZTests

printf '\nRunning processor checks on this Mac...\n'
ctest --test-dir "$build_dir" -C Release --output-on-failure

release_dir="$build_dir/HertZ_artefacts/Release"
app="$release_dir/Standalone/HertZ.app"
plugin="$release_dir/VST3/HertZ.vst3"
test_binary="$build_dir/HertZTests_artefacts/Release/HertZTests"
for binary in "$app/Contents/MacOS/HertZ" "$plugin/Contents/MacOS/HertZ" "$test_binary"; do
    [[ -x "$binary" ]] || fail "Expected executable not found: $binary"
    xcrun lipo -verify_arch arm64 x86_64 "$binary"
    printf 'Verified architectures: %s: ' "$binary"
    xcrun lipo -archs "$binary"
done

# Ad-hoc signing requires no developer account and does not notarize the bundles.
for bundle in "$app" "$plugin"; do
    codesign --force --sign - --timestamp=none "$bundle"
    codesign --verify --deep --strict --verbose=2 "$bundle"
done

mkdir -p "$dist_dir"
stage_parent="$(mktemp -d "$build_dir/package.XXXXXX")"
stage="$stage_parent/HertZ"
mkdir "$stage"
ditto "$app" "$stage/HertZ.app"
ditto "$plugin" "$stage/HertZ.vst3"
ditto "$project_dir/JUCE-LICENSE.md" "$stage/JUCE-LICENSE.md"
[[ -x "$stage/HertZ.app/Contents/MacOS/HertZ" ]] || fail "The packaged app lost its executable permission."
[[ -x "$stage/HertZ.vst3/Contents/MacOS/HertZ" ]] || fail "The packaged plugin lost its executable permission."
ditto -c -k --sequesterRsrc --keepParent "$stage" "$stage_parent/HertZ-macOS-universal.zip"
mv -f "$stage_parent/HertZ-macOS-universal.zip" "$archive"

printf '\nNative processor checks passed; both architecture slices verified.\n'
printf 'Standalone app: %s\n' "$app"
printf 'VST3 plugin: %s\n' "$plugin"
printf 'Universal archive: %s\n' "$archive"
printf 'The bundles are ad-hoc signed and are not notarized.\n'
