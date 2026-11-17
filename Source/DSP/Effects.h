#pragma once
#include <juce_dsp/juce_dsp.h>
#include "DSPCommon.h"

namespace dsp66
{
struct FxParams
{
    bool  drvOn = false;  float drv = 0.3f, drvTone = 0.6f;
    bool  phsOn = false;  float phsRate = 0.4f, phsDepth = 0.6f, phsFb = 0.4f;
    bool  choOn = false;  float choRate = 0.8f, choDepth = 0.4f, choMix = 0.5f;
    bool  dlyOn = false;  float dlyTime = 380.0f, dlyFb = 0.35f, dlyMix = 0.25f;
    bool  revOn = true;   float revSize = 0.5f, revDamp = 0.5f, revMix = 0.18f;
    float masterGain = 0.5f;
};

// Fixed chain: TUBE DRIVE -> PHASER -> CHORUS -> TAPE ECHO -> PLATE REVERB -> MASTER
class EffectsChain
{
public:
    void prepare (double sampleRate, int maxBlock);
    void process (juce::AudioBuffer<float>& buffer, const FxParams& p);
    void reset();

private:
    float sr = 44100.0f;

    // drive
    OnePoleLP drvToneL, drvToneR;
    DCBlocker drvDcL, drvDcR;

    juce::dsp::Phaser<float> phaser;
    juce::dsp::Chorus<float> chorus;
    juce::dsp::Reverb reverb;

    // tape echo
    DelayLine dlyL, dlyR;
    OnePoleLP dlyLpL, dlyLpR;
    float dlyHpL = 0, dlyHpR = 0;
    float dlySmooth = 0.0f, wowPhase = 0.0f;

    juce::AudioBuffer<float> scratch;
    void copyToScratch (const juce::AudioBuffer<float>& src, int n);

    juce::SmoothedValue<float> master;
    juce::SmoothedValue<float> drvMix, phsMix, choMixS, dlyMixS, revMixS;
};
}
