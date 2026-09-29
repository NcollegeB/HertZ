# HertZ

## What is it?

Hertz is an open-source synth plugin for Ableton, FL studio, and works standalone.
Currently it contains a single oscillator, Built in MIDI input, volume, Sample rate, and, 

## How to use it

1. Cmake configure preset
2. Cmake configure
3. Cmake build

## How to build it

### Making:
After running cmake build, cmake will automatically try to download the pinned github JUCE version into your dependencies folder. This means cmake and or Apple command line tools 
are required to run. 


### PROBLEMS with your JUCE library
Please make sure to go into `CmakeUserPresets.json` and add your JUCE directory into the `"JUCE_SOURCE_DIR/JUCE": (your directory here)`
This enables the make file to grab all the necessary libraries before compilation. Common errors could be if this is pointing to the wrong file path. 


### Configuring Intellisense:
Intellisense should work after one successful make and Reload Window (If in VSCODE), If juce:: does not auto populate with attributes then please check the `CmakeLists.txt` file and find 
```
target_link_libraries(HertZ
    PRIVATE juce::juce_audio_utils
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
```
Configure this to make sure it is using your appropriate libraries. 

### If all else fails: 
Run the build-mac.command and the run-mac.command if all else fails and for some reason you cannot get anything to work these are just bash scripts that will do all the work automatically for you. 

### QNA:
