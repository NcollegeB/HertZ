#include "HertzLookAndFeel.h"

HertzLookAndFeel::HertzLookAndFeel()
{
    using namespace HertzTheme;
    setColour(juce::Label::textColourId, ink);
    setColour(juce::ComboBox::textColourId, ink);
    setColour(juce::ComboBox::backgroundColourId, juce::Colours::white);
    setColour(juce::ComboBox::outlineColourId, ink.withAlpha(0.15f));
    setColour(juce::ComboBox::arrowColourId, ink);
    setColour(juce::PopupMenu::backgroundColourId, background);
    setColour(juce::PopupMenu::textColourId, ink);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, mint);
    setColour(juce::PopupMenu::highlightedTextColourId, ink);
    setColour(juce::Slider::textBoxTextColourId, ink);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::white);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::TextEditor::textColourId, ink);
    setColour(juce::TextEditor::backgroundColourId, juce::Colours::white);
    setColour(juce::TextEditor::highlightColourId, mint);
    setColour(juce::TextEditor::focusedOutlineColourId, ink);
    setColour(juce::CaretComponent::caretColourId, ink);
    setColour(juce::TextButton::textColourOffId, ink);
    setColour(juce::TextButton::textColourOnId, juce::Colours::white);
}

juce::Font HertzLookAndFeel::getLabelFont(juce::Label&) { return HertzTheme::font(14.0f, true); }
juce::Font HertzLookAndFeel::getComboBoxFont(juce::ComboBox&) { return HertzTheme::font(16.0f, true); }
juce::Font HertzLookAndFeel::getPopupMenuFont() { return HertzTheme::font(16.0f); }
juce::Font HertzLookAndFeel::getTextButtonFont(juce::TextButton&, int height) { return HertzTheme::font(juce::jlimit(12.0f, 15.0f, static_cast<float>(height) * 0.44f), true); }

juce::Label* HertzLookAndFeel::createSliderTextBox(juce::Slider& slider)
{
    auto* label = juce::LookAndFeel_V4::createSliderTextBox(slider);
    label->setColour(juce::Label::textColourId, HertzTheme::ink);
    label->setColour(juce::Label::backgroundColourId, juce::Colours::white.withAlpha(0.9f));
    label->setColour(juce::Label::outlineColourId, HertzTheme::ink.withAlpha(0.15f));
    label->setColour(juce::Label::textWhenEditingColourId, HertzTheme::ink);
    label->setColour(juce::Label::backgroundWhenEditingColourId, juce::Colours::white);
    return label;
}

void HertzLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
    const juce::Colour&, bool over, bool down)
{
    g.setColour(button.getToggleState() ? HertzTheme::ink
                                       : juce::Colours::white.withAlpha(down ? 0.5f : over ? 1.0f : 0.8f));
    g.fillRoundedRectangle(button.getLocalBounds().toFloat().reduced(1.0f), 11.0f);
    if (button.hasKeyboardFocus(true))
    {
        g.setColour(HertzTheme::ink);
        g.drawRoundedRectangle(button.getLocalBounds().toFloat().reduced(1.0f), 11.0f, 1.5f);
    }
}

void HertzLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool down,
                                   int, int, int, int, juce::ComboBox& box)
{
    const auto bounds = juce::Rectangle<float>(0, 0, static_cast<float>(width), static_cast<float>(height)).reduced(0.5f);
    g.setColour(juce::Colours::white.withAlpha(box.isEnabled() ? 0.9f : 0.45f));
    g.fillRoundedRectangle(bounds, 7.0f);
    g.setColour(HertzTheme::ink.withAlpha(down || box.hasKeyboardFocus(true) ? 0.8f : 0.15f));
    g.drawRoundedRectangle(bounds, 7.0f, 1.0f);
    juce::Path arrow;
    const auto x = static_cast<float>(width - 21), y = static_cast<float>(height) / 2.0f;
    arrow.startNewSubPath(x - 4.0f, y - 2.0f);
    arrow.lineTo(x, y + 2.0f);
    arrow.lineTo(x + 4.0f, y - 2.0f);
    g.setColour(HertzTheme::ink);
    g.strokePath(arrow, juce::PathStrokeType(1.5f));
}

void HertzLookAndFeel::positionComboBoxText(juce::ComboBox& box, juce::Label& label)
{
    label.setBounds(10, 0, box.getWidth() - 38, box.getHeight());
    label.setFont(getComboBoxFont(box));
}

void HertzLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height)
{
    g.fillAll(HertzTheme::background);
    g.setColour(HertzTheme::line);
    g.drawRect(0, 0, width, height);
}

void HertzLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
    float sliderPos, float, float, juce::Slider::SliderStyle, juce::Slider&)
{
    const float centre = static_cast<float>(y) + static_cast<float>(height) / 2.0f;
    g.setColour(HertzTheme::ink.withAlpha(0.16f));
    g.fillRoundedRectangle(static_cast<float>(x), centre - 3.0f, static_cast<float>(width), 6.0f, 3.0f);
    g.setColour(HertzTheme::ink);
    g.fillRoundedRectangle(static_cast<float>(x), centre - 3.0f,
                           juce::jmax(0.0f, sliderPos - static_cast<float>(x)), 6.0f, 3.0f);
    g.fillEllipse(sliderPos - 8.0f, centre - 8.0f, 16.0f, 16.0f);
    g.setColour(juce::Colours::white);
    g.fillEllipse(sliderPos - 3.0f, centre - 3.0f, 6.0f, 6.0f);
}
