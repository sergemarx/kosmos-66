#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "SovietLookAndFeel.h"
#include "../Params.h"

namespace Soviet
{
using APVTS = juce::AudioProcessorValueTreeState;

// ---------------------------------------------------------------------------
// Rotary knob (bakelite) or rotary selector (chicken-head) with silkscreen labels
// ---------------------------------------------------------------------------
class Knob : public juce::Component
{
public:
    enum Size { Large, Medium, Small };
    Knob (APVTS&, const juce::String& paramId, Size size = Medium);

    void paint (juce::Graphics&) override;
    void resized() override;
    static juce::Rectangle<int> preferredSize (Size);
    void setPopupParent (juce::Component* p) { slider.setPopupDisplayEnabled (true, false, p); }

    juce::Slider slider;

private:
    const Params::Def* def = nullptr;
    Size size;
    bool stepped = false;
    juce::StringArray choices;
    std::unique_ptr<APVTS::SliderAttachment> attachment;
    juce::Rectangle<float> dial;
};

// ---------------------------------------------------------------------------
// Hammond drawbar
// ---------------------------------------------------------------------------
class Drawbar : public juce::Component
{
public:
    Drawbar (APVTS&, const juce::String& paramId, juce::Colour tip, const juce::String& footage);
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;

private:
    juce::Rectangle<float> slot() const;
    float value = 0.0f;
    int dragStartValue = 0;
    juce::Colour tipColour;
    juce::String footage;
    juce::ParameterAttachment attachment;
};

// ---------------------------------------------------------------------------
// Bat-handle toggle switch with jewel lamp
// ---------------------------------------------------------------------------
class BatSwitch : public juce::Button
{
public:
    BatSwitch (APVTS&, const juce::String& paramId, juce::Colour lampColour = Col::red, bool showLabels = true);
    void paintButton (juce::Graphics&, bool over, bool down) override;

private:
    const Params::Def* def = nullptr;
    juce::Colour lamp;
    bool labels;
    std::unique_ptr<APVTS::ButtonAttachment> attachment;
};

// ---------------------------------------------------------------------------
// Bakelite push key (generic button)
// ---------------------------------------------------------------------------
class PushKey : public juce::Button
{
public:
    enum Glyph { None, Left, Right };
    PushKey (const juce::String& text, juce::Colour face = Col::bakelite, Glyph glyph = None);
    void paintButton (juce::Graphics&, bool over, bool down) override;
    void setLamp (bool hasLamp, juce::Colour c = Col::amber) { lamp = hasLamp; lampColour = c; repaint(); }
    void setLampOn (bool on) { if (lampOn != on) { lampOn = on; repaint(); } }
    void setSubText (const juce::String& s) { if (sub != s) { sub = s; repaint(); } }
    void setLatched (bool l) { if (latched != l) { latched = l; repaint(); } }

private:
    juce::Colour face;
    Glyph glyph;
    juce::String sub;
    bool lamp = false, lampOn = false, latched = false;
    juce::Colour lampColour = Col::amber;
};

// ---------------------------------------------------------------------------
// Engine selector: four piano-key buttons with lamps, bound to "engine"
// ---------------------------------------------------------------------------
class EngineSelector : public juce::Component
{
public:
    explicit EngineSelector (APVTS&);
    void resized() override;
    void paint (juce::Graphics&) override;
    int current() const { return selected; }
    std::function<void (int)> onEngineChanged;

private:
    juce::OwnedArray<PushKey> keys;
    juce::ParameterAttachment attachment;
    int selected = 0;
};

// ---------------------------------------------------------------------------
// Nixie tube preset display
// ---------------------------------------------------------------------------
class NixieDisplay : public juce::Component, public juce::SettableTooltipClient
{
public:
    NixieDisplay();
    void setText (const juce::String& t) { if (text != t) { text = t; repaint(); } }
    void setSubText (const juce::String& t) { if (sub != t) { sub = t; repaint(); } }
    juce::String getText() const { return text; }
    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;

    std::function<void()> onClick;
    std::function<void (const juce::String&)> onRename;

private:
    juce::String text, sub;
    juce::TextEditor editor;
    static constexpr int numTubes = 16;
};

// ---------------------------------------------------------------------------
// Analog VU meter
// ---------------------------------------------------------------------------
class VUMeter : public juce::Component
{
public:
    explicit VUMeter (const juce::String& channelLabel);
    void setLevel (float linearPeak);   // call ~30 Hz
    void paint (juce::Graphics&) override;

private:
    juce::String label;
    float needle = 0.0f;   // 0..1 position
};
}
