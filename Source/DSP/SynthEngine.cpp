#include "SynthEngine.h"

namespace dsp66
{
void SynthEngine::prepare (double sampleRate, int maxBlockSize)
{
    sr = (float) sampleRate;
    for (int i = 0; i < maxVoices; ++i)
        voices[i].prepare (sr, 1234u + 7919u * (uint32_t) i);

    mono.assign ((size_t) maxBlockSize, 0.0f);
    lfoBuf.assign ((size_t) maxBlockSize, 0.0f);
    pmBuf.assign ((size_t) maxBlockSize, 0.0f);

    scanner.prepare ((int) (0.02f * sr));
    hornL.prepare ((int) (0.02f * sr));
    hornR.prepare ((int) (0.02f * sr));
    xover1.setCutoff (800.0f, sr);
    xover2.setCutoff (800.0f, sr);
    dcL.prepare (sr); dcR.prepare (sr);
    lfoNoise.seed (987654u);
    allNotesOff();
}

void SynthEngine::setParams (const EngineParams& e, const PostParams& p)
{
    if (e.engine != lastEngine)
    {
        allNotesOff();
        lastEngine = e.engine;
    }
    ep = e;
    pp = p;
}

void SynthEngine::allNotesOff()
{
    for (auto& v : voices) v.kill();
    std::fill (std::begin (keyDown), std::end (keyDown), false);
    monoStack.clear();
    sustain = false;
}

void SynthEngine::process (juce::AudioBuffer<float>& buffer, const juce::MidiBuffer& midi)
{
    const int numSamples = buffer.getNumSamples();
    if ((int) mono.size() < numSamples)
    {
        mono.resize ((size_t) numSamples); lfoBuf.resize ((size_t) numSamples); pmBuf.resize ((size_t) numSamples);
    }
    std::fill (mono.begin(), mono.begin() + numSamples, 0.0f);

    int pos = 0;
    for (const auto meta : midi)
    {
        const int t = juce::jlimit (0, numSamples, meta.samplePosition);
        if (t > pos) { renderVoices (pos, t - pos); pos = t; }
        handleMidi (meta.getMessage());
    }
    if (pos < numSamples) renderVoices (pos, numSamples - pos);

    float* L = buffer.getWritePointer (0);
    float* R = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : nullptr;
    std::copy (mono.begin(), mono.begin() + numSamples, L);
    if (R != nullptr)
    {
        std::copy (mono.begin(), mono.begin() + numSamples, R);
        postProcess (L, R, numSamples);
    }
    else
    {
        // mono output: process into scratch and fold down
        std::vector<float> scratch (mono.begin(), mono.begin() + numSamples);
        postProcess (L, scratch.data(), numSamples);
        for (int i = 0; i < numSamples; ++i) L[i] = 0.5f * (L[i] + scratch[(size_t) i]);
    }
}

void SynthEngine::handleMidi (const juce::MidiMessage& m)
{
    if (m.isNoteOn())             noteOn (m.getNoteNumber(), m.getFloatVelocity());
    else if (m.isNoteOff())       noteOff (m.getNoteNumber());
    else if (m.isPitchWheel())    bendSemis = 2.0f * ((float) m.getPitchWheelValue() - 8192.0f) / 8192.0f;
    else if (m.isController())
    {
        const int cc = m.getControllerNumber();
        if (cc == 1) modWheel = (float) m.getControllerValue() / 127.0f;
        else if (cc == 64)
        {
            const bool on = m.getControllerValue() >= 64;
            if (sustain && ! on)
                for (auto& v : voices)
                    if (v.active && v.held && ! keyDown[v.note & 127]) v.release();
            sustain = on;
        }
        else if (cc == 123 || cc == 120) allNotesOff();
    }
    else if (m.isAllNotesOff() || m.isAllSoundOff()) allNotesOff();
}

void SynthEngine::noteOn (int note, float velocity)
{
    note &= 127;
    bool anyDown = false;
    for (bool k : keyDown) anyDown |= k;
    keyDown[note] = true;

    // Moog mono / legato
    if (ep.engine == 0 && ep.mMono)
    {
        monoStack.erase (std::remove (monoStack.begin(), monoStack.end(), note), monoStack.end());
        monoStack.push_back (note);
        auto& v = voices[0];
        const bool legato = v.active && v.held;
        v.start (note, velocity, ep, legato, false);
        v.age = ++ageCounter;
        for (int i = 1; i < maxVoices; ++i) if (voices[i].active) voices[i].release();
        return;
    }

    Voice* target = nullptr;
    for (auto& v : voices)                       // same key already sounding -> reuse
        if (v.active && v.note == note) { target = &v; break; }
    if (! target)
        for (auto& v : voices)
            if (! v.active) { target = &v; break; }
    if (! target)                                // steal: oldest released, else oldest
    {
        uint64_t best = UINT64_MAX;
        for (auto& v : voices) if (! v.held && v.age < best) { best = v.age; target = &v; }
        if (! target)
            for (auto& v : voices) if (v.age < best) { best = v.age; target = &v; }
    }
    target->start (note, velocity, ep, false, ! anyDown);
    target->age = ++ageCounter;
}

void SynthEngine::noteOff (int note)
{
    note &= 127;
    keyDown[note] = false;

    if (ep.engine == 0 && ep.mMono)
    {
        monoStack.erase (std::remove (monoStack.begin(), monoStack.end(), note), monoStack.end());
        auto& v = voices[0];
        if (v.active && v.note == note)
        {
            if (! monoStack.empty())
                v.start (monoStack.back(), 0.8f, ep, true, false);  // legato back to held note
            else if (! sustain)
                v.release();
        }
        return;
    }

    if (sustain) return;
    for (auto& v : voices)
        if (v.active && v.held && v.note == note) v.release();
}

void SynthEngine::renderVoices (int start, int num)
{
    float* lfo = lfoBuf.data();
    float* pm  = pmBuf.data();
    const float lfoInc = pp.kLfoRate / sr;
    const float vibDepth = (ep.engine == 1) ? 0.0f : modWheel * 0.45f;

    for (int i = 0; i < num; ++i)
    {
        // MiniKorg LFO
        float l = 0.0f;
        switch (pp.kLfoWave)
        {
            case 0: l = 1.0f - 4.0f * std::abs (lfoPhase - 0.5f); break;
            case 1: l = lfoPhase < 0.5f ? 1.0f : -1.0f; break;
            case 2: l = 1.0f - 2.0f * lfoPhase; break;
            default: l = lfoSH; break;
        }
        lfoPhase += lfoInc;
        if (lfoPhase >= 1.0f) { lfoPhase -= 1.0f; lfoSH = lfoNoise.next(); }
        lfo[i] = l;

        vibPhase = wrap01 (vibPhase + 5.6f / sr);
        pm[i] = bendSemis + vibDepth * SineTable::get() (vibPhase);
    }

    float* out = mono.data() + start;
    for (auto& v : voices)
        if (v.active)
            v.render (out, num, ep, lfo, pm);
}

void SynthEngine::postProcess (float* L, float* R, int n)
{
    const auto& sine = SineTable::get();

    if (ep.engine == 1) // Hammond: tube preamp -> vibrato scanner -> rotary cabinet
    {
        static constexpr float vibDepthMs[7] = { 0.0f, 0.25f, 0.5f, 0.95f, 0.25f, 0.5f, 0.95f };
        const float depth = vibDepthMs[juce::jlimit (0, 6, pp.hVib)] * 0.001f * sr;
        const bool chorus = pp.hVib >= 4;
        const float g = 1.0f + pp.hDrive * 11.0f;
        const float bias = 0.2f * pp.hDrive;
        const float norm = 1.0f / std::sqrt (g);

        int mode = pp.hLeslie;
        if (mode > 0 && modWheel > 0.5f) mode = 2;
        const float hornT = mode == 0 ? 0.0f : (mode == 1 ? 0.83f : 6.7f);
        const float drumT = mode == 0 ? 0.0f : (mode == 1 ? 0.67f : 5.6f);
        const float hornA = 1.0f - std::exp (-1.0f / (0.6f * sr));
        const float drumA = 1.0f - std::exp (-1.0f / (2.6f * sr));
        const float mix = pp.hLeslieMix;
        const float baseD = 0.0026f * sr, dopD = 0.00032f * sr;

        for (int i = 0; i < n; ++i)
        {
            float x = L[i];
            if (pp.hDrive > 0.001f)
                x = (fastTanh (x * g + bias) - fastTanh (bias)) * norm * 1.4f;

            if (pp.hVib > 0)
            {
                scanner.push (x);
                scanPhase = wrap01 (scanPhase + 6.9f / sr);
                const float wet = scanner.read (1.0f + 0.0008f * sr + depth * (0.5f + 0.5f * sine (scanPhase)));
                x = chorus ? 0.5f * (x + wet) : wet;
            }

            // rotary
            hornSpeed += (hornT - hornSpeed) * hornA;
            drumSpeed += (drumT - drumSpeed) * drumA;
            hornAngle = wrap01 (hornAngle + hornSpeed / sr);
            drumAngle = wrap01 (drumAngle + drumSpeed / sr);

            const float low = xover2.process (xover1.process (x));
            const float high = x - low;
            hornL.push (high); hornR.push (high);

            const float hs = sine (hornAngle), hc = sine (wrap01 (hornAngle + 0.25f));
            const float dc = sine (wrap01 (drumAngle + 0.25f));
            const float hl = hornL.read (baseD + dopD * hs) * (1.0f + 0.38f * hc);
            const float hr = hornR.read (baseD - dopD * hs) * (1.0f - 0.38f * hc);
            const float dl = low * (1.0f + 0.18f * dc);
            const float dr = low * (1.0f - 0.18f * dc);

            L[i] = x * (1.0f - mix) + (hl + dl) * mix;
            R[i] = x * (1.0f - mix) + (hr + dr) * mix;
        }
    }
    else if (ep.engine == 3) // E-piano: tone + stereo tremolo (suitcase style)
    {
        epTone.setCutoff (1500.0f * std::pow (10.0f, pp.eTone * 1.03f), sr);
        const float depth = juce::jlimit (0.0f, 1.0f, pp.eTremDepth + modWheel * 0.6f);
        for (int i = 0; i < n; ++i)
        {
            const float m = epTone.process (L[i]);
            tremPhase = wrap01 (tremPhase + pp.eTremRate / sr);
            const float s = sine (tremPhase);
            L[i] = m * (1.0f - depth * (0.5f + 0.5f * s));
            R[i] = m * (1.0f - depth * (0.5f - 0.5f * s));
        }
    }

    for (int i = 0; i < n; ++i)
    {
        L[i] = dcL.process (L[i]);
        R[i] = dcR.process (R[i]);
    }
}
}
