#include "PluginProcessor.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace
{
void check (bool condition, const char* message)
{
    if (! condition)
        throw std::runtime_error (message);
}

struct Event
{
    int sample;
    juce::MidiMessage message;
};

juce::MidiMessage on (int note, float velocity = 1.0f, int channel = 1)
{
    return juce::MidiMessage::noteOn (channel, note, velocity);
}

juce::MidiMessage off (int note, int channel = 1)
{
    return juce::MidiMessage::noteOff (channel, note);
}

std::vector<float> render (HertZAudioProcessor& processor, int sampleCount,
                           const std::vector<Event>& events = {}, int blockSize = 256)
{
    std::vector<float> result;
    result.reserve (static_cast<size_t> (sampleCount));
    for (int position = 0; position < sampleCount; position += blockSize)
    {
        const int count = std::min (blockSize, sampleCount - position);
        juce::AudioBuffer<float> audio (processor.getTotalNumOutputChannels(), count);
        for (int channel = 0; channel < audio.getNumChannels(); ++channel)
            std::fill_n (audio.getWritePointer (channel), count, 0.375f);
        juce::MidiBuffer midi;
        for (const auto& event : events)
            if (event.sample >= position && event.sample < position + count)
                midi.addEvent (event.message, event.sample - position);

        processor.processBlock (audio, midi);
        check (midi.isEmpty(), "Processor must consume incoming MIDI");
        for (int sample = 0; sample < count; ++sample)
        {
            const float value = audio.getSample (0, sample);
            check (std::isfinite (value), "Audio contains NaN or infinity");
            check (std::abs (value) <= 1.0f, "Default-level output exceeds full scale");
            for (int channel = 1; channel < audio.getNumChannels(); ++channel)
                check (std::abs (audio.getSample (channel, sample) - value) < 1.0e-7f,
                       "Stereo channels must carry the same mono tone");
            result.push_back (value);
        }
    }
    return result;
}

float peak (const std::vector<float>& samples, size_t start = 0)
{
    float result = 0.0f;
    for (size_t sample = start; sample < samples.size(); ++sample)
        result = std::max (result, std::abs (samples[sample]));
    return result;
}

double frequency (const std::vector<float>& samples, double sampleRate)
{
    std::vector<double> crossings;
    // Ignore the attack and gain smoothing before measuring complete periods.
    for (size_t sample = static_cast<size_t> (0.04 * sampleRate) + 1;
         sample < samples.size(); ++sample)
    {
        const float previous = samples[sample - 1];
        const float current = samples[sample];
        if (previous <= 0.0f && current > 0.0f)
            crossings.push_back (static_cast<double> (sample - 1)
                                 - previous / static_cast<double> (current - previous));
    }
    check (crossings.size() >= 3, "Tone has too few crossings to measure pitch");
    return (crossings.size() - 1) * sampleRate / (crossings.back() - crossings.front());
}

void checkRate (double sampleRate)
{
    const auto samples = [sampleRate] (double seconds)
    {
        return static_cast<int> (std::lround (seconds * sampleRate));
    };
    HertZAudioProcessor processor;
    check (processor.acceptsMidi() && ! processor.producesMidi(), "Wrong MIDI capabilities");
    check (processor.getTotalNumInputChannels() == 0, "Synth must not expose audio inputs");
    check (processor.getTotalNumOutputChannels() == 2, "Expected default stereo output");
    processor.prepareToPlay (sampleRate, 512);
    check (peak (render (processor, 1024)) < 1.0e-7f, "Initial output must be silent");

    const auto a4 = render (processor, samples (0.25), { { 0, on (69) } });
    check (peak (a4) > 0.01f, "Note-on must produce audible output");
    check (std::abs (frequency (a4, sampleRate) - 440.0) < 0.5, "A4 must be 440 Hz");
    const auto a5 = render (processor, samples (0.25), { { 0, on (81) } });
    check (std::abs (frequency (a5, sampleRate) - 880.0) < 0.5, "Newest note must replace old note");
    const auto stolenOff = render (processor, samples (0.15), { { 0, off (69) } });
    check (std::abs (frequency (stolenOff, sampleRate) - 880.0) < 0.5,
           "Old note-off must not stop the newest note");
    const auto released = render (processor, samples (0.08), { { 0, off (81) } });
    check (peak (released, static_cast<size_t> (samples (0.04))) < 1.0e-7f,
           "Note-off release must become silent without reviving a held note");
    render (processor, 256, { { 0, on (60) }, { 31, on (72) } });
    check (peak (render (processor, samples (0.08), { { 0, off (72) } }),
                 static_cast<size_t> (samples (0.04))) < 1.0e-7f,
           "Releasing the newest note must not revive a still-held older note");

    processor.reset();
    const auto quiet = render (processor, samples (0.12), { { 0, on (69, 0.25f) } });
    processor.reset();
    const auto loud = render (processor, samples (0.12), { { 0, on (69) } });
    check (peak (quiet) > 0.0001f && peak (loud) > 2.5f * peak (quiet),
           "MIDI velocity must control note loudness");
    processor.reset();
    check (peak (render (processor, 512)) < 1.0e-7f, "Reset must immediately silence voices");

    const auto delayed = render (processor, 512, { { 137, on (69) } }, 512);
    check (std::all_of (delayed.begin(), delayed.begin() + 137,
                       [] (float value) { return std::abs (value) < 1.0e-7f; }),
           "MIDI note must not start before its sample offset");
    check (peak (delayed, 137) > 0.001f, "Offset note-on must produce audio within its block");
    const auto zeroVelocity = juce::MidiMessage::noteOn (1, 69, static_cast<juce::uint8> (0));
    check (peak (render (processor, samples (0.08), { { 0, zeroVelocity } }),
                 static_cast<size_t> (samples (0.04))) < 1.0e-7f,
           "Zero-velocity note-on must act as note-off");

    for (int channel = 1; channel <= 16; ++channel)
    {
        check (peak (render (processor, samples (0.04), { { 0, on (69, 1.0f, channel) } })) > 0.01f,
               "Every MIDI channel must trigger the oscillator");
        check (peak (render (processor, samples (0.08),
                             { { 0, juce::MidiMessage::allNotesOff (channel) } }),
                     static_cast<size_t> (samples (0.04))) < 1.0e-7f,
               "All-notes-off must silence its MIDI channel");
    }
    processor.releaseResources();
    std::cout << "PASS: audio, pitch, velocity, MIDI timing and lifecycle at " << sampleRate << " Hz\n";
}

void checkPartitioning()
{
    const std::vector<Event> events { { 17, on (69) }, { 23, on (81) }, { 39, off (69) },
                                      { 1801, off (81) }, { 3107, on (60, 0.5f, 16) },
                                      { 4503, off (60, 16) } };
    HertZAudioProcessor referenceProcessor;
    referenceProcessor.prepareToPlay (48000.0, 512);
    const auto reference = render (referenceProcessor, 6500, events, 512);
    for (int blockSize : { 1, 37, 127 })
    {
        HertZAudioProcessor processor;
        processor.prepareToPlay (48000.0, 512);
        const auto actual = render (processor, 6500, events, blockSize);
        for (size_t sample = 0; sample < actual.size(); ++sample)
            check (std::abs (actual[sample] - reference[sample]) < 1.0e-6f,
                   "MIDI/audio result changes with host block partitioning");
    }
    std::cout << "PASS: sample-accurate MIDI across 1, 37, 127 and 512 sample blocks\n";
}

void checkStateAndMono()
{
    HertZAudioProcessor original;
    check (original.getParameters().size() == 1, "Expected exactly one exposed parameter");
    auto* volume = dynamic_cast<juce::RangedAudioParameter*> (original.getParameters()[0]);
    check (volume != nullptr, "Volume must be a ranged parameter");
    check (std::abs (volume->convertFrom0to1 (volume->getValue()) + 18.0f) < 0.01f,
           "Default volume must be -18 dB");
    volume->setValueNotifyingHost (volume->convertTo0to1 (-31.0f));
    juce::MemoryBlock state;
    original.getStateInformation (state);
    check (state.getSize() > 0, "Saved state must contain volume");
    HertZAudioProcessor restored;
    restored.setStateInformation (state.getData(), static_cast<int> (state.getSize()));
    auto* restoredVolume = dynamic_cast<juce::RangedAudioParameter*> (restored.getParameters()[0]);
    check (restoredVolume != nullptr
           && std::abs (restoredVolume->convertFrom0to1 (restoredVolume->getValue()) + 31.0f) < 0.01f,
           "State round trip must restore volume");
    auto layout = restored.getBusesLayout();
    layout.outputBuses.set (0, juce::AudioChannelSet::mono());
    check (restored.setBusesLayout (layout), "Mono output layout must be supported");
    restored.prepareToPlay (48000.0, 512);
    const auto restoredPeak = peak (render (restored, 6000, { { 0, on (69) } }));
    check (std::abs (restoredPeak - juce::Decibels::decibelsToGain (-31.0f)) < 0.0001f,
           "Restored volume must control the actual mono audio level");
    std::cout << "PASS: default parameter, volume state round trip and mono output\n";
}
}

int main()
{
    juce::ScopedJuceInitialiser_GUI initialiseJuce;
    try
    {
        for (double rate : { 44100.0, 48000.0, 96000.0 })
            checkRate (rate);
        checkPartitioning();
        checkStateAndMono();
        std::cout << "All HertZ processor tests passed.\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
