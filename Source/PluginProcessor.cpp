#include "PluginProcessor.h"

HertZAudioProcessor::HertZAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "BareToneState",
      {
          std::make_unique<juce::AudioParameterFloat>(
              juce::ParameterID { "volume", 1 }, "Volume",
              juce::NormalisableRange<float> { -60.0f, 0.0f, 0.1f }, -18.0f,
              juce::AudioParameterFloatAttributes().withLabel("dB"))
      })
{
    volumeDb = parameters.getRawParameterValue("volume");
    synth.addSound(new SineSound());
    synth.addVoice(new SineVoice());
    synth.setNoteStealingEnabled(true);
    synth.setMinimumRenderingSubdivisionSize(1, true);
}

void HertZAudioProcessor::prepareToPlay(double sampleRate, int)
{
    synth.setCurrentPlaybackSampleRate(sampleRate);
    gain.reset(sampleRate, 0.01);
    reset();
}

void HertZAudioProcessor::reset()
{
    synth.allNotesOff(0, false);
    gain.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(volumeDb->load()));
}

bool HertZAudioProcessor::isBusesLayoutSupported(const BusesLayout& layout) const
{
    return layout.getMainInputChannelSet().isDisabled()
        && (layout.getMainOutputChannelSet() == juce::AudioChannelSet::mono()
         || layout.getMainOutputChannelSet() == juce::AudioChannelSet::stereo());
}

void HertZAudioProcessor::processBlock(juce::AudioBuffer<float>& audio, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    // A synth generates every sample; stale host buffer contents must not pass through.
    audio.clear();
    synth.renderNextBlock(audio, midi, 0, audio.getNumSamples());
    midi.clear();

    gain.setTargetValue(juce::Decibels::decibelsToGain(volumeDb->load()));
    for (int sample = 0; sample < audio.getNumSamples(); ++sample)
    {
        const auto nextGain = gain.getNextValue();
        for (int channel = 0; channel < audio.getNumChannels(); ++channel)
            audio.getWritePointer(channel)[sample] *= nextGain;
    }
}

juce::AudioProcessorEditor* HertZAudioProcessor::createEditor()
{
    // JUCE supplies the complete UI from the parameter list.
    return new juce::GenericAudioProcessorEditor(*this);
}

void HertZAudioProcessor::getStateInformation(juce::MemoryBlock& destination)
{
    if (const auto xml = parameters.copyState().createXml())
        copyXmlToBinary(*xml, destination);
}

void HertZAudioProcessor::setStateInformation(const void* data, int size)
{
    if (const auto xml = getXmlFromBinary(data, size))
        if (xml->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HertZAudioProcessor();
}
