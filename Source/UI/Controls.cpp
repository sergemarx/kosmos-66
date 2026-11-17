#include "Controls.h"

namespace Soviet
{
static juce::String fmtValue (const Params::Def* d, double v)
{
    if (d == nullptr) return juce::String (v, 2);
    const juce::String unit (d->unit);
    if (d->type == Params::Type::Choice)
        return d->choices[juce::jlimit (0, d->choices.size() - 1, (int) std::lround (v))].toUpperCase();
    if (d->type == Params::Type::Int)
        return (v > 0 && d->min < 0 ? "+" : "") + juce::String ((int) std::lround (v));
    if (unit == "Hz")  return v >= 1000.0 ? juce::String (v / 1000.0, 2) + " kHz" : juce::String (v, v < 10 ? 2 : 0) + " Hz";
    if (unit == "s")   return v < 1.0 ? juce::String (v * 1000.0, 0) + " ms" : juce::String (v, 2) + " s";
    if (unit == "ms")  return juce::String (v, 0) + " ms";
    if (unit == "ct")  return (v > 0 ? "+" : "") + juce::String (v, 1) + " ct";
    if (unit == "dB")  return (v > 0 ? "+" : "") + juce::String (v, 1) + " dB";
    // unitless 0..1 -> vintage 0..10 scale; bipolar -> -5..+5
    if (d->min < 0.0f) return (v > 0 ? "+" : "") + juce::String (v * 5.0, 1);
    return juce::String ((v - d->min) / (d->max - d->min) * 10.0, 1);
}

// ===========================================================================
Knob::Knob (APVTS& s, const juce::String& id, Size sz) : def (Params::find (id)), size (sz)
{
    jassert (def != nullptr);
    stepped = def->type == Params::Type::Choice;
    if (stepped) choices = def->choices;

    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider.getProperties().set ("chicken", stepped);

    const float pi = juce::MathConstants<float>::pi;
    if (stepped)
    {
        const float span = juce::jmin (270.0f, (float) (choices.size() - 1) * (choices.size() > 4 ? 42.0f : 55.0f));
        const float half = juce::degreesToRadians (span * 0.5f);
        slider.setRotaryParameters (2.0f * pi - half, 2.0f * pi + half, true);
        slider.setMouseDragSensitivity (60 * choices.size());
    }
    else
    {
        slider.setRotaryParameters (pi * 1.25f, pi * 2.75f, true);
        slider.setMouseDragSensitivity (size == Large ? 300 : 220);
    }
    addAndMakeVisible (slider);

    attachment = std::make_unique<APVTS::SliderAttachment> (s, id, slider);
    const auto* d = def;
    slider.textFromValueFunction = [d] (double v) { return fmtValue (d, v); };
    slider.setTooltip (juce::String::fromUTF8 (def->ru) + " / " + def->en);
}

juce::Rectangle<int> Knob::preferredSize (Size s)
{
    switch (s)
    {
        case Large:  return { 0, 0, 104, 122 };
        case Medium: return { 0, 0, 84, 100 };
        default:     return { 0, 0, 58, 82 };
    }
}

void Knob::resized()
{
    auto b = getLocalBounds().toFloat();
    const float labelH = size == Small ? 24.0f : 28.0f;
    auto top = b.withTrimmedBottom (labelH);
    float d = juce::jmin (top.getWidth(), top.getHeight());
    dial = top.withSizeKeepingCentre (d, d);
    if (stepped)
    {
        d = top.getHeight() * (choices.size() > 4 ? 0.50f : 0.54f);
        const float R = d * 0.5f;
        dial = juce::Rectangle<float> (d, d).withCentre ({ top.getCentreX(), top.getY() + R * 1.08f + 19.0f });
    }
    slider.setBounds (dial.toNearestInt());
}

void Knob::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    const float labelH = size == Small ? 24.0f : 28.0f;
    auto lab = b.removeFromBottom (labelH);

    g.setColour (Col::cream);
    g.setFont (fontSpaced (size == Small ? 10.0f : 12.0f, true, 0.02f));
    g.drawFittedText (juce::String::fromUTF8 (def->ru), lab.removeFromTop (labelH * 0.55f).toNearestInt(), juce::Justification::centredBottom, 1, 0.7f);
    g.setColour (Col::creamDim);
    g.setFont (fontSpaced (size == Small ? 8.0f : 8.8f, false, 0.12f));
    g.drawFittedText (def->en, lab.toNearestInt(), juce::Justification::centredTop, 1, 0.7f);

    if (stepped)
    {
        // labels / waveform glyphs printed around the selector
        const auto c = dial.getCentre();
        const float R = dial.getWidth() * 0.5f;
        const auto rp = slider.getRotaryParameters();
        const int n = choices.size();
        for (int i = 0; i < n; ++i)
        {
            const float a = rp.startAngleRadians + (rp.endAngleRadians - rp.startAngleRadians) * (float) i / (float) (n - 1);
            const float sx = std::sin (a), cy = -std::cos (a);
            const bool sel = (int) std::lround (slider.getValue()) == i;
            g.setColour (sel ? Col::amber : Col::cream.withAlpha (0.85f));
            g.fillEllipse (c.x + sx * R * 1.08f - 1.5f, c.y + cy * R * 1.08f - 1.5f, 3.0f, 3.0f);
            const float lr = R * 1.08f + 13.0f;
            auto cell = juce::Rectangle<float> (26.0f, 12.0f).withCentre ({ c.x + sx * lr, c.y + cy * lr });
            const auto& ch = choices[i];
            const bool glyph = ch == "saw" || ch == "tri" || ch == "sqr" || ch == "pls" || ch == "noise" || ch == "s&h";
            if (glyph)
                drawWaveGlyph (g, ch, cell.withSizeKeepingCentre (16.0f, 9.0f), sel ? Col::amber : Col::cream);
            else
            {
                g.setFont (font (n > 4 ? 9.0f : 9.5f));
                g.drawText (ch, cell.expanded (6, 0), juce::Justification::centred, false);
            }
        }
    }
}

// ===========================================================================
Drawbar::Drawbar (APVTS& s, const juce::String& id, juce::Colour tip, const juce::String& ft)
    : tipColour (tip), footage (ft),
      attachment (*s.getParameter (id), [this] (float v) { value = v; repaint(); }, nullptr)
{
    attachment.sendInitialUpdate();
    setRepaintsOnMouseActivity (false);
    setMouseCursor (juce::MouseCursor::UpDownResizeCursor);
}

juce::Rectangle<float> Drawbar::slot() const
{
    return getLocalBounds().toFloat().withTrimmedTop (22.0f).withSizeKeepingCentre (getWidth() * 0.62f, getHeight() - 22.0f);
}

void Drawbar::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    g.setColour (Col::cream);
    g.setFont (font (12.0f));
    g.drawText (footage, b.removeFromTop (18.0f), juce::Justification::centred);

    auto s = slot();
    const float travel = s.getHeight() - 46.0f;
    const float ext = travel * value / 8.0f;

    // housing slot
    g.setColour (juce::Colours::black.withAlpha (0.85f));
    g.fillRoundedRectangle (s.getX() + 3, s.getY(), s.getWidth() - 6, 10.0f, 2.0f);

    // shaft (pulled-out part), ivory with printed 1..8
    auto shaft = juce::Rectangle<float> (s.getCentreX() - s.getWidth() * 0.22f, s.getY() + 4, s.getWidth() * 0.44f, ext + 4);
    juce::ColourGradient sg (juce::Colour (0xffd8d2c0), shaft.getX(), 0, juce::Colour (0xfffbf6e8), shaft.getCentreX(), 0, false);
    sg.addColour (1.0, juce::Colour (0xffb7b09b));
    g.setGradientFill (sg);
    g.fillRect (shaft);
    g.setFont (font (9.0f));
    for (int k = 1; k <= (int) value; ++k)
    {
        const float y = s.getY() + 4 + travel * (float) k / 8.0f - 10.0f;
        g.setColour (juce::Colour (0xff2a2622));
        g.drawText (juce::String (k), juce::Rectangle<float> (shaft.getX(), y, shaft.getWidth(), 10.0f), juce::Justification::centred);
        g.setColour (juce::Colours::black.withAlpha (0.25f));
        g.drawHorizontalLine ((int) (y + 10.0f), shaft.getX(), shaft.getRight());
    }

    // tip knob
    auto tip = juce::Rectangle<float> (s.getX(), s.getY() + 4 + ext, s.getWidth(), 40.0f);
    g.setColour (juce::Colours::black.withAlpha (0.45f));
    g.fillRoundedRectangle (tip.translated (2.0f, 3.0f), 6.0f);
    juce::ColourGradient tg (tipColour.brighter (0.45f), tip.getX(), tip.getY(), tipColour.darker (0.55f), tip.getRight(), tip.getBottom(), false);
    g.setGradientFill (tg);
    g.fillRoundedRectangle (tip, 6.0f);
    g.setColour (juce::Colours::white.withAlpha (0.25f));
    g.fillRoundedRectangle (tip.reduced (4, 4).withHeight (6.0f), 3.0f);
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.drawRoundedRectangle (tip, 6.0f, 1.0f);
    // value engraved on tip
    g.setColour (tipColour.getPerceivedBrightness() > 0.5f ? Col::bakelite : Col::ivory);
    g.setFont (font (15.0f));
    g.drawText (juce::String ((int) value), tip.withTrimmedTop (8.0f), juce::Justification::centred);
}

void Drawbar::mouseDown (const juce::MouseEvent&)
{
    dragStartValue = (int) value;
    attachment.beginGesture();
}

void Drawbar::mouseDrag (const juce::MouseEvent& e)
{
    const float travel = slot().getHeight() - 46.0f;
    const int v = juce::jlimit (0, 8, dragStartValue + (int) std::round ((float) e.getDistanceFromDragStartY() / (travel / 8.0f)));
    if (v != (int) value) attachment.setValueAsPartOfGesture ((float) v);
}

void Drawbar::mouseUp (const juce::MouseEvent&) { attachment.endGesture(); }

void Drawbar::mouseDoubleClick (const juce::MouseEvent&)
{
    attachment.setValueAsCompleteGesture (value > 0.5f ? 0.0f : 8.0f);
}

void Drawbar::mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& w)
{
    const int v = juce::jlimit (0, 8, (int) value + (w.deltaY < 0 ? 1 : -1));
    attachment.setValueAsCompleteGesture ((float) v);
}

// ===========================================================================
BatSwitch::BatSwitch (APVTS& s, const juce::String& id, juce::Colour lc, bool showLabels)
    : juce::Button (id), def (Params::find (id)), lamp (lc), labels (showLabels)
{
    setClickingTogglesState (true);
    attachment = std::make_unique<APVTS::ButtonAttachment> (s, id, *this);
    if (def) setTooltip (juce::String::fromUTF8 (def->ru) + " / " + def->en);
}

void BatSwitch::paintButton (juce::Graphics& g, bool over, bool)
{
    auto b = getLocalBounds().toFloat();
    const bool on = getToggleState();
    auto labelArea = labels ? b.removeFromBottom (24.0f) : juce::Rectangle<float>();

    const float lampD = juce::jmin (16.0f, b.getWidth() * 0.4f);
    drawLamp (g, juce::Rectangle<float> (lampD, lampD).withCentre ({ b.getCentreX(), b.getY() + lampD * 0.5f + 2 }), lamp, on);
    b.removeFromTop (lampD + 6);

    // mounting plate (hex nut)
    const auto c = b.getCentre();
    const float R = juce::jmin (b.getWidth(), b.getHeight()) * 0.26f;
    juce::Path hex;
    hex.addPolygon (c, 6, R * 1.25f, 0.0f);
    juce::ColourGradient hg (juce::Colour (0xffe1ddd2), c.x - R, c.y - R, juce::Colour (0xff5f5c55), c.x + R, c.y + R, false);
    g.setGradientFill (hg);
    g.fillPath (hex);
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.strokePath (hex, juce::PathStrokeType (0.8f));
    g.setColour (juce::Colour (0xff2a2825));
    g.fillEllipse (c.x - R * 0.55f, c.y - R * 0.55f, R * 1.1f, R * 1.1f);

    // lever: up = on
    const float len = R * 2.3f;
    const float dir = on ? -1.0f : 1.0f;
    const juce::Point<float> tipP (c.x, c.y + dir * len);
    juce::Path lever;
    lever.startNewSubPath (c.x - R * 0.28f, c.y);
    lever.lineTo (tipP.x - R * 0.17f, tipP.y);
    lever.lineTo (tipP.x + R * 0.17f, tipP.y);
    lever.lineTo (c.x + R * 0.28f, c.y);
    lever.closeSubPath();
    g.setColour (juce::Colours::black.withAlpha (0.4f));
    g.fillPath (lever, juce::AffineTransform::translation (2.0f, 2.5f));
    juce::ColourGradient lg (juce::Colour (0xfff4f1ea), c.x - R * 0.3f, 0, juce::Colour (0xff6a675f), c.x + R * 0.3f, 0, false);
    g.setGradientFill (lg);
    g.fillPath (lever);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xfff7f5ef), tipP.x - R * 0.3f, tipP.y - R * 0.3f, juce::Colour (0xff77746c), tipP.x + R * 0.3f, tipP.y + R * 0.3f, true));
    g.fillEllipse (tipP.x - R * 0.36f, tipP.y - R * 0.36f, R * 0.72f, R * 0.72f);
    if (over)
    {
        g.setColour (Col::amber.withAlpha (0.15f));
        g.fillEllipse (c.x - R * 1.4f, c.y - R * 1.4f, R * 2.8f, R * 2.8f);
    }

    if (labels && def)
    {
        g.setColour (Col::cream);
        g.setFont (fontSpaced (10.5f, true, 0.04f));
        g.drawFittedText (juce::String::fromUTF8 (def->ru), labelArea.removeFromTop (13).toNearestInt(), juce::Justification::centredBottom, 1, 0.7f);
        g.setColour (Col::creamDim);
        g.setFont (fontSpaced (8.0f, false, 0.12f));
        g.drawFittedText (def->en, labelArea.toNearestInt(), juce::Justification::centredTop, 1, 0.7f);
    }
}

// ===========================================================================
PushKey::PushKey (const juce::String& text, juce::Colour f, Glyph gl) : juce::Button (text), face (f), glyph (gl) {}

void PushKey::paintButton (juce::Graphics& g, bool over, bool down)
{
    auto b = getLocalBounds().toFloat().reduced (1.0f);
    const bool pressed = down || latched;

    auto key = b;
    // recess
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.fillRoundedRectangle (key, 4.0f);
    key = key.reduced (2.0f);
    if (! pressed) key = key.withTrimmedBottom (3.0f);
    else key = key.withTrimmedTop (2.0f).withTrimmedBottom (1.0f);

    // side (key depth)
    g.setColour (face.darker (0.8f));
    g.fillRoundedRectangle (key.translated (0, pressed ? 1.0f : 3.0f), 3.5f);
    juce::ColourGradient kg (face.brighter (pressed ? 0.05f : 0.35f), key.getX(), key.getY(), face.darker (0.25f), key.getX(), key.getBottom(), false);
    g.setGradientFill (kg);
    g.fillRoundedRectangle (key, 3.5f);
    g.setColour (juce::Colours::white.withAlpha (over ? 0.22f : 0.12f));
    g.drawRoundedRectangle (key.reduced (0.5f), 3.5f, 1.0f);

    if (lamp)
    {
        const float d = juce::jmin (12.0f, key.getHeight() * 0.3f);
        drawLamp (g, juce::Rectangle<float> (d, d).withCentre ({ key.getRight() - d * 0.5f - 5, key.getY() + d * 0.5f + 5 }), lampColour, lampOn);
    }
    const auto ink = face.getPerceivedBrightness() > 0.55f ? Col::bakelite : Col::ivory;
    g.setColour (ink);
    if (glyph != None)
    {
        juce::Path t;
        const auto c = key.getCentre();
        const float s = juce::jmin (key.getWidth(), key.getHeight()) * 0.22f;
        if (glyph == Left) t.addTriangle (c.x + s, c.y - s, c.x + s, c.y + s, c.x - s, c.y);
        else               t.addTriangle (c.x - s, c.y - s, c.x - s, c.y + s, c.x + s, c.y);
        g.fillPath (t);
        return;
    }
    auto textArea = key.reduced (4.0f, 2.0f);
    if (lamp) { textArea.removeFromRight (10.0f); textArea.removeFromLeft (10.0f); }
    if (sub.isNotEmpty())
    {
        g.setFont (fontSpaced (juce::jmin (15.0f, key.getHeight() * 0.36f), true, 0.05f));
        g.drawFittedText (getButtonText(), textArea.removeFromTop (textArea.getHeight() * 0.55f).toNearestInt(), juce::Justification::centredBottom, 1, 0.6f);
        g.setColour (ink.withAlpha (0.7f));
        g.setFont (fontSpaced (9.0f, false, 0.08f));
        g.drawFittedText (sub, textArea.toNearestInt(), juce::Justification::centredTop, 1, 0.6f);
    }
    else
    {
        g.setFont (fontSpaced (juce::jmin (14.0f, key.getHeight() * 0.42f), true, 0.06f));
        g.drawFittedText (getButtonText(), textArea.toNearestInt(), juce::Justification::centred, 2, 0.6f);
    }
}

// ===========================================================================
EngineSelector::EngineSelector (APVTS& s)
    : attachment (*s.getParameter ("engine"), [this] (float v)
      {
          selected = juce::jlimit (0, 3, (int) std::lround (v));
          for (int i = 0; i < keys.size(); ++i) { keys[i]->setLampOn (i == selected); keys[i]->setLatched (i == selected); }
          if (onEngineChanged) onEngineChanged (selected);
      }, nullptr)
{
    const auto ru = Params::engineNamesRu();
    const auto en = Params::engineNames();
    for (int i = 0; i < 4; ++i)
    {
        auto* k = keys.add (new PushKey (ru[i], Col::ivory));
        k->setSubText (en[i]);
        k->setLamp (true, Col::red);
        k->onClick = [this, i] { attachment.setValueAsCompleteGesture ((float) i); };
        addAndMakeVisible (k);
    }
    attachment.sendInitialUpdate();
}

void EngineSelector::resized()
{
    auto b = getLocalBounds().withTrimmedTop (24);
    const int h = b.getHeight() / 4;
    for (auto* k : keys) k->setBounds (b.removeFromTop (h).reduced (2, 4));
}

void EngineSelector::paint (juce::Graphics& g)
{
    g.setColour (Col::cream);
    g.setFont (fontSpaced (12.5f, true, 0.1f));
    g.drawText (juce::String::fromUTF8 (u8"РЕЖИМ · ENGINE"), getLocalBounds().removeFromTop (20), juce::Justification::centred);
}

// ===========================================================================
NixieDisplay::NixieDisplay()
{
    editor.setFont (font (20.0f));
    editor.setJustification (juce::Justification::centred);
    editor.setInputRestrictions (24);
    editor.onReturnKey = [this]
    {
        editor.setVisible (false);
        if (onRename) onRename (editor.getText().trim().toUpperCase());
    };
    editor.onEscapeKey = [this] { editor.setVisible (false); };
    editor.onFocusLost = [this] { editor.setVisible (false); };
    addChildComponent (editor);
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
    setTooltip ("Click: preset list.  Double-click: rename patch");
}

void NixieDisplay::resized() { editor.setBounds (getLocalBounds().reduced (14, 18)); }

void NixieDisplay::mouseUp (const juce::MouseEvent& e)
{
    if (e.getNumberOfClicks() == 1 && onClick) onClick();
}

void NixieDisplay::mouseDoubleClick (const juce::MouseEvent&)
{
    editor.setText (text, false);
    editor.setVisible (true);
    editor.grabKeyboardFocus();
    editor.selectAll();
}

void NixieDisplay::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();

    // bezel
    juce::ColourGradient bz (juce::Colour (0xffcfcbc0), b.getX(), b.getY(), juce::Colour (0xff4e4b45), b.getX(), b.getBottom(), false);
    g.setGradientFill (bz);
    g.fillRoundedRectangle (b, 7.0f);
    auto glass = b.reduced (6.0f);
    g.setColour (Col::nixieBg);
    g.fillRoundedRectangle (glass, 4.0f);

    // honeycomb anode mesh
    g.setColour (juce::Colour (0xff2c241d));
    for (float y = glass.getY() + 3; y < glass.getBottom(); y += 5.0f)
        for (float x = glass.getX() + (((int) (y / 5.0f)) % 2 ? 2.5f : 0.0f); x < glass.getRight(); x += 5.0f)
            g.fillEllipse (x, y, 1.4f, 1.4f);

    // tubes
    auto row = glass.reduced (8.0f, 8.0f);
    auto subRow = row.removeFromBottom (13.0f);
    const float tw = row.getWidth() / (float) numTubes;
    const auto shown = text.toUpperCase().substring (0, numTubes);
    const int offset = (numTubes - shown.length()) / 2;

    for (int i = 0; i < numTubes; ++i)
    {
        auto t = juce::Rectangle<float> (row.getX() + tw * (float) i, row.getY(), tw, row.getHeight()).reduced (1.5f, 0.0f);
        juce::ColourGradient tg (juce::Colours::white.withAlpha (0.08f), t.getX(), t.getY(), juce::Colours::white.withAlpha (0.01f), t.getRight(), t.getY(), false);
        g.setGradientFill (tg);
        g.fillRoundedRectangle (t, t.getWidth() * 0.45f);
        g.setColour (juce::Colours::white.withAlpha (0.10f));
        g.drawRoundedRectangle (t, t.getWidth() * 0.45f, 0.8f);

        const int ci = i - offset;
        if (ci < 0 || ci >= shown.length()) continue;
        const juce::String ch = shown.substring (ci, ci + 1);
        auto f = font (juce::jmin (t.getHeight() * 0.62f, t.getWidth() * 1.45f), false);
        g.setFont (f);
        // glow passes
        for (int r = 3; r >= 1; --r)
        {
            g.setColour (Col::amber.withAlpha (0.10f));
            for (int k = 0; k < 8; ++k)
            {
                const float a = (float) k * 0.785f;
                g.drawFittedText (ch, t.translated (std::cos (a) * (float) r, std::sin (a) * (float) r).toNearestInt(), juce::Justification::centred, 1, 0.4f);
            }
        }
        g.setColour (juce::Colour (0xffff9a2e));
        g.drawFittedText (ch, t.toNearestInt(), juce::Justification::centred, 1, 0.4f);
        g.setColour (juce::Colour (0xffffe2b0).withAlpha (0.6f));
        g.drawFittedText (ch, t.reduced (0.5f).toNearestInt(), juce::Justification::centred, 1, 0.4f);
    }

    g.setColour (Col::amber.withAlpha (0.65f));
    g.setFont (fontSpaced (10.0f, true, 0.15f));
    g.drawText (sub, subRow, juce::Justification::centred);

    // glass reflection
    juce::ColourGradient refl (juce::Colours::white.withAlpha (0.10f), glass.getX(), glass.getY(), juce::Colours::transparentWhite, glass.getX(), glass.getCentreY(), false);
    g.setGradientFill (refl);
    g.fillRoundedRectangle (glass.withHeight (glass.getHeight() * 0.5f), 4.0f);
}

// ===========================================================================
VUMeter::VUMeter (const juce::String& l) : label (l) {}

void VUMeter::setLevel (float peak)
{
    // map -20 dBFS..+3 to 0..1, with mechanical ballistics
    const float db = juce::Decibels::gainToDecibels (peak, -60.0f) + 6.0f; // 0 VU ≈ -6 dBFS peak
    const float target = juce::jlimit (0.0f, 1.08f, (db + 20.0f) / 23.0f);
    needle += (target - needle) * (target > needle ? 0.35f : 0.12f);
    repaint();
}

void VUMeter::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    // black bakelite case
    g.setColour (Col::bakelite);
    g.fillRoundedRectangle (b, 6.0f);
    auto face = b.reduced (5.0f);
    juce::ColourGradient fg (juce::Colour (0xfff6e9c4), face.getCentreX(), face.getY(), juce::Colour (0xffd9c48f), face.getCentreX(), face.getBottom(), false);
    g.setGradientFill (fg);
    g.fillRoundedRectangle (face, 3.0f);

    juce::Graphics::ScopedSaveState clipState (g);
    g.reduceClipRegion (face.toNearestInt());
    const juce::Point<float> pivot (face.getCentreX(), face.getBottom() + face.getHeight() * 0.18f);
    const float R = face.getHeight() * 0.98f;
    const float a0 = -0.82f, a1 = 0.82f;
    auto angleFor = [&] (float db) { return a0 + (a1 - a0) * juce::jlimit (0.0f, 1.0f, (db + 20.0f) / 23.0f); };

    // scale arc: black then red zone
    juce::Path arc, red;
    arc.addCentredArc (pivot.x, pivot.y, R * 0.80f, R * 0.80f, 0.0f, a0, angleFor (0.0f), true);
    red.addCentredArc (pivot.x, pivot.y, R * 0.80f, R * 0.80f, 0.0f, angleFor (0.0f), a1, true);
    g.setColour (juce::Colour (0xff1f1b17));
    g.strokePath (arc, juce::PathStrokeType (1.4f));
    g.setColour (Col::red);
    g.strokePath (red, juce::PathStrokeType (4.0f));

    const float marks[] = { -20, -10, -7, -5, -3, -1, 0, 1, 2, 3 };
    g.setFont (font (8.5f));
    for (float m : marks)
    {
        const float a = angleFor (m);
        const float sx = std::sin (a), cy = -std::cos (a);
        g.setColour (m >= 0 ? Col::red : juce::Colour (0xff1f1b17));
        g.drawLine (pivot.x + sx * R * 0.80f, pivot.y + cy * R * 0.80f, pivot.x + sx * R * 0.88f, pivot.y + cy * R * 0.88f, 1.2f);
        if (m == -20 || m == -10 || m == -5 || m == -3 || m == 0 || m == 3)
            g.drawText (juce::String ((int) std::abs (m)), juce::Rectangle<float> (20, 10).withCentre ({ pivot.x + sx * R * 0.97f, pivot.y + cy * R * 0.97f }), juce::Justification::centred);
    }
    g.setColour (juce::Colour (0xff1f1b17));
    g.setFont (fontSpaced (11.0f, true, 0.1f));
    g.drawText ("VU", face.withTrimmedTop (face.getHeight() * 0.48f).withHeight (14), juce::Justification::centred);
    g.setFont (font (7.5f, false));
    g.drawText (juce::String::fromUTF8 (u8"ЭЛЕКТРОПРИБОР · М-4200"), face.withTrimmedTop (face.getHeight() * 0.66f).withHeight (10), juce::Justification::centred);
    g.setFont (font (10.0f));
    g.drawText (label, face.reduced (5, 3), juce::Justification::bottomLeft);

    // needle
    const float a = a0 + (a1 - a0) * juce::jlimit (0.0f, 1.08f, needle);
    const float sx = std::sin (a), cy = -std::cos (a);
    g.setColour (juce::Colours::black.withAlpha (0.25f));
    g.drawLine (pivot.x + 1.5f, pivot.y + 1.5f, pivot.x + sx * R * 0.9f + 1.5f, pivot.y + cy * R * 0.9f + 1.5f, 1.4f);
    g.setColour (juce::Colour (0xff141210));
    g.drawLine (pivot.x, pivot.y, pivot.x + sx * R * 0.9f, pivot.y + cy * R * 0.9f, 1.3f);

    // glass
    juce::ColourGradient refl (juce::Colours::white.withAlpha (0.28f), face.getX(), face.getY(), juce::Colours::transparentWhite, face.getCentreX(), face.getCentreY(), false);
    g.setGradientFill (refl);
    g.fillRoundedRectangle (face, 3.0f);
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.drawRoundedRectangle (face, 3.0f, 1.0f);
}
}
