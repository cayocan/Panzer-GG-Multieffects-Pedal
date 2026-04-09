#pragma once
#include "DspModule.h"

class AmpProcessor : public DspModule
{
public:
    AmpProcessor (std::atomic<float>* ampType,
                  std::atomic<float>* gain,
                  std::atomic<float>* treble,
                  std::atomic<float>* middle,
                  std::atomic<float>* bass,
                  std::atomic<float>* ampVol);

    void prepare (double sampleRate, int samplesPerBlock) override;
    void reset() override;
    void process (juce::AudioBuffer<float>& buffer) override;

private:
    std::atomic<float>* ampType;
    std::atomic<float>* gain;
    std::atomic<float>* treble;
    std::atomic<float>* middle;
    std::atomic<float>* bass;
    std::atomic<float>* ampVol;

    double sampleRate = 44100.0;

    static constexpr int kMaxChannels = 2;

    using IIRCoeff  = juce::dsp::IIR::Coefficients<float>;
    using IIRFilter = juce::dsp::IIR::Filter<float>;

    IIRFilter bassFilters  [kMaxChannels];
    IIRFilter midFilters   [kMaxChannels];
    IIRFilter trebleFilters[kMaxChannels];

    float cachedBass   = -9999.f;
    float cachedMiddle = -9999.f;
    float cachedTreble = -9999.f;

    void  updateEQIfChanged (float t, float m, float b);
    static float waveshape (float x, int type);

    juce::dsp::Oversampling<float> oversampling {
        kMaxChannels,
        1,   // 2^1 = 2x
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
        true };
};
