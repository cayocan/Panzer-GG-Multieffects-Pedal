#pragma once
#include <JuceHeader.h>

class DspModule
{
public:
    virtual ~DspModule() = default;
    virtual void prepare (double sampleRate, int samplesPerBlock) = 0;
    virtual void reset() = 0;
    virtual void process (juce::AudioBuffer<float>& buffer) = 0;
};
