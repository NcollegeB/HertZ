#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>

class SineSound final : public juce::SynthesiserSound
{
public:
    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};

class SineVoice final : public juce::SynthesiserVoice
{
public:
    bool canPlaySound(juce::SynthesiserSound *sound) override
    {
        return dynamic_cast<SineSound *>(sound) != nullptr;
    }

    void setCurrentPlaybackSampleRate(double rate) override
    {
        juce::SynthesiserVoice::setCurrentPlaybackSampleRate(rate);
        if (rate > 0.0)
            envelope.setSampleRate(rate);
        envelope.setParameters({0.005f, 0.0f, 1.0f, 0.02f});
    }

    void startNote(int midiNote, float velocity, juce::SynthesiserSound *, int) override
    {
        // MIDI 69 is A4 (440 Hz). Each semitone multiplies frequency by 2^(1/12).
        phaseStep = juce::MathConstants<double>::twoPi * juce::MidiMessage::getMidiNoteInHertz(midiNote) / getSampleRate();
        level = velocity;
        phase = 0.0;
        envelope.reset();
        envelope.noteOn();
    }

    void stopNote(float, bool allowTailOff) override
    {
        if (allowTailOff)
            envelope.noteOff();
        else
        {
            envelope.reset();
            clearCurrentNote();
        }
    }

    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}

    void renderNextBlock(juce::AudioBuffer<float> &output, int start, int count) override
    {
        if (!isVoiceActive())
            return;

        for (int i = 0; i < count; ++i)
        {
            const auto sample = static_cast<float>(std::sin(phase)) * level * envelope.getNextSample();
            phase += phaseStep;
            if (phase >= juce::MathConstants<double>::twoPi)
                phase -= juce::MathConstants<double>::twoPi;

            for (int channel = 0; channel < output.getNumChannels(); ++channel)
                output.addSample(channel, start + i, sample);

            if (!envelope.isActive())
            {
                clearCurrentNote();
                break;
            }
        }
    }

private:
    juce::ADSR envelope;
    double phase = 0.0, phaseStep = 0.0;
    float level = 0.0f;
};
