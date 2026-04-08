#pragma once
#include "DspModule.h"

// DelayProcessor — 3-zone knob selects type and controls wet/dry mix:
//   Zone 0: ANALOG  — simple delay, mild LPF on feedback
//   Zone 1: TAPE    — delay with wow + flutter LFO modulation
//   Zone 2: DUAL    — ping-pong stereo (L→R, R→L cross-feedback)
//   Gap: DELAY OFF
//
// dlyTime [0..1000 ms] controls the delay time.

class DelayProcessor : public DspModule
{
public:
    DelayProcessor (std::atomic<float>* dlyMix,
                    std::atomic<float>* dlyTime);

    void prepare (double sampleRate, int samplesPerBlock) override;
    void reset() override;
    void process (juce::AudioBuffer<float>& buffer) override;

private:
    std::atomic<float>* dlyMix;
    std::atomic<float>* dlyTime;

    double sampleRate = 44100.0;

    // Delay buffers — heap allocated in prepare() to support any sample rate
    std::vector<float> delayBufL, delayBufR;
    int writePosL = 0, writePosR = 0;

    // Feedback state (one sample history)
    float feedbackL = 0.f, feedbackR = 0.f;

    // One-pole LPF state on feedback path
    float lpfStateL = 0.f, lpfStateR = 0.f;

    // Tape wow + flutter LFO phases
    float wowPhase     = 0.f;
    float flutterPhase = 0.f;

    static constexpr float kFeedback = 0.45f;

    // Read a sample from a delay buffer with linear interpolation
    static float readDelayed (const std::vector<float>& buf,
                              int writePos, float delaySamples);

    // Write one sample and advance write position
    static void writeSample (std::vector<float>& buf, int& wPos, float x);

    // One-pole LPF: a controls cutoff (0=full pass, 1=full block)
    static float lpf (float x, float& state, float a);
};
