#include "GateProcessor.h"

static float timeToCoeff (float timeMs, double sr)
{
    return std::exp (-1.f / (float (sr) * timeMs * 0.001f));
}

GateProcessor::GateProcessor (std::atomic<float>* thresholdDb)
    : thresholdDb (thresholdDb)
{
}

void GateProcessor::prepare (double sr, int)
{
    sampleRate      = sr;
    attackCoeff     = timeToCoeff (1.f,   sr);
    releaseCoeff    = timeToCoeff (100.f, sr);
    gainOpenCoeff   = timeToCoeff (1.f,   sr);
    gainCloseCoeff  = timeToCoeff (50.f,  sr);
    reset();
}

void GateProcessor::reset()
{
    envelope     = 0.f;
    smoothedGain = 0.f;
}

void GateProcessor::process (juce::AudioBuffer<float>& buffer)
{
    const float thresh = thresholdDb->load();

    // -80 dB = gate OFF → pass-through
    if (thresh <= kOffThresholdDb)
        return;

    const int numChannels = buffer.getNumChannels();
    const int numSamples  = buffer.getNumSamples();

    for (int s = 0; s < numSamples; ++s)
    {
        // --- envelope follower (peak, per-sample across all channels) ---
        float peak = 0.f;
        for (int ch = 0; ch < numChannels; ++ch)
            peak = std::max (peak, std::abs (buffer.getSample (ch, s)));

        if (peak > envelope)
            envelope = attackCoeff  * envelope + (1.f - attackCoeff)  * peak;
        else
            envelope = releaseCoeff * envelope + (1.f - releaseCoeff) * peak;

        // --- gain computer (hard gate) ---
        const float levelDb   = juce::Decibels::gainToDecibels (envelope, -120.f);
        const float targetGain = (levelDb >= thresh) ? 1.f : 0.f;

        // --- gain smoothing ---
        const float coeff = (targetGain > smoothedGain) ? gainOpenCoeff : gainCloseCoeff;
        smoothedGain = coeff * smoothedGain + (1.f - coeff) * targetGain;

        // --- apply gain ---
        for (int ch = 0; ch < numChannels; ++ch)
            buffer.setSample (ch, s, buffer.getSample (ch, s) * smoothedGain);
    }
}
