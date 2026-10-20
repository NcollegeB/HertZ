#pragma once

#include "PluginProcessor.h"
#include "UI/ControlCard.h"
#include "UI/ChoiceButtons.h"
#include "UI/HertzKeyboard.h"
#include "UI/HertzLookAndFeel.h"

class HertzAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit HertzAudioProcessorEditor(HertzAudioProcessor&);
    ~HertzAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    HertzLookAndFeel colourStyle;
    HertzKeyboard keyboardComp;
    ControlCard tempoCard {"Tempo", HertzTheme::coral};
    ControlCard chordCard {"Chord", HertzTheme::mint};
    ControlCard playbackCard {"Playback", HertzTheme::yellow};
    ControlCard outputCard {"Output", HertzTheme::blue};
    std::array<ControlCard*, 4> cards {&tempoCard, &chordCard, &playbackCard, &outputCard};

    ChoiceButtons timingButtons, chordButtons, playbackButtons;
    juce::Slider bpmSlider, volSlider;
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    // Controls and parent cards outlive their attachments.
    std::unique_ptr<SliderAttachment> bpmAttachment, volAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HertzAudioProcessorEditor)
};
