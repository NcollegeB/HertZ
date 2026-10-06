#include "PluginEditor.h"

HertzAudioProcessorEditor::HertzAudioProcessorEditor(HertzAudioProcessor& processorToUse) 
: juce::AudioProcessorEditor(processorToUse), 
    keyboardComp(
        processorToUse.getKeyboardState(),
        juce::MidiKeyboardComponent::horizontalKeyboard),
    volAttachment(
        processorToUse.getParameterState(),
        "volume",
        volSlider)
{
    keyboardComp.setAvailableRange(0,127);
    keyboardComp.setLowestVisibleKey(0);
    keyboardComp.setKeyWidth(24.0f);
    
    addAndMakeVisible(keyboardComp);
    
    setSize(640, 300);
}

void HertzAudioProcessorEditor::paint(juce::Graphics &g)
{
    //bg
    g.fillAll(juce::Colour(0xff20232a));

    //tcolor tsize
    g.setColour(juce::Colours::white);
    g.setFont(36.0f);

    g.drawText("Hertz", getLocalBounds(), juce::Justification::centred, true);

}

void HertzAudioProcessorEditor::resized()
{
    // controls l8r g8r
}
