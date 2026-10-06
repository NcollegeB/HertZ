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

    volSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    volSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 24);

    volSlider.setTextValueSuffix(" dB");

    addAndMakeVisible(volSlider);


    // label
    volLabel.setText("Volume", juce::dontSendNotification);
    volLabel.setJustificationType(juce::Justification::centredLeft);
    volLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    setSize(640, 300);
}

void HertzAudioProcessorEditor::paint(juce::Graphics &g)
{
    //bg
    g.fillAll(juce::Colour(0xff20232a));

    //tcolor tsize
    g.setColour(juce::Colours::white);
    g.setFont(36.0f);

    g.drawText("Hertz", 20, 20, getWidth() - 40, 50, juce::Justification::centred, true);

}

void HertzAudioProcessorEditor::resized()
{
    volLabel.setBounds(20, 90, 70, 30);

    volSlider.setBounds(100, 90, getWidth() - 120, 30); 

    keyboardComp.setBounds(20, getHeight() - 120, getWidth() - 40, 100);
}
