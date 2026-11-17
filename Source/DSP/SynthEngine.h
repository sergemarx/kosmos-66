#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include "Voice.h"

namespace dsp66
{
struct PostParams
{
    // Hammond cabinet stage
    int   hVib = 0, hLeslie = 1;
    float hLeslieMix = 0.8f, hDrive = 0.0f;
    // MiniKorg LFO
    int   kLfoWave = 0;
    float kLfoRate = 4.0f;
    // E-piano
    float eTone = 0.6f, eTremRate = 4.5f, eTremDepth = 0.3f;
};

class SynthEngine
{
public:
    static constexpr int maxVoices = 16;

    void prepare (double sampleRate, int maxBlockSize);
    void setParams (const EngineParams& e, const PostParams& p);
    void process (juce::AudioBuffer<float>& buffer, const juce::MidiBuffer& midi);
    void allNotesOff();

private:
    void handleMidi (const juce::MidiMessage& m);
    void noteOn (int note, float velocity);
    void noteOff (int note);
    void renderVoices (int start, int num);
    void postProcess (float* L, float* R, int n);

    float sr = 44100.0f;
    EngineParams ep;
    PostParams pp;
    int lastEngine = -1;

    Voice voices[maxVoices];
    uint64_t ageCounter = 0;
    bool keyDown[128] {};
    std::vector<int> monoStack;
    bool sustain = false;

    float bendSemis = 0.0f, modWheel = 0.0f;
    float vibPhase = 0.0f;

    // LFO (MiniKorg)
    float lfoPhase = 0.0f, lfoSH = 0.0f;
    Noise lfoNoise;

    std::vector<float> mono, lfoBuf, pmBuf;

    // Hammond cabinet
    DelayLine scanner, hornL, hornR;
    float scanPhase = 0.0f;
    OnePoleLP xover1, xover2;
    float hornAngle = 0.0f, drumAngle = 0.0f, hornSpeed = 0.8f, drumSpeed = 0.7f;

    // E-piano
    OnePoleLP epTone;
    float tremPhase = 0.0f;

    DCBlocker dcL, dcR;
};
}
