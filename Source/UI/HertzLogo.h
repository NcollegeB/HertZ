#pragma once

#include "HertzTheme.h"

namespace HertzTheme
{
inline void drawLogo(juce::Graphics& g, juce::Rectangle<float> area)
{
    const juce::Colour colours[] {coral, yellow, mint, blue, mint, yellow, coral};
    constexpr float heights[] {0.27f, 0.53f, 0.78f, 1.0f, 0.78f, 0.53f, 0.27f};
    const float step = area.getWidth() / 7.0f;
    const float width = step * 0.65f;
    for (int i = 0; i < 7; ++i)
    {
        const float height = area.getHeight() * heights[i];
        g.setColour(colours[i]);
        g.fillRoundedRectangle(area.getX() + step * static_cast<float>(i),
                               area.getCentreY() - height / 2.0f, width, height, width / 2.0f);
    }
}
}
