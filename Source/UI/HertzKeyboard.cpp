#include "HertzKeyboard.h"
#include "HertzTheme.h"

HertzKeyboard::HertzKeyboard(juce::MidiKeyboardState& state)
    : MidiKeyboardComponent(state, horizontalKeyboard)
{
    setAvailableRange(0, 127);
    setLowestVisibleKey(48);
    setKeyWidth(32.0f);
    setOctaveForMiddleC(4);
    setColour(whiteNoteColourId, HertzTheme::background);
    setColour(blackNoteColourId, juce::Colours::black);
    setColour(keySeparatorLineColourId, HertzTheme::line);
    setColour(shadowColourId, juce::Colours::transparentBlack);
    setColour(upDownButtonBackgroundColourId, HertzTheme::background);
    setColour(upDownButtonArrowColourId, HertzTheme::ink);
}

namespace
{
juce::Colour keyColour(int note)
{
    const juce::Colour palette[] {HertzTheme::coral, HertzTheme::mint, HertzTheme::yellow, HertzTheme::blue};
    return palette[(note % 12) / 3];
}
}

void HertzKeyboard::drawWhiteNote(int note, juce::Graphics& g, juce::Rectangle<float> area,
                                 bool down, bool over, juce::Colour, juce::Colour)
{
    using namespace HertzTheme;
    const auto colour = keyColour(note);
    const auto key = area.reduced(1.2f, 1.0f);
    g.setColour(down ? colour.darker(0.18f) : over ? colour.brighter(0.15f)
                                                : colour.interpolatedWith(background, 0.2f));
    g.fillRoundedRectangle(key, 6.0f);
    if (down)
    {
        g.setColour(ink.withAlpha(0.65f));
        g.drawRoundedRectangle(key.reduced(1.0f), 5.0f, 2.0f);
    }
    if (note % 12 == 0 && area.getX() >= 20.0f
        && area.getRight() <= static_cast<float>(getWidth() - 20))
    {
        g.setColour(ink);
        g.setFont(font(11.0f, true));
        g.drawText(getWhiteNoteText(note), area.toNearestInt().removeFromBottom(22),
                   juce::Justification::centred);
    }
}

void HertzKeyboard::drawBlackNote(int note, juce::Graphics& g, juce::Rectangle<float> area,
                                 bool down, bool over, juce::Colour)
{
    using namespace HertzTheme;
    const auto colour = keyColour(note);
    g.setColour(down ? colour : ink.interpolatedWith(colour, over ? 0.55f : 0.28f));
    g.fillRoundedRectangle(area.reduced(0.8f, 0.5f), 4.0f);
}
