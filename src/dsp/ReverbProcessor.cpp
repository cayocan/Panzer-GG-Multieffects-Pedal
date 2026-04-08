#include "ReverbProcessor.h"
#include "ZoneKnob.h"

ReverbProcessor::ReverbProcessor (std::atomic<float>* rvbDecay,
                                   std::atomic<float>* rvbMix)
    : rvbDecay (rvbDecay), rvbMix (rvbMix)
{
}

void ReverbProcessor::prepare (double sr, int)
{
    sampleRate = sr;
    cachedType = -1;  // force reconfigure on first process()
    reset();
}

void ReverbProcessor::reset()
{
    for (int i = 0; i < kNumCombs;   ++i) { combsL[i].clear(); combsR[i].clear(); }
    for (int i = 0; i < kNumAllpass; ++i) { apL[i].clear();    apR[i].clear(); }
}

void ReverbProcessor::configureForType (int type)
{
    if (type == cachedType) return;
    cachedType = type;

    const float sr = (float) sampleRate;
    constexpr float stereoFactor = 1.03f;

    for (int i = 0; i < kNumCombs; ++i)
    {
        combsL[i].resize (juce::roundToInt (kCombMs[type][i] * sr * 0.001f));
        combsR[i].resize (juce::roundToInt (kCombMs[type][i] * sr * 0.001f * stereoFactor));
    }
    for (int i = 0; i < kNumAllpass; ++i)
    {
        apL[i].resize (juce::roundToInt (kApMs[type][i] * sr * 0.001f));
        apR[i].resize (juce::roundToInt (kApMs[type][i] * sr * 0.001f * stereoFactor));
    }
}

float ReverbProcessor::processMono (float in, float g,
                                     CombFilter* combs, AllpassFilter* ap)
{
    // 4 comb filters in parallel
    float wet = 0.f;
    for (int i = 0; i < kNumCombs; ++i)
        wet += combs[i].process (in, g);
    wet *= 0.25f;  // normalise

    // 2 allpass diffusers in series
    for (int i = 0; i < kNumAllpass; ++i)
        wet = ap[i].process (wet);

    return wet;
}

void ReverbProcessor::process (juce::AudioBuffer<float>& buffer)
{
    const ZoneResult zone = zoneKnob (rvbDecay->load());

    if (! zone.isOn)
        return;  // dead zone — pass-through

    configureForType (zone.type);

    // Feedback coefficient per type
    float g;
    switch (zone.type)
    {
        case 0:  g = 0.30f + 0.55f * zone.param; break;  // ROOM:   0.30–0.85
        case 1:  g = 0.40f + 0.48f * zone.param; break;  // SPRING: 0.40–0.88
        default: g = 0.70f + 0.28f * zone.param; break;  // CLOUD:  0.70–0.98
    }

    const float mix    = juce::jmap (rvbMix->load(), 0.f, 100.f, 0.f, 1.f);
    const int numCh    = buffer.getNumChannels();
    const int numSamps = buffer.getNumSamples();

    for (int s = 0; s < numSamps; ++s)
    {
        const float dryL = buffer.getSample (0, s);
        const float dryR = (numCh > 1) ? buffer.getSample (1, s) : dryL;

        const float wetL = processMono (dryL, g, combsL, apL);
        const float wetR = processMono (dryR, g, combsR, apR);

        buffer.setSample (0, s, dryL * (1.f - mix) + wetL * mix);
        if (numCh > 1)
            buffer.setSample (1, s, dryR * (1.f - mix) + wetR * mix);
    }
}
