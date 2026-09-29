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
This enables the make file to grab all the necessary libraries before compilation. 
since we have a `HertZ/build/<selected-preset>/_deps/juce-src`.

coming soon.
