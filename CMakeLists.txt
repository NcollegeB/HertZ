cmake_minimum_required(VERSION 3.22)
project(HertZ VERSION 0.1.0 LANGUAGES C CXX)

# Let the editor use the same include paths and definitions as the real build.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Keep the Windows plugin self-contained with respect to the MSVC runtime.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Point this at an existing JUCE checkout, or let CMake fetch the pinned release.
set(JUCE_SOURCE_DIR "" CACHE PATH "Optional path to JUCE source")
if(JUCE_SOURCE_DIR)
    add_subdirectory("${JUCE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/JUCE")
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 72782788ce18c2d4d760b28e0921d6ffc6431102) # JUCE 9.0.2
    FetchContent_MakeAvailable(JUCE)
endif()

juce_add_plugin(HertZ
    COMPANY_NAME "HertZ"
    BUNDLE_ID "com.baretone.synth"
    PLUGIN_MANUFACTURER_CODE Bton
    PLUGIN_CODE Btsn
    PRODUCT_NAME "HertZ"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    NEEDS_WEB_BROWSER FALSE
    NEEDS_CURL FALSE
    VST3_CATEGORIES Instrument Synth
    FORMATS VST3 Standalone)

target_sources(HertZ PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/SineVoice.h)
target_compile_features(HertZ PUBLIC cxx_std_17)
target_compile_definitions(HertZ PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(HertZ
    PRIVATE juce::juce_audio_utils
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

include(CTest)
if(BUILD_TESTING)
    juce_add_console_app(HertZTests PRODUCT_NAME "HertZ checks")
    target_sources(HertZTests PRIVATE Tests/ProcessorTests.cpp)
    target_include_directories(HertZTests PRIVATE Source)
    target_link_libraries(HertZTests PRIVATE HertZ juce::juce_audio_processors)
    add_test(NAME HertZProcessor COMMAND HertZTests)
endif()
