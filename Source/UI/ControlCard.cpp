#include "ControlCard.h"

ControlCard::ControlCard(juce::String title, juce::Colour colour)
    : heading(std::move(title)), fill(colour)
{
    setName(heading);
    addAndMakeVisible(body);
}

void ControlCard::paint(juce::Graphics& g)
{
    g.setColour(fill);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 24.0f);
    g.setColour(HertzTheme::ink);
    g.setFont(HertzTheme::font(19.0f, true));
    g.drawText(heading, 22, 15, getWidth() - 44, 26, juce::Justification::centredLeft);
}

void ControlCard::resized()
{
    body.setBounds(22, 48, juce::jmax(0, getWidth() - 44), juce::jmax(0, getHeight() - 64));
}
