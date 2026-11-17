#include "SovietLookAndFeel.h"
#include "BinaryData.h"

namespace Soviet
{
juce::Typeface::Ptr LookAndFeel::boldFace()
{
    static auto t = juce::Typeface::createSystemTypefaceFor (BinaryData::DejaVuSansCondensedBold_ttf,
                                                             BinaryData::DejaVuSansCondensedBold_ttfSize);
    return t;
}

juce::Typeface::Ptr LookAndFeel::regularFace()
{
    static auto t = juce::Typeface::createSystemTypefaceFor (BinaryData::DejaVuSansCondensed_ttf,
                                                             BinaryData::DejaVuSansCondensed_ttfSize);
    return t;
}

juce::Font font (float height, bool bold)
{
    return juce::Font (juce::FontOptions (bold ? LookAndFeel::boldFace() : LookAndFeel::regularFace()).withHeight (height));
}

juce::Font fontSpaced (float height, bool bold, float kerning)
{
    return juce::Font (juce::FontOptions (bold ? LookAndFeel::boldFace() : LookAndFeel::regularFace())
                           .withHeight (height).withKerningFactor (kerning));
}

// ---------------------------------------------------------------------------
void drawScrew (juce::Graphics& g, juce::Point<float> c, float r, float angle)
{
    g.setColour (juce::Colours::black.withAlpha (0.45f));
    g.fillEllipse (c.x - r + 0.8f, c.y - r + 1.2f, 2 * r, 2 * r);
    juce::ColourGradient cg (juce::Colour (0xffe8e4da), c.x - r * 0.6f, c.y - r * 0.6f,
                             juce::Colour (0xff6d6a62), c.x + r, c.y + r, true);
    g.setGradientFill (cg);
    g.fillEllipse (c.x - r, c.y - r, 2 * r, 2 * r);
    g.setColour (juce::Colour (0xff3a3833));
    g.drawEllipse (c.x - r, c.y - r, 2 * r, 2 * r, 0.7f);
    const float dx = std::cos (angle) * r * 0.75f, dy = std::sin (angle) * r * 0.75f;
    g.setColour (juce::Colour (0xff2b2925));
    g.drawLine (c.x - dx, c.y - dy, c.x + dx, c.y + dy, std::max (1.0f, r * 0.28f));
}

void drawLamp (juce::Graphics& g, juce::Rectangle<float> r, juce::Colour c, bool on)
{
    const auto ctr = r.getCentre();
    const float rad = r.getWidth() * 0.5f;
    // chrome bezel
    juce::ColourGradient bez (juce::Colour (0xffd9d5ca), ctr.x - rad, ctr.y - rad, juce::Colour (0xff57544d), ctr.x + rad, ctr.y + rad, false);
    g.setGradientFill (bez);
    g.fillEllipse (r);
    auto jewel = r.reduced (rad * 0.22f);
    if (on)
    {
        for (int i = 4; i >= 1; --i)
        {
            g.setColour (c.withAlpha (0.07f * (float) i));
            g.fillEllipse (r.expanded ((float) (5 - i) * rad * 0.35f));
        }
        juce::ColourGradient jg (c.brighter (0.9f), jewel.getCentreX() - rad * 0.2f, jewel.getCentreY() - rad * 0.25f,
                                 c.darker (0.2f), jewel.getRight(), jewel.getBottom(), true);
        g.setGradientFill (jg);
    }
    else
    {
        juce::ColourGradient jg (c.darker (1.2f).withSaturation (0.6f), jewel.getCentreX(), jewel.getY(),
                                 c.darker (3.0f), jewel.getRight(), jewel.getBottom(), true);
        g.setGradientFill (jg);
    }
    g.fillEllipse (jewel);
    // facet glint
    g.setColour (juce::Colours::white.withAlpha (on ? 0.7f : 0.25f));
    g.fillEllipse (jewel.getX() + jewel.getWidth() * 0.22f, jewel.getY() + jewel.getHeight() * 0.18f,
                   jewel.getWidth() * 0.25f, jewel.getHeight() * 0.2f);
}

void drawSectionFrame (juce::Graphics& g, juce::Rectangle<float> r, const juce::String& ru, const juce::String& en)
{
    g.setColour (Col::cream.withAlpha (0.8f));
    const float titleH = 15.0f;
    auto f = fontSpaced (12.5f, true, 0.08f);
    const juce::String title = ru + (en.isNotEmpty() ? juce::String::fromUTF8 (u8"  ·  ") + en : juce::String());
    const float tw = juce::GlyphArrangement::getStringWidth (f, title) + 16.0f;

    juce::Path p;
    const float x = r.getX(), y = r.getY() + titleH * 0.5f, w = r.getWidth(), h = r.getHeight() - titleH * 0.5f;
    const float tx0 = x + 14.0f, tx1 = tx0 + tw;
    p.startNewSubPath (tx0, y);
    p.lineTo (x + 4, y); p.quadraticTo (x, y, x, y + 4);
    p.lineTo (x, y + h - 4); p.quadraticTo (x, y + h, x + 4, y + h);
    p.lineTo (x + w - 4, y + h); p.quadraticTo (x + w, y + h, x + w, y + h - 4);
    p.lineTo (x + w, y + 4); p.quadraticTo (x + w, y, x + w - 4, y);
    p.lineTo (tx1, y);
    g.strokePath (p, juce::PathStrokeType (1.3f));

    g.setFont (f);
    g.setColour (Col::cream);
    g.drawText (title, juce::Rectangle<float> (tx0, r.getY(), tw, titleH), juce::Justification::centred);
}

void drawStar (juce::Graphics& g, juce::Point<float> c, float radius, juce::Colour fill)
{
    juce::Path star;
    star.addStar (c, 5, radius * 0.4f, radius, 0.0f);
    g.setColour (juce::Colours::black.withAlpha (0.35f));
    g.fillPath (star, juce::AffineTransform::translation (1.0f, 1.5f));
    juce::ColourGradient cg (fill.brighter (0.5f), c.x, c.y - radius, fill.darker (0.5f), c.x, c.y + radius, false);
    g.setGradientFill (cg);
    g.fillPath (star);
    g.setColour (juce::Colour (0xffe8c25a));
    g.strokePath (star, juce::PathStrokeType (radius * 0.07f));
}

void drawWaveGlyph (juce::Graphics& g, const juce::String& wave, juce::Rectangle<float> r, juce::Colour c)
{
    juce::Path p;
    const float x = r.getX(), y = r.getY(), w = r.getWidth(), h = r.getHeight();
    const float top = y + h * 0.15f, bot = y + h * 0.85f, mid = y + h * 0.5f;
    if (wave == "saw")
    {
        p.startNewSubPath (x, bot); p.lineTo (x + w * 0.5f, top); p.lineTo (x + w * 0.5f, bot); p.lineTo (x + w, top); p.lineTo (x + w, bot);
    }
    else if (wave == "tri")
    {
        p.startNewSubPath (x, mid); p.lineTo (x + w * 0.25f, top); p.lineTo (x + w * 0.75f, bot); p.lineTo (x + w, mid);
    }
    else if (wave == "sqr")
    {
        p.startNewSubPath (x, bot); p.lineTo (x, top); p.lineTo (x + w * 0.5f, top); p.lineTo (x + w * 0.5f, bot); p.lineTo (x + w, bot); p.lineTo (x + w, top);
    }
    else if (wave == "pls")
    {
        p.startNewSubPath (x, bot); p.lineTo (x, top); p.lineTo (x + w * 0.2f, top); p.lineTo (x + w * 0.2f, bot); p.lineTo (x + w * 0.6f, bot); p.lineTo (x + w * 0.6f, top); p.lineTo (x + w * 0.8f, top); p.lineTo (x + w * 0.8f, bot);
    }
    else if (wave == "noise")
    {
        juce::Random rnd (7);
        p.startNewSubPath (x, mid);
        for (int i = 1; i <= 9; ++i) p.lineTo (x + w * (float) i / 9.0f, top + rnd.nextFloat() * (bot - top));
    }
    else if (wave == "s&h")
    {
        const float lv[] = { 0.7f, 0.2f, 0.9f, 0.45f };
        p.startNewSubPath (x, y + h * lv[0]);
        for (int i = 0; i < 4; ++i)
        {
            p.lineTo (x + w * (float) (i + 1) / 4.0f, y + h * lv[i]);
            if (i < 3) p.lineTo (x + w * (float) (i + 1) / 4.0f, y + h * lv[i + 1]);
        }
    }
    else
    {
        g.setColour (c);
        g.setFont (font (h * 0.9f));
        g.drawText (wave.toUpperCase(), r.expanded (12, 0), juce::Justification::centred, false);
        return;
    }
    g.setColour (c);
    g.strokePath (p, juce::PathStrokeType (1.4f, juce::PathStrokeType::mitered, juce::PathStrokeType::butt));
}

juce::Image makeHammertone (int w, int h, juce::Colour base, juce::uint32 seed)
{
    juce::Image img (juce::Image::RGB, std::max (1, w), std::max (1, h), true);
    juce::Graphics g (img);
    g.fillAll (base);
    juce::Random rnd ((juce::int64) seed);
    const int dimples = w * h / 120;
    for (int i = 0; i < dimples; ++i)
    {
        const float x = rnd.nextFloat() * (float) w, y = rnd.nextFloat() * (float) h;
        const float r = 2.0f + rnd.nextFloat() * 7.0f;
        juce::ColourGradient cg (juce::Colours::white.withAlpha (0.045f), x - r * 0.5f, y - r * 0.5f,
                                 juce::Colours::black.withAlpha (0.06f), x + r * 0.6f, y + r * 0.6f, false);
        g.setGradientFill (cg);
        g.fillEllipse (x - r, y - r, 2 * r, 2 * r);
    }
    // fine grain
    for (int i = 0; i < w * h / 25; ++i)
    {
        g.setColour ((rnd.nextBool() ? juce::Colours::white : juce::Colours::black).withAlpha (0.035f));
        g.fillRect ((float) rnd.nextInt (w), (float) rnd.nextInt (h), 1.0f, 1.0f);
    }
    return img;
}

juce::Image makeWood (int w, int h, juce::uint32 seed)
{
    juce::Image img (juce::Image::RGB, std::max (1, w), std::max (1, h), true);
    juce::Graphics g (img);
    juce::ColourGradient cg (Col::wood.brighter (0.25f), 0, 0, Col::wood.darker (0.5f), (float) w, 0, false);
    cg.addColour (0.45, Col::wood.brighter (0.1f));
    g.setGradientFill (cg);
    g.fillAll();
    juce::Random rnd ((juce::int64) seed);
    for (int i = 0; i < w * 2; ++i)
    {
        const float x0 = rnd.nextFloat() * (float) w;
        const float amp = 1.0f + rnd.nextFloat() * 3.0f, freq = 0.004f + rnd.nextFloat() * 0.01f, ph = rnd.nextFloat() * 6.28f;
        juce::Path p;
        p.startNewSubPath (x0, 0);
        for (int y = 0; y <= h; y += 8)
            p.lineTo (x0 + amp * std::sin ((float) y * freq + ph), (float) y);
        g.setColour ((rnd.nextBool() ? juce::Colour (0xff2a160b) : juce::Colour (0xff8a5a37)).withAlpha (0.10f + rnd.nextFloat() * 0.18f));
        g.strokePath (p, juce::PathStrokeType (0.6f + rnd.nextFloat() * 1.2f));
    }
    // varnish sheen
    juce::ColourGradient sheen (juce::Colours::white.withAlpha (0.10f), (float) w * 0.3f, 0,
                                juce::Colours::transparentWhite, (float) w * 0.9f, 0, false);
    g.setGradientFill (sheen);
    g.fillAll();
    return img;
}

// ---------------------------------------------------------------------------
LookAndFeel::LookAndFeel()
{
    setColour (juce::PopupMenu::backgroundColourId, Col::bakelite);
    setColour (juce::PopupMenu::textColourId, Col::cream);
    setColour (juce::PopupMenu::headerTextColourId, Col::amber);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, Col::red);
    setColour (juce::PopupMenu::highlightedTextColourId, Col::ivory);
    setColour (juce::TextEditor::backgroundColourId, Col::nixieBg);
    setColour (juce::TextEditor::textColourId, Col::amber);
    setColour (juce::TextEditor::highlightColourId, Col::red);
    setColour (juce::TextEditor::outlineColourId, Col::amber.withAlpha (0.5f));
    setColour (juce::TextEditor::focusedOutlineColourId, Col::amber);
    setColour (juce::CaretComponent::caretColourId, Col::amber);
    setColour (juce::TooltipWindow::backgroundColourId, Col::ivory);
    setColour (juce::TooltipWindow::textColourId, Col::bakelite);
}

juce::Typeface::Ptr LookAndFeel::getTypefaceForFont (const juce::Font& f)
{
    return f.isBold() ? boldFace() : regularFace();
}

void LookAndFeel::drawBakeliteKnob (juce::Graphics& g, juce::Rectangle<float> r, float angle)
{
    const auto c = r.getCentre();
    const float R = r.getWidth() * 0.5f;

    // drop shadow
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.fillEllipse (r.translated (R * 0.06f, R * 0.12f).expanded (R * 0.03f));

    // fluted skirt
    juce::ColourGradient skirt (juce::Colour (0xff4a423b), c.x - R, c.y - R, juce::Colour (0xff070605), c.x + R, c.y + R, false);
    g.setGradientFill (skirt);
    g.fillEllipse (r);
    const int flutes = 30;
    for (int i = 0; i < flutes; ++i)
    {
        const float a = juce::MathConstants<float>::twoPi * (float) i / (float) flutes;
        const float lit = 0.5f + 0.5f * std::cos (a + 2.4f);
        g.setColour (juce::Colours::white.withAlpha (0.05f + 0.12f * lit));
        g.drawLine (c.x + std::cos (a) * R * 0.80f, c.y + std::sin (a) * R * 0.80f,
                    c.x + std::cos (a) * R * 0.98f, c.y + std::sin (a) * R * 0.98f, 1.1f);
    }
    // body
    const float Rb = R * 0.78f;
    juce::ColourGradient body (juce::Colour (0xff3d3630), c.x - Rb * 0.6f, c.y - Rb * 0.7f, juce::Colour (0xff0b0908), c.x + Rb * 0.5f, c.y + Rb * 0.8f, true);
    g.setGradientFill (body);
    g.fillEllipse (c.x - Rb, c.y - Rb, 2 * Rb, 2 * Rb);
    g.setColour (juce::Colours::white.withAlpha (0.08f));
    g.drawEllipse (c.x - Rb, c.y - Rb, 2 * Rb, 2 * Rb, 1.0f);

    // aluminium cap with machined rings
    const float Rc = R * 0.44f;
    juce::ColourGradient cap (juce::Colour (0xffeeeae0), c.x - Rc, c.y - Rc, juce::Colour (0xff77736a), c.x + Rc, c.y + Rc, false);
    g.setGradientFill (cap);
    g.fillEllipse (c.x - Rc, c.y - Rc, 2 * Rc, 2 * Rc);
    for (float k = 0.35f; k < 1.0f; k += 0.22f)
    {
        g.setColour (juce::Colours::black.withAlpha (0.08f));
        g.drawEllipse (c.x - Rc * k, c.y - Rc * k, 2 * Rc * k, 2 * Rc * k, 0.6f);
    }

    // pointer line (cream inlay) + red cap dot
    const float sx = std::sin (angle), cy = -std::cos (angle);
    g.setColour (Col::ivory);
    g.drawLine (c.x + sx * Rc * 1.05f, c.y + cy * Rc * 1.05f, c.x + sx * R * 0.93f, c.y + cy * R * 0.93f, std::max (2.0f, R * 0.09f));
    g.setColour (Col::red);
    const float d = R * 0.11f;
    g.fillEllipse (c.x + sx * Rc * 0.55f - d * 0.5f, c.y + cy * Rc * 0.55f - d * 0.5f, d, d);
}

void LookAndFeel::drawChickenHead (juce::Graphics& g, juce::Rectangle<float> r, float angle)
{
    const auto c = r.getCentre();
    const float R = r.getWidth() * 0.5f;

    juce::Path head;
    head.startNewSubPath (0.0f, -R * 0.98f);
    head.cubicTo (R * 0.16f, -R * 0.80f, R * 0.50f, -R * 0.10f, R * 0.52f, R * 0.30f);
    head.cubicTo (R * 0.54f, R * 0.70f, R * 0.30f, R * 0.82f, 0.0f, R * 0.82f);
    head.cubicTo (-R * 0.30f, R * 0.82f, -R * 0.54f, R * 0.70f, -R * 0.52f, R * 0.30f);
    head.cubicTo (-R * 0.50f, -R * 0.10f, -R * 0.16f, -R * 0.80f, 0.0f, -R * 0.98f);
    head.closeSubPath();
    const auto xf = juce::AffineTransform::rotation (angle).translated (c.x, c.y);

    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.fillPath (head, xf.translated (R * 0.07f, R * 0.12f));

    // round base
    juce::ColourGradient base (juce::Colour (0xff3c352f), c.x - R * 0.5f, c.y - R * 0.5f, juce::Colour (0xff080706), c.x + R * 0.5f, c.y + R * 0.6f, false);
    g.setGradientFill (base);
    g.fillEllipse (c.x - R * 0.62f, c.y - R * 0.62f, R * 1.24f, R * 1.24f);

    juce::ColourGradient hg (juce::Colour (0xff4b433c), c.x - R * 0.4f, c.y - R * 0.6f, juce::Colour (0xff0d0b09), c.x + R * 0.5f, c.y + R * 0.7f, false);
    g.setGradientFill (hg);
    g.fillPath (head, xf);
    g.setColour (juce::Colours::white.withAlpha (0.12f));
    g.strokePath (head, juce::PathStrokeType (1.0f), xf);

    // white inlay
    const float sx = std::sin (angle), cy = -std::cos (angle);
    g.setColour (Col::ivory);
    g.drawLine (c.x + sx * R * 0.1f, c.y + cy * R * 0.1f, c.x + sx * R * 0.86f, c.y + cy * R * 0.86f, std::max (1.8f, R * 0.08f));
}

void LookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                    float startAngle, float endAngle, juce::Slider& s)
{
    const bool stepped = (bool) s.getProperties().getWithDefault ("chicken", false);
    auto area = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h);
    const float side = std::min (area.getWidth(), area.getHeight());
    auto sq = area.withSizeKeepingCentre (side, side);
    const float angle = startAngle + pos * (endAngle - startAngle);
    const auto c = sq.getCentre();

    if (! stepped)
    {
        // printed scale: 11 ticks, numbered 0 / 5 / 10 on larger knobs
        const float R = side * 0.5f;
        const bool big = side > 58.0f;
        for (int i = 0; i <= 10; ++i)
        {
            const float a = startAngle + (endAngle - startAngle) * (float) i / 10.0f;
            const float sx = std::sin (a), cy = -std::cos (a);
            const bool major = (i % 5 == 0);
            g.setColour (Col::cream.withAlpha (major ? 0.95f : 0.7f));
            g.drawLine (c.x + sx * R * 0.86f, c.y + cy * R * 0.86f, c.x + sx * R * (major ? 1.0f : 0.96f), c.y + cy * R * (major ? 1.0f : 0.96f), major ? 1.6f : 1.0f);
        }
        if (big)
        {
            g.setFont (font (9.0f));
            g.setColour (Col::cream.withAlpha (0.8f));
            const float a0 = startAngle, a1 = endAngle;
            g.drawText ("0", juce::Rectangle<float> (c.x + std::sin (a0) * R * 1.0f - 12, c.y - std::cos (a0) * R * 1.0f, 10, 10), juce::Justification::centredRight);
            g.drawText ("10", juce::Rectangle<float> (c.x + std::sin (a1) * R * 1.0f + 1, c.y - std::cos (a1) * R * 1.0f, 14, 10), juce::Justification::centredLeft);
        }
        drawBakeliteKnob (g, sq.reduced (side * 0.17f), angle);
    }
    else
    {
        drawChickenHead (g, sq.reduced (side * 0.06f), angle);
    }
}

void LookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float minPos, float maxPos,
                                    juce::Slider::SliderStyle st, juce::Slider& s)
{
    LookAndFeel_V4::drawLinearSlider (g, x, y, w, h, pos, minPos, maxPos, st, s);
}

void LookAndFeel::drawBubble (juce::Graphics& g, juce::BubbleComponent&, const juce::Point<float>& tip,
                              const juce::Rectangle<float>& body)
{
    juce::Path p;
    p.addBubble (body.reduced (0.5f), body.getUnion (juce::Rectangle<float> (tip.x, tip.y, 1.0f, 1.0f)), tip, 3.0f, 8.0f);
    g.setColour (juce::Colours::black.withAlpha (0.35f));
    g.fillPath (p, juce::AffineTransform::translation (1.5f, 2.0f));
    g.setColour (Col::ivory);
    g.fillPath (p);
    g.setColour (Col::bakelite);
    g.strokePath (p, juce::PathStrokeType (1.2f));
}

void LookAndFeel::drawPopupMenuBackground (juce::Graphics& g, int w, int h)
{
    g.fillAll (Col::bakelite);
    g.setColour (Col::red);
    g.drawRect (0, 0, w, h, 2);
}
}
