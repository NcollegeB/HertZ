#pragma once 

#include "PluginProcessor.h"

class HertzAudioProcessorEditor final : public juce::AudioProcessorEditor
{
    public:
    explicit HertzAudioProcessorEditor(HertzAudioProcessor& processorToUse);

    ~HertzAudioProcessorEditor() override = default;

    void paint(juce::Graphics& g) override;

    void resized() override;

    private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HertzAudioProcessorEditor);
};


