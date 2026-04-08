#include "ModProcessor.h"
#include "ZoneKnob.h"

// modSpeed [0..100] → LFO frequency [0.1..8] Hz
static float speedToHz (float speed)
{
    return juce::jmap (speed, 0.f, 100.f, 0.1f, 8.f);
}

ModProcessor::ModProcessor (std::atomic<float>* modFx,
                             std::atomic<float>* modSpeed)
    : modFx (modFx), modSpeed (modSpeed)
{
}

void ModProcessor::prepare (double sr, int)
{
    sampleRate = sr;
    reset();
}

void ModProcessor::reset()
{
    lfoPhase  = 0.f;
    lfoPhaseL = 0.f;
    lfoPhaseR = juce::MathConstants<float>::halfPi; // 90° offset for stereo chorus

    writePosL = writePosR = 0;
    juce::zeromem (delayBufL, sizeof (delayBufL));
    juce::zeromem (delayBufR, sizeof (delayBufR));
    juce::zeromem (apStateL,  sizeof (apStateL));
    juce::zeromem (apStateR,  sizeof (apStateR));
    apLastOut = 0.f;
}

// ── CHORUS ───────────────────────────────────────────────────────────────────
// Modulated delay: center 12 ms, depth ±6 ms. mix = wet/dry.
float ModProcessor::chorusSample (float in, float mix, float lfo,
                                   float* buf, int& wPos)
{
    const float sr = (float) sampleRate;
    const float centerMs = 12.f;
    const float depthMs  = 6.f;

    // Write
    buf[wPos] = in;

    // Compute read position with LFO modulation
    const float delaySamples = (centerMs + depthMs * lfo) * sr * 0.001f;
    float readPos = (float) wPos - delaySamples;
    while (readPos < 0.f) readPos += kMaxDelaySamples;

    // Linear interpolation
    const int   i0  = (int) readPos % kMaxDelaySamples;
    const int   i1  = (i0 + 1)      % kMaxDelaySamples;
    const float frac = readPos - std::floor (readPos);
    const float wet  = buf[i0] + frac * (buf[i1] - buf[i0]);

    wPos = (wPos + 1) % kMaxDelaySamples;

    return in * (1.f - mix) + wet * mix;
}

// ── PHASER ───────────────────────────────────────────────────────────────────
// 4-stage first-order allpass. LFO sweeps allpass coefficient.
// depth [0..1] controls sweep width.
float ModProcessor::phaserSample (float in, float depth, float lfo, float* state)
{
    const float sr = (float) sampleRate;

    // LFO sweeps centre frequency: 300 Hz + depth × 1700 Hz
    const float fc  = 300.f + depth * 1700.f * (0.5f + 0.5f * lfo);
    const float tan_ = std::tan (juce::MathConstants<float>::pi * fc / sr);
    const float a   = (tan_ - 1.f) / (tan_ + 1.f);   // allpass coefficient

    float x = in;
    for (int i = 0; i < kPhaserStages; ++i)
    {
        const float y = a * x + state[i] - a * (state[i] != 0.f ? state[i] : 0.f);
        // Direct form II allpass: y[n] = a*x[n] + x[n-1] - a*y[n-1]
        const float yn = a * x + state[i];
        state[i] = x - a * yn;
        x = yn;
    }
    // Mix dry + phased 50/50 (depth controls intensity via frequency sweep only)
    return 0.5f * in + 0.5f * x;
}

// ── TREMOLO ──────────────────────────────────────────────────────────────────
// Amplitude modulation by sine LFO.
// depth [0..1]: 0 = no effect, 1 = full tremolo (signal reaches 0 at trough).
float ModProcessor::tremoloSample (float in, float depth, float lfo)
{
    // gain oscillates between (1-depth) and 1
    const float gain = 1.f - depth * (0.5f - 0.5f * lfo);
    return in * gain;
}

// ── PROCESS ──────────────────────────────────────────────────────────────────
void ModProcessor::process (juce::AudioBuffer<float>& buffer)
{
    const ZoneResult zone = zoneKnob (modFx->load());

    if (! zone.isOn)
        return;  // dead zone — pass-through

    const float speed    = modSpeed->load();
    const float lfoHz    = speedToHz (speed);
    const float lfoStep  = juce::MathConstants<float>::twoPi * lfoHz
                           / (float) sampleRate;

    const int numCh      = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    for (int s = 0; s < numSamples; ++s)
    {
        const float lfoSinL = std::sin (lfoPhaseL);
        const float lfoSinR = std::sin (lfoPhaseR);

        for (int ch = 0; ch < numCh; ++ch)
        {
            const float lfoSin = (ch == 0) ? lfoSinL : lfoSinR;
            float x = buffer.getSample (ch, s);

            switch (zone.type)
            {
                case 0: // CHORUS
                    if (ch == 0)
                        x = chorusSample (x, zone.param, lfoSin, delayBufL, writePosL);
                    else
                        x = chorusSample (x, zone.param, lfoSin, delayBufR, writePosR);
                    break;

                case 1: // PHASER
                    x = phaserSample (x, zone.param,
                                      lfoSin,
                                      ch == 0 ? apStateL : apStateR);
                    break;

                case 2: // TREMOLO
                    x = tremoloSample (x, zone.param, lfoSin);
                    break;

                default: break;
            }

            buffer.setSample (ch, s, x);
        }

        // Advance LFO phases
        lfoPhaseL = std::fmod (lfoPhaseL + lfoStep,
                               juce::MathConstants<float>::twoPi);
        lfoPhaseR = std::fmod (lfoPhaseR + lfoStep,
                               juce::MathConstants<float>::twoPi);
    }
}
