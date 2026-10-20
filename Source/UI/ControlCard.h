#pragma once

#include "HertzTheme.h"

class ControlCard final : public juce::Component
{
public:
    ControlCard(juce::String title, juce::Colour colour);
    juce::Component& content() { return body; }
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    juce::String heading;
    juce::Colour fill;
    juce::Component body;
};
