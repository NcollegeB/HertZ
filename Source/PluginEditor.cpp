#include "PluginEditor.h"
#include "UI/HertzLogo.h"

HertzAudioProcessorEditor::HertzAudioProcessorEditor(HertzAudioProcessor& processorToUse)
    : juce::AudioProcessorEditor(processorToUse),
      keyboardComp(processorToUse.getKeyboardState()),
      timingButtons(processorToUse.getParameterState(), "timing", 4),
      chordButtons(processorToUse.getParameterState(), "chord", 3),
      playbackButtons(processorToUse.getParameterState(), "playback", 1)
{
    setLookAndFeel(&colourStyle);
    setOpaque(true);
    addAndMakeVisible(keyboardComp);
    for (auto* card : cards)
        addAndMakeVisible(*card);

    tempoCard.content().addAndMakeVisible(timingButtons);
    chordCard.content().addAndMakeVisible(chordButtons);
    playbackCard.content().addAndMakeVisible(playbackButtons);

    auto& state = processorToUse.getParameterState();
    tempoCard.content().addAndMakeVisible(bpmSlider);
    bpmSlider.setSliderStyle(juce::Slider::IncDecButtons);
    bpmSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 112, 36);
    bpmSlider.setTextValueSuffix(" BPM");
    bpmSlider.setName("Tempo BPM");
    bpmSlider.setNumDecimalPlacesToDisplay(1);
    bpmSlider.setIncDecButtonsMode(juce::Slider::incDecButtonsDraggable_Horizontal);
    bpmSlider.setTooltip("Tempo: 30 to 300 BPM. Type a value or use the buttons.");
    bpmAttachment = std::make_unique<SliderAttachment>(state, "bpm", bpmSlider);

    outputCard.content().addAndMakeVisible(volSlider);
    volSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    volSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 96, 36);
    volSlider.setTextValueSuffix(" dB");
    volSlider.setName("Master volume");
    volSlider.setNumDecimalPlacesToDisplay(1);
    volAttachment = std::make_unique<SliderAttachment>(state, "volume", volSlider);

    setResizable(true, true);
    setResizeLimits(820, 660, 1400, 1000);
    setSize(1000, 700);
}

HertzAudioProcessorEditor::~HertzAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void HertzAudioProcessorEditor::paint(juce::Graphics& g)
{
    using namespace HertzTheme;
    g.fillAll(background);
    drawLogo(g, {30.0f, 29.0f, 63.0f, 44.0f});
    g.setColour(ink);
    g.setFont(font(44.0f, true));
    g.drawText("HertZ", 110, 24, 220, 56, juce::Justification::centredLeft);
}

void HertzAudioProcessorEditor::resized()
{
    constexpr int margin = 28, gap = 18;
    const int width = (getWidth() - 2 * margin - gap) / 2;
    const int height = (getHeight() - 268 - gap) / 2;
    for (size_t i = 0; i < cards.size(); ++i)
        cards[i]->setBounds(margin + static_cast<int>(i % 2) * (width + gap),
                            96 + static_cast<int>(i / 2) * (height + gap), width, height);

    const int contentWidth = tempoCard.content().getWidth();
    const int contentHeight = tempoCard.content().getHeight();
    bpmSlider.setBounds(0, 0, contentWidth, 36);
    timingButtons.setBounds(0, 48, contentWidth, 64);
    chordButtons.setBounds(0, 0, contentWidth, juce::jmin(94, contentHeight));
    playbackButtons.setBounds(0, 0, contentWidth, juce::jmin(112, contentHeight));
    volSlider.setBounds(0, (contentHeight - 40) / 2, contentWidth, 40);
    keyboardComp.setBounds(margin, getHeight() - 136, getWidth() - 2 * margin, 108);
}
