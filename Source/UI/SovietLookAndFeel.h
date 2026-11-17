#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace Soviet
{
namespace Col
{
    const juce::Colour body      { 0xff5f6857 };   // hammertone green-grey enamel
    const juce::Colour bodyDark  { 0xff4b5345 };
    const juce::Colour inset     { 0xff535b4c };
    const juce::Colour cream     { 0xffeee4c8 };   // silkscreen
    const juce::Colour creamDim  { 0xffc9bf9f };
    const juce::Colour red       { 0xffb3261e };
    const juce::Colour redDark   { 0xff7a1712 };
    const juce::Colour amber     { 0xffffa833 };
    const juce::Colour green     { 0xff7cd65a };
    const juce::Colour bakelite  { 0xff1b1714 };
    const juce::Colour alu       { 0xffbdb9ae };
    const juce::Colour wood      { 0xff5b3420 };
    const juce::Colour ivory     { 0xfff3ead2 };
    const juce::Colour nixieBg   { 0xff15110e };
}

juce::Font font (float height, bool bold = true);
juce::Font fontSpaced (float height, bool bold, float kerning);

// decorative helpers
void drawScrew (juce::Graphics&, juce::Point<float> centre, float radius, float angle);
void drawLamp (juce::Graphics&, juce::Rectangle<float> r, juce::Colour c, bool on);
void drawSectionFrame (juce::Graphics&, juce::Rectangle<float> r, const juce::String& ru, const juce::String& en);
void drawStar (juce::Graphics&, juce::Point<float> c, float radius, juce::Colour fill);
void drawWaveGlyph (juce::Graphics&, const juce::String& wave, juce::Rectangle<float> r, juce::Colour c);
juce::Image makeHammertone (int w, int h, juce::Colour base, juce::uint32 seed);
juce::Image makeWood (int w, int h, juce::uint32 seed);

class LookAndFeel : public juce::LookAndFeel_V4
{
public:
    LookAndFeel();

    juce::Typeface::Ptr getTypefaceForFont (const juce::Font&) override;

    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;
    void drawLinearSlider (juce::Graphics&, int x, int y, int w, int h, float pos, float minPos, float maxPos,
                           juce::Slider::SliderStyle, juce::Slider&) override;
    void drawBubble (juce::Graphics&, juce::BubbleComponent&, const juce::Point<float>& tip,
                     const juce::Rectangle<float>& body) override;
    juce::Font getSliderPopupFont (juce::Slider&) override { return font (13.0f); }
    int getSliderPopupPlacement (juce::Slider&) override { return juce::BubbleComponent::above; }

    void drawPopupMenuBackground (juce::Graphics&, int w, int h) override;
    juce::Font getPopupMenuFont() override { return font (15.0f); }

    static juce::Typeface::Ptr boldFace();
    static juce::Typeface::Ptr regularFace();

private:
    void drawBakeliteKnob (juce::Graphics&, juce::Rectangle<float> r, float angle);
    void drawChickenHead (juce::Graphics&, juce::Rectangle<float> r, float angle);
};
}
