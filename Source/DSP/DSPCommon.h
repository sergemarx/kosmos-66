#pragma once
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <memory>

namespace dsp66
{
constexpr float twoPi = 6.28318530717958647692f;
constexpr float pi    = 3.14159265358979323846f;

inline float clampf (float x, float lo, float hi) { return std::min (hi, std::max (lo, x)); }

// Fast rational tanh approximation (accurate to ~1e-3, saturates cleanly)
inline float fastTanh (float x)
{
    x = clampf (x, -4.5f, 4.5f);
    const float x2 = x * x;
    return x * (27.0f + x2) / (27.0f + 9.0f * x2);
}

inline float midiToHz (float note) { return 440.0f * std::exp2 ((note - 69.0f) / 12.0f); }

// ---------------------------------------------------------------------------
// Sine table (shared)
// ---------------------------------------------------------------------------
struct SineTable
{
    static constexpr int size = 4096;
    float t[size + 1];
    SineTable()
    {
        for (int i = 0; i <= size; ++i)
            t[i] = std::sin (twoPi * (float) i / (float) size);
    }
    // phase in [0,1)
    inline float operator() (float phase) const
    {
        const float p = phase * (float) size;
        const int i = (int) p;
        const float f = p - (float) i;
        return t[i & (size - 1)] + f * (t[(i & (size - 1)) + 1] - t[i & (size - 1)]);
    }
    static const SineTable& get() { static SineTable s; return s; }
};

inline float wrap01 (float p) { return p - std::floor (p); }

// ---------------------------------------------------------------------------
// PolyBLEP oscillator: saw / triangle / square / pulse
// ---------------------------------------------------------------------------
inline float polyBlep (float t, float dt)
{
    if (t < dt)        { t /= dt;            return t + t - t * t - 1.0f; }
    if (t > 1.0f - dt) { t = (t - 1.0f) / dt; return t * t + t + t + 1.0f; }
    return 0.0f;
}

struct BlepOsc
{
    enum Wave { Saw, Tri, Square, Pulse };
    float phase = 0.0f;
    float triState = 0.0f;

    void reset (float p = 0.0f) { phase = p; triState = 0.0f; }

    // inc = freq / sampleRate, pw = pulse width (0..1)
    inline float next (int wave, float inc, float pw = 0.5f)
    {
        inc = clampf (inc, 1.0e-6f, 0.45f);
        float out = 0.0f;
        switch (wave)
        {
            case Saw:
                out = 2.0f * phase - 1.0f;
                out -= polyBlep (phase, inc);
                break;
            case Square:
            case Pulse:
            {
                const float w = (wave == Square) ? 0.5f : clampf (pw, 0.03f, 0.97f);
                out = phase < w ? 1.0f : -1.0f;
                out += polyBlep (phase, inc);
                out -= polyBlep (wrap01 (phase + 1.0f - w), inc);
                if (wave == Pulse) out += (2.0f * w - 1.0f); // remove DC
                break;
            }
            case Tri:
            {
                // integrate a band-limited square -> triangle
                float sq = phase < 0.5f ? 1.0f : -1.0f;
                sq += polyBlep (phase, inc);
                sq -= polyBlep (wrap01 (phase + 0.5f), inc);
                triState = inc * 4.0f * sq + (1.0f - 0.0008f) * triState;
                out = triState;
                break;
            }
            default: break;
        }
        phase += inc;
        if (phase >= 1.0f) phase -= 1.0f;
        return out;
    }
};

// ---------------------------------------------------------------------------
// Noise
// ---------------------------------------------------------------------------
struct Noise
{
    uint32_t s = 22222u;
    void seed (uint32_t v) { s = v ? v : 1u; }
    inline float next()
    {
        s ^= s << 13; s ^= s >> 17; s ^= s << 5;
        return (float) (int32_t) s * (1.0f / 2147483648.0f);
    }
};

// ---------------------------------------------------------------------------
// One-pole filters
// ---------------------------------------------------------------------------
struct OnePoleLP
{
    float z = 0.0f, a = 1.0f;
    void setCutoff (float hz, float sr) { a = 1.0f - std::exp (-twoPi * clampf (hz, 5.0f, sr * 0.49f) / sr); }
    inline float process (float x) { z += a * (x - z); return z; }
    void reset() { z = 0.0f; }
};

struct DCBlocker
{
    float x1 = 0.0f, y1 = 0.0f, r = 0.995f;
    void prepare (float sr) { r = 1.0f - (twoPi * 12.0f / sr); }
    inline float process (float x) { const float y = x - x1 + r * y1; x1 = x; y1 = y; return y; }
    void reset() { x1 = y1 = 0.0f; }
};

// ---------------------------------------------------------------------------
// Moog-style 4-pole transistor ladder (zero-delay-feedback, tanh input stage)
// ---------------------------------------------------------------------------
struct LadderFilter
{
    float s[4] { 0, 0, 0, 0 };
    float sr = 44100.0f;

    void reset() { s[0] = s[1] = s[2] = s[3] = 0.0f; }

    // res: 0..1 (≈1 self-oscillates)
    inline float process (float x, float cutoffHz, float res)
    {
        const float fc = clampf (cutoffHz, 10.0f, sr * 0.45f);
        const float g = std::tan (pi * fc / sr);
        const float G = g / (1.0f + g);
        const float k = 4.1f * res;

        // feedback estimate from current states (TPT one-pole cascade)
        const float G2 = G * G, G3 = G2 * G, G4 = G3 * G;
        const float S = (G3 * s[0] + G2 * s[1] + G * s[2] + s[3]) / (1.0f + g);

        // passband gain compensation keeps the bass when emphasis is up
        const float in = x * (1.0f + 0.6f * k);
        float u = (in - k * S) / (1.0f + k * G4);
        u = fastTanh (u);

        float y = u;
        for (int i = 0; i < 4; ++i)
        {
            const float v = (y - s[i]) * G;
            const float lp = v + s[i];
            s[i] = lp + v;
            y = lp;
        }
        return y;
    }
};

// ---------------------------------------------------------------------------
// TPT state-variable filter with optional saturating resonance (MS-20 flavour)
// ---------------------------------------------------------------------------
struct SVF
{
    float ic1 = 0.0f, ic2 = 0.0f, sr = 44100.0f;
    void reset() { ic1 = ic2 = 0.0f; }

    // mode 0 = LP, 1 = HP, 2 = BP. res 0..1. drive>0 adds aggressive nonlinear peak
    inline float process (float x, float cutoffHz, float res, int mode, float drive = 0.0f)
    {
        const float fc = clampf (cutoffHz, 10.0f, sr * 0.45f);
        const float g = std::tan (pi * fc / sr);
        const float k = 2.0f - 1.97f * clampf (res, 0.0f, 1.0f);
        const float a1 = 1.0f / (1.0f + g * (g + k));
        const float a2 = g * a1;
        const float a3 = g * a2;

        const float v3 = x - ic2;
        float v1 = a1 * ic1 + a2 * v3;
        const float v2 = ic2 + a2 * ic1 + a3 * v3;

        if (drive > 0.0f)
            v1 = fastTanh (v1 * (1.0f + drive)) / (1.0f + drive * 0.5f);

        ic1 = 2.0f * v1 - ic1;
        ic2 = 2.0f * v2 - ic2;

        switch (mode)
        {
            case 1:  return x - k * v1 - v2;
            case 2:  return v1;
            default: return v2;
        }
    }
};

// ---------------------------------------------------------------------------
// Simple fractional delay line
// ---------------------------------------------------------------------------
struct DelayLine
{
    float* buf = nullptr;
    int size = 0, w = 0;
    std::unique_ptr<float[]> mem;

    void prepare (int maxSamples)
    {
        size = 1;
        while (size < maxSamples + 4) size <<= 1;
        mem.reset (new float[(size_t) size]);
        buf = mem.get();
        clear();
    }
    void clear() { if (buf) std::fill (buf, buf + size, 0.0f); w = 0; }
    inline void push (float x) { buf[w] = x; w = (w + 1) & (size - 1); }
    // delay in samples (>=1)
    inline float read (float d) const
    {
        d = clampf (d, 1.0f, (float) (size - 3));
        float rp = (float) w - d;
        while (rp < 0.0f) rp += (float) size;
        const int i0 = (int) rp;
        const float f = rp - (float) i0;
        const float a = buf[i0 & (size - 1)];
        const float b = buf[(i0 + 1) & (size - 1)];
        return a + f * (b - a);
    }
};
}
