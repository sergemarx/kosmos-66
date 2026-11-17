#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

// One table describes every parameter: id, Cyrillic panel label, English sub-label,
// type and range. The processor builds its layout from it and the UI pulls labels from it.
namespace Params
{
    enum class Type { Float, Int, Choice, Bool };

    struct Def
    {
        const char* id;
        const char* ru;      // silkscreen label on the panel
        const char* en;      // small English sub-label
        Type type;
        float min = 0.0f, max = 1.0f, def = 0.0f;
        float skewCentre = -1.0f;      // >0: logarithmic-ish skew around this value
        juce::StringArray choices {};
        const char* unit = "";
    };

    const std::vector<Def>& all();
    const Def* find (const juce::String& id);

    juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    enum Engine { Moog = 0, Hammond, MiniKorg, EPiano, NumEngines };
    juce::StringArray engineNames();       // "MOOG" ...
    juce::StringArray engineNamesRu();     // Cyrillic
}
