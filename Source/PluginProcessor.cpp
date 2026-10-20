#include "PluginProcessor.h"
#include "PluginEditor.h"

HertzAudioProcessor::HertzAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "BareToneState",
                 {std::make_unique<juce::AudioParameterFloat>(
                     juce::ParameterID{"volume", 1}, "Volume",
                     juce::NormalisableRange<float>{-60.0f, 0.0f, 0.1f}, -18.0f,
                     juce::AudioParameterFloatAttributes().withLabel("dB")),
                  std::make_unique<juce::AudioParameterChoice>(
                      juce::ParameterID{"timing", 1}, "Timing",
                      juce::StringArray{"1/1", "1/2", "1/4", "1/8", "1/16", "1/32", "1/64"}, 3),
                  std::make_unique<juce::AudioParameterChoice>(
                      juce::ParameterID{"chord", 1}, "Chord",
                      juce::StringArray{"Major", "Minor", "Maj7", "Min7", "Dominant 7"}, 0),
                  std::make_unique<juce::AudioParameterChoice>(
                      juce::ParameterID{"playback", 1}, "Playback",
                      juce::StringArray{"Ascending", "Descending", "Chord press"}, 0),
                  std::make_unique<juce::AudioParameterFloat>(
                      juce::ParameterID{"bpm", 1}, "Tempo",
                      juce::NormalisableRange<float>{30.0f, 300.0f, 0.1f}, 120.0f,
                      juce::AudioParameterFloatAttributes().withLabel("BPM"))})
{
    volumeDb = parameters.getRawParameterValue("volume");
    tempoBpm = parameters.getRawParameterValue("bpm");
    timingChoice = parameters.getRawParameterValue("timing");
    chordChoice = parameters.getRawParameterValue("chord");
    playbackChoice = parameters.getRawParameterValue("playback");

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
    // The first audio block calculates the selected note length and tempo.
    samplesPerStep = 1.0;

    arpBuffer.ensureSize(8192);

    synth.setCurrentPlaybackSampleRate(sampleRate);
    gain.reset(sampleRate, 0.01);
    reset();
}

void HertzAudioProcessor::reset()
{

    arpRoot = -1;
    activeNoteCount = 0;
    lastTiming = lastChord = lastPlayback = -1;
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

    // Play the generated chord sequence instead of the original held note.
    synth.renderNextBlock(audio, arpBuffer, 0, audio.getNumSamples());

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

    if (numSamples <= 0)
        return;

    constexpr int chordIntervals[][4] {
        {0, 4, 7, 0}, {0, 3, 7, 0}, {0, 4, 7, 11},
        {0, 3, 7, 10}, {0, 4, 7, 10}
    };
    constexpr double denominators[] {1, 2, 4, 8, 16, 32, 64};
    const int timing = juce::jlimit(0, 6, static_cast<int>(timingChoice->load()));
    const int chord = juce::jlimit(0, 4, static_cast<int>(chordChoice->load()));
    const int playback = juce::jlimit(0, 2, static_cast<int>(playbackChoice->load()));
    const int toneCount = chord < 2 ? 3 : 4;
    const double bpm = juce::jlimit(30.0, 300.0, static_cast<double>(tempoBpm->load()));
    const double nextStepLength = synth.getSampleRate() * (60.0 / bpm) * 4.0 / denominators[timing];
    // Keep the remaining fraction of the step when tempo changes during a held note.
    if (playback != 2 && samplesUntilStep > 0.0 && samplesPerStep > 0.0)
        samplesUntilStep *= nextStepLength / samplesPerStep;
    samplesPerStep = nextStepLength;

    auto stopCurrentNote = [this](int samplePosition)
    {
        for (int i = 0; i < activeNoteCount; ++i)
            arpBuffer.addEvent(juce::MidiMessage::noteOff(arpChannel, activeNotes[static_cast<size_t>(i)]), samplePosition);
        activeNoteCount = 0;
    };

    // Apply control changes on the audio thread and release the old chord first.
    if (timing != lastTiming || chord != lastChord || playback != lastPlayback)
    {
        stopCurrentNote(0);
        arpStep = 0;
        samplesUntilStep = 0.0;
        lastTiming = timing;
        lastChord = chord;
        lastPlayback = playback;
    }

    auto startTone = [&](int tone, int samplePosition)
    {
        const int note = arpRoot + chordIntervals[chord][tone];
        if (note <= 127)
        {
            arpBuffer.addEvent(juce::MidiMessage::noteOn(arpChannel, note, arpVelocity), samplePosition);
            activeNotes[static_cast<size_t>(activeNoteCount++)] = note;
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

        if (samplesUntilStep <= 0.0)
        {
            stopCurrentNote(sample);
            if (playback == 2)
            {
                for (int tone = 0; tone < toneCount; ++tone)
                    startTone(tone, sample);
                // Chord press sustains until release or a control/key change.
                samplesUntilStep = 1.0;
            }
            else
            {
                const int tone = playback == 1 ? toneCount - 1 - arpStep : arpStep;
                startTone(tone, sample);
                arpStep = (arpStep + 1) % toneCount;
                samplesUntilStep += samplesPerStep;
            }
        }

        if (playback != 2)
            samplesUntilStep -= 1.0;
    }
}
