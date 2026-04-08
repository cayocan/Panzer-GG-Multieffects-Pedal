#pragma once
#include "DspModule.h"

// ReverbProcessor — 3-zone knob selects type and controls decay:
//   Zone 0: ROOM   — Schroeder reverb, short/natural  (g: 0.30–0.85)
//   Zone 1: SPRING — metallic bouncy character         (g: 0.40–0.88)
//   Zone 2: CLOUD  — long lush ambient                 (g: 0.70–0.98)
//   Gap: REVERB OFF
//
// rvbMix [0..100] controls wet/dry balance.
// Algorithm: 4 parallel feedback comb filters → 2 series allpass diffusers.

class ReverbProcessor : public DspModule
{
public:
    ReverbProcessor (std::atomic<float>* rvbDecay,
                     std::atomic<float>* rvbMix);

    void prepare (double sampleRate, int samplesPerBlock) override;
    void reset() override;
    void process (juce::AudioBuffer<float>& buffer) override;

private:
    std::atomic<float>* rvbDecay;
    std::atomic<float>* rvbMix;

    double sampleRate = 44100.0;
    int    cachedType = -1;

    // ── Schroeder building blocks ─────────────────────────────────────────
    struct CombFilter
    {
        std::vector<float> buf;
        int   wPos = 0;
        float process (float in, float g)
        {
            const float out = buf[wPos];
            buf[wPos] = in + g * out;
            wPos = (wPos + 1) % (int) buf.size();
            return out;
        }
        void resize (int n) { buf.assign (n, 0.f); wPos = 0; }
        void clear()        { std::fill (buf.begin(), buf.end(), 0.f); wPos = 0; }
    };

    struct AllpassFilter
    {
        std::vector<float> buf;
        int   wPos = 0;
        float g    = 0.7f;
        float process (float in)
        {
            const float bufOut = buf[wPos];
            const float out    = -g * in + bufOut;
            buf[wPos] = in + g * bufOut;
            wPos = (wPos + 1) % (int) buf.size();
            return out;
        }
        void resize (int n) { buf.assign (n, 0.f); wPos = 0; }
        void clear()        { std::fill (buf.begin(), buf.end(), 0.f); wPos = 0; }
    };

    static constexpr int kNumCombs    = 4;
    static constexpr int kNumAllpass  = 2;

    CombFilter    combsL[kNumCombs],   combsR[kNumCombs];
    AllpassFilter apL[kNumAllpass],    apR[kNumAllpass];

    // Delay times in ms for each type — R channel uses ×1.03 for stereo width
    static constexpr float kCombMs[3][kNumCombs] = {
        { 29.7f, 37.1f, 41.1f, 43.7f },   // ROOM
        { 35.2f, 49.6f, 54.3f, 63.1f },   // SPRING
        { 67.5f, 83.2f, 91.1f, 107.3f }   // CLOUD
    };
    static constexpr float kApMs[3][kNumAllpass] = {
        { 5.0f, 1.7f },   // ROOM
        { 6.0f, 2.1f },   // SPRING
        { 12.0f, 5.0f }   // CLOUD
    };

    void   configureForType (int type);
    float  processMono (float in, float g,
                        CombFilter* combs, AllpassFilter* ap);
};
