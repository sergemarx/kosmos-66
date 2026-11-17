#pragma once
#include "DSPCommon.h"

namespace dsp66
{
struct EnvSettings { float a = 0.01f, d = 0.3f, s = 0.7f, r = 0.2f; };

// Per-block snapshot of all engine parameters (read from the APVTS once per block)
struct EngineParams
{
    int engine = 0;

    // Moog
    int   mWave[3] {}, mOct[3] {};
    float mDet[3] {}, mLvl[3] {};
    float mNoise = 0, mCut = 1000, mRes = 0, mEnvAmt = 0, mKey = 0, mGlide = 0;
    bool  mMono = true;
    EnvSettings mFEnv, mAEnv;

    // Hammond
    float hDb[9] {};
    bool  hPerc = true;
    int   hPercHarm = 1, hPercDecay = 0, hPercVol = 0;
    float hClick = 0;

    // MiniKorg
    int   kW1 = 0, kW2 = 0;
    float kPW = 0.3f, kSemi = 0, kDet = 0, kMix = 0.5f, kRing = 0, kHpf = 20, kLpf = 2000, kRes = 0, kEnvAmt = 0;
    float kLfoPitch = 0, kLfoFilter = 0, kLfoPW = 0;
    EnvSettings kEnv;

    // E-Piano
    float eTine = 0.5f, eBark = 0.5f, eDecay = 4, eRel = 0.2f, eVel = 0.7f, eDetune = 3;
};

// Exponential "analog" ADSR
struct Envelope
{
    enum Stage { Idle, Attack, Decay, Sustain, Release };
    Stage stage = Idle;
    float level = 0.0f, sr = 44100.0f;
    float aC = 0, dC = 0, rC = 0, sus = 1;

    void set (const EnvSettings& e)
    {
        aC = 1.0f - std::exp (-1.47f / (std::max (0.0005f, e.a) * sr));
        dC = 1.0f - std::exp (-6.9f  / (std::max (0.002f,  e.d) * sr));
        rC = 1.0f - std::exp (-6.9f  / (std::max (0.002f,  e.r) * sr));
        sus = e.s;
    }
    void gateOn (bool retrigger = true) { if (retrigger || stage == Idle || stage == Release) stage = Attack; }
    void gateOff() { if (stage != Idle) stage = Release; }
    void reset() { stage = Idle; level = 0.0f; }
    bool active() const { return stage != Idle; }

    inline float next()
    {
        switch (stage)
        {
            case Attack:
                level += (1.3f - level) * aC;
                if (level >= 1.0f) { level = 1.0f; stage = Decay; }
                break;
            case Decay:
                level += (sus - level) * dC;
                if (std::abs (level - sus) < 1.0e-4f) { level = sus; stage = Sustain; }
                break;
            case Sustain: level = sus; break;
            case Release:
                level += (0.0f - level) * rC;
                if (level < 1.0e-4f) { level = 0.0f; stage = Idle; }
                break;
            default: break;
        }
        return level;
    }
};

class Voice
{
public:
    void prepare (float sampleRate, uint32_t seed);
    void start (int midiNote, float velocity, const EngineParams& p, bool legato, bool triggerPerc);
    void release();
    void kill();

    // Adds the voice's mono output into out[0..n). pitchMod = per-sample semitone offset (bend + vibrato).
    void render (float* out, int n, const EngineParams& p, const float* lfo, const float* pitchMod);

    bool active = false;
    bool held   = false;
    int  note   = -1;
    uint64_t age = 0;
    int engineAtStart = 0;

private:
    float sr = 44100.0f;
    float vel = 1.0f;
    float curNote = 60.0f, targetNote = 60.0f;

    BlepOsc osc[3];
    Noise noise;
    LadderFilter ladder;
    SVF hpf, lpf;
    Envelope ampEnv, filtEnv;

    // Hammond
    float hPhase[9] {};
    float percLevel = 0.0f, percCoef = 0.0f;
    float clickLevel = 0.0f;
    float hGate = 0.0f;
    bool  hReleasing = false;
    OnePoleLP clickFilter;

    // E-piano
    float ePhC = 0, ePhM = 0, ePhB = 0, ePhC2 = 0;
    float eAmp = 0, eModEnv = 0, eBell = 0;
    float eAmpCoef = 0, eModCoef = 0, eBellCoef = 0;
    bool  eReleased = false;

    void renderMoog    (float* out, int n, const EngineParams& p, const float* pm);
    void renderHammond (float* out, int n, const EngineParams& p, const float* pm);
    void renderKorg    (float* out, int n, const EngineParams& p, const float* lfo, const float* pm);
    void renderEPiano  (float* out, int n, const EngineParams& p, const float* pm);
};
}
