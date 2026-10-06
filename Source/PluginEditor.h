#pragma once 

#include "PluginProcessor.h"
#include <juce_audio_utils/juce_audio_utils.h>

class HertzAudioProcessorEditor final : public juce::AudioProcessorEditor
{
    public:
    explicit HertzAudioProcessorEditor(HertzAudioProcessor& processorToUse);

    ~HertzAudioProcessorEditor() override = default;

    void paint(juce::Graphics& g) override;

    void resized() override;
    
    private:
    juce::MidiKeyboardComponent keyboardComp;

    juce::Slider volSlider;
    juce::Label volLabel;
    
    //slider should outlive its attachment
    juce::AudioProcessorValueTreeState::SliderAttachment volAttachment;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HertzAudioProcessorEditor);
};


