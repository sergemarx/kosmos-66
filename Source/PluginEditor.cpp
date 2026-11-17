#include "PluginEditor.h"

namespace Soviet
{
// ===========================================================================
// Engine panels
// ===========================================================================
Knob* EnginePanel::knob (const juce::String& id, Knob::Size sz, int cx, int y)
{
    auto* k = knobs.add (new Knob (state, id, sz));
    auto ps = Knob::preferredSize (sz);
    if (Params::find (id) != nullptr && Params::find (id)->type == Params::Type::Choice)
        ps.setWidth (ps.getWidth() + 30);
    k->setBounds (cx - ps.getWidth() / 2, y, ps.getWidth(), ps.getHeight());
    addAndMakeVisible (k);
    return k;
}

BatSwitch* EnginePanel::sw (const juce::String& id, int x, int y, int w, int h, juce::Colour lamp)
{
    auto* s = new BatSwitch (state, id, lamp);
    others.add (s);
    s->setBounds (x, y, w, h);
    addAndMakeVisible (s);
    return s;
}

void EnginePanel::paint (juce::Graphics& g)
{
    paintDecor (g);
    for (auto& s : sections)
        drawSectionFrame (g, s.r, s.ru, s.en);
}

// ---------------------------------------------------------------------------
MoogPanel::MoogPanel (APVTS& s) : EnginePanel (s)
{
    section (8, 4, 330, 354, u8"ГЕНЕРАТОРЫ", "OSCILLATORS");
    for (int k = 0; k < 3; ++k)
    {
        const int y = 26 + k * 110;
        knob ("m_o" + juce::String (k + 1), Knob::Medium, 62, y);
        knob ("m_w" + juce::String (k + 1), Knob::Medium, 166, y);
        if (k > 0) knob ("m_d" + juce::String (k + 1), Knob::Medium, 272, y);
    }
    section (346, 4, 104, 354, u8"МИКШЕР", "");
    knob ("m_l1", Knob::Small, 398, 26);
    knob ("m_l2", Knob::Small, 398, 110);
    knob ("m_l3", Knob::Small, 398, 194);
    knob ("m_noise", Knob::Small, 398, 278);

    section (458, 4, 470, 172, u8"ФИЛЬТР", "LADDER FILTER");
    knob ("m_cut", Knob::Large, 514, 30);
    knob ("m_res", Knob::Medium, 610, 44);
    knob ("m_env", Knob::Medium, 688, 44);
    knob ("m_key", Knob::Medium, 766, 44);
    knob ("m_glide", Knob::Small, 844, 52);
    sw ("m_mono", 880, 40, 46, 112, Col::amber);

    section (458, 184, 232, 174, u8"КОНТУР ФИЛЬТРА", "FILTER EG");
    knob ("m_fa", Knob::Small, 490, 222);
    knob ("m_fd", Knob::Small, 546, 222);
    knob ("m_fs", Knob::Small, 602, 222);
    knob ("m_fr", Knob::Small, 658, 222);
    section (696, 184, 232, 174, u8"УСИЛИТЕЛЬ", "AMP EG");
    knob ("m_aa", Knob::Small, 728, 222);
    knob ("m_ad", Knob::Small, 784, 222);
    knob ("m_as", Knob::Small, 840, 222);
    knob ("m_ar", Knob::Small, 896, 222);
}

// ---------------------------------------------------------------------------
HammondPanel::HammondPanel (APVTS& s) : EnginePanel (s)
{
    section (8, 4, 482, 354, u8"РЕГИСТРЫ", "DRAWBARS");
    const juce::Colour brown (0xff6b3a22), white (0xffefe8d6), black (0xff1d1915);
    const juce::Colour tips[9] = { brown, brown, white, white, black, white, black, black, white };
    const char* feet[9] = { u8"16'", u8"5⅓'", u8"8'", u8"4'", u8"2⅔'", u8"2'", u8"1⅗'", u8"1⅓'", u8"1'" };
    for (int i = 0; i < 9; ++i)
    {
        auto* d = new Drawbar (s, "h_db" + juce::String (i + 1), tips[i], juce::String::fromUTF8 (feet[i]));
        others.add (d);
        d->setBounds (22 + i * 51, 24, 48, 326);
        addAndMakeVisible (d);
    }

    section (498, 4, 430, 172, u8"УДАРНЫЕ", "PERCUSSION");
    sw ("h_perc", 510, 34, 52, 116, Col::amber);
    knob ("h_pharm", Knob::Medium, 616, 44);
    knob ("h_pdec", Knob::Medium, 706, 44);
    knob ("h_pvol", Knob::Medium, 796, 44);
    knob ("h_click", Knob::Medium, 880, 44);

    section (498, 184, 430, 174, u8"ВИБРАТО И ЛЕСЛИ", "SCANNER / ROTARY");
    knob ("h_vib", Knob::Medium, 560, 228);
    knob ("h_leslie", Knob::Medium, 662, 228);
    knob ("h_lmix", Knob::Medium, 764, 222);
    knob ("h_drive", Knob::Medium, 866, 222);
}

// ---------------------------------------------------------------------------
KorgPanel::KorgPanel (APVTS& s) : EnginePanel (s)
{
    section (8, 4, 350, 236, u8"ГЕНЕРАТОРЫ", "VCO 1 / VCO 2");
    knob ("k_w1", Knob::Medium, 52, 28);
    knob ("k_pw", Knob::Medium, 136, 28);
    knob ("k_mix", Knob::Medium, 220, 28);
    knob ("k_ring", Knob::Medium, 304, 28);
    knob ("k_w2", Knob::Medium, 52, 132);
    knob ("k_semi", Knob::Medium, 136, 132);
    knob ("k_det", Knob::Medium, 220, 132);

    section (366, 4, 250, 236, u8"ФИЛЬТРЫ", "VCF");
    knob ("k_hpf", Knob::Medium, 420, 28);
    knob ("k_lpf", Knob::Large, 540, 24);
    knob ("k_res", Knob::Medium, 420, 132);
    knob ("k_env", Knob::Medium, 540, 140);

    section (624, 4, 304, 236, u8"ОГИБАЮЩАЯ", "ENVELOPE");
    knob ("k_a", Knob::Medium, 662, 80);
    knob ("k_d", Knob::Medium, 738, 80);
    knob ("k_s", Knob::Medium, 814, 80);
    knob ("k_r", Knob::Medium, 890, 80);

    section (8, 248, 920, 110, u8"ГЕНЕРАТОР НИЗКОЙ ЧАСТОТЫ", "LFO");
    knob ("k_lwave", Knob::Small, 90, 270);
    knob ("k_lrate", Knob::Small, 240, 270);
    knob ("k_lpitch", Knob::Small, 390, 270);
    knob ("k_lfilt", Knob::Small, 540, 270);
    knob ("k_lpw", Knob::Small, 690, 270);
}

void KorgPanel::paintDecor (juce::Graphics& g)
{
    // riveted identification plate in the LFO row
    auto plate = juce::Rectangle<float> (752, 274, 160, 64);
    g.setColour (juce::Colours::black.withAlpha (0.4f));
    g.fillRoundedRectangle (plate.translated (1.5f, 2.0f), 3.0f);
    juce::ColourGradient pg (Col::alu.brighter (0.3f), plate.getX(), plate.getY(), Col::alu.darker (0.3f), plate.getRight(), plate.getBottom(), false);
    g.setGradientFill (pg);
    g.fillRoundedRectangle (plate, 3.0f);
    g.setColour (Col::bakelite);
    g.setFont (fontSpaced (13.0f, true, 0.12f));
    g.drawText (juce::String::fromUTF8 (u8"МИНИ-КОРГ"), plate.withHeight (30).translated (0, 6), juce::Justification::centred);
    g.setFont (fontSpaced (8.0f, false, 0.1f));
    g.drawText (juce::String::fromUTF8 (u8"ДВУХГЕНЕРАТОРНЫЙ БЛОК · №0420"), plate.withTrimmedTop (34).withHeight (14), juce::Justification::centred);
    for (auto p : { plate.getTopLeft() + juce::Point<float> (7, 7), plate.getTopRight() + juce::Point<float> (-7, 7),
                    plate.getBottomLeft() + juce::Point<float> (7, -7), plate.getBottomRight() + juce::Point<float> (-7, -7) })
        drawScrew (g, p, 3.0f, 0.7f);
}

// ---------------------------------------------------------------------------
EPianoPanel::EPianoPanel (APVTS& s) : EnginePanel (s)
{
    section (8, 4, 456, 172, u8"МОЛОТОЧКИ", "HAMMERS & TINES");
    knob ("e_tine", Knob::Large, 72, 32);
    knob ("e_bark", Knob::Large, 184, 32);
    knob ("e_vel", Knob::Medium, 294, 44);
    knob ("e_detune", Knob::Medium, 394, 44);
    section (8, 184, 456, 174, u8"ЗВУЧАНИЕ", "SUSTAIN & TONE");
    knob ("e_decay", Knob::Large, 72, 214);
    knob ("e_rel", Knob::Large, 184, 214);
    knob ("e_tone", Knob::Large, 330, 214);
    section (472, 4, 456, 354, u8"ТРЕМОЛО", "SUITCASE TREMOLO");
    knob ("e_trate", Knob::Large, 610, 230);
    knob ("e_tdepth", Knob::Large, 790, 230);
}

void EPianoPanel::paintDecor (juce::Graphics& g)
{
    // speaker grille cloth with badge
    auto grille = juce::Rectangle<float> (494, 28, 412, 192);
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.fillRoundedRectangle (grille.expanded (4).translated (1.5f, 2.0f), 8.0f);
    g.setColour (Col::bakelite);
    g.fillRoundedRectangle (grille.expanded (4), 8.0f);
    g.setColour (juce::Colour (0xff3b302a));
    g.fillRoundedRectangle (grille, 6.0f);
    {
        juce::Graphics::ScopedSaveState ss (g);
        g.reduceClipRegion (grille.toNearestInt());
        g.setColour (juce::Colour (0xff6a5a4c).withAlpha (0.55f));
        for (float x = grille.getX() - grille.getHeight(); x < grille.getRight(); x += 4.0f)
            g.drawLine (x, grille.getBottom(), x + grille.getHeight(), grille.getY(), 1.0f);
        g.setColour (juce::Colour (0xff1e1814).withAlpha (0.5f));
        for (float x = grille.getX(); x < grille.getRight() + grille.getHeight(); x += 4.0f)
            g.drawLine (x, grille.getBottom(), x - grille.getHeight(), grille.getY(), 0.8f);
    }
    // chrome badge
    auto badge = juce::Rectangle<float> (0, 0, 200, 66).withCentre (grille.getCentre());
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.fillRoundedRectangle (badge.translated (2, 3), 33.0f);
    juce::ColourGradient bg (juce::Colour (0xfff3efe5), badge.getX(), badge.getY(), juce::Colour (0xff76726a), badge.getX(), badge.getBottom(), false);
    bg.addColour (0.5, juce::Colour (0xffbab5aa));
    g.setGradientFill (bg);
    g.fillRoundedRectangle (badge, 33.0f);
    g.setColour (Col::red);
    g.fillRoundedRectangle (badge.reduced (5), 28.0f);
    g.setColour (Col::ivory);
    g.setFont (fontSpaced (17.0f, true, 0.12f));
    g.drawText (juce::String::fromUTF8 (u8"ЭЛЕКТРОНИКА"), badge.withTrimmedTop (12).withHeight (24), juce::Justification::centred);
    g.setFont (fontSpaced (9.0f, true, 0.3f));
    g.drawText (juce::String::fromUTF8 (u8"ЭП-66"), badge.withTrimmedTop (38).withHeight (14), juce::Justification::centred);
}

// ---------------------------------------------------------------------------
FxRack::FxRack (APVTS& s) : EnginePanel (s)
{
    struct M { const char* ru; const char* en; const char* on; std::vector<const char*> k; };
    const std::vector<M> mods = {
        { u8"ПЕРЕГРУЗ", "TUBE DRIVE", "fx_drv_on", { "fx_drv", "fx_drv_tone" } },
        { u8"ФАЗЕР", "PHASER", "fx_phs_on", { "fx_phs_rate", "fx_phs_depth", "fx_phs_fb" } },
        { u8"ХОРУС", "CHORUS", "fx_cho_on", { "fx_cho_rate", "fx_cho_depth", "fx_cho_mix" } },
        { u8"ЭХО", "TAPE ECHO", "fx_dly_on", { "fx_dly_time", "fx_dly_fb", "fx_dly_mix" } },
        { u8"РЕВЕРБ", "PLATE", "fx_rev_on", { "fx_rev_size", "fx_rev_damp", "fx_rev_mix" } } };

    const int mw = 216;
    for (int i = 0; i < (int) mods.size(); ++i)
    {
        const int x = i * (mw + 5);
        section (x, 0, mw, 150, mods[(size_t) i].ru, mods[(size_t) i].en);
        sw (mods[(size_t) i].on, x + 8, 24, 46, 118, i == 0 ? Col::red : Col::green);
        int cx = x + 84;
        const int step = mods[(size_t) i].k.size() == 2 ? 74 : 54;
        if (mods[(size_t) i].k.size() == 2) cx += 10;
        for (auto* id : mods[(size_t) i].k)
        {
            knob (id, Knob::Small, cx, 46);
            cx += step;
        }
    }
}

// ===========================================================================
// Faceplate
// ===========================================================================
Faceplate::Faceplate (Kosmos66Processor& p)
    : proc (p), engineSelector (p.apvts), moog (p.apvts), hammond (p.apvts), korg (p.apvts), epiano (p.apvts),
      fx (p.apvts), master (p.apvts, "master", Knob::Medium),
      keyboard (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    hammertone = makeHammertone (W, H, Col::body, 66u);
    woodL = makeWood (34, H, 11u);
    woodR = makeWood (34, H, 29u);

    addAndMakeVisible (engineSelector);
    for (auto* panel : std::initializer_list<EnginePanel*> { &moog, &hammond, &korg, &epiano, &fx })
    {
        addChildComponent (panel);
        panel->setPopupParent (this);
    }
    fx.setVisible (true);
    master.setPopupParent (this);
    addAndMakeVisible (master);

    addAndMakeVisible (nixie);
    nixie.onClick = [this] { showPresetMenu(); };
    nixie.onRename = [this] (const juce::String& n) { if (n.isNotEmpty()) proc.presets.setCurrentName (n); };
    addAndMakeVisible (prevKey);
    addAndMakeVisible (nextKey);
    prevKey.onClick = [this] { proc.presets.step (-1); };
    nextKey.onClick = [this] { proc.presets.step (+1); };
    prevKey.setTooltip ("Previous preset");
    nextKey.setTooltip ("Next preset");

    addAndMakeVisible (vuL);
    addAndMakeVisible (vuR);

    storeKey.setSubText ("STORE");
    storeKey.setLamp (true, Col::red);
    storeKey.setTooltip ("Arm, then click a memory slot to save the current patch into it");
    storeKey.onClick = [this] { storeArmed = ! storeArmed; storeKey.setLatched (storeArmed); };
    importKey.setSubText ("LOAD FILE");
    exportKey.setSubText ("SAVE FILE");
    importKey.onClick = [this] { doImport(); };
    exportKey.onClick = [this] { doExport(); };
    for (auto* k : { &storeKey, &importKey, &exportKey }) addAndMakeVisible (k);

    for (int i = 0; i < PresetManager::numSlots; ++i)
    {
        auto* k = slotKeys.add (new PushKey (juce::String (i + 1), Col::ivory));
        k->setLamp (true, Col::amber);
        k->onClick = [this, i] { slotClicked (i, juce::ModifierKeys::getCurrentModifiers()); };
        k->setTooltip ("Click: load (empty slot: save).  With STORE armed: save.  Right-click: options");
        addAndMakeVisible (k);
    }

    keyboard.setColour (juce::MidiKeyboardComponent::whiteNoteColourId, Col::ivory);
    keyboard.setColour (juce::MidiKeyboardComponent::blackNoteColourId, Col::bakelite);
    keyboard.setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId, juce::Colour (0xff8c846e));
    keyboard.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, Col::amber.withAlpha (0.25f));
    keyboard.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, Col::red.withAlpha (0.55f));
    keyboard.setColour (juce::MidiKeyboardComponent::shadowColourId, juce::Colours::black.withAlpha (0.4f));
    keyboard.setColour (juce::MidiKeyboardComponent::upDownButtonBackgroundColourId, Col::bakelite);
    keyboard.setColour (juce::MidiKeyboardComponent::upDownButtonArrowColourId, Col::cream);
    keyboard.setAvailableRange (24, 108);
    keyboard.setOctaveForMiddleC (4);
    addAndMakeVisible (keyboard);

    engineSelector.onEngineChanged = [this] (int e) { showEngine (e); };
    showEngine (engineSelector.current());

    proc.presets.onChange = [this] { refreshSlots(); };
    refreshSlots();
    setSize (W, H);
    startTimerHz (30);
}

Faceplate::~Faceplate()
{
    proc.presets.onChange = nullptr;
    stopTimer();
}

void Faceplate::showEngine (int e)
{
    moog.setVisible (e == 0);
    hammond.setVisible (e == 1);
    korg.setVisible (e == 2);
    epiano.setVisible (e == 3);
}

void Faceplate::resized()
{
    engineSelector.setBounds (48, 196, 152, 366);
    for (auto* panel : std::initializer_list<juce::Component*> { &moog, &hammond, &korg, &epiano })
        panel->setBounds (214, 198, 936, 364);
    fx.setBounds (50, 574, 1100, 152);

    prevKey.setBounds (394, 34, 34, 52);
    nixie.setBounds (432, 12, 340, 96);
    nextKey.setBounds (776, 34, 34, 52);
    master.setBounds (826, 10, 84, 100);
    vuL.setBounds (922, 16, 112, 88);
    vuR.setBounds (1040, 16, 112, 88);

    storeKey.setBounds (50, 126, 96, 56);
    for (int i = 0; i < slotKeys.size(); ++i)
        slotKeys[i]->setBounds (156 + i * 98, 126, 92, 56);
    importKey.setBounds (948, 126, 100, 56);
    exportKey.setBounds (1052, 126, 100, 56);

    keyboard.setBounds (54, 752, 1092, 60);
    keyboard.setKeyWidth (1092.0f / 50.0f);
    keyboard.setLowestVisibleKey (36);
}

void Faceplate::paint (juce::Graphics& g)
{
    g.drawImageAt (hammertone, 0, 0);

    // darker inset behind engine panel & selector
    auto inset = juce::Rectangle<float> (206, 190, 952, 378);
    g.setColour (juce::Colours::black.withAlpha (0.14f));
    g.fillRoundedRectangle (inset, 6.0f);
    g.setColour (juce::Colours::black.withAlpha (0.35f));
    g.drawRoundedRectangle (inset, 6.0f, 1.2f);
    g.setColour (juce::Colours::white.withAlpha (0.08f));
    g.drawRoundedRectangle (inset.translated (0.8f, 1.2f), 6.0f, 1.0f);

    // aluminium name plate
    auto plate = juce::Rectangle<float> (50, 14, 328, 94);
    g.setColour (juce::Colours::black.withAlpha (0.45f));
    g.fillRoundedRectangle (plate.translated (2, 3), 5.0f);
    juce::ColourGradient pg (juce::Colour (0xffe3dfd4), plate.getX(), plate.getY(), juce::Colour (0xff9a968b), plate.getX(), plate.getBottom(), false);
    g.setGradientFill (pg);
    g.fillRoundedRectangle (plate, 5.0f);
    {
        juce::Random rnd (3);
        for (float y = plate.getY() + 2; y < plate.getBottom() - 2; y += 1.0f)
        {
            g.setColour ((rnd.nextBool() ? juce::Colours::white : juce::Colours::black).withAlpha (0.04f + rnd.nextFloat() * 0.04f));
            g.drawHorizontalLine ((int) y, plate.getX() + 2, plate.getRight() - 2);
        }
    }
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.drawRoundedRectangle (plate, 5.0f, 1.0f);
    drawStar (g, { plate.getX() + 42, plate.getY() + 42 }, 26.0f, Col::red);
    g.setColour (Col::red);
    g.setFont (fontSpaced (37.0f, true, 0.06f));
    g.drawText (juce::String::fromUTF8 (u8"КОСМОС-66"), juce::Rectangle<float> (plate.getX() + 76, plate.getY() + 8, 246, 44), juce::Justification::centredLeft);
    g.setColour (Col::bakelite);
    g.setFont (fontSpaced (10.5f, true, 0.12f));
    g.drawText (juce::String::fromUTF8 (u8"ПОЛИФОНИЧЕСКИЙ СИНТЕЗАТОР"), juce::Rectangle<float> (plate.getX() + 78, plate.getY() + 52, 246, 14), juce::Justification::centredLeft);
    g.setFont (fontSpaced (8.5f, false, 0.2f));
    g.drawText ("KOSMOS-66  POLYPHONIC SYNTHESIZER", juce::Rectangle<float> (plate.getX() + 78, plate.getY() + 66, 246, 12), juce::Justification::centredLeft);
    for (auto pt : { plate.getTopLeft() + juce::Point<float> (7, 7), plate.getTopRight() + juce::Point<float> (-7, 7),
                     plate.getBottomLeft() + juce::Point<float> (7, -7), plate.getBottomRight() + juce::Point<float> (-7, -7) })
        drawScrew (g, pt, 3.2f, 0.5f);

    // memory strip caption + divider rules
    g.setColour (Col::cream.withAlpha (0.55f));
    g.drawHorizontalLine (118, 50, 1150);
    g.drawHorizontalLine (188, 50, 1150);
    g.setFont (fontSpaced (8.5f, true, 0.3f));
    g.setColour (Col::cream.withAlpha (0.8f));
    g.drawText (juce::String::fromUTF8 (u8"ЯЧЕЙКИ ПАМЯТИ · MEMORY"), juce::Rectangle<float> (156, 119, 300, 8), juce::Justification::centredLeft);

    // keyboard bed: bakelite rail with maker's mark
    auto bed = juce::Rectangle<float> (46, 732, 1108, 84);
    g.setColour (Col::bakelite);
    g.fillRoundedRectangle (bed, 5.0f);
    g.setColour (Col::cream.withAlpha (0.7f));
    g.setFont (fontSpaced (9.0f, true, 0.35f));
    g.drawText (juce::String::fromUTF8 (u8"ЗАВОД «ЭЛЕКТРОПРИБОР»  ·  СДЕЛАНО В СССР  ·  1966  ·  №000066"),
                bed.withHeight (19.0f), juce::Justification::centred);
    g.setColour (Col::red);
    g.fillRect (bed.getX() + 6, bed.getY() + 18, bed.getWidth() - 12, 1.5f);

    // wood cheeks
    g.drawImageAt (woodL, 0, 0);
    g.drawImageAt (woodR, W - 34, 0);
    g.setColour (juce::Colours::black.withAlpha (0.55f));
    g.fillRect (34, 0, 3, H);
    g.fillRect (W - 37, 0, 3, H);

    // corner screws
    for (auto pt : { juce::Point<float> (44, 8), juce::Point<float> (1156, 8), juce::Point<float> (44, 724), juce::Point<float> (1156, 724) })
        drawScrew (g, pt, 4.5f, 0.9f);
}

void Faceplate::timerCallback()
{
    vuL.setLevel (proc.meterL.load());
    vuR.setLevel (proc.meterR.load());

    auto& pm = proc.presets;
    nixie.setText (pm.getCurrentName());
    juce::String sub;
    const int slot = pm.getActiveSlot();
    const int eng = engineSelector.current();
    if (slot >= 0) sub = juce::String::fromUTF8 (u8"ЯЧЕЙКА ") + juce::String (slot + 1);
    else if (pm.getCurrentFactoryIndex() >= 0)
        sub = juce::String::fromUTF8 (u8"ЗАВОД ") + juce::String (pm.getCurrentFactoryIndex() + 1).paddedLeft ('0', 2)
              + "/" + juce::String ((int) pm.factory().size());
    else sub = juce::String::fromUTF8 (u8"ПОЛЬЗОВАТЕЛЬ");
    nixie.setSubText (sub + juce::String::fromUTF8 (u8"  ·  ") + Params::engineNames()[eng]);

    if (storeArmed)
        storeKey.setLampOn ((++blink / 8) % 2 == 0);
    else
        storeKey.setLampOn (false);
}

void Faceplate::refreshSlots()
{
    auto& pm = proc.presets;
    for (int i = 0; i < slotKeys.size(); ++i)
    {
        slotKeys[i]->setSubText (pm.isSlotUsed (i) ? pm.getSlotName (i) : juce::String::fromUTF8 (u8"— пусто —"));
        slotKeys[i]->setLampOn (pm.getActiveSlot() == i);
    }
}

void Faceplate::slotClicked (int i, const juce::ModifierKeys& mods)
{
    auto& pm = proc.presets;
    if (mods.isPopupMenu())
    {
        juce::PopupMenu m;
        m.addSectionHeader (juce::String::fromUTF8 (u8"ЯЧЕЙКА ") + juce::String (i + 1));
        m.addItem (1, "Load", pm.isSlotUsed (i));
        m.addItem (2, "Save current patch here");
        m.addItem (3, "Clear slot", pm.isSlotUsed (i));
        m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (slotKeys[i]),
                         [this, i] (int r)
                         {
                             auto& p = proc.presets;
                             if (r == 1) p.loadSlot (i);
                             else if (r == 2) p.saveToSlot (i);
                             else if (r == 3) p.clearSlot (i);
                         });
        return;
    }
    if (storeArmed || ! pm.isSlotUsed (i))
    {
        pm.saveToSlot (i);
        storeArmed = false;
        storeKey.setLatched (false);
    }
    else
        pm.loadSlot (i);
}

void Faceplate::showPresetMenu()
{
    auto& pm = proc.presets;
    juce::PopupMenu menu;
    menu.addSectionHeader (juce::String::fromUTF8 (u8"ЗАВОДСКИЕ ТЕМБРЫ · FACTORY"));
    const auto& f = pm.factory();
    for (int e = 0; e < 4; ++e)
    {
        juce::PopupMenu sub;
        for (int i = 0; i < (int) f.size(); ++i)
            if (f[(size_t) i].engine == e)
                sub.addItem (i + 1, f[(size_t) i].name, true, pm.getCurrentFactoryIndex() == i);
        menu.addSubMenu (Params::engineNamesRu()[e] + "  /  " + Params::engineNames()[e], sub);
    }
    menu.addSeparator();
    menu.addSectionHeader (juce::String::fromUTF8 (u8"ЯЧЕЙКИ ПАМЯТИ · USER SLOTS"));
    for (int s = 0; s < PresetManager::numSlots; ++s)
        menu.addItem (1000 + s, juce::String (s + 1) + ".  " + pm.getSlotName (s), pm.isSlotUsed (s), pm.getActiveSlot() == s);

    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&nixie),
                        [this] (int r)
                        {
                            if (r >= 1000) proc.presets.loadSlot (r - 1000);
                            else if (r > 0) proc.presets.loadFactory (r - 1);
                        });
}

void Faceplate::doImport()
{
    chooser = std::make_unique<juce::FileChooser> ("Load KOSMOS-66 patch",
        juce::File::getSpecialLocation (juce::File::userDocumentsDirectory), "*.k66;*.xml");
    chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                          [this] (const juce::FileChooser& fc)
                          {
                              auto file = fc.getResult();
                              if (file.existsAsFile()) proc.presets.importPatch (file);
                          });
}

void Faceplate::doExport()
{
    const auto name = proc.presets.getCurrentName().replaceCharacters (" /\\:", "____");
    chooser = std::make_unique<juce::FileChooser> ("Save KOSMOS-66 patch",
        juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile (name + ".k66"), "*.k66");
    chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles
                              | juce::FileBrowserComponent::warnAboutOverwriting,
                          [this] (const juce::FileChooser& fc)
                          {
                              auto file = fc.getResult();
                              if (file != juce::File()) proc.presets.exportPatch (file.withFileExtension (".k66"));
                          });
}
}

// ===========================================================================
Kosmos66Editor::Kosmos66Editor (Kosmos66Processor& p)
    : AudioProcessorEditor (&p), proc (p), faceplate (p)
{
    setLookAndFeel (&lnf);
    juce::LookAndFeel::setDefaultLookAndFeel (&lnf);
    faceplate.setLookAndFeel (&lnf);
    addAndMakeVisible (faceplate);

    const double scale = (double) proc.apvts.state.getProperty ("uiScale", 0.9);
    setResizable (true, true);
    setResizeLimits ((int) (Soviet::Faceplate::W * 0.55), (int) (Soviet::Faceplate::H * 0.55),
                     (int) (Soviet::Faceplate::W * 1.8), (int) (Soviet::Faceplate::H * 1.8));
    if (auto* c = getConstrainer())
        c->setFixedAspectRatio ((double) Soviet::Faceplate::W / (double) Soviet::Faceplate::H);
    setSize ((int) (Soviet::Faceplate::W * scale), (int) (Soviet::Faceplate::H * scale));
}

Kosmos66Editor::~Kosmos66Editor()
{
    juce::LookAndFeel::setDefaultLookAndFeel (nullptr);
    faceplate.setLookAndFeel (nullptr);
    setLookAndFeel (nullptr);
}

void Kosmos66Editor::resized()
{
    const float s = (float) getWidth() / (float) Soviet::Faceplate::W;
    faceplate.setTransform (juce::AffineTransform::scale (s));
    faceplate.setBounds (0, 0, Soviet::Faceplate::W, Soviet::Faceplate::H);
    proc.apvts.state.setProperty ("uiScale", s, nullptr);
}
