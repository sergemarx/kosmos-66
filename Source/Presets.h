#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

struct FactoryPreset
{
    juce::String name;
    int engine;
    std::vector<std::pair<juce::String, float>> values; // real (denormalised) values; rest = defaults
};

// Handles factory presets, the 8 user save slots (persisted to disk and in the DAW
// session), and import / export of single patches.
class PresetManager
{
public:
    static constexpr int numSlots = 8;

    explicit PresetManager (juce::AudioProcessorValueTreeState& state);

    const std::vector<FactoryPreset>& factory() const { return presets; }
    void loadFactory (int index);
    void step (int delta);                       // prev / next through factory list

    juce::String getCurrentName() const { return currentName; }
    void setCurrentName (const juce::String& n) { currentName = n; }
    int getCurrentFactoryIndex() const { return currentFactory; }

    // user slots
    bool isSlotUsed (int slot) const;
    juce::String getSlotName (int slot) const;
    void saveToSlot (int slot);
    void loadSlot (int slot);
    void clearSlot (int slot);
    int  getActiveSlot() const { return activeSlot; }

    // persistence
    juce::ValueTree slotsToTree() const;
    void slotsFromTree (const juce::ValueTree& t);
    void saveSlotsToDisk() const;
    void loadSlotsFromDisk();

    bool exportPatch (const juce::File& f) const;
    bool importPatch (const juce::File& f);

    std::function<void()> onChange;   // UI refresh hook (message thread)

private:
    juce::ValueTree capturePatch() const;
    void applyPatch (const juce::ValueTree& patch);
    void setParam (const juce::String& id, float realValue);

    juce::AudioProcessorValueTreeState& apvts;
    std::vector<FactoryPreset> presets;
    juce::ValueTree slots[numSlots];
    juce::String currentName { "INIT" };
    int currentFactory = -1;
    int activeSlot = -1;

    static juce::File slotFile();
};
