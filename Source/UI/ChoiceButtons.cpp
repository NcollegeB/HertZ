#include "ChoiceButtons.h"

ChoiceButtons::ChoiceButtons(juce::AudioProcessorValueTreeState& state, const char* parameterId, int columns)
    : columnCount(juce::jmax(1, columns))
{
    setName(parameterId);
    auto* choice = dynamic_cast<juce::AudioParameterChoice*>(state.getParameter(parameterId));
    jassert(choice != nullptr);
    if (choice == nullptr)
        return;

    for (int i = 0; i < choice->choices.size(); ++i)
    {
        auto button = std::make_unique<juce::TextButton>(choice->choices[i]);
        button->setName(choice->choices[i]);
        button->setTooltip(choice->choices[i]);
        button->setClickingTogglesState(false);
        button->onClick = [this, i] { attachment->setValueAsCompleteGesture(static_cast<float>(i)); };
        addAndMakeVisible(*button);
        buttons.push_back(std::move(button));
    }
    attachment = std::make_unique<juce::ParameterAttachment>(*choice, [this](float value)
    {
        for (size_t i = 0; i < buttons.size(); ++i)
            buttons[i]->setToggleState(static_cast<int>(i) == juce::roundToInt(value), juce::dontSendNotification);
    }, state.undoManager);
    attachment->sendInitialUpdate();
}

void ChoiceButtons::resized()
{
    const int rows = (static_cast<int>(buttons.size()) + columnCount - 1) / columnCount;
    if (rows == 0)
        return;
    constexpr int gap = 6;
    const int width = juce::jmax(0, (getWidth() - gap * (columnCount - 1)) / columnCount);
    const int height = juce::jmax(0, (getHeight() - gap * (rows - 1)) / rows);
    for (size_t i = 0; i < buttons.size(); ++i)
    {
        const int index = static_cast<int>(i);
        buttons[i]->setBounds((index % columnCount) * (width + gap),
                              (index / columnCount) * (height + gap), width, height);
    }
}
