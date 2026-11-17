#include "Presets.h"
#include "Params.h"

using V = std::vector<std::pair<juce::String, float>>;

static std::vector<FactoryPreset> makeFactory()
{
    std::vector<FactoryPreset> p;

    // ===================== MOOG =====================
    p.push_back ({ "Vostok Bass", 0, V {
        { "m_w1", 0 }, { "m_w2", 0 }, { "m_w3", 2 }, { "m_o1", 1 }, { "m_o2", 1 }, { "m_o3", 0 },
        { "m_l1", 1 }, { "m_l2", 0.8f }, { "m_l3", 0.6f }, { "m_d2", 6 }, { "m_d3", -3 },
        { "m_cut", 280 }, { "m_res", 0.35f }, { "m_env", 0.55f }, { "m_key", 0.3f },
        { "m_fa", 0.002f }, { "m_fd", 0.35f }, { "m_fs", 0.15f }, { "m_fr", 0.15f },
        { "m_aa", 0.002f }, { "m_ad", 0.6f }, { "m_as", 0.85f }, { "m_ar", 0.1f },
        { "m_glide", 0.04f }, { "m_mono", 1 },
        { "fx_drv_on", 1 }, { "fx_drv", 0.25f }, { "fx_drv_tone", 0.5f }, { "fx_rev_on", 0 } } });

    p.push_back ({ "Sputnik Lead", 0, V {
        { "m_w1", 0 }, { "m_w2", 2 }, { "m_w3", 0 }, { "m_o1", 2 }, { "m_o2", 2 }, { "m_o3", 3 },
        { "m_l1", 1 }, { "m_l2", 0.7f }, { "m_l3", 0.3f }, { "m_d2", 7 }, { "m_d3", -4 },
        { "m_cut", 1800 }, { "m_res", 0.45f }, { "m_env", 0.4f }, { "m_key", 0.6f },
        { "m_fa", 0.01f }, { "m_fd", 0.5f }, { "m_fs", 0.5f }, { "m_fr", 0.3f },
        { "m_aa", 0.005f }, { "m_ad", 0.4f }, { "m_as", 0.9f }, { "m_ar", 0.25f },
        { "m_glide", 0.12f }, { "m_mono", 1 },
        { "fx_dly_on", 1 }, { "fx_dly_time", 330 }, { "fx_dly_fb", 0.35f }, { "fx_dly_mix", 0.22f },
        { "fx_rev_on", 1 }, { "fx_rev_mix", 0.2f } } });

    p.push_back ({ "Polar Fat Bass", 0, V {
        { "m_w1", 0 }, { "m_w2", 0 }, { "m_w3", 0 }, { "m_o1", 0 }, { "m_o2", 1 }, { "m_o3", 1 },
        { "m_l1", 1 }, { "m_l2", 0.9f }, { "m_l3", 0.9f }, { "m_d2", 9 }, { "m_d3", -9 },
        { "m_cut", 180 }, { "m_res", 0.2f }, { "m_env", 0.7f }, { "m_key", 0.2f },
        { "m_fa", 0.002f }, { "m_fd", 0.25f }, { "m_fs", 0.0f }, { "m_fr", 0.2f },
        { "m_aa", 0.002f }, { "m_ad", 1.2f }, { "m_as", 0.7f }, { "m_ar", 0.2f },
        { "m_mono", 1 }, { "fx_drv_on", 1 }, { "fx_drv", 0.45f }, { "fx_drv_tone", 0.4f }, { "fx_rev_on", 0 } } });

    p.push_back ({ "Soyuz Brass", 0, V {
        { "m_w1", 0 }, { "m_w2", 0 }, { "m_w3", 0 }, { "m_o1", 2 }, { "m_o2", 2 }, { "m_o3", 2 },
        { "m_l1", 1 }, { "m_l2", 0.8f }, { "m_l3", 0.6f }, { "m_d2", 9 }, { "m_d3", -9 },
        { "m_cut", 600 }, { "m_res", 0.15f }, { "m_env", 0.65f }, { "m_key", 0.5f },
        { "m_fa", 0.08f }, { "m_fd", 0.6f }, { "m_fs", 0.45f }, { "m_fr", 0.4f },
        { "m_aa", 0.05f }, { "m_ad", 0.6f }, { "m_as", 0.8f }, { "m_ar", 0.35f },
        { "m_mono", 0 }, { "fx_cho_on", 1 }, { "fx_cho_depth", 0.5f }, { "fx_cho_mix", 0.4f },
        { "fx_rev_on", 1 }, { "fx_rev_size", 0.6f }, { "fx_rev_mix", 0.25f } } });

    p.push_back ({ "Taiga Sweep", 0, V {
        { "m_w1", 0 }, { "m_w2", 1 }, { "m_w3", 2 }, { "m_o1", 2 }, { "m_o2", 3 }, { "m_o3", 1 },
        { "m_l1", 1 }, { "m_l2", 0.6f }, { "m_l3", 0.5f }, { "m_d2", 5 }, { "m_d3", -6 },
        { "m_cut", 350 }, { "m_res", 0.6f }, { "m_env", 0.8f }, { "m_key", 0.4f },
        { "m_fa", 1.8f }, { "m_fd", 2.5f }, { "m_fs", 0.3f }, { "m_fr", 1.5f },
        { "m_aa", 1.0f }, { "m_ad", 2.0f }, { "m_as", 0.9f }, { "m_ar", 2.0f },
        { "m_mono", 0 }, { "fx_phs_on", 1 }, { "fx_phs_rate", 0.15f }, { "fx_cho_on", 1 },
        { "fx_rev_on", 1 }, { "fx_rev_size", 0.85f }, { "fx_rev_mix", 0.35f } } });

    // ===================== HAMMOND =====================
    auto bars = [] (const char* reg) {
        V v;
        for (int i = 0; i < 9; ++i) v.push_back ({ "h_db" + juce::String (i + 1), (float) (reg[i] - '0') });
        return v;
    };
    auto join = [] (V a, const V& b) { a.insert (a.end(), b.begin(), b.end()); return a; };

    p.push_back ({ "Leningrad Jazz", 1, join (bars ("888000000"), V {
        { "h_perc", 1 }, { "h_pharm", 1 }, { "h_pdec", 0 }, { "h_pvol", 0 }, { "h_click", 0.4f },
        { "h_vib", 6 }, { "h_leslie", 1 }, { "h_drive", 0.15f }, { "fx_rev_mix", 0.12f } }) });

    p.push_back ({ "Full Steam 888", 1, join (bars ("888888888"), V {
        { "h_perc", 0 }, { "h_click", 0.5f }, { "h_vib", 6 }, { "h_leslie", 2 }, { "h_drive", 0.45f },
        { "fx_rev_mix", 0.15f } }) });

    p.push_back ({ "Cathedral Gospel", 1, join (bars ("888800008"), V {
        { "h_perc", 0 }, { "h_click", 0.25f }, { "h_vib", 6 }, { "h_leslie", 1 }, { "h_drive", 0.3f },
        { "fx_rev_on", 1 }, { "fx_rev_size", 0.9f }, { "fx_rev_mix", 0.35f } }) });

    p.push_back ({ "Smoky Bar", 1, join (bars ("006800000"), V {
        { "h_perc", 1 }, { "h_pharm", 0 }, { "h_pdec", 1 }, { "h_pvol", 1 }, { "h_click", 0.2f },
        { "h_vib", 2 }, { "h_leslie", 1 }, { "h_lmix", 0.6f }, { "h_drive", 0.1f }, { "fx_rev_mix", 0.2f } }) });

    p.push_back ({ "Volga Rock", 1, join (bars ("888800000"), V {
        { "h_perc", 0 }, { "h_click", 0.6f }, { "h_vib", 0 }, { "h_leslie", 1 }, { "h_drive", 0.8f },
        { "fx_drv_on", 1 }, { "fx_drv", 0.3f }, { "fx_drv_tone", 0.45f }, { "fx_rev_mix", 0.15f } }) });

    // ===================== MINIKORG =====================
    p.push_back ({ "Gagarin Siren", 2, V {
        { "k_w1", 0 }, { "k_w2", 1 }, { "k_semi", 7 }, { "k_det", 4 }, { "k_mix", 0.45f },
        { "k_lpf", 1200 }, { "k_res", 0.85f }, { "k_env", 0.5f },
        { "k_lwave", 0 }, { "k_lrate", 0.6f }, { "k_lpitch", 0.7f },
        { "k_a", 0.01f }, { "k_d", 0.5f }, { "k_s", 0.8f }, { "k_r", 0.4f },
        { "fx_dly_on", 1 }, { "fx_dly_time", 450 }, { "fx_dly_fb", 0.45f }, { "fx_dly_mix", 0.3f } } });

    p.push_back ({ "Lunokhod Squelch", 2, V {
        { "k_w1", 3 }, { "k_pw", 0.15f }, { "k_w2", 0 }, { "k_semi", -12 }, { "k_mix", 0.4f },
        { "k_lpf", 400 }, { "k_res", 0.9f }, { "k_env", 0.85f },
        { "k_lwave", 0 }, { "k_lrate", 0.8f }, { "k_lpw", 0.5f },
        { "k_a", 0.001f }, { "k_d", 0.25f }, { "k_s", 0.1f }, { "k_r", 0.15f },
        { "fx_drv_on", 1 }, { "fx_drv", 0.4f }, { "fx_rev_on", 0 } } });

    p.push_back ({ "Kosmos Ring", 2, V {
        { "k_w1", 1 }, { "k_w2", 2 }, { "k_semi", 5 }, { "k_det", 0 }, { "k_ring", 0.8f },
        { "k_lpf", 3000 }, { "k_res", 0.3f }, { "k_env", 0.2f },
        { "k_lwave", 3 }, { "k_lrate", 8.0f }, { "k_lfilt", 0.5f },
        { "k_a", 0.001f }, { "k_d", 0.6f }, { "k_s", 0.3f }, { "k_r", 0.6f },
        { "fx_dly_on", 1 }, { "fx_dly_time", 250 }, { "fx_dly_fb", 0.5f }, { "fx_dly_mix", 0.3f },
        { "fx_rev_on", 1 }, { "fx_rev_mix", 0.3f } } });

    p.push_back ({ "Mir Bass", 2, V {
        { "k_w1", 0 }, { "k_w2", 1 }, { "k_semi", -12 }, { "k_det", 0 }, { "k_mix", 0.5f },
        { "k_hpf", 60 }, { "k_lpf", 300 }, { "k_res", 0.75f }, { "k_env", 0.7f },
        { "k_a", 0.001f }, { "k_d", 0.2f }, { "k_s", 0.2f }, { "k_r", 0.1f },
        { "fx_drv_on", 1 }, { "fx_drv", 0.3f }, { "fx_rev_on", 0 } } });

    p.push_back ({ "Tundra Strings", 2, V {
        { "k_w1", 0 }, { "k_w2", 0 }, { "k_semi", 0 }, { "k_det", 12 }, { "k_mix", 0.5f },
        { "k_lpf", 2200 }, { "k_res", 0.15f }, { "k_env", 0.2f },
        { "k_lwave", 0 }, { "k_lrate", 5.0f }, { "k_lpitch", 0.08f },
        { "k_a", 0.6f }, { "k_d", 1.0f }, { "k_s", 0.85f }, { "k_r", 1.2f },
        { "fx_cho_on", 1 }, { "fx_cho_depth", 0.6f }, { "fx_cho_mix", 0.6f },
        { "fx_rev_on", 1 }, { "fx_rev_size", 0.8f }, { "fx_rev_mix", 0.35f } } });

    // ===================== E-PIANO =====================
    p.push_back ({ "Moscow Lounge", 3, V {
        { "e_tine", 0.5f }, { "e_bark", 0.45f }, { "e_decay", 4 }, { "e_rel", 0.25f }, { "e_vel", 0.7f },
        { "e_tone", 0.6f }, { "e_detune", 3 }, { "e_trate", 4.5f }, { "e_tdepth", 0.35f },
        { "fx_rev_on", 1 }, { "fx_rev_mix", 0.2f } } });

    p.push_back ({ "Bolshoi Bell", 3, V {
        { "e_tine", 0.95f }, { "e_bark", 0.2f }, { "e_decay", 7 }, { "e_rel", 0.5f },
        { "e_tone", 0.8f }, { "e_tdepth", 0.0f }, { "fx_cho_on", 1 }, { "fx_cho_depth", 0.3f },
        { "fx_dly_on", 1 }, { "fx_dly_time", 420 }, { "fx_dly_fb", 0.3f }, { "fx_dly_mix", 0.18f },
        { "fx_rev_on", 1 }, { "fx_rev_size", 0.8f }, { "fx_rev_mix", 0.35f } } });

    p.push_back ({ "Neva Suitcase", 3, V {
        { "e_tine", 0.45f }, { "e_bark", 0.7f }, { "e_decay", 4 }, { "e_tone", 0.5f },
        { "e_trate", 5.5f }, { "e_tdepth", 0.7f },
        { "fx_drv_on", 1 }, { "fx_drv", 0.25f }, { "fx_drv_tone", 0.5f }, { "fx_rev_mix", 0.15f } } });

    p.push_back ({ "Dacha Ballad", 3, V {
        { "e_tine", 0.35f }, { "e_bark", 0.3f }, { "e_decay", 6 }, { "e_rel", 0.4f }, { "e_tone", 0.45f },
        { "e_trate", 3.5f }, { "e_tdepth", 0.2f }, { "fx_cho_on", 1 }, { "fx_cho_depth", 0.35f },
        { "fx_cho_mix", 0.4f }, { "fx_rev_on", 1 }, { "fx_rev_size", 0.7f }, { "fx_rev_mix", 0.3f } } });

    p.push_back ({ "Growl Funk", 3, V {
        { "e_tine", 0.6f }, { "e_bark", 1.0f }, { "e_decay", 3 }, { "e_rel", 0.12f }, { "e_vel", 0.9f },
        { "e_tone", 0.75f }, { "e_tdepth", 0.0f }, { "fx_phs_on", 1 }, { "fx_phs_rate", 0.5f },
        { "fx_rev_mix", 0.1f } } });

    return p;
}

// ---------------------------------------------------------------------------
PresetManager::PresetManager (juce::AudioProcessorValueTreeState& s) : apvts (s), presets (makeFactory())
{
    for (auto& t : slots) t = {};
    loadSlotsFromDisk();
}

void PresetManager::setParam (const juce::String& id, float realValue)
{
    if (auto* p = apvts.getParameter (id))
    {
        p->beginChangeGesture();
        p->setValueNotifyingHost (p->convertTo0to1 (realValue));
        p->endChangeGesture();
    }
}

void PresetManager::loadFactory (int index)
{
    if (! juce::isPositiveAndBelow (index, (int) presets.size())) return;
    const auto& pr = presets[(size_t) index];

    for (auto& d : Params::all())
        if (juce::String (d.id) != "master")
            setParam (d.id, d.def);
    setParam ("engine", (float) pr.engine);
    for (auto& [id, v] : pr.values)
        setParam (id, v);

    currentFactory = index;
    activeSlot = -1;
    currentName = pr.name;
    if (onChange) onChange();
}

void PresetManager::step (int delta)
{
    const int n = (int) presets.size();
    const int start = currentFactory < 0 ? (delta > 0 ? -1 : 0) : currentFactory;
    loadFactory (((start + delta) % n + n) % n);
}

juce::ValueTree PresetManager::capturePatch() const
{
    juce::ValueTree patch ("PATCH");
    patch.setProperty ("name", currentName, nullptr);
    for (auto& d : Params::all())
    {
        if (juce::String (d.id) == "master") continue;
        if (auto* p = apvts.getParameter (d.id))
        {
            juce::ValueTree pt ("P");
            pt.setProperty ("id", juce::String (d.id), nullptr);
            pt.setProperty ("v", p->convertFrom0to1 (p->getValue()), nullptr);
            patch.appendChild (pt, nullptr);
        }
    }
    return patch;
}

void PresetManager::applyPatch (const juce::ValueTree& patch)
{
    for (auto& d : Params::all())
        if (juce::String (d.id) != "master")
            setParam (d.id, d.def);
    for (const auto& pt : patch)
        if (pt.hasType ("P"))
            setParam (pt["id"].toString(), (float) pt["v"]);
    currentName = patch.getProperty ("name", "USER").toString();
    currentFactory = -1;
}

bool PresetManager::isSlotUsed (int s) const { return juce::isPositiveAndBelow (s, numSlots) && slots[s].isValid(); }

juce::String PresetManager::getSlotName (int s) const
{
    return isSlotUsed (s) ? slots[s].getProperty ("name").toString() : juce::String ("---");
}

void PresetManager::saveToSlot (int s)
{
    if (! juce::isPositiveAndBelow (s, numSlots)) return;
    slots[s] = capturePatch();
    activeSlot = s;
    saveSlotsToDisk();
    if (onChange) onChange();
}

void PresetManager::loadSlot (int s)
{
    if (! isSlotUsed (s)) return;
    applyPatch (slots[s]);
    activeSlot = s;
    if (onChange) onChange();
}

void PresetManager::clearSlot (int s)
{
    if (! juce::isPositiveAndBelow (s, numSlots)) return;
    slots[s] = {};
    if (activeSlot == s) activeSlot = -1;
    saveSlotsToDisk();
    if (onChange) onChange();
}

juce::ValueTree PresetManager::slotsToTree() const
{
    juce::ValueTree t ("SLOTS");
    for (int i = 0; i < numSlots; ++i)
    {
        juce::ValueTree s ("SLOT");
        s.setProperty ("index", i, nullptr);
        if (slots[i].isValid()) s.appendChild (slots[i].createCopy(), nullptr);
        t.appendChild (s, nullptr);
    }
    t.setProperty ("current", currentName, nullptr);
    t.setProperty ("factory", currentFactory, nullptr);
    t.setProperty ("active", activeSlot, nullptr);
    return t;
}

void PresetManager::slotsFromTree (const juce::ValueTree& t)
{
    if (! t.hasType ("SLOTS")) return;
    for (const auto& s : t)
    {
        const int i = s["index"];
        if (juce::isPositiveAndBelow (i, numSlots))
        {
            // slots are global (stored on disk); a session only fills slots that are empty here
            auto patch = s.getChildWithName ("PATCH");
            if (patch.isValid() && ! slots[i].isValid())
                slots[i] = patch.createCopy();
        }
    }
    currentName = t.getProperty ("current", currentName).toString();
    currentFactory = t.getProperty ("factory", -1);
    activeSlot = t.getProperty ("active", -1);
    if (onChange) onChange();
}

juce::File PresetManager::slotFile()
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
        .getChildFile ("KOSMOS-66").getChildFile ("UserSlots.xml");
}

void PresetManager::saveSlotsToDisk() const
{
    auto f = slotFile();
    f.getParentDirectory().createDirectory();
    if (auto xml = slotsToTree().createXml())
        xml->writeTo (f);
}

void PresetManager::loadSlotsFromDisk()
{
    if (auto xml = juce::XmlDocument::parse (slotFile()))
    {
        auto t = juce::ValueTree::fromXml (*xml);
        // only take the slot contents from disk; the session decides the current patch
        for (const auto& s : t)
        {
            const int i = s["index"];
            if (juce::isPositiveAndBelow (i, numSlots))
            {
                auto patch = s.getChildWithName ("PATCH");
                slots[i] = patch.isValid() ? patch.createCopy() : juce::ValueTree();
            }
        }
    }
}

bool PresetManager::exportPatch (const juce::File& f) const
{
    if (auto xml = capturePatch().createXml())
        return xml->writeTo (f);
    return false;
}

bool PresetManager::importPatch (const juce::File& f)
{
    if (auto xml = juce::XmlDocument::parse (f))
    {
        auto t = juce::ValueTree::fromXml (*xml);
        if (t.hasType ("PATCH"))
        {
            applyPatch (t);
            if (currentName.isEmpty()) currentName = f.getFileNameWithoutExtension();
            activeSlot = -1;
            if (onChange) onChange();
            return true;
        }
    }
    return false;
}
