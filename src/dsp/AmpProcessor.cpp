#include "AmpProcessor.h"

// Maps param [0..100] → EQ gain in dB [-12..+12], 50 = 0 dB (flat)
static float eqGainDb (float param)
{
    return juce::jmap (param, 0.f, 100.f, -12.f, 12.f);
}

AmpProcessor::AmpProcessor (std::atomic<float>* ampType,
                             std::atomic<float>* gain,
                             std::atomic<float>* treble,
                             std::atomic<float>* middle,
                             std::atomic<float>* bass,
                             std::atomic<float>* ampVol)
    : ampType (ampType), gain (gain), treble (treble),
      middle (middle), bass (bass), ampVol (ampVol)
{
}

void AmpProcessor::prepare (double sr, int blockSize)
{
    sampleRate = sr * 2.0;   // EQ operates at oversampled rate
    cachedBass = cachedMiddle = cachedTreble = -9999.f; // force first update
    oversampling.initProcessing ((size_t) blockSize);
    updateEQIfChanged (50.f, 50.f, 50.f);               // init with flat EQ
    reset();
}

void AmpProcessor::reset()
{
    oversampling.reset();
    for (int ch = 0; ch < kMaxChannels; ++ch)
    {
        bassFilters  [ch].reset();
        midFilters   [ch].reset();
        trebleFilters[ch].reset();
    }
}

void AmpProcessor::updateEQIfChanged (float t, float m, float b)
{
    if (t == cachedTreble && m == cachedMiddle && b == cachedBass)
        return;

    cachedTreble = t;
    cachedMiddle = m;
    cachedBass   = b;

    const auto sr = (float) sampleRate;

    auto bassCoeff   = IIRCoeff::makeLowShelf  (sr, 200.f,  0.707f, eqGainDb (b));
    auto midCoeff    = IIRCoeff::makePeakFilter (sr, 800.f,  0.9f,   eqGainDb (m));
    auto trebleCoeff = IIRCoeff::makeHighShelf  (sr, 3500.f, 0.707f, eqGainDb (t));

    for (int ch = 0; ch < kMaxChannels; ++ch)
    {
        bassFilters  [ch].coefficients = bassCoeff;
        midFilters   [ch].coefficients = midCoeff;
        trebleFilters[ch].coefficients = trebleCoeff;
    }
}

float AmpProcessor::waveshape (float x, int type)
{
    using std::tanh;
    using std::atan;
    using juce::jlimit;
    constexpr float pi = juce::MathConstants<float>::pi;

    switch (type)
    {
        case 0: // Clean — cubic soft clip, barely saturates
        {
            float y = jlimit (-1.5f, 1.5f, x);
            return y - (y * y * y) / 6.75f;     // asymptote ≈ ±1
        }
        case 1: // Blues — gentle tube tanh
            return tanh (x * 0.9f);

        case 2: // Crunch — moderate tanh
            return tanh (x * 1.6f) * 0.85f;

        case 3: // Lead — asymmetric (adds even harmonics)
            return tanh (x) * 0.65f
                   + 0.35f * jlimit (-1.f, 1.f, x * 2.5f);

        case 4: // Metal — hard clip
            return jlimit (-0.85f, 0.85f, x * 1.4f);

        case 5: // British — x - x³/3 (Marshall-like even harmonics)
        {
            float y = jlimit (-1.4f, 1.4f, x);
            return y - (y * y * y) / 3.f;
        }
        case 6: // American — atan asymmetric soft clip
            return atan (x * 1.2f) * (2.f / pi)
                   + 0.05f * atan (x * 0.5f);

        case 7: // Boutique — atan, smooth and warm
            return atan (x * 0.7f) * (2.f / pi) * 1.4f;

        case 8: // Fuzz — hard clip + wave-fold
        {
            float y = x * 1.6f;
            if (y >  1.f) y =  2.f - y;
            if (y < -1.f) y = -2.f - y;
            return jlimit (-1.f, 1.f, y);
        }
        default:
            return tanh (x);
    }
}

void AmpProcessor::process (juce::AudioBuffer<float>& buffer)
{
    const int   type    = static_cast<int> (ampType->load());
    const float gainVal = gain->load();
    const float t       = treble->load();
    const float m       = middle->load();
    const float b       = bass->load();
    const float vol     = ampVol->load();

    updateEQIfChanged (t, m, b);

    // Pre-gain: gain=0 → 0.1×, gain=50 → ~3×, gain=100 → ~32×  (log feel)
    const float preGain    = std::pow (10.f, juce::jmap (gainVal, 0.f, 100.f, -1.f, 1.5f));
    const float outputGain = juce::jmap (vol, 0.f, 100.f, 0.f, 1.5f);

    const int numCh = juce::jmin (buffer.getNumChannels(), kMaxChannels);

    juce::dsp::AudioBlock<float> inputBlock (buffer);
    auto oversampledBlock = oversampling.processSamplesUp (inputBlock);

    const int numOsSamples = (int) oversampledBlock.getNumSamples();

    for (int ch = 0; ch < numCh; ++ch)
    {
        auto* data = oversampledBlock.getChannelPointer (ch);
        for (int s = 0; s < numOsSamples; ++s)
        {
            float x = waveshape (data[s] * preGain, type);
            x = bassFilters  [ch].processSample (x);
            x = midFilters   [ch].processSample (x);
            x = trebleFilters[ch].processSample (x);
            data[s] = x * outputGain;
        }
    }

    oversampling.processSamplesDown (inputBlock);
}
