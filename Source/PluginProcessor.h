#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "SineVoice.h"

class HertZAudioProcessor final : public juce::AudioProcessor
{
public:
    HertZAudioProcessor();

    void prepareToPlay(double sampleRate, int maximumBlockSize) override;
    void releaseResources() override { reset(); }
    void reset() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;
    bool isBusesLayoutSupported(const BusesLayout&) const override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "HertZ"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.02; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

private:
    juce::Synthesiser synth;
    juce::AudioProcessorValueTreeState parameters;
    std::atomic<float>* volumeDb = nullptr;
    juce::SmoothedValue<float> gain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HertZAudioProcessor)
};