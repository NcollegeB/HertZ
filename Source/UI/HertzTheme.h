#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace HertzTheme
{
inline const juce::Colour background { 0xfff5f2eb };
inline const juce::Colour ink { 0xff243d45 };
inline const juce::Colour muted { 0xff63767b };
inline const juce::Colour coral { 0xfff28a71 };
inline const juce::Colour mint { 0xff66c6b2 };
inline const juce::Colour yellow { 0xffedc868 };
inline const juce::Colour blue { 0xff86b9da };
inline const juce::Colour line { 0xffd4dcd7 };

inline juce::Font font(float height, bool bold = false)
{
    return juce::Font(juce::FontOptions(height, bold ? juce::Font::bold : juce::Font::plain));
}
}
