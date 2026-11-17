# КОСМОС-66 · KOSMOS-66

A polyphonic VST3 / AU / Standalone instrument with four synthesis engines, a five-stage FX chain and a 1960s Soviet-instrument faceplate.

## Engines

| Engine | What it is | Controls |
|---|---|---|
| **МУГ / MOOG** | 3 oscillators into a 4-pole transistor-ladder filter (zero-delay feedback, saturating) | Range 32'–4', wave (saw/tri/square/pulse), detune, mixer + noise, cutoff, emphasis, contour, key-track, filter and amp ADSR, glide, mono/legato |
| **ОРГАН / HAMMOND** | 9-drawbar tonewheel organ | Drawbars (16' … 1'), percussion (2nd/3rd, fast/slow, normal/soft, single-trigger, steals the 1' bar like the real one), key click, scanner vibrato/chorus V1–C3, Leslie (stop/slow/fast with horn and drum acceleration), tube overdrive. The mod wheel switches the Leslie to fast. |
| **КОРГ / MINIKORG** | 2-VCO synth with MS-20-style high-pass and screaming low-pass | VCO1 wave + pulse width, VCO2 wave/semitone/fine, mix, ring mod, HPF, LPF, peak, EG→LPF, LFO (tri/square/saw/S&H) → pitch/LPF/PW, ADSR |
| **ЭЛ. ПИАНО / E-PIANO** | FM tine model with an asymmetric pickup | Tine, bark (growl), velocity, detune, decay, damper, tone, suitcase stereo tremolo |

**FX chain** (fixed order): tube drive → phaser → chorus → ping-pong tape echo (wow, darkening repeats) → plate reverb → master. Each stage has a bat-handle on/off switch, and switching crossfades with no clicks.

**MIDI:** velocity, pitch bend (±2), mod wheel (vibrato on Moog/Korg, Leslie fast on Hammond, extra tremolo on E-piano), sustain pedal, all-notes-off. 16 voices; Moog can run mono/legato with glide.

## Presets & memory

* **20 factory presets**, 5 per engine. Click the nixie display for the menu, or use ◄ ► to step through them. They also show up as host programs.
* **8 memory slots** (ЯЧЕЙКИ ПАМЯТИ):
  * Clicking an empty slot saves the current patch into it.
  * Clicking a used slot loads it.
  * To overwrite a used slot, arm **ЗАПИСЬ / STORE** and then click the slot.
  * Right-click a slot for Load / Save / Clear.
  * Slots are shared by every project. They're saved to `KOSMOS-66/UserSlots.xml` in your user application-data folder and also stored in the DAW session.
* **Rename** the current patch by double-clicking the nixie display before you store it.
* **ИМПОРТ / ЭКСПОРТ** load and save single patches as `.k66` files.
* The window is resizable (55%–180%) and keeps its aspect ratio.

## Building

You need CMake ≥ 3.22 and a C++17 compiler (Visual Studio 2022, Xcode, or GCC/Clang). JUCE 9.0.5 is downloaded automatically.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
```

The plugins end up in `build/Kosmos66_artefacts/Release/` under `VST3/`, `AU/` (macOS only) and `Standalone/`. Copy the `.vst3` to:

* Windows: `C:\Program Files\Common Files\VST3\`
* macOS: `~/Library/Audio/Plug-Ins/VST3/` (AU: `~/Library/Audio/Plug-Ins/Components/`)
* Linux: `~/.vst3/`

If you already have JUCE checked out, add `-DJUCE_DIR=/path/to/JUCE` to the first command.

**Linux packages:** `libasound2-dev libjack-jackd2-dev libx11-dev libxcomposite-dev libxcursor-dev libxext-dev libxi-dev libxinerama-dev libxrandr-dev libxrender-dev libfreetype6-dev libfontconfig1-dev libgl1-mesa-dev`

**macOS note:** local builds are ad-hoc signed. If you copy a binary built on another Mac, clear quarantine with `xattr -cr KOSMOS-66.vst3`.

### Builds without a compiler: GitHub Actions

Push this folder to a GitHub repository. The workflow in `.github/workflows/build.yml` builds Windows, macOS (universal) and Linux. Download the zips from the run's **Artifacts** section.

### Test tool

`cmake -B build -DKOSMOS_BUILD_TESTS=ON` builds `KosmosTest`. It renders every factory preset to WAV, checks levels and NaNs, round-trips the plugin state, and saves a PNG snapshot of the UI for each engine.

## Source layout

```
Source/
  Params.*            parameter table (one place: ids, Cyrillic/English labels, ranges)
  Presets.*           factory presets, 8 memory slots, .k66 import/export
  PluginProcessor.*   APVTS, MIDI, state
  PluginEditor.*      faceplate layout, engine panels, FX rack, memory strip
  DSP/DSPCommon.h     PolyBLEP oscillators, ladder filter, SVF, delay line
  DSP/Voice.*         per-voice engines (Moog / Hammond / Korg / E-piano)
  DSP/SynthEngine.*   voice allocation, mono/legato, sustain, Leslie, vibrato scanner, tremolo
  DSP/Effects.*       drive, phaser, chorus, tape echo, reverb
  UI/                 LookAndFeel (bakelite knobs, chicken-heads, lamps, hammertone, wood) and controls
Resources/Fonts/      DejaVu Sans Condensed (embedded so the Cyrillic labels look the same everywhere)
```

"Moog", "Hammond", "Korg" and "Leslie" are trademarks of their owners. They are used here only to describe the style of sound. KOSMOS-66 is an independent emulation and is not affiliated with any of them. The DejaVu fonts are under the Bitstream Vera / DejaVu licence, which allows embedding and redistribution.
