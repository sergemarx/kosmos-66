#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "DSP/SynthEngine.h"
#include "DSP/Effects.h"
#include "Presets.h"

class Kosmos66Processor : public juce::AudioProcessor
{
public:
    Kosmos66Processor();
    ~Kosmos66Processor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "KOSMOS-66"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    PresetManager presets;
    juce::MidiKeyboardState keyboardState;

    std::atomic<float> meterL { 0.0f }, meterR { 0.0f };

private:
    void readParams();
    float get (const char* id) const;

    dsp66::SynthEngine synth;
    dsp66::EffectsChain fx;
    dsp66::EngineParams ep;
    dsp66::PostParams pp;
    dsp66::FxParams fxp;
    std::unordered_map<std::string, std::atomic<float>*> raw;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Kosmos66Processor)
};
