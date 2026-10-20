#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

// A visible grid of choices bound to one automatable choice parameter.
class ChoiceButtons final : public juce::Component
{
public:
    ChoiceButtons(juce::AudioProcessorValueTreeState&, const char* parameterId, int columns);
    void resized() override;

private:
    int columnCount;
    std::vector<std::unique_ptr<juce::TextButton>> buttons;
    // Disconnect before any button is destroyed.
    std::unique_ptr<juce::ParameterAttachment> attachment;
};
