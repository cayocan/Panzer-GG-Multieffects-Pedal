#pragma once
#include "DspModule.h"

class GateProcessor : public DspModule
{
public:
    explicit GateProcessor (std::atomic<float>* thresholdDb);

    void prepare (double sampleRate, int samplesPerBlock) override;
    void reset() override;
    void process (juce::AudioBuffer<float>& buffer) override;

private:
    std::atomic<float>* thresholdDb;

    double sampleRate   = 44100.0;
    float  envelope     = 0.f;   // peak envelope follower state
    float  smoothedGain = 0.f;   // smoothed gain (avoids clicks)

    float  attackCoeff     = 0.f;  // envelope attack  (~1 ms)
    float  releaseCoeff    = 0.f;  // envelope release (~100 ms)
    float  gainOpenCoeff   = 0.f;  // gain smoothing when gate opens (~1 ms)
    float  gainCloseCoeff  = 0.f;  // gain smoothing when gate closes (~50 ms)

    static constexpr float kOffThresholdDb = -79.9f; // threshold <= this => gate OFF
};
