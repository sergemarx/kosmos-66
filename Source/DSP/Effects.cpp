#include "Effects.h"

namespace dsp66
{
void EffectsChain::prepare (double sampleRate, int maxBlock)
{
    sr = (float) sampleRate;
    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) maxBlock, 2 };
    phaser.prepare (spec);
    chorus.prepare (spec);
    reverb.prepare (spec);
    phaser.setCentreFrequency (900.0f);
    phaser.setMix (0.5f);
    chorus.setCentreDelay (8.0f);
    chorus.setFeedback (0.0f);

    dlyL.prepare ((int) (1.6f * sr));
    dlyR.prepare ((int) (1.6f * sr));
    dlyLpL.setCutoff (3200.0f, sr);
    dlyLpR.setCutoff (3200.0f, sr);
    drvDcL.prepare (sr); drvDcR.prepare (sr);

    scratch.setSize (2, maxBlock);
    for (auto* s : { &master, &drvMix, &phsMix, &choMixS, &dlyMixS, &revMixS })
        s->reset (sampleRate, 0.03);
    reset();
}

void EffectsChain::copyToScratch (const juce::AudioBuffer<float>& src, int n)
{
    scratch.setSize (2, n, false, false, true);
    scratch.copyFrom (0, 0, src, 0, 0, n);
    scratch.copyFrom (1, 0, src, 1, 0, n);
}

void EffectsChain::reset()
{
    phaser.reset(); chorus.reset(); reverb.reset();
    dlyL.clear(); dlyR.clear();
    dlyHpL = dlyHpR = 0.0f;
    dlySmooth = 0.0f;
}

void EffectsChain::process (juce::AudioBuffer<float>& buffer, const FxParams& p)
{
    const int n = buffer.getNumSamples();
    if (buffer.getNumChannels() < 2) { buffer.applyGain (p.masterGain); return; }
    float* L = buffer.getWritePointer (0);
    float* R = buffer.getWritePointer (1);

    // on/off switches crossfade instead of clicking
    drvMix.setTargetValue (p.drvOn ? 1.0f : 0.0f);
    phsMix.setTargetValue (p.phsOn ? 1.0f : 0.0f);
    choMixS.setTargetValue (p.choOn ? p.choMix : 0.0f);
    dlyMixS.setTargetValue (p.dlyOn ? p.dlyMix : 0.0f);
    revMixS.setTargetValue (p.revOn ? p.revMix : 0.0f);
    master.setTargetValue (p.masterGain);

    // ---- 1. tube drive -------------------------------------------------
    {
        const float g = 1.0f + p.drv * p.drv * 30.0f;
        const float norm = 1.0f / std::pow (g, 0.62f);
        const float toneHz = 700.0f * std::pow (20.0f, p.drvTone);
        drvToneL.setCutoff (toneHz, sr); drvToneR.setCutoff (toneHz, sr);
        if (p.drvOn || drvMix.isSmoothing())
            for (int i = 0; i < n; ++i)
            {
                const float m = drvMix.getNextValue();
                const float wl = drvDcL.process (drvToneL.process (fastTanh (L[i] * g + 0.12f) - 0.1194f) * norm * 1.5f);
                const float wr = drvDcR.process (drvToneR.process (fastTanh (R[i] * g + 0.12f) - 0.1194f) * norm * 1.5f);
                L[i] += m * (wl - L[i]);
                R[i] += m * (wr - R[i]);
            }
        else drvMix.setCurrentAndTargetValue (0.0f);
    }

    // ---- 2. phaser -------------------------------------------------------
    {
        phaser.setRate (p.phsRate);
        phaser.setDepth (p.phsDepth);
        phaser.setFeedback (p.phsFb);
        if (p.phsOn || phsMix.isSmoothing())
        {
            copyToScratch (buffer, n);
            auto& dry = scratch;
            juce::dsp::AudioBlock<float> block (buffer);
            phaser.process (juce::dsp::ProcessContextReplacing<float> (block));
            for (int i = 0; i < n; ++i)
            {
                const float m = phsMix.getNextValue();
                L[i] = dry.getSample (0, i) + m * (L[i] - dry.getSample (0, i));
                R[i] = dry.getSample (1, i) + m * (R[i] - dry.getSample (1, i));
            }
        }
        else phsMix.setCurrentAndTargetValue (0.0f);
    }

    // ---- 3. chorus -------------------------------------------------------
    {
        chorus.setRate (p.choRate);
        chorus.setDepth (juce::jlimit (0.0f, 1.0f, p.choDepth));
        chorus.setMix (1.0f);
        if (p.choOn || choMixS.isSmoothing())
        {
            copyToScratch (buffer, n);
            auto& dry = scratch;
            juce::dsp::AudioBlock<float> block (buffer);
            chorus.process (juce::dsp::ProcessContextReplacing<float> (block));
            for (int i = 0; i < n; ++i)
            {
                const float m = choMixS.getNextValue();
                L[i] = dry.getSample (0, i) * (1.0f - 0.5f * m) + m * L[i];
                R[i] = dry.getSample (1, i) * (1.0f - 0.5f * m) + m * R[i];
            }
        }
        else choMixS.setCurrentAndTargetValue (0.0f);
    }

    // ---- 4. tape echo (ping-pong, darkening repeats, wow) ---------------
    {
        const float target = p.dlyTime * 0.001f * sr;
        if (dlySmooth <= 0.0f) dlySmooth = target;
        const float tc = 1.0f - std::exp (-1.0f / (0.12f * sr));
        const float hpA = 1.0f - std::exp (-dsp66::twoPi * 140.0f / sr);
        const auto& sine = SineTable::get();
        const bool run = p.dlyOn || dlyMixS.isSmoothing() || dlyMixS.getCurrentValue() > 0.0f;
        for (int i = 0; i < n; ++i)
        {
            const float m = dlyMixS.getNextValue();
            dlySmooth += (target - dlySmooth) * tc;
            wowPhase = wrap01 (wowPhase + 0.55f / sr);
            const float wow = 0.0006f * sr * sine (wowPhase);

            const float tl = dlyL.read (dlySmooth + wow);
            const float tr = dlyR.read (dlySmooth - wow);

            float fl = dlyLpL.process (tl);
            float fr = dlyLpR.process (tr);
            dlyHpL += hpA * (fl - dlyHpL); fl -= dlyHpL;
            dlyHpR += hpA * (fr - dlyHpR); fr -= dlyHpR;

            const float inMono = 0.5f * (L[i] + R[i]);
            const float send = run ? 1.0f : 0.0f;
            // ping-pong: input feeds left, left feeds right, right feeds left
            dlyL.push (fastTanh (inMono * send + fr * p.dlyFb));
            dlyR.push (fastTanh (fl * p.dlyFb));

            L[i] += m * tl;
            R[i] += m * tr;
        }
    }

    // ---- 5. plate reverb --------------------------------------------------
    {
        juce::dsp::Reverb::Parameters rp;
        rp.roomSize = 0.35f + 0.63f * p.revSize;
        rp.damping = p.revDamp;
        rp.width = 1.0f;
        rp.wetLevel = 1.0f;
        rp.dryLevel = 0.0f;
        rp.freezeMode = 0.0f;
        reverb.setParameters (rp);

        if (p.revOn || revMixS.isSmoothing())
        {
            copyToScratch (buffer, n);
            auto& wet = scratch;
            juce::dsp::AudioBlock<float> block (wet.getArrayOfWritePointers(), 2, (size_t) n);
            reverb.process (juce::dsp::ProcessContextReplacing<float> (block));
            for (int i = 0; i < n; ++i)
            {
                const float m = revMixS.getNextValue();
                L[i] = L[i] * (1.0f - 0.35f * m) + m * 0.6f * wet.getSample (0, i);
                R[i] = R[i] * (1.0f - 0.35f * m) + m * 0.6f * wet.getSample (1, i);
            }
        }
        else revMixS.setCurrentAndTargetValue (0.0f);
    }

    // ---- master ------------------------------------------------------------
    for (int i = 0; i < n; ++i)
    {
        const float g = master.getNextValue();
        L[i] *= g;
        R[i] *= g;
    }
}
}
