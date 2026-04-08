#pragma once
#include "DspModule.h"

// ModProcessor — 3-zone knob selects and controls one of:
//   Zone 0: CHORUS  (modFx param = wet/dry mix 0..1)
//   Zone 1: PHASER  (modFx param = sweep depth 0..1)
//   Zone 2: TREMOLO (modFx param = depth 0..1)
//   Gap        : MOD OFF (pass-through)
//
// MOD SPEED controls the LFO frequency for all types.

class ModProcessor : public DspModule
{
public:
    ModProcessor (std::atomic<float>* modFx,
                  std::atomic<float>* modSpeed);

    void prepare (double sampleRate, int samplesPerBlock) override;
    void reset() override;
    void process (juce::AudioBuffer<float>& buffer) override;

private:
    std::atomic<float>* modFx;
    std::atomic<float>* modSpeed;

    double sampleRate = 44100.0;

    // ── LFO ──────────────────────────────────────────────────────────────
    float lfoPhase = 0.f;   // 0..2π
    float lfoPhaseL = 0.f;  // left  channel (chorus stereo offset)
    float lfoPhaseR = 0.f;  // right channel

    // ── Chorus delay line ────────────────────────────────────────────────
    static constexpr int kMaxDelaySamples = 4096;  // ~93 ms @ 44.1 kHz
    float delayBufL[kMaxDelaySamples] = {};
    float delayBufR[kMaxDelaySamples] = {};
    int   writePosL = 0;
    int   writePosR = 0;

    // ── Phaser — 4 first-order allpass stages per channel ────────────────
    static constexpr int kPhaserStages = 4;
    float apStateL[kPhaserStages] = {};  // allpass delay state, left
    float apStateR[kPhaserStages] = {};  // allpass delay state, right
    float apLastOut = 0.f;              // feedback sample (mono, shared)

    // helpers
    float chorusSample  (float in, float mix,   float lfo, float* buf, int& wPos);
    float phaserSample  (float in, float depth, float lfo, float* state);
    float tremoloSample (float in, float depth, float lfo);
};
