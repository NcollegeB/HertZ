#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

class HertzKeyboard final : public juce::MidiKeyboardComponent
{
public:
    explicit HertzKeyboard(juce::MidiKeyboardState&);

private:
    void drawWhiteNote(int, juce::Graphics&, juce::Rectangle<float>, bool, bool,
                       juce::Colour, juce::Colour) override;
    void drawBlackNote(int, juce::Graphics&, juce::Rectangle<float>, bool, bool,
                       juce::Colour) override;
};
