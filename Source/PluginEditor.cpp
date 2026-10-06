#include "PluginEditor.h"

HertzAudioProcessorEditor::HertzAudioProcessorEditor(
    HertzAudioProcessor& processorToUse)
    : juce::AudioProcessorEditor(processorToUse)
{
    setSize(500, 320);
}