// Offline test: renders every factory preset to WAV, checks levels/NaNs,
// and snapshots the editor for each engine to PNG.
#include <juce_audio_utils/juce_audio_utils.h>
#include "../Source/PluginProcessor.h"
#include "../Source/PluginEditor.h"

static void writeWav (const juce::File& f, const juce::AudioBuffer<float>& b, double sr)
{
    f.deleteFile();
    juce::WavAudioFormat wav;
    std::unique_ptr<juce::OutputStream> os (f.createOutputStream().release());
    auto opts = juce::AudioFormatWriterOptions().withSampleRate (sr).withNumChannels (b.getNumChannels()).withBitsPerSample (24);
    if (auto w = wav.createWriterFor (os, opts))
        w->writeFromAudioSampleBuffer (b, 0, b.getNumSamples());
}

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI gui;
    const juce::File outDir (argc > 1 ? argv[1] : "/tmp/kosmos_out");
    outDir.createDirectory();

    const double sr = 48000.0;
    const int block = 256;
    Kosmos66Processor proc;
    proc.setPlayConfigDetails (0, 2, sr, block);
    proc.prepareToPlay (sr, block);

    int failures = 0;
    const auto& presets = proc.presets.factory();
    for (int p = 0; p < (int) presets.size(); ++p)
    {
        proc.presets.loadFactory (p);
        const int total = (int) (sr * 3.5);
        juce::AudioBuffer<float> out (2, total);
        out.clear();

        // phrase: chord, then a melodic line (exercises mono/legato & glide)
        const int notesChord[] = { 48, 55, 60, 64 };
        const int line[] = { 60, 63, 67, 70, 72 };
        juce::MidiBuffer all;
        for (int n : notesChord) all.addEvent (juce::MidiMessage::noteOn (1, n, (juce::uint8) 100), 100);
        for (int n : notesChord) all.addEvent (juce::MidiMessage::noteOff (1, n), (int) (sr * 1.2));
        for (int i = 0; i < 5; ++i)
        {
            const int t = (int) (sr * (1.4 + 0.25 * i));
            all.addEvent (juce::MidiMessage::noteOn (1, line[i], (juce::uint8) (70 + i * 10)), t);
            all.addEvent (juce::MidiMessage::noteOff (1, line[i]), t + (int) (sr * 0.3));
        }

        juce::AudioBuffer<float> buf (2, block);
        for (int pos = 0; pos < total; pos += block)
        {
            const int n = std::min (block, total - pos);
            buf.setSize (2, n, false, false, true);
            juce::MidiBuffer mb;
            for (const auto m : all)
                if (m.samplePosition >= pos && m.samplePosition < pos + n)
                    mb.addEvent (m.getMessage(), m.samplePosition - pos);
            proc.processBlock (buf, mb);
            for (int ch = 0; ch < 2; ++ch) out.copyFrom (ch, pos, buf, ch, 0, n);
        }

        float peak = 0.0f; double sumSq = 0.0; bool bad = false;
        for (int ch = 0; ch < 2; ++ch)
            for (int i = 0; i < total; ++i)
            {
                const float v = out.getSample (ch, i);
                if (! std::isfinite (v)) bad = true;
                peak = std::max (peak, std::abs (v));
                sumSq += (double) v * v;
            }
        const float rmsDb = (float) juce::Decibels::gainToDecibels (std::sqrt (sumSq / (2.0 * total)) + 1e-9);
        const float peakDb = juce::Decibels::gainToDecibels (peak + 1e-9f);
        const bool ok = ! bad && peak > 0.01f && peak < 1.6f;
        if (! ok) ++failures;
        printf ("%-2d %-18s engine=%d peak=%6.1f dBFS rms=%6.1f dB %s\n", p + 1, presets[(size_t) p].name.toRawUTF8(),
                presets[(size_t) p].engine, peakDb, rmsDb, ok ? "OK" : "FAIL");
        writeWav (outDir.getChildFile (juce::String (p + 1).paddedLeft ('0', 2) + "_" + presets[(size_t) p].name.replaceCharacters (" ", "_") + ".wav"), out, sr);
    }

    // state round trip
    {
        proc.presets.loadFactory (7);
        juce::MemoryBlock mb;
        proc.getStateInformation (mb);
        proc.presets.loadFactory (0);
        proc.setStateInformation (mb.getData(), (int) mb.getSize());
        const auto eng = (int) proc.apvts.getRawParameterValue ("engine")->load();
        printf ("state round-trip: engine=%d name=%s %s\n", eng, proc.presets.getCurrentName().toRawUTF8(), eng == 1 ? "OK" : "FAIL");
        if (eng != 1) ++failures;
    }

    // GUI snapshots, one per engine
    {
        const int pick[] = { 0, 5, 10, 15 };
        std::unique_ptr<juce::AudioProcessorEditor> ed (proc.createEditor());
        ed->setSize (Soviet::Faceplate::W, Soviet::Faceplate::H);
        for (int e = 0; e < 4; ++e)
        {
            proc.presets.loadFactory (pick[e]);
            for (int i = 0; i < 10; ++i) juce::MessageManager::getInstance()->runDispatchLoopUntil (20);
            auto img = ed->createComponentSnapshot (ed->getLocalBounds(), true, 1.0f);
            juce::PNGImageFormat png;
            auto f = outDir.getChildFile ("ui_engine" + juce::String (e) + ".png");
            f.deleteFile();
            if (auto os = f.createOutputStream()) png.writeImageToStream (img, *os);
        }
        ed.reset();
    }

    printf ("failures: %d\n", failures);
    return failures == 0 ? 0 : 1;
}
