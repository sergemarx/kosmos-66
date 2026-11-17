#include "Voice.h"

namespace dsp66
{
static constexpr float hammondRatios[9] = { 0.5f, 1.5f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 8.0f };

void Voice::prepare (float sampleRate, uint32_t seed)
{
    sr = sampleRate;
    noise.seed (seed);
    ladder.sr = hpf.sr = lpf.sr = sr;
    ampEnv.sr = filtEnv.sr = sr;
    clickFilter.setCutoff (3500.0f, sr);
    kill();
}

void Voice::kill()
{
    active = held = false;
    note = -1;
    ampEnv.reset(); filtEnv.reset();
    ladder.reset(); hpf.reset(); lpf.reset();
    hGate = percLevel = clickLevel = 0.0f;
    eAmp = eModEnv = eBell = 0.0f;
}

void Voice::start (int midiNote, float velocity, const EngineParams& p, bool legato, bool triggerPerc)
{
    const bool wasActive = active;
    note = midiNote;
    vel = velocity;
    targetNote = (float) midiNote;
    held = true;
    active = true;
    engineAtStart = p.engine;

    if (! wasActive || p.mGlide < 0.001f || p.engine != 0)
        curNote = targetNote;

    switch (p.engine)
    {
        case 0: // Moog
            ampEnv.set (p.mAEnv);
            filtEnv.set (p.mFEnv);
            if (! wasActive) { ladder.reset(); for (auto& o : osc) o.reset (noise.next() * 0.5f + 0.5f); }
            ampEnv.gateOn (! legato);
            filtEnv.gateOn (! legato);
            break;

        case 1: // Hammond
        {
            hReleasing = false;
            if (! wasActive)
            {
                // tonewheels free-run, so start each at a random phase
                for (auto& ph : hPhase) ph = noise.next() * 0.5f + 0.5f;
                hGate = 0.0f;
            }
            clickLevel = p.hClick;
            if (triggerPerc && p.hPerc)
            {
                percLevel = 1.0f;
                const float t = p.hPercDecay == 0 ? 0.25f : 1.1f;
                percCoef = std::exp (-6.9f / (t * sr));
            }
            else
                percLevel = 0.0f;
            break;
        }

        case 2: // MiniKorg
            ampEnv.set (p.kEnv);
            if (! wasActive) { hpf.reset(); lpf.reset(); osc[0].reset(); osc[1].reset (0.37f); }
            ampEnv.gateOn (true);
            break;

        case 3: // E-piano
        {
            const float pitchScale = std::exp2 (-((float) midiNote - 60.0f) / 24.0f);
            const float T  = clampf (p.eDecay * pitchScale, 0.25f, 16.0f);
            const float Tm = clampf (T * 0.22f, 0.04f, 1.4f);
            const float Tb = clampf (0.35f * pitchScale, 0.03f, 0.8f);
            eAmpCoef  = std::exp (-6.9f / (T * sr));
            eModCoef  = std::exp (-6.9f / (Tm * sr));
            eBellCoef = std::exp (-6.9f / (Tb * sr));
            eAmp     = (1.0f - p.eVel) + p.eVel * vel;
            eModEnv  = 1.0f;
            eBell    = p.eTine * (0.3f + 0.7f * vel);
            eReleased = false;
            ePhC = ePhC2 = ePhM = ePhB = 0.0f;
            break;
        }
        default: break;
    }
}

void Voice::release()
{
    held = false;
    ampEnv.gateOff();
    filtEnv.gateOff();
    hReleasing = true;
    eReleased = true;
}

void Voice::render (float* out, int n, const EngineParams& p, const float* lfo, const float* pm)
{
    if (! active) return;
    switch (engineAtStart)
    {
        case 0: renderMoog (out, n, p, pm); break;
        case 1: renderHammond (out, n, p, pm); break;
        case 2: renderKorg (out, n, p, lfo, pm); break;
        case 3: renderEPiano (out, n, p, pm); break;
        default: break;
    }
}

// ---------------------------------------------------------------------------
void Voice::renderMoog (float* out, int n, const EngineParams& p, const float* pm)
{
    static constexpr int waveMap[4] = { BlepOsc::Saw, BlepOsc::Tri, BlepOsc::Square, BlepOsc::Pulse };
    float mul[3];
    for (int k = 0; k < 3; ++k)
        mul[k] = std::exp2 ((float) (p.mOct[k] - 2) + p.mDet[k] / 1200.0f);

    const float glideCoef = p.mGlide > 0.001f ? 1.0f - std::exp (-1.0f / (p.mGlide * 0.35f * sr)) : 1.0f;
    const float velGain = 0.55f + 0.45f * vel;
    const float invSr = 1.0f / sr;

    for (int i = 0; i < n; ++i)
    {
        curNote += (targetNote - curNote) * glideCoef;
        const float nn = curNote + pm[i];
        const float f0 = midiToHz (nn);

        float s = 0.0f;
        for (int k = 0; k < 3; ++k)
            if (p.mLvl[k] > 0.0005f)
                s += p.mLvl[k] * osc[k].next (waveMap[p.mWave[k]], f0 * mul[k] * invSr, 0.22f);
            else
                osc[k].next (waveMap[p.mWave[k]], f0 * mul[k] * invSr, 0.22f);
        s += p.mNoise * noise.next();

        const float fe = filtEnv.next();
        const float cutoff = p.mCut * std::exp2 (p.mEnvAmt * fe * 6.0f + p.mKey * (nn - 60.0f) / 12.0f);
        float y = ladder.process (s * 0.45f, cutoff, p.mRes);

        y *= ampEnv.next() * velGain;
        out[i] += y * 0.85f;
    }
    if (! ampEnv.active()) kill();
}

// ---------------------------------------------------------------------------
void Voice::renderHammond (float* out, int n, const EngineParams& p, const float* pm)
{
    const auto& sine = SineTable::get();
    float gain[9];
    for (int k = 0; k < 9; ++k)
        gain[k] = p.hDb[k] < 0.5f ? 0.0f : std::pow (10.0f, -3.0f * (8.0f - p.hDb[k]) / 20.0f);
    if (p.hPerc) gain[8] = 0.0f; // the real thing steals the 1' bar for percussion

    const int percIdx = p.hPercHarm == 0 ? 3 : 4;   // 4' (2nd) or 2⅔' (3rd)
    const float percGain = p.hPercVol == 0 ? 1.1f : 0.55f;
    const float gateUp = 1.0f - std::exp (-1.0f / (0.0015f * sr));
    const float gateDn = 1.0f - std::exp (-1.0f / (0.006f * sr));
    const float clickCoef = std::exp (-1.0f / (0.0025f * sr));
    const float nyq = 0.45f * sr;
    const float invSr = 1.0f / sr;

    for (int i = 0; i < n; ++i)
    {
        const float f = midiToHz (curNote + pm[i]);
        float s = 0.0f, perc = 0.0f;
        for (int k = 0; k < 9; ++k)
        {
            const float fk = f * hammondRatios[k];
            hPhase[k] += fk * invSr;
            if (hPhase[k] >= 1.0f) hPhase[k] -= 1.0f;
            if (fk < nyq)
            {
                const float v = sine (hPhase[k]);
                s += gain[k] * v;
                if (k == percIdx) perc = v;
            }
        }
        s += perc * percLevel * percGain * 1.6f;
        percLevel *= percCoef;

        hGate += ((hReleasing ? 0.0f : 1.0f) - hGate) * (hReleasing ? gateDn : gateUp);

        const float click = clickFilter.process (noise.next()) * clickLevel * 2.5f;
        clickLevel *= clickCoef;

        out[i] += (s * 0.16f + click) * hGate;
    }
    if (hReleasing && hGate < 1.0e-4f) kill();
}

// ---------------------------------------------------------------------------
void Voice::renderKorg (float* out, int n, const EngineParams& p, const float* lfo, const float* pm)
{
    static constexpr int map1[4] = { BlepOsc::Saw, BlepOsc::Square, BlepOsc::Tri, BlepOsc::Pulse };
    static constexpr int map2[3] = { BlepOsc::Saw, BlepOsc::Square, BlepOsc::Tri };
    const float ratio2 = std::exp2 ((p.kSemi + p.kDet / 100.0f) / 12.0f);
    const float pitchDepth = p.kLfoPitch * p.kLfoPitch * 12.0f;
    const float velGain = 0.55f + 0.45f * vel;
    const float invSr = 1.0f / sr;

    for (int i = 0; i < n; ++i)
    {
        const float l = lfo[i];
        const float nn = curNote + pm[i] + l * pitchDepth;
        const float f1 = midiToHz (nn);
        const float pw = clampf (p.kPW + l * p.kLfoPW * 0.4f, 0.04f, 0.96f);

        const float o1 = osc[0].next (map1[p.kW1], f1 * invSr, pw);
        const float o2 = p.kW2 == 3 ? noise.next() * 0.7f : osc[1].next (map2[p.kW2], f1 * ratio2 * invSr, 0.5f);

        float mix = o1 * (1.0f - p.kMix) + o2 * p.kMix;
        mix += p.kRing * (o1 * o2 * 1.4f - mix);

        const float env = ampEnv.next();
        const float hp = hpf.process (mix, p.kHpf, 0.3f, 1);
        const float lpCut = p.kLpf * std::exp2 (p.kEnvAmt * env * 6.0f + l * p.kLfoFilter * 3.0f
                                                 + 0.35f * (nn - 60.0f) / 12.0f);
        float y = lpf.process (hp * 0.8f, lpCut, p.kRes, 0, 1.0f + p.kRes * 3.0f);

        out[i] += y * env * velGain * 0.55f;
    }
    if (! ampEnv.active()) kill();
}

// ---------------------------------------------------------------------------
void Voice::renderEPiano (float* out, int n, const EngineParams& p, const float* pm)
{
    const auto& sine = SineTable::get();
    const float relCoef = std::exp (-6.9f / (std::max (0.01f, p.eRel) * sr));
    const float det = std::exp2 (p.eDetune / 1200.0f);
    const float vMod = 0.25f + 0.75f * vel;
    const float growl = p.eBark * vel * 0.45f;
    const float invSr = 1.0f / sr;
    const float invTwoPi = 1.0f / twoPi;

    for (int i = 0; i < n; ++i)
    {
        const float f = midiToHz (curNote + pm[i]);
        ePhM  = wrap01 (ePhM + f * invSr);
        ePhC  = wrap01 (ePhC + f * invSr);
        ePhC2 = wrap01 (ePhC2 + f * det * invSr);

        const float mod = sine (ePhM);
        const float index = (0.15f + p.eBark * vMod * eModEnv * 3.2f) * invTwoPi;
        const float c = 0.55f * sine (wrap01 (ePhC + index * mod)) + 0.45f * sine (wrap01 (ePhC2 + index * mod));

        float bell = 0.0f;
        if (f * 7.0f < 0.45f * sr)
        {
            ePhB = wrap01 (ePhB + f * 7.0f * invSr);
            bell = sine (ePhB) * eBell;
        }

        float y = c + bell * 0.55f;
        y += growl * y * y;       // asymmetric pickup -> "bark"
        y *= eAmp;

        eAmp *= eAmpCoef;
        if (eReleased) eAmp *= relCoef;
        eModEnv *= eModCoef;
        eBell *= eBellCoef;

        out[i] += y * 0.38f;
    }
    if (eAmp < 1.0e-4f) kill();
}
}
