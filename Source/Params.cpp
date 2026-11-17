#include "Params.h"

namespace Params
{
using T = Type;

static juce::StringArray wavesMoog()  { return { "saw", "tri", "sqr", "pls" }; }
static juce::StringArray wavesKorg1() { return { "saw", "sqr", "tri", "pls" }; }
static juce::StringArray wavesKorg2() { return { "saw", "sqr", "tri", "noise" }; }
static juce::StringArray footage()    { return { "32'", "16'", "8'", "4'" }; }

static std::vector<Def> buildDefs()
{
    std::vector<Def> d;
    auto F = [&d] (const char* id, const char* ru, const char* en, float mn, float mx, float df,
                   float skew = -1.0f, const char* unit = "")
    { d.push_back ({ id, ru, en, T::Float, mn, mx, df, skew, {}, unit }); };
    auto I = [&d] (const char* id, const char* ru, const char* en, int mn, int mx, int df)
    { d.push_back ({ id, ru, en, T::Int, (float) mn, (float) mx, (float) df }); };
    auto C = [&d] (const char* id, const char* ru, const char* en, juce::StringArray ch, int df)
    { d.push_back ({ id, ru, en, T::Choice, 0.0f, (float) (ch.size() - 1), (float) df, -1.0f, ch }); };
    auto B = [&d] (const char* id, const char* ru, const char* en, bool df)
    { d.push_back ({ id, ru, en, T::Bool, 0.0f, 1.0f, df ? 1.0f : 0.0f }); };

    // ---- global ----------------------------------------------------------
    C ("engine", u8"РЕЖИМ", "ENGINE", { "MOOG", "HAMMOND", "MINIKORG", "E-PIANO" }, 0);
    F ("master", u8"ГРОМКОСТЬ", "MASTER", -36.0f, 6.0f, -6.0f, -1.0f, "dB");

    // ---- Moog (3-osc ladder mono/poly) ----------------------------------
    C ("m_w1", u8"ФОРМА 1", "WAVE 1", wavesMoog(), 0);
    C ("m_w2", u8"ФОРМА 2", "WAVE 2", wavesMoog(), 0);
    C ("m_w3", u8"ФОРМА 3", "WAVE 3", wavesMoog(), 2);
    C ("m_o1", u8"РЕГИСТР 1", "RANGE 1", footage(), 2);
    C ("m_o2", u8"РЕГИСТР 2", "RANGE 2", footage(), 2);
    C ("m_o3", u8"РЕГИСТР 3", "RANGE 3", footage(), 1);
    F ("m_d2", u8"РАССТР. 2", "DETUNE 2", -50.0f, 50.0f, 7.0f, -1.0f, "ct");
    F ("m_d3", u8"РАССТР. 3", "DETUNE 3", -50.0f, 50.0f, -5.0f, -1.0f, "ct");
    F ("m_l1", u8"УРОВЕНЬ 1", "LEVEL 1", 0.0f, 1.0f, 1.0f);
    F ("m_l2", u8"УРОВЕНЬ 2", "LEVEL 2", 0.0f, 1.0f, 0.7f);
    F ("m_l3", u8"УРОВЕНЬ 3", "LEVEL 3", 0.0f, 1.0f, 0.0f);
    F ("m_noise", u8"ШУМ", "NOISE", 0.0f, 1.0f, 0.0f);
    F ("m_cut", u8"СРЕЗ", "CUTOFF", 30.0f, 16000.0f, 1200.0f, 900.0f, "Hz");
    F ("m_res", u8"ПОДЪЁМ", "EMPHASIS", 0.0f, 1.0f, 0.3f);
    F ("m_env", u8"КОНТУР", "CONTOUR", 0.0f, 1.0f, 0.5f);
    F ("m_key", u8"СЛЕЖЕНИЕ", "KEY TRACK", 0.0f, 1.0f, 0.5f);
    F ("m_fa", u8"АТАКА", "F.ATTACK", 0.001f, 5.0f, 0.005f, 0.3f, "s");
    F ("m_fd", u8"СПАД", "F.DECAY", 0.005f, 5.0f, 0.4f, 0.5f, "s");
    F ("m_fs", u8"УДЕРЖ.", "F.SUSTAIN", 0.0f, 1.0f, 0.2f);
    F ("m_fr", u8"ЗАТУХ.", "F.RELEASE", 0.005f, 5.0f, 0.3f, 0.5f, "s");
    F ("m_aa", u8"АТАКА", "ATTACK", 0.001f, 5.0f, 0.003f, 0.3f, "s");
    F ("m_ad", u8"СПАД", "DECAY", 0.005f, 5.0f, 0.5f, 0.5f, "s");
    F ("m_as", u8"УДЕРЖ.", "SUSTAIN", 0.0f, 1.0f, 0.8f);
    F ("m_ar", u8"ЗАТУХ.", "RELEASE", 0.005f, 5.0f, 0.15f, 0.5f, "s");
    F ("m_glide", u8"ПОРТАМЕНТО", "GLIDE", 0.0f, 1.0f, 0.0f, 0.15f, "s");
    B ("m_mono", u8"МОНО", "MONO", true);

    // ---- Hammond (tonewheel organ) ------------------------------------
    const char* dbRu[] = { u8"16'", u8"5⅓'", u8"8'", u8"4'", u8"2⅔'", u8"2'", u8"1⅗'", u8"1⅓'", u8"1'" };
    const int dbDef[]  = { 8, 8, 8, 0, 0, 0, 0, 0, 0 };
    static const char* dbIds[] = { "h_db1", "h_db2", "h_db3", "h_db4", "h_db5", "h_db6", "h_db7", "h_db8", "h_db9" };
    for (int i = 0; i < 9; ++i)
        I (dbIds[i], dbRu[i], "DRAWBAR", 0, 8, dbDef[i]);
    B ("h_perc", u8"УДАР", "PERC", true);
    C ("h_pharm", u8"ГАРМОНИКА", "PERC HARM", { "2ND", "3RD" }, 1);
    C ("h_pdec", u8"ЗАТУХАНИЕ", "PERC DECAY", { "FAST", "SLOW" }, 0);
    C ("h_pvol", u8"СИЛА", "PERC VOL", { "NORM", "SOFT" }, 0);
    F ("h_click", u8"ЩЕЛЧОК", "KEY CLICK", 0.0f, 1.0f, 0.35f);
    C ("h_vib", u8"ВИБРАТО", "VIB/CHORUS", { "OFF", "V1", "V2", "V3", "C1", "C2", "C3" }, 6);
    C ("h_leslie", u8"ЛЕСЛИ", "ROTARY", { "STOP", "SLOW", "FAST" }, 1);
    F ("h_lmix", u8"ЛЕСЛИ МИКС", "ROTARY MIX", 0.0f, 1.0f, 0.85f);
    F ("h_drive", u8"ЛАМПА", "TUBE DRIVE", 0.0f, 1.0f, 0.2f);

    // ---- MiniKorg (2-osc, HPF/LPF, LFO) --------------------------------
    C ("k_w1", u8"ГЕН 1", "VCO1 WAVE", wavesKorg1(), 0);
    F ("k_pw", u8"ШИРИНА", "PULSE W", 0.05f, 0.5f, 0.3f);
    C ("k_w2", u8"ГЕН 2", "VCO2 WAVE", wavesKorg2(), 1);
    I ("k_semi", u8"ИНТЕРВАЛ", "VCO2 SEMI", -12, 12, 0);
    F ("k_det", u8"РАССТР.", "VCO2 FINE", -50.0f, 50.0f, 8.0f, -1.0f, "ct");
    F ("k_mix", u8"БАЛАНС", "VCO MIX", 0.0f, 1.0f, 0.5f);
    F ("k_ring", u8"КОЛЬЦО", "RING MOD", 0.0f, 1.0f, 0.0f);
    F ("k_hpf", u8"ФВЧ", "HIGH PASS", 20.0f, 4000.0f, 20.0f, 300.0f, "Hz");
    F ("k_lpf", u8"ФНЧ", "LOW PASS", 40.0f, 16000.0f, 2500.0f, 1000.0f, "Hz");
    F ("k_res", u8"ПИК", "PEAK", 0.0f, 1.0f, 0.4f);
    F ("k_env", u8"ОГИБ. ФНЧ", "EG > LPF", -1.0f, 1.0f, 0.4f);
    C ("k_lwave", u8"НЧГ ФОРМА", "LFO WAVE", { "tri", "sqr", "saw", "s&h" }, 0);
    F ("k_lrate", u8"НЧГ ЧАСТОТА", "LFO RATE", 0.05f, 30.0f, 4.0f, 3.0f, "Hz");
    F ("k_lpitch", u8"НЧГ > ТОН", "LFO > PITCH", 0.0f, 1.0f, 0.0f);
    F ("k_lfilt", u8"НЧГ > ФНЧ", "LFO > LPF", 0.0f, 1.0f, 0.0f);
    F ("k_lpw", u8"НЧГ > ШИР.", "LFO > PW", 0.0f, 1.0f, 0.0f);
    F ("k_a", u8"АТАКА", "ATTACK", 0.001f, 5.0f, 0.005f, 0.3f, "s");
    F ("k_d", u8"СПАД", "DECAY", 0.005f, 5.0f, 0.4f, 0.5f, "s");
    F ("k_s", u8"УДЕРЖ.", "SUSTAIN", 0.0f, 1.0f, 0.6f);
    F ("k_r", u8"ЗАТУХ.", "RELEASE", 0.005f, 5.0f, 0.25f, 0.5f, "s");

    // ---- E-Piano (FM tine model) ----------------------------------------
    F ("e_tine", u8"КАМЕРТОН", "TINE", 0.0f, 1.0f, 0.5f);
    F ("e_bark", u8"РЫК", "BARK", 0.0f, 1.0f, 0.45f);
    F ("e_decay", u8"ЗВУЧАНИЕ", "DECAY", 0.5f, 10.0f, 4.0f, 3.0f, "s");
    F ("e_rel", u8"ДЕМПФЕР", "DAMPER", 0.02f, 2.0f, 0.25f, 0.3f, "s");
    F ("e_vel", u8"ДИНАМИКА", "VELOCITY", 0.0f, 1.0f, 0.7f);
    F ("e_tone", u8"ТЕМБР", "TONE", 0.0f, 1.0f, 0.6f);
    F ("e_detune", u8"РАССТР.", "DETUNE", 0.0f, 10.0f, 3.0f, -1.0f, "ct");
    F ("e_trate", u8"ТРЕМОЛО Ч.", "TREM RATE", 0.5f, 10.0f, 4.5f, -1.0f, "Hz");
    F ("e_tdepth", u8"ТРЕМОЛО Г.", "TREM DEPTH", 0.0f, 1.0f, 0.3f);

    // ---- FX chain ---------------------------------------------------------
    B ("fx_drv_on", u8"ВКЛ", "ON", false);
    F ("fx_drv", u8"ПЕРЕГРУЗ", "DRIVE", 0.0f, 1.0f, 0.3f);
    F ("fx_drv_tone", u8"ТЕМБР", "TONE", 0.0f, 1.0f, 0.6f);
    B ("fx_phs_on", u8"ВКЛ", "ON", false);
    F ("fx_phs_rate", u8"СКОРОСТЬ", "RATE", 0.05f, 5.0f, 0.4f, 0.8f, "Hz");
    F ("fx_phs_depth", u8"ГЛУБИНА", "DEPTH", 0.0f, 1.0f, 0.6f);
    F ("fx_phs_fb", u8"ОБР. СВЯЗЬ", "FEEDBACK", 0.0f, 0.9f, 0.4f);
    B ("fx_cho_on", u8"ВКЛ", "ON", false);
    F ("fx_cho_rate", u8"СКОРОСТЬ", "RATE", 0.1f, 5.0f, 0.8f, 1.0f, "Hz");
    F ("fx_cho_depth", u8"ГЛУБИНА", "DEPTH", 0.0f, 1.0f, 0.4f);
    F ("fx_cho_mix", u8"СМЕСЬ", "MIX", 0.0f, 1.0f, 0.5f);
    B ("fx_dly_on", u8"ВКЛ", "ON", false);
    F ("fx_dly_time", u8"ВРЕМЯ", "TIME", 20.0f, 1500.0f, 380.0f, 300.0f, "ms");
    F ("fx_dly_fb", u8"ПОВТОРЫ", "FEEDBACK", 0.0f, 0.95f, 0.35f);
    F ("fx_dly_mix", u8"СМЕСЬ", "MIX", 0.0f, 1.0f, 0.25f);
    B ("fx_rev_on", u8"ВКЛ", "ON", true);
    F ("fx_rev_size", u8"ЗАЛ", "SIZE", 0.0f, 1.0f, 0.5f);
    F ("fx_rev_damp", u8"ГЛУШЕНИЕ", "DAMPING", 0.0f, 1.0f, 0.5f);
    F ("fx_rev_mix", u8"СМЕСЬ", "MIX", 0.0f, 1.0f, 0.18f);

    return d;
}

const std::vector<Def>& all()
{
    static const std::vector<Def> defs = buildDefs();
    return defs;
}

const Def* find (const juce::String& id)
{
    for (auto& d : all())
        if (id == d.id)
            return &d;
    return nullptr;
}

juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    for (auto& d : all())
    {
        const juce::String name = juce::String (d.en) + " (" + juce::String::fromUTF8 (d.id) + ")";
        const juce::ParameterID pid { d.id, 1 };

        switch (d.type)
        {
            case T::Float:
            {
                juce::NormalisableRange<float> r (d.min, d.max);
                if (d.skewCentre > 0.0f)
                    r.setSkewForCentre (d.skewCentre);
                layout.add (std::make_unique<juce::AudioParameterFloat> (
                    pid, name, r, d.def,
                    juce::AudioParameterFloatAttributes().withLabel (d.unit)));
                break;
            }
            case T::Int:
                layout.add (std::make_unique<juce::AudioParameterInt> (pid, name, (int) d.min, (int) d.max, (int) d.def));
                break;
            case T::Choice:
                layout.add (std::make_unique<juce::AudioParameterChoice> (pid, name, d.choices, (int) d.def));
                break;
            case T::Bool:
                layout.add (std::make_unique<juce::AudioParameterBool> (pid, name, d.def > 0.5f));
                break;
        }
    }
    return layout;
}

juce::StringArray engineNames()   { return { "MOOG", "HAMMOND", "MINIKORG", "E-PIANO" }; }
juce::StringArray engineNamesRu() { return { juce::String::fromUTF8 (u8"МУГ"), juce::String::fromUTF8 (u8"ОРГАН"),
                                             juce::String::fromUTF8 (u8"КОРГ"), juce::String::fromUTF8 (u8"ЭЛ. ПИАНО") }; }
}
