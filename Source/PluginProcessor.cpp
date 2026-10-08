#include "PluginProcessor.h"
#include "PluginEditor.h"

HertzAudioProcessor::HertzAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "BareToneState",
                 {std::make_unique<juce::AudioParameterFloat>(
                     juce::ParameterID{"volume", 1}, "Volume",
                     juce::NormalisableRange<float>{-60.0f, 0.0f, 0.1f}, -18.0f,
                     juce::AudioParameterFloatAttributes().withLabel("dB"))})
{
    volumeDb = parameters.getRawParameterValue("volume");

    constexpr int numOfVoices = 8;

    synth.addSound(new SineSound());

    for(int i = 0; i < numOfVoices; i++)
    {
        synth.addVoice(new SineVoice());
    }

    synth.setNoteStealingEnabled(true);
    synth.setMinimumRenderingSubdivisionSize(1, true);
}

void HertzAudioProcessor::prepareToPlay(double sampleRate, int)
{
    constexpr double bpm = 120.0;
    constexpr double stepsPerBeat = 2.0; 

    samplesPerStep = sampleRate * 60.0 / bpm / stepsPerBeat;

    arpBuffer.ensureSize(8192);

    synth.setCurrentPlaybackSampleRate(sampleRate);
    gain.reset(sampleRate, 0.01);
    reset();
}

void HertzAudioProcessor::reset()
{

    arpRoot = -1;
    arpNote = -1;
    arpStep = 0; 

    samplesUntilStep = 0.0;
    arpBuffer.clear();

    keyboardState.reset();

    synth.allNotesOff(0, false);

    gain.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(volumeDb->load()));

}

bool HertzAudioProcessor::isBusesLayoutSupported(const BusesLayout& layout) const
{
    return layout.getMainInputChannelSet().isDisabled() && (layout.getMainOutputChannelSet() == juce::AudioChannelSet::mono() || layout.getMainOutputChannelSet() == juce::AudioChannelSet::stereo());
}

void HertzAudioProcessor::processBlock(juce::AudioBuffer<float>& audio, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    // A synth generates every sample, stale host buffer contents must not pass through.
    audio.clear();

    // add notes to on-screen kb
    if(audio.getNumSamples() > 0)
    {
        keyboardState.processNextMidiBuffer(midi, 0, audio.getNumSamples(), true);
    }


    createArp(midi, audio.getNumSamples());

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

juce::AudioProcessorEditor* HertzAudioProcessor::createEditor()
{
    return new HertzAudioProcessorEditor(*this);
}

void HertzAudioProcessor::getStateInformation(juce::MemoryBlock& destination)
{
    if (const auto xml = parameters.copyState().createXml())
        copyXmlToBinary(*xml, destination);
}

void HertzAudioProcessor::setStateInformation(const void* data, int size)
{
    if (const auto xml = getXmlFromBinary(data, size))
        if (xml->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new HertzAudioProcessor();
}


void HertzAudioProcessor::createArp(const juce::MidiBuffer& input, int numSamples)
{
    arpBuffer.clear();

    constexpr int intervals[] {0,4,7};

    auto stopCurrentNote = [this](int samplePosition)
    {
        if(arpNote >= 0)
        {
            arpBuffer.addEvent(
                juce::MidiMessage::noteOff(arpChannel, arpNote),
                samplePosition);
            
                arpNote = -1;
        }
    };

    auto event = input.cbegin();

    // handle midi
    for(int sample = 0; sample < numSamples; ++sample)
    {
        while(event != input.cend() && (*event).samplePosition <= sample)
        {
            const auto message = (*event).getMessage();
            ++event;

            if(message.isNoteOn())
            {
                stopCurrentNote(sample);

                arpRoot = message.getNoteNumber();
                arpChannel = message.getChannel();
                arpVelocity = message.getFloatVelocity();

                arpStep = 0; 
                samplesUntilStep = 0.0;
            }
            else if (message.isNoteOff())
            {
                if(message.getNoteNumber() == arpRoot && message.getChannel() == arpChannel)
                {
                    stopCurrentNote(sample);
                    arpRoot = -1;
                }
            }
            else
            {
                // stop playback if midi input
                if((message.isAllNotesOff() || message.isAllSoundOff()) && message.getChannel() == arpChannel)
                {
                    stopCurrentNote(sample);
                    arpRoot = -1;
                } 

                arpBuffer.addEvent(message, sample);
            }
        }

        if (arpRoot < 0)
        {
            continue;
        }

        // start next chord tone

        if(samplesUntilStep <= 0.0)
        {
            stopCurrentNote(sample);

            const int nextNote = arpRoot + intervals[arpStep];

            if(nextNote <= 127)
            {
                arpBuffer.addEvent(juce::MidiMessage::noteOn(arpChannel, nextNote ,arpVelocity), sample);

                arpNote = nextNote;
            }

            arpStep = (arpStep + 1) % 3;
            samplesUntilStep += samplesPerStep;
        }

        samplesUntilStep -= 1.0;

    }

}