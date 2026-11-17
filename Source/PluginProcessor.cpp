#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Params.h"

Kosmos66Processor::Kosmos66Processor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "KOSMOS66", Params::createLayout()),
      presets (apvts)
{
    for (auto& d : Params::all())
        raw[d.id] = apvts.getRawParameterValue (d.id);
    presets.loadFactory (0);
}

float Kosmos66Processor::get (const char* id) const
{
    auto it = raw.find (id);
    jassert (it != raw.end() && it->second != nullptr);
    return it->second->load (std::memory_order_relaxed);
}

bool Kosmos66Processor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::stereo() || out == juce::AudioChannelSet::mono();
}

void Kosmos66Processor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.prepare (sampleRate, samplesPerBlock);
    fx.prepare (sampleRate, samplesPerBlock);
    keyboardState.reset();
}

void Kosmos66Processor::readParams()
{
    auto i = [this] (const char* id) { return (int) std::lround (get (id)); };
    auto b = [this] (const char* id) { return get (id) > 0.5f; };

    ep.engine = i ("engine");

    static const char* mw[] = { "m_w1", "m_w2", "m_w3" };
    static const char* mo[] = { "m_o1", "m_o2", "m_o3" };
    static const char* ml[] = { "m_l1", "m_l2", "m_l3" };
    for (int k = 0; k < 3; ++k)
    {
        ep.mWave[k] = juce::jlimit (0, 3, i (mw[k]));
        ep.mOct[k]  = juce::jlimit (0, 3, i (mo[k]));
        ep.mLvl[k]  = get (ml[k]);
    }
    ep.mDet[0] = 0.0f; ep.mDet[1] = get ("m_d2"); ep.mDet[2] = get ("m_d3");
    ep.mNoise = get ("m_noise");
    ep.mCut = get ("m_cut"); ep.mRes = get ("m_res"); ep.mEnvAmt = get ("m_env"); ep.mKey = get ("m_key");
    ep.mFEnv = { get ("m_fa"), get ("m_fd"), get ("m_fs"), get ("m_fr") };
    ep.mAEnv = { get ("m_aa"), get ("m_ad"), get ("m_as"), get ("m_ar") };
    ep.mGlide = get ("m_glide");
    ep.mMono = b ("m_mono");

    static const char* db[] = { "h_db1", "h_db2", "h_db3", "h_db4", "h_db5", "h_db6", "h_db7", "h_db8", "h_db9" };
    for (int k = 0; k < 9; ++k) ep.hDb[k] = get (db[k]);
    ep.hPerc = b ("h_perc"); ep.hPercHarm = i ("h_pharm"); ep.hPercDecay = i ("h_pdec");
    ep.hPercVol = i ("h_pvol"); ep.hClick = get ("h_click");

    ep.kW1 = juce::jlimit (0, 3, i ("k_w1")); ep.kW2 = juce::jlimit (0, 3, i ("k_w2"));
    ep.kPW = get ("k_pw"); ep.kSemi = get ("k_semi"); ep.kDet = get ("k_det");
    ep.kMix = get ("k_mix"); ep.kRing = get ("k_ring");
    ep.kHpf = get ("k_hpf"); ep.kLpf = get ("k_lpf"); ep.kRes = get ("k_res"); ep.kEnvAmt = get ("k_env");
    ep.kLfoPitch = get ("k_lpitch"); ep.kLfoFilter = get ("k_lfilt"); ep.kLfoPW = get ("k_lpw");
    ep.kEnv = { get ("k_a"), get ("k_d"), get ("k_s"), get ("k_r") };

    ep.eTine = get ("e_tine"); ep.eBark = get ("e_bark"); ep.eDecay = get ("e_decay");
    ep.eRel = get ("e_rel"); ep.eVel = get ("e_vel"); ep.eDetune = get ("e_detune");

    pp.hVib = i ("h_vib"); pp.hLeslie = i ("h_leslie"); pp.hLeslieMix = get ("h_lmix"); pp.hDrive = get ("h_drive");
    pp.kLfoWave = i ("k_lwave"); pp.kLfoRate = get ("k_lrate");
    pp.eTone = get ("e_tone"); pp.eTremRate = get ("e_trate"); pp.eTremDepth = get ("e_tdepth");

    fxp.drvOn = b ("fx_drv_on"); fxp.drv = get ("fx_drv"); fxp.drvTone = get ("fx_drv_tone");
    fxp.phsOn = b ("fx_phs_on"); fxp.phsRate = get ("fx_phs_rate"); fxp.phsDepth = get ("fx_phs_depth"); fxp.phsFb = get ("fx_phs_fb");
    fxp.choOn = b ("fx_cho_on"); fxp.choRate = get ("fx_cho_rate"); fxp.choDepth = get ("fx_cho_depth"); fxp.choMix = get ("fx_cho_mix");
    fxp.dlyOn = b ("fx_dly_on"); fxp.dlyTime = get ("fx_dly_time"); fxp.dlyFb = get ("fx_dly_fb"); fxp.dlyMix = get ("fx_dly_mix");
    fxp.revOn = b ("fx_rev_on"); fxp.revSize = get ("fx_rev_size"); fxp.revDamp = get ("fx_rev_damp"); fxp.revMix = get ("fx_rev_mix");
    fxp.masterGain = juce::Decibels::decibelsToGain (get ("master"), -36.0f);
}

void Kosmos66Processor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    keyboardState.processNextMidiBuffer (midi, 0, buffer.getNumSamples(), true);

    readParams();
    synth.setParams (ep, pp);
    synth.process (buffer, midi);
    fx.process (buffer, fxp);

    // safety + meter
    float pkL = 0.0f, pkR = 0.0f;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* d = buffer.getWritePointer (ch);
        float pk = 0.0f;
        for (int s = 0; s < buffer.getNumSamples(); ++s)
        {
            if (! std::isfinite (d[s])) d[s] = 0.0f;
            d[s] = juce::jlimit (-2.0f, 2.0f, d[s]);
            pk = std::max (pk, std::abs (d[s]));
        }
        (ch == 0 ? pkL : pkR) = pk;
    }
    if (buffer.getNumChannels() == 1) pkR = pkL;
    meterL.store (std::max (pkL, meterL.load() * 0.9f));
    meterR.store (std::max (pkR, meterR.load() * 0.9f));
}

int Kosmos66Processor::getNumPrograms() { return (int) presets.factory().size(); }
int Kosmos66Processor::getCurrentProgram() { return std::max (0, presets.getCurrentFactoryIndex()); }
void Kosmos66Processor::setCurrentProgram (int index) { presets.loadFactory (index); }
const juce::String Kosmos66Processor::getProgramName (int index)
{
    return juce::isPositiveAndBelow (index, getNumPrograms()) ? presets.factory()[(size_t) index].name : juce::String();
}

void Kosmos66Processor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    juce::ValueTree root ("KOSMOS66_STATE");
    root.setProperty ("version", 1, nullptr);
    root.appendChild (state, nullptr);
    root.appendChild (presets.slotsToTree(), nullptr);
    if (auto xml = root.createXml())
        copyXmlToBinary (*xml, destData);
}

void Kosmos66Processor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        auto root = juce::ValueTree::fromXml (*xml);
        auto params = root.getChildWithName (apvts.state.getType());
        if (params.isValid())
            apvts.replaceState (params);
        presets.slotsFromTree (root.getChildWithName ("SLOTS"));
    }
}

juce::AudioProcessorEditor* Kosmos66Processor::createEditor() { return new Kosmos66Editor (*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new Kosmos66Processor(); }
