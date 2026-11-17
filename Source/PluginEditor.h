#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"
#include "UI/Controls.h"

namespace Soviet
{
// Base for the four engine faceplates: fixed 936 x 364 design size
class EnginePanel : public juce::Component
{
public:
    EnginePanel (APVTS& s) : state (s) {}
    void paint (juce::Graphics&) override;
    void setPopupParent (juce::Component* p) { for (auto* k : knobs) k->setPopupParent (p); }

protected:
    Knob* knob (const juce::String& id, Knob::Size sz, int cx, int y);
    BatSwitch* sw (const juce::String& id, int x, int y, int w = 50, int h = 104, juce::Colour lamp = Col::red);
    void section (int x, int y, int w, int h, const char* ru, const char* en) { sections.push_back ({ { (float) x, (float) y, (float) w, (float) h }, juce::String::fromUTF8 (ru), en }); }
    virtual void paintDecor (juce::Graphics&) {}

    APVTS& state;
    juce::OwnedArray<Knob> knobs;
    juce::OwnedArray<juce::Component> others;
    struct Section { juce::Rectangle<float> r; juce::String ru, en; };
    std::vector<Section> sections;
};

class MoogPanel    : public EnginePanel { public: explicit MoogPanel (APVTS&); };
class HammondPanel : public EnginePanel { public: explicit HammondPanel (APVTS&); };
class KorgPanel    : public EnginePanel { public: explicit KorgPanel (APVTS&); void paintDecor (juce::Graphics&) override; };
class EPianoPanel  : public EnginePanel { public: explicit EPianoPanel (APVTS&); void paintDecor (juce::Graphics&) override; };

class FxRack : public EnginePanel { public: explicit FxRack (APVTS&); };

// The whole faceplate at its 1200 x 820 design size; the editor scales it.
class Faceplate : public juce::Component, private juce::Timer
{
public:
    static constexpr int W = 1200, H = 820;
    explicit Faceplate (Kosmos66Processor&);
    ~Faceplate() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void showEngine (int e);
    void showPresetMenu();
    void slotClicked (int i, const juce::ModifierKeys& mods);
    void refreshSlots();
    void doImport();
    void doExport();

    Kosmos66Processor& proc;
    EngineSelector engineSelector;
    MoogPanel moog;
    HammondPanel hammond;
    KorgPanel korg;
    EPianoPanel epiano;
    FxRack fx;
    NixieDisplay nixie;
    PushKey prevKey { "<", Col::bakelite, PushKey::Left }, nextKey { ">", Col::bakelite, PushKey::Right };
    Knob master;
    VUMeter vuL { "L" }, vuR { "R" };
    PushKey storeKey { juce::String::fromUTF8 (u8"ЗАПИСЬ"), Col::red };
    PushKey importKey { juce::String::fromUTF8 (u8"ИМПОРТ"), Col::bakelite };
    PushKey exportKey { juce::String::fromUTF8 (u8"ЭКСПОРТ"), Col::bakelite };
    juce::OwnedArray<PushKey> slotKeys;
    juce::MidiKeyboardComponent keyboard;
    juce::TooltipWindow tooltips { this, 600 };

    bool storeArmed = false;
    int blink = 0;
    juce::Image hammertone, woodL, woodR;
    std::unique_ptr<juce::FileChooser> chooser;
};
}

class Kosmos66Editor : public juce::AudioProcessorEditor
{
public:
    explicit Kosmos66Editor (Kosmos66Processor&);
    ~Kosmos66Editor() override;
    void resized() override;
    void paint (juce::Graphics& g) override { g.fillAll (juce::Colours::black); }

private:
    Kosmos66Processor& proc;
    Soviet::LookAndFeel lnf;
    Soviet::Faceplate faceplate;
};
